#ifndef GLOBALS_ESPEASY_CONSOLE_H
#define GLOBALS_ESPEASY_CONSOLE_H


// Include this global definition of ESPEasy_Console only from a .cpp file
// Problem is USBCDC, which must be defined as extern and not as a class member.

#include "../../ESPEasy_common.h"

#include "../ESPEasyCore/ESPEasy_Console.h"

extern EspEasy_Console_t ESPEasy_Console;

#endif