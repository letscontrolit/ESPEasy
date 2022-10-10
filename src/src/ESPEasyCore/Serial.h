#ifndef ESPEASYCORE_SERIAL_H
#define ESPEASYCORE_SERIAL_H

#include "../../ESPEasy_common.h"

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
#include <ESPeasySerial.h>
#endif


#define INPUT_BUFFER_SIZE          128

extern uint8_t SerialInByte;
extern int  SerialInByteCounter;
extern char InputBuffer_Serial[INPUT_BUFFER_SIZE + 2];

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
// Cache the used settings, so we can check whether to change the console serial
extern  uint8_t console_serial_port;
extern  int8_t  console_serial_rxpin;
extern  int8_t  console_serial_txpin;

extern ESPeasySerial* console_serial_ptr;
extern ESPeasySerial& ESPEASY_SERIAL_CONSOLE_PORT;

#else
  // Using the standard Serial0 HW serial port.
  #define ESPEASY_SERIAL_CONSOLE_PORT Serial
#endif



void initSerial();

void serial();

void addToSerialBuffer(const __FlashStringHelper * line);
void addToSerialBuffer(const String& line);

void addNewlineToSerialBuffer();

void process_serialWriteBuffer();

// For now, only send it to the serial buffer and try to process it.
// Later we may want to wrap it into a log.
void serialPrint(const __FlashStringHelper * text);
void serialPrint(const String& text);

void serialPrintln(const __FlashStringHelper * text);
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
