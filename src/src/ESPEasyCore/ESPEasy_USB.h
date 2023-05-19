#ifndef ESPEASYCORE_ESPEASY_USB_H
#define ESPEASYCORE_ESPEASY_USB_H

// Do not include this file, but rather include "../Globals/ESPEasy_USB.h"
// from a .cpp file.


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
#    include "HWCDC.h"
#    define USES_HWCDC 1
extern  HWCDC* _hwcdc_serial;
#   else // No ARDUINO_USB_MODE
#    include "USB.h"
#    include "USBCDC.h"
#    define USES_USBCDC 1
extern  USBCDC* _usbcdc_serial;
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



#endif