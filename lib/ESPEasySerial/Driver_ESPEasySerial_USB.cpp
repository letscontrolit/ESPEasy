#include "Driver_ESPEasySerial_USB.h"


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
#    if ARDUINO_USB_CDC_ON_BOOT // Serial used for USB CDC
HWCDC *_hwcdc_serial = &Serial;
#    else // if ARDUINO_USB_CDC_ON_BOOT
HWCDC *_hwcdc_serial = &USBSerial;
#    endif // if ARDUINO_USB_CDC_ON_BOOT
#   else // No ARDUINO_USB_MODE
/*
#    if ARDUINO_USB_CDC_ON_BOOT// Serial used for USB CDC
USBCDC *_usbcdc_serial = &Serial;
#    else // if ARDUINO_USB_CDC_ON_BOOT
USBCDC  _usbcdc_serial_stack_allocated;
USBCDC *_usbcdc_serial = &_usbcdc_serial_stack_allocated;
#    endif // if ARDUINO_USB_CDC_ON_BOOT
*/
#   endif // ARDUINO_USB_MODE
#  endif  // ifdef USE_USB_CDC_CONSOLE
# endif   // if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#endif    // ifdef ESP32
