#include "../ESPEasyCore/ESPEasy_Console_Port.h"

#include "../Commands/ExecuteCommand.h"

#include "../DataStructs/TimingStats.h"

#include "../DataTypes/ESPEasy_plugin_functions.h"

#include "../Globals/Cache.h"
#include "../Globals/Logging.h"
#include "../Globals/Plugins.h"
#include "../Globals/Settings.h"

#include "../Helpers/Memory.h"

#include <ESPEasySerialPort.h>


/*
 #if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
 # include "../Helpers/_Plugin_Helper_serial.h"
 #endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
 */

EspEasy_Console_Port::~EspEasy_Console_Port()
{
#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  if (_serial != nullptr) {
    delete _serial;
    _serial = nullptr;
  }
#endif
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

void EspEasy_Console_Port::addToSerialBuffer(char c)
{
  if (_serial != nullptr) {
    _serialWriteBuffer.add(c);
  }
}

void EspEasy_Console_Port::addToSerialBuffer(const String& line)
{
  if (_serial != nullptr) {
    _serialWriteBuffer.add(line);
  }
}

void EspEasy_Console_Port::addNewlineToSerialBuffer()
{
  if (_serial != nullptr) {
    _serialWriteBuffer.addNewline();
  }
}

bool EspEasy_Console_Port::process_serialWriteBuffer()
{
  if (_serial != nullptr) {
    const int snip = _serial->availableForWrite();
    
    if (snip > 0) {
      return _serialWriteBuffer.write(*_serial, snip) != 0;
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
      addToSerialBuffer('>');
      String cmd(InputBuffer_Serial);
      addToSerialBuffer(cmd);
      addToSerialBuffer('\n');
      ExecuteCommand_all({EventValueSource::Enum::VALUE_SOURCE_SERIAL, std::move(cmd)}, true);
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
