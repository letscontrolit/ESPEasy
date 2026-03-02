#include "../ESPEasyCore/ESPEasy_Console_Port.h"

#include "../Commands/ExecuteCommand.h"

#include "../DataStructs/TimingStats.h"

#include "../DataTypes/ESPEasy_plugin_functions.h"

#include "../Globals/Cache.h"
#include "../Globals/Logging.h"
#include "../Globals/Plugins.h"
#include "../Globals/Settings.h"

#include "../Helpers/Memory.h"
#include "../Helpers/StringConverter.h"

#include <ESPEasySerialPort.h>


#ifdef ESP32
# define CONSOLE_INPUT_BUFFER_SIZE          1280
#else
# define CONSOLE_INPUT_BUFFER_SIZE          128
#endif // ifdef ESP32


/*
 #if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
 # include "../Helpers/_Plugin_Helper_serial.h"
 #endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
 */

EspEasy_Console_Port::EspEasy_Console_Port(LogDestination log_destination)
  : _serialWriteBuffer(log_destination)
{
  InputBuffer_Serial = (char *)calloc(1, CONSOLE_INPUT_BUFFER_SIZE);
}

EspEasy_Console_Port::~EspEasy_Console_Port()
{
#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  if (_serial != nullptr) {
    delete _serial;
    _serial = nullptr;
  }
#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  free(InputBuffer_Serial);
}

EspEasy_Console_Port::operator bool() const
{
  if (_serial != nullptr) {
    return true;

    //    return _serial->operator bool();
  }
  return false;
}

int EspEasy_Console_Port::read()
{
#if FEATURE_IMPROV
  uint8_t b;

  if (_improv.getFromBuffer(b)) {
    return b;
  }

#endif // if FEATURE_IMPROV
  int res = -1;

  if (_serial != nullptr)
  {
    res = _serial->read();
#if FEATURE_IMPROV

    if (res >= 0) {
      if (_improv.handle(res, _serial)) {
        // Looks like it might be an IMPROV command, so like we didn't have any data
        return -1;
      }

      if (_improv.getFromBuffer(b)) {
        return b;
      }
    }

#endif // if FEATURE_IMPROV
  }
  return res;
}

size_t EspEasy_Console_Port::available() const
{
  size_t res = 0u;

#if FEATURE_IMPROV
  res += _improv.available();
#endif // if FEATURE_IMPROV

  if (_serial != nullptr) {
    res += _serial->available();
  }
  return res;
}

void EspEasy_Console_Port::endPort()
{
  if (_serial != nullptr) {
    _serial->end();
  }
}

bool EspEasy_Console_Port::process_serialWriteBuffer()
{
  if (_serial != nullptr) {
#ifdef ESP32

    if (!xPortCanYield()) { return false; }
#endif // ifdef ESP32
    size_t availableForWrite = _serial->availableForWrite();

    if (availableForWrite == 0) { return false; }

    if (availableForWrite == 1) {
      // For only a single byte, just write it directly
      return _serialWriteBuffer.process(_serial, availableForWrite);
    }

    if (availableForWrite > 64) {
      // Set to max. of 64 bytes as this is the optimum 'chunk size' for most
      // serial ports, like the CDC ports and I2C to UART.
      // Also it is relatively fast to allocate.
      availableForWrite = 64;
    }

    PrintToString str;
    str.reserve(availableForWrite);

    if (_serialWriteBuffer.process(&str, availableForWrite)) {
      _serial->write(str.get().c_str(), str.length());
      return true;
    }
  }
  return false;
}

bool EspEasy_Console_Port::process_consoleInput(uint8_t SerialInByte)
{
  if (isprint(SerialInByte))
  {
    if (SerialInByteCounter < CONSOLE_INPUT_BUFFER_SIZE) { // add char to string if it still fits
      InputBuffer_Serial[SerialInByteCounter++] = SerialInByte;
    }
  }

  if ((SerialInByte == '\b') && (SerialInByteCounter > 0)) // Correct a typo using BackSpace
  {
    --SerialInByteCounter;
  } else
  if ((SerialInByte == '\r') || (SerialInByte == '\n'))
  {
    // Ignore empty command
    if (SerialInByteCounter != 0) {
      InputBuffer_Serial[SerialInByteCounter] = 0; // serial data completed

      String cmd(InputBuffer_Serial);
#if !FEATURE_COLORIZE_CONSOLE_LOGS
      Logging.consolePrintln(concat('>', cmd));
#endif

      ExecuteCommand_all({ EventValueSource::Enum::VALUE_SOURCE_SERIAL, std::move(cmd) }, true);
      SerialInByteCounter   = 0;
      InputBuffer_Serial[0] = 0; // serial data processed, clear buffer
      return true;
    }
  }
  return false;
}

String EspEasy_Console_Port::getPortDescription() const
{
  if (_serial != nullptr) {
  #if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
    return _serial->getPortDescription();
  #else // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
    String res = F("HW Serial0 @ ");
    res += _serial->baudRate();
    return res;
  #endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  }

  return F("-");
}
