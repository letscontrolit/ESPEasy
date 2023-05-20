#include "ESPEasySerialPort.h"

const __FlashStringHelper* ESPEasySerialPort_toString(ESPEasySerialPort serType)
{
  switch (serType) {
    case ESPEasySerialPort::not_set:         break;
#if USES_I2C_SC16IS752
    case ESPEasySerialPort::sc16is752:       return F("I2C Serial");
#endif
#ifdef ESP8266
    case ESPEasySerialPort::serial0_swap:    return F("HW Serial0 swap");
#endif
    case ESPEasySerialPort::serial0:         return F("HW Serial0");
    case ESPEasySerialPort::serial1:         return F("HW Serial1");
#if HAS_SERIAL2
    case ESPEasySerialPort::serial2:         return F("HW Serial2");
#endif
#if USES_SW_SERIAL
    case ESPEasySerialPort::software:        return F("SoftwareSerial");
#endif
#if USES_HWCDC
    case ESPEasySerialPort::usb_hw_cdc:      return F("USB HWCDC");
#endif 
#if USES_USBCDC
    case ESPEasySerialPort::usb_cdc_0:       return F("USB CDC0");
    case ESPEasySerialPort::usb_cdc_1:       return F("USB CDC1");
#endif 
    case ESPEasySerialPort::MAX_SERIAL_TYPE: break;

      // Do not include "default:" to let the compiler check if we miss some case
  }
  return F("");
}

bool isHWserial(ESPEasySerialPort serType)
{
  switch (serType) {
#ifdef ESP8266
    case ESPEasySerialPort::serial0_swap:
#endif
    case ESPEasySerialPort::serial0:
    case ESPEasySerialPort::serial1:
#if HAS_SERIAL2
    case ESPEasySerialPort::serial2:
#endif
      return true;
    default:
      break;
  }
  return false;
}
