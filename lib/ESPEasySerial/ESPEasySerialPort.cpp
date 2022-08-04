#include "ESPEasySerialPort.h"

const __FlashStringHelper* ESPEasySerialPort_toString(ESPEasySerialPort serType)
{
  switch (serType) {
    case ESPEasySerialPort::not_set:         break;
    case ESPEasySerialPort::sc16is752:       return F("I2C Serial");
    case ESPEasySerialPort::serial0_swap:    return F("HW Serial0 swap");
    case ESPEasySerialPort::serial0:         return F("HW Serial0");
    case ESPEasySerialPort::serial1:         return F("HW Serial1");
    case ESPEasySerialPort::serial2:         return F("HW Serial2");
    case ESPEasySerialPort::software:        return F("SoftwareSerial");
    case ESPEasySerialPort::MAX_SERIAL_TYPE: break;

      // Do not include "default:" to let the compiler check if we miss some case
  }
  return F("");
}
