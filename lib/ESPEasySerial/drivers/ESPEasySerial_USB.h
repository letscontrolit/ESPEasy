#ifndef ESPEASYCORE_ESPEASY_USB_H
#define ESPEASYCORE_ESPEASY_USB_H

// Do not include this file, but rather include "../Globals/ESPEasy_USB.h"
// from a .cpp file.

#include "../ESPEasySerial_common_defines.h"

#if USES_HWCDC
// ESP32C3/S3 embedded USB using JTAG interface
#include <HWCDC.h>
extern HWCDC *_hwcdc_serial;
#endif

#if USES_USBCDC
#include <USB.h>
#include <USBCDC.h>
extern USBCDC *_usbcdc_serial;
#endif

#endif // ifndef ESPEASYCORE_ESPEASY_USB_H
