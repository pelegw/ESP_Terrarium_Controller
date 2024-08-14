#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <SparkFunDS3234RTC.h>
#include "config.h"
#include "modbus.h"
#include <ModbusIP_ESP8266.h>
#include "custom_time.h"


//global vars
hw_timer_t *timer = NULL;
bool has_expired = false;



 #define DS13074_CS_PIN 10
 //#define INTERRUPT_PIN 2 // DeadOn RTC SQW/interrupt pin (optional)

//Modbus Variables
ModbusIP mb;

void IRAM_ATTR timerInterrupcion() {
 has_expired = true;
}

void setup() {
  Serial.begin(115200);

  //Setup Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting to WiFi Network ..");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  delay(1000);
  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  //SetUp RTC
  #ifdef INTERRUPT_PIN // If using the SQW pin as an interrupt
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  #endif
  rtc.begin(DS13074_CS_PIN);
  rtc.autoTime();
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

}

void loop() {
  static int8_t lastSecond = -1;
  rtc.update();
  if (rtc.second() != lastSecond) // If the second has changed
  {
    printTime(); // Print the new time
        lastSecond = rtc.second(); // Update lastSecond value
        mb.task();
  }

  if (rtc.alarm1())
    {
      Serial.println("ALARM 1!");
       timerRestart(timer);    
      timerAlarmWrite(timer, 25000000, false); // Interrupt every 25 seconds
      timerAlarmEnable(timer); // Enable the alarm
      // Re-set the alarm for when s=30:
      rtc.setAlarm1(30);
    }
  
  if(has_expired) //interrupt called
  {
     // Tasks to perform when the Timer interrupt is triggered
    Serial.println("20 Seconds Timer exapired!");
    has_expired = false; 
    
  }

}

