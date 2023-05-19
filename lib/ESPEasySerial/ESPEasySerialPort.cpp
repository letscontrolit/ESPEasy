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
  #ifdef ESP32
  # if USES_USBCDC
    case ESPEasySerialPort::usb_cdc_0:       return F("USB CDC0");
    case ESPEasySerialPort::usb_cdc_1:       return F("USB CDC1");
  # else // if USES_USBCDC
    case ESPEasySerialPort::usb_cdc_0:
    case ESPEasySerialPort::usb_cdc_1:
      break;
  # endif // if USES_USBCDC
  # if USES_HWCDC
    case ESPEasySerialPort::usb_hw_cdc:      return F("USB HWCDC");
  # else // if USES_HWCDC
    case ESPEasySerialPort::usb_hw_cdc: break;
  # endif // if USES_HWCDC
  #endif // ifdef ESP32
    case ESPEasySerialPort::MAX_SERIAL_TYPE: break;

      // Do not include "default:" to let the compiler check if we miss some case
  }
  return F("");
}

bool isHWserial(ESPEasySerialPort serType)
{
  switch (serType) {
    case ESPEasySerialPort::serial0_swap:
    case ESPEasySerialPort::serial0:
    case ESPEasySerialPort::serial1:
    case ESPEasySerialPort::serial2:
      return true;
    default:
      break;
  }
  return false;
}
