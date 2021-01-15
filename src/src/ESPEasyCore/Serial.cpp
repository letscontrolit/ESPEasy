#include "../ESPEasyCore/Serial.h"


#include "../Commands/InternalCommands.h"

#include "../Globals/Cache.h"
#include "../Globals/Logging.h" //  For serialWriteBuffer
#include "../Globals/Settings.h"

#include "../Helpers/Memory.h"


/********************************************************************************************\
 * Get data from Serial Interface
 \*********************************************************************************************/

byte SerialInByte;
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

void addToSerialBuffer(const char *line) {
  process_serialWriteBuffer(); // Try to make some room first.
  int roomLeft = getMaxFreeBlock();

  if (roomLeft < 1000) {
    roomLeft = 0;                              // Do not append to buffer.
  } else if (roomLeft < 4000) {
    roomLeft = 128 - serialWriteBuffer.size(); // 1 buffer.
  } else {
    roomLeft -= 4000;                          // leave some free for normal use.
  }

  const char *c = line;

  while (roomLeft > 0) {
    // Must use PROGMEM aware functions here.
    char ch = pgm_read_byte(c++);

    if (ch == '\0') {
      return;
    } else {
      serialWriteBuffer.push_back(ch);
      --roomLeft;
    }
  }
}

void addNewlineToSerialBuffer() {
  process_serialWriteBuffer(); // Try to make some room first.
  serialWriteBuffer.push_back('\r');
  serialWriteBuffer.push_back('\n');
}

void process_serialWriteBuffer() {
  if (serialWriteBuffer.size() == 0) { return; }
  size_t snip = Serial.availableForWrite();

  if (snip > 0) {
    size_t bytes_to_write = serialWriteBuffer.size();

    if (snip < bytes_to_write) { bytes_to_write = snip; }

    while (bytes_to_write > 0) {
      const char c = serialWriteBuffer.front();
      Serial.write(c);
      serialWriteBuffer.pop_front();
      --bytes_to_write;
    }
  }
}

// For now, only send it to the serial buffer and try to process it.
// Later we may want to wrap it into a log.
void serialPrint(const String& text) {
  addToSerialBuffer(text.c_str());
  process_serialWriteBuffer();
}

void serialPrintln(const String& text) {
  addToSerialBuffer(text.c_str());
  addNewlineToSerialBuffer();
  process_serialWriteBuffer();
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
