// Callback function for client connect. Returns true to allow connection.
#include <ModbusIP_ESP8266.h>
#include <Arduino.h>
//Define coil numbers

uint16_t cbFanOn(TRegister* reg, uint16_t val);
uint16_t cbFanOff(TRegister* reg, uint16_t val);
bool cbConn(IPAddress ip);

