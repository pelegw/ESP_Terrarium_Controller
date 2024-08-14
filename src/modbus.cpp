// Callback function for client connect. Returns true to allow connection.
#include <ModbusIP_ESP8266.h>
#include <Arduino.h>

bool cbConn(IPAddress ip) {
  Serial.println(ip);
  return true;
}

// Callback function for write (set) Coil. Returns value to store.
uint16_t cbFanOn(TRegister* reg, uint16_t val) {
  //Fan On value set - check if it's 0 or 1 then 
  if (COIL_BOOL(val)) {
    Serial.println("Fan On Command Recieved");
  }
  else {
    Serial.println("Fan On Command Released"); //We reset the coil so that no false trigerring of fan happens.
  }
  return val;
}

uint16_t cbFanOff(TRegister* reg, uint16_t val) {
  //Fan On value set - check if it's 0 or 1 then 
  if (COIL_BOOL(val)) {
    Serial.println("Fan Off Command Recieved");
  }
  else {
    Serial.println("Fan Off Command Released"); //We reset the coil so that no false trigerring of fan happens.
  }
  return val;
}
