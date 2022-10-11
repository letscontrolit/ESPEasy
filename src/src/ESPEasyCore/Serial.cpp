#include "../ESPEasyCore/Serial.h"


#include "../Commands/InternalCommands.h"

#include "../Globals/Cache.h"
#include "../Globals/Logging.h" //  For serialWriteBuffer
#include "../Globals/Settings.h"

#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Memory.h"

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
#include "../Helpers/_Plugin_Helper_serial.h"
#endif

/********************************************************************************************\
 * Get data from Serial Interface
 \*********************************************************************************************/

uint8_t SerialInByte;
int  SerialInByteCounter = 0;
char InputBuffer_Serial[INPUT_BUFFER_SIZE + 2];

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
uint8_t console_serial_port = 2; // ESPEasySerialPort::serial0
int8_t  console_serial_rxpin = 3;
int8_t  console_serial_txpin = 1;

ESPeasySerial ESPEASY_SERIAL_CONSOLE_PORT(
  static_cast<ESPEasySerialPort>(console_serial_port), 
  console_serial_rxpin, 
  console_serial_txpin, 
  false, 
  64);

#endif


void initSerial()
{
  updateActiveTaskUseSerial0();
  if (!Settings.UseSerial) {
    return;
  }
  
#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  // FIXME TD-er: Must detect whether we should swap software serial on pin 3&1 for HW serial if Serial0 is not being used anymore.
  const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(Settings.console_serial_port);
  if (port == ESPEasySerialPort::serial0 || port == ESPEasySerialPort::serial0_swap) {
    if (activeTaskUseSerial0()) {
      return;
    }
  }
#else
  if (activeTaskUseSerial0()) {
    return;
  }
#endif

  if (log_to_serial_disabled) {
    return;
  }


#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  if (Settings.console_serial_port != console_serial_port ||
      Settings.console_serial_rxpin != console_serial_rxpin ||
      Settings.console_serial_txpin != console_serial_txpin) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("Serial : Change serial console port from: ");
      log += ESPEasySerialPort_toString(static_cast<ESPEasySerialPort>(console_serial_port));
      log += F(" to: ");
      log += ESPEasySerialPort_toString(static_cast<ESPEasySerialPort>(Settings.console_serial_port));
      addLogMove(LOG_LEVEL_INFO, log);      
    }
    process_serialWriteBuffer();

    // Update cached values
    console_serial_port  = Settings.console_serial_port;
    console_serial_rxpin = Settings.console_serial_rxpin;
    console_serial_txpin = Settings.console_serial_txpin;

    ESPEASY_SERIAL_CONSOLE_PORT.resetConfig(
      static_cast<ESPEasySerialPort>(console_serial_port), 
      console_serial_rxpin, 
      console_serial_txpin, 
      false, 
      64
      #ifdef ESP8266
      , activeTaskUseSerial0()
      #endif
      );
  }

#endif

  // make sure previous serial buffers are flushed before resetting baudrate
  ESPEASY_SERIAL_CONSOLE_PORT.flush();
  ESPEASY_SERIAL_CONSOLE_PORT.begin(Settings.BaudRate);

  // ESPEASY_SERIAL_CONSOLE_PORT.setDebugOutput(true);
}

void serial()
{
  if (ESPEASY_SERIAL_CONSOLE_PORT.available())
  {
    String dummy;

    if (PluginCall(PLUGIN_SERIAL_IN, 0, dummy)) {
      return;
    }
  }
#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  // FIXME TD-er: Must add check whether SW serial may be using the same pins as Serial0
  if (!Settings.UseSerial) { return; }
#else
  if (!Settings.UseSerial || activeTaskUseSerial0()) { return; }
#endif

  while (ESPEASY_SERIAL_CONSOLE_PORT.available())
  {
    delay(0);
    SerialInByte = ESPEASY_SERIAL_CONSOLE_PORT.read();

    if (SerialInByte == 255) // binary data...
    {
      ESPEASY_SERIAL_CONSOLE_PORT.flush();
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
      ESPEASY_SERIAL_CONSOLE_PORT.write('>');
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
  size_t bytes_to_write = serialWriteBuffer.size();
  if (bytes_to_write == 0) { return; }
  const uint32_t timeout = millis() + 5; // Allow for max 5 msec loop

  while (!timeOutReached(timeout)) {
    size_t snip = ESPEASY_SERIAL_CONSOLE_PORT.availableForWrite();

    if (snip > 0) {

      if (snip < bytes_to_write) { bytes_to_write = snip; }

      while (bytes_to_write > 0 && !serialWriteBuffer.empty()) {
        const char c = serialWriteBuffer.front();
        if (Settings.UseSerial) {
          ESPEASY_SERIAL_CONSOLE_PORT.write(c);
        }
        serialWriteBuffer.pop_front();
        --bytes_to_write;
      }
    }
    bytes_to_write = serialWriteBuffer.size();
    if (bytes_to_write == 0) return;
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
