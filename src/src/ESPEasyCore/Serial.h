#ifndef ESPEASYCORE_SERIAL_H
#define ESPEASYCORE_SERIAL_H

#include "../../ESPEasy_common.h"

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
# include <ESPeasySerial.h>
#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT


#define INPUT_BUFFER_SIZE          128

extern int     SerialInByteCounter;
extern char    InputBuffer_Serial[INPUT_BUFFER_SIZE + 2];

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

// Cache the used settings, so we can check whether to change the console serial
extern  uint8_t console_serial_port;
extern  int8_t  console_serial_rxpin;
extern  int8_t  console_serial_txpin;

extern ESPeasySerial ESPEASY_SERIAL_CONSOLE_PORT;

#else // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT


# ifdef ESP32

/*
 #if CONFIG_IDF_TARGET_ESP32C3 ||  // support USB via HWCDC using JTAG interface
     CONFIG_IDF_TARGET_ESP32S2 ||  // support USB via USBCDC
     CONFIG_IDF_TARGET_ESP32S3     // support USB via HWCDC using JTAG interface or USBCDC
 */
#  if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3

// #if CONFIG_TINYUSB_CDC_ENABLED              // This define is not recognized here so use USE_USB_CDC_CONSOLE
#   ifdef USE_USB_CDC_CONSOLE
#    if ARDUINO_USB_MODE
// ESP32C3/S3 embedded USB using JTAG interface
extern HWCDC ESPEASY_SERIAL_CONSOLE_PORT;
#    else // No ARDUINO_USB_MODE
#     include "USB.h"
#     include "USBCDC.h"
extern USBCDC ESPEASY_SERIAL_CONSOLE_PORT;
#    endif // ARDUINO_USB_MODE

#   else // No USE_USB_CDC_CONSOLE
#    define ESPEASY_SERIAL_CONSOLE_PORT Serial     // Fallback serial interface for ESP32C3, S2 and S3 if no USB_SERIAL defined
#   endif // USE_USB_CDC_CONSOLE

#  else // No ESP32C3, S2 or S3
#   define ESPEASY_SERIAL_CONSOLE_PORT Serial      // Fallback serial interface for non ESP32C3, S2 and S3
#  endif // ESP32C3, S2 or S3

# else  // No ESP32
// Using the standard Serial0 HW serial port.
  #  define ESPEASY_SERIAL_CONSOLE_PORT Serial
# endif // ifdef ESP32

#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
void checkSerialConflict(ESPEasySerialPort port,
                         int               receivePin,
                         int               transmitPin);
#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

void initSerial();

void serial();

void addToSerialBuffer(const __FlashStringHelper *line);
void addToSerialBuffer(const String& line);

void addNewlineToSerialBuffer();

// Return true when something got written, or when the buffer was already empty
bool process_serialWriteBuffer();

// For now, only send it to the serial buffer and try to process it.
// Later we may want to wrap it into a log.
void serialPrint(const __FlashStringHelper *text);
void serialPrint(const String& text);

void serialPrintln(const __FlashStringHelper *text);
void serialPrintln(const String& text);

void serialPrintln();

// Do not add helper functions for other types, since those types can only be
// explicit matched at a constructor, not a function declaration.

/*
   void serialPrint(char c);

   void serialPrint(unsigned long value);

   void serialPrint(long value);

   void serialPrintln(unsigned long value);
 */


#endif // ifndef ESPEASYCORE_SERIAL_H
