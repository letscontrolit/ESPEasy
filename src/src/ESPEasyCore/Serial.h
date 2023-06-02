#ifndef ESPEASYCORE_SERIAL_H
#define ESPEASYCORE_SERIAL_H

#include "../../ESPEasy_common.h"


void initSerial();

void serial();

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
