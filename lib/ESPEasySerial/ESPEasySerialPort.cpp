#include "ESPEasySerialPort.h"

const __FlashStringHelper* ESPEasySerialPort_toString(ESPEasySerialPort port)
{
  switch (port) {
    case ESPEasySerialPort::not_set:         break;
#if USES_I2C_SC16IS752
    case ESPEasySerialPort::sc16is752:       return F("I2C Serial");
#endif // if USES_I2C_SC16IS752
#ifdef ESP8266
    case ESPEasySerialPort::serial0_swap:    return F("HW Serial0 swap");
#endif // ifdef ESP8266
    case ESPEasySerialPort::serial0:         return F("HW Serial0");
#if SOC_UART_NUM > 1
    case ESPEasySerialPort::serial1:         return F("HW Serial1");
#endif // if SOC_UART_NUM > 1
#if SOC_UART_NUM > 2
    case ESPEasySerialPort::serial2:         return F("HW Serial2");
#endif // if SOC_UART_NUM > 2
#if USES_SW_SERIAL
    case ESPEasySerialPort::software:        return F("SW Serial");
#endif // if USES_SW_SERIAL
#if USES_HWCDC
    case ESPEasySerialPort::usb_hw_cdc:      return F("USB HWCDC");
#endif // if USES_HWCDC
#if USES_USBCDC
    case ESPEasySerialPort::usb_cdc_0:       return F("USB CDC");
//    case ESPEasySerialPort::usb_cdc_1:       return F("USB CDC1");
#endif // if USES_USBCDC
    case ESPEasySerialPort::MAX_SERIAL_TYPE: break;

      // Do not include "default:" to let the compiler check if we miss some case
  }
  return F("");
}

bool isHWserial(ESPEasySerialPort port)
{
  // FIXME TD-er: Still needed?
  switch (port) {
#ifdef ESP8266
    case ESPEasySerialPort::serial0_swap:
#endif // ifdef ESP8266
    case ESPEasySerialPort::serial0:
#if SOC_UART_NUM > 1
    case ESPEasySerialPort::serial1:
#endif // if SOC_UART_NUM > 1
#if SOC_UART_NUM > 2
    case ESPEasySerialPort::serial2:
#endif // if SOC_UART_NUM > 2
      return true;
    default:
      break;
  }
  return false;
}

bool useGPIOpins(ESPEasySerialPort port)
{
  switch (port) {
    case ESPEasySerialPort::serial0:
#ifdef ESP8266
    case ESPEasySerialPort::serial0_swap:
#endif // ifdef ESP8266
#if SOC_UART_NUM > 1
    case ESPEasySerialPort::serial1:
#endif // if SOC_UART_NUM > 1
#if SOC_UART_NUM > 2
    case ESPEasySerialPort::serial2:
#endif // if SOC_UART_NUM > 2
#if USES_SW_SERIAL
    case ESPEasySerialPort::software:
#endif // if USES_SW_SERIAL
      return true;

    default:
      break;
  }
  return false;
}

bool validSerialPort(ESPEasySerialPort port)
{
  switch (port) {
#if USES_I2C_SC16IS752
    case ESPEasySerialPort::sc16is752:
#endif // if USES_I2C_SC16IS752
    case ESPEasySerialPort::serial0:
#ifdef ESP8266
    case ESPEasySerialPort::serial0_swap:
#endif // ifdef ESP8266
#if SOC_UART_NUM > 1
    case ESPEasySerialPort::serial1:
#endif // if SOC_UART_NUM > 1
#if SOC_UART_NUM > 2
    case ESPEasySerialPort::serial2:
#endif // if SOC_UART_NUM > 2
#if USES_SW_SERIAL
    case ESPEasySerialPort::software:
#endif // if USES_SW_SERIAL
#if USES_HWCDC
    case ESPEasySerialPort::usb_hw_cdc:
#endif // if USES_HWCDC
#if USES_USBCDC
    case ESPEasySerialPort::usb_cdc_0:
//    case ESPEasySerialPort::usb_cdc_1:
#endif // if USES_USBCDC
      return true;

    default:
      break;
  }
  return false;
}
