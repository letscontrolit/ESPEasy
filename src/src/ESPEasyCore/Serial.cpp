#include "../ESPEasyCore/Serial.h"


#include "../Commands/InternalCommands.h"

#include "../Globals/Cache.h"
#include "../Globals/Logging.h" //  For serialWriteBuffer
#include "../Globals/Settings.h"

#include "../Helpers/Memory.h"


/********************************************************************************************\
 * Get data from Serial Interface
 \*********************************************************************************************/

uint8_t SerialInByte;
int  SerialInByteCounter = 0;
char InputBuffer_Serial[INPUT_BUFFER_SIZE + 2];

void initSerial()
{
  if (log_to_serial_disabled || !Settings.UseSerial || activeTaskUseSerial0()) {
    return;
  }

  // make sure previous serial buffers are flushed before resetting baudrate
  Serial.flush();
  Serial.begin(Settings.BaudRate);

  // Serial.setDebugOutput(true);
}

void serial()
{
  if (Serial.available())
  {
    String dummy;

    if (PluginCall(PLUGIN_SERIAL_IN, 0, dummy)) {
      return;
    }
  }

  if (!Settings.UseSerial || activeTaskUseSerial0()) { return; }

  while (Serial.available())
  {
    delay(0);
    SerialInByte = Serial.read();

    if (SerialInByte == 255) // binary data...
    {
      Serial.flush();
      return;
    }

    if (isprint(SerialInByte))
    {
      if (SerialInByteCounter < INPUT_BUFFER_SIZE) { // add char to string if it still fits
        InputBuffer_Serial[SerialInByteCounter++] = SerialInByte;
      }
    }

    if ((SerialInByte == '\r') || (SerialInByte == '\n'))
    {
      if (SerialInByteCounter == 0) {              // empty command?
        break;
      }
      InputBuffer_Serial[SerialInByteCounter] = 0; // serial data completed
      Serial.write('>');
      serialPrintln(InputBuffer_Serial);
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_SERIAL, InputBuffer_Serial);
      SerialInByteCounter   = 0;
      InputBuffer_Serial[0] = 0; // serial data processed, clear buffer
    }
  }
}

int getRoomLeft() {
  #ifdef USE_SECOND_HEAP
  // If stored in 2nd heap, we must check this for space
  HeapSelectIram ephemeral;
  #endif

  int roomLeft = getMaxFreeBlock();

  if (roomLeft < 1000) {
    roomLeft = 0;                              // Do not append to buffer.
  } else if (roomLeft < 4000) {
    roomLeft = 128 - serialWriteBuffer.size(); // 1 buffer.
  } else {
    roomLeft -= 4000;                          // leave some free for normal use.
  }
  return roomLeft;
}

void addToSerialBuffer(const __FlashStringHelper * line)
{
  addToSerialBuffer(String(line));
}

void addToSerialBuffer(const String& line) {
  process_serialWriteBuffer(); // Try to make some room first.
  {
    #ifdef USE_SECOND_HEAP
    // Allow to store the logs in 2nd heap if present.
    HeapSelectIram ephemeral;
    #endif
    int roomLeft = getRoomLeft();

    auto it = line.begin();
    while (roomLeft > 0 && it != line.end()) {
      serialWriteBuffer.push_back(*it);
      --roomLeft;
      ++it;
    }
  }
  process_serialWriteBuffer();
}

void addNewlineToSerialBuffer() {
  process_serialWriteBuffer(); // Try to make some room first.
  {
    #ifdef USE_SECOND_HEAP
    // Allow to store the logs in 2nd heap if present.
    HeapSelectIram ephemeral;
    #endif

    serialWriteBuffer.push_back('\r');
    serialWriteBuffer.push_back('\n');
  }
}

void process_serialWriteBuffer() {
  if (serialWriteBuffer.size() == 0) { return; }
  size_t snip = Serial.availableForWrite();

  if (snip > 0) {
    size_t bytes_to_write = serialWriteBuffer.size();

    if (snip < bytes_to_write) { bytes_to_write = snip; }

    while (bytes_to_write > 0 && !serialWriteBuffer.empty()) {
      const char c = serialWriteBuffer.front();
      if (Settings.UseSerial) {
        Serial.write(c);
      }
      serialWriteBuffer.pop_front();
      --bytes_to_write;
    }
  }
}

// For now, only send it to the serial buffer and try to process it.
// Later we may want to wrap it into a log.
void serialPrint(const __FlashStringHelper * text) {
  addToSerialBuffer(text);
}

void serialPrint(const String& text) {
  addToSerialBuffer(text);
}

void serialPrintln(const __FlashStringHelper * text) {
  addToSerialBuffer(text);
  serialPrintln();
}

void serialPrintln(const String& text) {
  addToSerialBuffer(text);
  serialPrintln();
}

void serialPrintln() {
  addNewlineToSerialBuffer();
  process_serialWriteBuffer();
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
