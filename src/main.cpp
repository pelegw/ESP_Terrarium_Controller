#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <SparkFunDS3234RTC.h>
#include "config.h"
#include "modbus.h"
#include <ModbusIP_ESP8266.h>
#include "custom_time.h"
#include "time.h"
#include <ezTime.h>
#include "Adafruit_SHT31.h"
#include "sht31.h"
#include "pump_schedule.h"
#include <PubSubClient.h>
#include <stdint.h>
#include "helpers.h"
//#include "outputs.h"

//global vars that I can't define in config.h
bool sht_status;
hw_timer_t *timer = NULL;
bool has_expired = false;
float my_hum,my_temp;
WiFiClient espClient;
PubSubClient client(espClient);


 #define DS13074_CS_PIN 10
 //#define INTERRUPT_PIN 2 // DeadOn RTC SQW/interrupt pin (optional)

//Modbus Variables
ModbusIP mb;
Adafruit_SHT31 sht31 = Adafruit_SHT31();
void callback(char *topic, byte *payload, unsigned int length);
void getSensors();
void setOutputs();
void IRAM_ATTR timerInterrupcion() {
     digitalWrite(PUMP_RELAY, LOW);
}

/************************Setup Function - Quite  alot of stuff here for the controller to prepare ******************************************************/
void setup() {
  Serial.begin(115200);
  int counter;
  //Set Relays to Off
  pinMode(PUMP_RELAY, OUTPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(LIGHT_RELAY_PIN_2, OUTPUT);
  pinMode(LIGHT_RELAY_PIN_1,OUTPUT);
  digitalWrite(PUMP_RELAY, LOW);
  digitalWrite(FAN_RELAY_PIN, LOW);
  digitalWrite(LIGHT_RELAY_PIN_2, LOW);
  digitalWrite(LIGHT_RELAY_PIN_1, LOW);


  //Setup Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting to WiFi Network ..");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  delay(3000);
  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  //get ntp time
  waitForSync();
  Serial.println("UTC: " + UTC.dateTime());
	Timezone tz;
	tz.setPosix("IST-2IDT,M3.4.4/26,M10.5.0");
	Serial.println("Israel: " + tz.dateTime());
  //this is very ugly but IL has stupid DST rules.

  //SetUp RTC and update it from NTP.
  #ifdef INTERRUPT_PIN // If using the SQW pin as an interrupt
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  #endif
  rtc.begin(DS13074_CS_PIN);
  rtc.setTime(tz.second(),tz.minute(),tz.hour(),tz.weekday(),tz.day(),tz.month(),tz.year()); //This is also pretty ugly as there is no good call from eztime (I can get a time_t value, but it still would require lots of calls, no easy way about it I think)
  rtc.update();
  rtc.setAlarm1(30);

  //setup internal timers
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &timerInterrupcion, true); // Attach the interrupt handling function - terrarium pump interrupt

  //SetUp Modbus Server
  mb.onConnect(cbConn);
  mb.server();
  mb.addCoil(FAN_ON_COIL);
  mb.addCoil(FAN_OFF_COIL);
  mb.onSetCoil(FAN_ON_COIL, cbFanOn); // Add callback on Coil FAN COILs value set - enable us to control the fan via coil
  mb.onSetCoil(FAN_OFF_COIL, cbFanOff); // Add callback on Coil FAN COILs value set - enable us to control the fan via coil
  mb.addHreg(SHT_HUMIDITY_MSB,0xABCA );
  mb.addHreg(SHT_HUMIDITY_LSB,0xABCA );
  mb.addHreg(SHT_TEMP_MSB,0xABCB );
  mb.addHreg(SHT_TEMP_LSB,0xABCB );
  mb.addHreg(TANK_DEPTH,0xABCC );
  mb.addHreg(FAN_STATUS,0xABCD );
  mb.addCoil(FAN_STATUS);
  mb.addCoil(PUMP_COIL);
  mb.addCoil(LIGHT_COIL_1);
  mb.addCoil(LIGHT_COIL_2);
  //
  
  //setup MQTT
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public EMQX MQTT broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
  client.publish(topic, "Terrarium Controller Logged In");
  client.subscribe(topic);

  //setup SHT31
  Serial.println("SHT31 test");
  counter=0;
  if (! sht31.begin(0x44)) {
    Serial.println("Couldn't find SHT31");
    sht_status = false;
  }
  else {
    sht_status = true;
  }


}
/*******************************************************************************END OF SETUP*****************************************************/

void loop() {
  //The basic loop consits of two functions - getSensors() which reads all the sensors, and setOutputs() which sets the outputs based on sensors (some outputs are set in an interrupt handler as they need to be handled in a timely manner)
  mb.task();
  getSensors();
  setOutputs() ;
  delay(1000);



}



/************************************************************************** Helper Functions ***********************************************************/
void getSensors() {
    Serial.println("Getting Sensors");
    if (sht_status) {
    my_hum = getHumidity(&sht31);
    my_temp = getTemp(&sht31);
    }
    else {
      my_hum = 0.0;
      my_temp = 0.0;
    }
    Serial.println("Got Sensors");
}

void setOutputs() {
  uint16_t hum_whole,temp_whole;
  int run_time=0;

  hum_whole = (uint16_t)my_hum;
  temp_whole = (uint16_t)my_temp;
  mb.Hreg(SHT_HUMIDITY_LSB,f_2uint_int2(my_hum));
  mb.Hreg(SHT_HUMIDITY_MSB,f_2uint_int1(my_hum));
  mb.Hreg(SHT_TEMP_LSB ,f_2uint_int2(my_temp));
  mb.Hreg(SHT_TEMP_MSB ,f_2uint_int1(my_temp));
  if (digitalRead(FAN_RELAY_PIN)==HIGH) {
    
  }
  //check pump and light status
  run_time = check_pump_schedule();
  if (run_time!=0) {
    //set pump pin to high, set timer as needed and run. function returns how long to run
    timerRestart(timer); 
    timerAlarmWrite(timer, run_time*1000000, false);
    timerAlarmEnable(timer);
    printTime();
    Serial.println(" Starting Terrarium Pump");
    digitalWrite(PUMP_RELAY, HIGH);


    
  }
  return;
}



void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}