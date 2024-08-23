 #include <Arduino.h>
#include "sht31.h"

float getTemp(Adafruit_SHT31 *sht) {
  float tmp = sht->readTemperature();
  if (! isnan(tmp)) {  // check if 'is not a number'
   Serial.print("Temp *C = "); Serial.print(tmp); Serial.print("\t\t");
 } else { 
    Serial.println("Failed to read temperature");
    return -10000;
    }
  return tmp;
}
float getHumidity(Adafruit_SHT31 *sht) {
  float hum = sht->readHumidity();
  if (! isnan(hum)) {  // check if 'is not a number'
   Serial.print("Hunidity *% = "); Serial.print(hum); Serial.print("\t\t");
 } else { 
    Serial.println("Failed to read temperature");
    return -10000;
    }
  return hum;
}
