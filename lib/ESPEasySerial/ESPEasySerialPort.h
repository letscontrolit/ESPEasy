#ifndef ESPEASY_SERIAL_ESPEASYSERIALPORT_H
#define ESPEASY_SERIAL_ESPEASYSERIALPORT_H

#include "ESPEasySerial_common_defines.h"


// Keep value assigned as it is used in scripts and stored in the Settings.TaskDevicePort
enum class ESPEasySerialPort : uint8_t {
  not_set = 0,
#if USES_I2C_SC16IS752
  sc16is752 = 1,
#endif // if USES_I2C_SC16IS752
  serial0 = 2,
#ifdef ESP8266
  serial0_swap = 3,
#endif // ifdef ESP8266
#if SOC_UART_NUM > 1
  serial1 = 4,
#endif // if SOC_UART_NUM > 1
#if SOC_UART_NUM > 2
  serial2 = 5,
#endif // if SOC_UART_NUM > 2
#if USES_SW_SERIAL
  software = 6,
#endif // if USES_SW_SERIAL

#if USES_HWCDC
  usb_hw_cdc = 7,
#endif // if USES_HWCDC
#if USES_USBCDC
  usb_cdc_0 = 8,
//  usb_cdc_1 = 9,  // TD-er: Disable 2nd USB CDC port as it doesn't seem to work well
#endif // if USES_USBCDC

  MAX_SERIAL_TYPE
};


const __FlashStringHelper* ESPEasySerialPort_toString(ESPEasySerialPort port);

bool                       isHWserial(ESPEasySerialPort port);

bool                       useGPIOpins(ESPEasySerialPort port);

bool                       validSerialPort(ESPEasySerialPort port);

#endif // ifndef ESPEASY_SERIAL_ESPEASYSERIALPORT_H
