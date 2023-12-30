#include "../ESPEasyCore/Serial.h"


#include "../Commands/ExecuteCommand.h"

#include "../Globals/Cache.h"
#include "../Globals/ESPEasy_Console.h"
//#include "../Globals/Logging.h" //  For serialWriteBuffer
#include "../Globals/Settings.h"

#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Memory.h"



void initSerial()
{
  ESPEasy_Console.init();
}

void serial()
{
  ESPEasy_Console.loop();
}

bool process_serialWriteBuffer()
{
  return ESPEasy_Console.process_serialWriteBuffer();
}

// For now, only send it to the serial buffer and try to process it.
// Later we may want to wrap it into a log.
void serialPrint(const __FlashStringHelper *text) {
  ESPEasy_Console.addToSerialBuffer(text);
}

void serialPrint(const String& text) {
  ESPEasy_Console.addToSerialBuffer(text);
}

void serialPrintln(const __FlashStringHelper *text) {
  ESPEasy_Console.addToSerialBuffer(text);
  serialPrintln();
}

void serialPrintln(const String& text) {
  ESPEasy_Console.addToSerialBuffer(text);
  serialPrintln();
}

void serialPrintln() {
  ESPEasy_Console.addNewlineToSerialBuffer();
  ESPEasy_Console.process_serialWriteBuffer();
}

// Do not add helper functions for other types, since those types can only be
// explicit matched at a constructor, not a function declaration.

/*
   void serialPrint(char c) {
   serialPrint(String(c));
   }


   void serialPrint(unsigned long value) {
   serialPrint(String(value));
   }

   void serialPrint(long value) {
   serialPrint(String(value));
   }

   void serialPrintln(unsigned long value) {
   serialPrintln(String(value));
   }
 */
