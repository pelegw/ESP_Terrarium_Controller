#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <SparkFunDS3234RTC.h>
#include "config.h"



 #define DS13074_CS_PIN 10
 //#define INTERRUPT_PIN 2 // DeadOn RTC SQW/interrupt pin (optional)


//Modbus Variables

// put function declarations here:
//nt myFunction(int, int);
void printTime();

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

}

void loop() {
  static int8_t lastSecond = -1;
  rtc.update();
  if (rtc.second() != lastSecond) // If the second has changed
  {
    printTime(); // Print the new time
    
    lastSecond = rtc.second(); // Update lastSecond value
  }

  if (rtc.alarm1())
    {
      Serial.println("ALARM 1!");
      // Re-set the alarm for when s=30:
      rtc.setAlarm1(30);
    }

}

// put function definitions here:
void printTime()
{
  Serial.print(String(rtc.hour()) + ":"); // Print hour
  if (rtc.minute() < 10)
    Serial.print('0'); // Print leading '0' for minute
  Serial.print(String(rtc.minute()) + ":"); // Print minute
  if (rtc.second() < 10)
    Serial.print('0'); // Print leading '0' for second
  Serial.print(String(rtc.second())); // Print second

  if (rtc.is12Hour()) // If we're in 12-hour mode
  {
    // Use rtc.pm() to read the AM/PM state of the hour
    if (rtc.pm()) Serial.print(" PM"); // Returns true if PM
    else Serial.print(" AM");
  }
  
  Serial.print(" | ");

  // Few options for printing the day, pick one:
  Serial.print(rtc.dayStr()); // Print day string
  //Serial.print(rtc.dayC()); // Print day character
  //Serial.print(rtc.day()); // Print day integer (1-7, Sun-Sat)
  Serial.print(" - ");
#ifdef PRINT_USA_DATE
  Serial.print(String(rtc.month()) + "/" +   // Print month
                 String(rtc.date()) + "/");  // Print date
#else
  Serial.print(String(rtc.date()) + "/" +    // (or) print date
                 String(rtc.month()) + "/"); // Print month
#endif
  Serial.println(String(rtc.year()));        // Print year
}