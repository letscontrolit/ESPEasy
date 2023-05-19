#ifndef ESPEASY_SERIAL_ESPEASYSERIALPORT_H
#define ESPEASY_SERIAL_ESPEASYSERIALPORT_H

#include <Arduino.h>


// FIXME TD-er: Share this code to define USES_xxCDC

#ifdef ESP32

  /*
   #if CONFIG_IDF_TARGET_ESP32C3 ||  // support USB via HWCDC using JTAG interface
       CONFIG_IDF_TARGET_ESP32S2 ||  // support USB via USBCDC
       CONFIG_IDF_TARGET_ESP32S3     // support USB via HWCDC using JTAG interface or USBCDC
   */
# if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
// #if CONFIG_TINYUSB_CDC_ENABLED              // This define is not recognized here so use USE_USB_CDC_CONSOLE
#  ifdef USE_USB_CDC_CONSOLE
#   if ARDUINO_USB_MODE
// ESP32C3/S3 embedded USB using JTAG interface
#    define USES_HWCDC 1
#   else // No ARDUINO_USB_MODE
#    define USES_USBCDC 1
#   endif // ARDUINO_USB_MODE
#  endif  // ifdef USE_USB_CDC_CONSOLE
# endif   // if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#endif    // ifdef ESP32

#ifndef USES_HWCDC
# define USES_HWCDC 0
#endif // ifndef USES_HWCDC

#ifndef USES_USBCDC
# define USES_USBCDC 0
#endif // ifndef USES_USBCDC




// Keep value assigned as it is used in scripts and stored in the Settings.TaskDevicePort
enum class ESPEasySerialPort : uint8_t {
  not_set      = 0,
  sc16is752    = 1,
  serial0      = 2,
  serial0_swap = 3,
  serial1      = 4,
  serial2      = 5,
  software     = 6,
#if defined(ESP32) && defined(USE_USB_CDC_CONSOLE)
  usb_hw_cdc   = 7,
  usb_cdc_0    = 8,
  usb_cdc_1    = 9,
#endif

  MAX_SERIAL_TYPE
};

const __FlashStringHelper* ESPEasySerialPort_toString(ESPEasySerialPort serType);

bool isHWserial(ESPEasySerialPort serType);

#endif // ifndef ESPEASY_SERIAL_ESPEASYSERIALPORT_H
