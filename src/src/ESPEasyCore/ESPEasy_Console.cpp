#include "../ESPEasyCore/ESPEasy_Console.h"


#include "../Commands/InternalCommands.h"

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
  if (_serial != nullptr) {
    delete _serial;
    _serial = nullptr;
  }
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
  bool res = false;

  if (_serial != nullptr) {
    const int snip = _serial->availableForWrite();

    if  (snip > 0) {
      res = _serialWriteBuffer.write(*_serial, snip) != 0;
    }
  }
  return res;
}

bool EspEasy_Console_Port::process_consoleInput(uint8_t SerialInByte)
{
  if (SerialInByte == 255) // binary data...
  {
    _serial->flush();
    return false;
  }

  if (isprint(SerialInByte))
  {
    if (SerialInByteCounter < CONSOLE_INPUT_BUFFER_SIZE) { // add char to string if it still fits
      InputBuffer_Serial[SerialInByteCounter++] = SerialInByte;
    }
  }

  if ((SerialInByte == '\r') || (SerialInByte == '\n'))
  {
    if (SerialInByteCounter == 0) {              // empty command?
      return false;
    }
    InputBuffer_Serial[SerialInByteCounter] = 0; // serial data completed
    addToSerialBuffer('>');
    addToSerialBuffer(String(InputBuffer_Serial));
    ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_SERIAL, InputBuffer_Serial);
    SerialInByteCounter   = 0;
    InputBuffer_Serial[0] = 0; // serial data processed, clear buffer
  }
  return true;
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

EspEasy_Console_t::EspEasy_Console_t()
{
#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(_console_serial_port);

# if USES_USBCDC

  /*
     if (port == ESPEasySerialPort::usb_cdc_0 ||
        port == ESPEasySerialPort::usb_cdc_1)
     {
      USB.manufacturerName(F("ESPEasy"));
      USB.productName()
     }
   */
# endif // if USES_USBCDC

# ifdef ESP8266
  constexpr size_t buffSize = 256;
# endif // ifdef ESP8266
# ifdef ESP32

  // Ideal buffer size is a trade-off between bootspeed
  // and not missing data when the ESP is busy processing stuff.
  // Since we do have a separate buffer in the console,
  // it may just take less time in the background tasks to dump
  // any logs as larger chunks can be transferred at once.
  constexpr size_t buffSize = 512;
# endif // ifdef ESP32

  ESPEasySerialConfig config;
  config.port          = port;
  config.baud          = DEFAULT_SERIAL_BAUD;
  config.receivePin    = _console_serial_rxpin;
  config.transmitPin   = _console_serial_txpin;
  config.inverse_logic = false;
  config.rxBuffSize    = 256;
  config.txBuffSize    = buffSize;

  _mainSerial._serial = new (std::nothrow) ESPeasySerial(config);

# if USES_ESPEASY_CONSOLE_FALLBACK_PORT

  if (Settings.console_serial0_fallback && (port != ESPEasySerialPort::serial0)) {
    config.port        = ESPEasySerialPort::serial0;
    config.receivePin  = SOC_RX0;
    config.transmitPin = SOC_TX0;

    _fallbackSerial._serial = new (std::nothrow) ESPeasySerial(config);
  }
# endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT


#endif  // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
}

void EspEasy_Console_t::reInit()
{
  updateActiveTaskUseSerial0();
#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(Settings.console_serial_port);

  const bool consoleUseSerial0 = (
# ifdef ESP8266
    (port == ESPEasySerialPort::serial0_swap) ||
# endif // ifdef ESP8266
    port == ESPEasySerialPort::serial0);

  const bool canUseSerial0 = !activeTaskUseSerial0() && !log_to_serial_disabled;


  bool mustHaveSerial = Settings.UseSerial && (!consoleUseSerial0 || canUseSerial0);


# if USES_ESPEASY_CONSOLE_FALLBACK_PORT

  bool mustHaveFallback = false;

  if (Settings.UseSerial) {
    if (consoleUseSerial0 && canUseSerial0 && (_fallbackSerial._serial != nullptr)) {
      // Should not destruct an already running fallback serial port
      mustHaveFallback = true;
      mustHaveSerial   = false;
    } else {
      if (Settings.console_serial0_fallback && !consoleUseSerial0 && canUseSerial0)
      {
        mustHaveFallback = true;
      }
    }
  }

  if (!mustHaveFallback) {
    if (_fallbackSerial._serial != nullptr) {
      _fallbackSerial._serial->flush();
      _fallbackSerial._serial->end();
      delete _fallbackSerial._serial;
      _fallbackSerial._serial = nullptr;
    }
  }
# endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT


  if ((_console_serial_port != Settings.console_serial_port) ||
      (_console_serial_rxpin != Settings.console_serial_rxpin) ||
      (_console_serial_txpin != Settings.console_serial_txpin) ||
      !mustHaveSerial) {
    if (_mainSerial._serial != nullptr) {
      _mainSerial._serial->flush();
      _mainSerial._serial->end();
      delete _mainSerial._serial;
      _mainSerial._serial = nullptr;
    }

    _console_serial_port  = Settings.console_serial_port;
    _console_serial_rxpin = Settings.console_serial_rxpin;
    _console_serial_txpin = Settings.console_serial_txpin;
  }

  if ((_mainSerial._serial == nullptr) && mustHaveSerial) {
    _mainSerial._serial = new (std::nothrow) ESPeasySerial(
      static_cast<ESPEasySerialPort>(_console_serial_port),
      _console_serial_rxpin,
      _console_serial_txpin);
  }
# if USES_ESPEASY_CONSOLE_FALLBACK_PORT

  if ((_fallbackSerial._serial == nullptr) && mustHaveFallback) {
    _fallbackSerial._serial = new (std::nothrow) ESPeasySerial(
      ESPEasySerialPort::serial0,
      SOC_RX0,
      SOC_TX0);
  }
# endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT


# if USES_ESPEASY_CONSOLE_FALLBACK_PORT

  if (_fallbackSerial._serial == nullptr) {
    _fallbackSerial._serialWriteBuffer.clear();
  }

# endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT

  if (_mainSerial._serial == nullptr) {
    _mainSerial._serialWriteBuffer.clear();
  }
#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  begin(Settings.BaudRate);
}

void EspEasy_Console_t::begin(uint32_t baudrate)
{
  updateActiveTaskUseSerial0();
  _baudrate = baudrate;

  if (_mainSerial._serial != nullptr) {
#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
    _mainSerial._serial->begin(baudrate);
    addLog(LOG_LEVEL_INFO, F("ESPEasy console using ESPEasySerial"));
#else // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
# ifdef ESP8266
    _mainSerial._serial->begin(baudrate);
    addLog(LOG_LEVEL_INFO, F("ESPEasy console using HW Serial"));
# endif // ifdef ESP8266
# ifdef ESP32

    // Allow to flush data from the serial buffers
    // When not opening the USB serial port, the ESP may hang at boot.
    delay(10);
    _mainSerial._serial->end();
    delay(10);
    _mainSerial._serial->begin(baudrate);
    _mainSerial._serial->flush();
# endif // ifdef ESP32
#endif  // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  }
#if USES_ESPEASY_CONSOLE_FALLBACK_PORT

  if (_fallbackSerial._serial != nullptr) {
    _fallbackSerial._serial->begin(baudrate);
    addLog(LOG_LEVEL_INFO, F("ESPEasy console fallback enabled"));
  }
#endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
}

void EspEasy_Console_t::init() {
#if FEATURE_IMPROV
  _mainSerial._improv.init();
# if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
#  if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  _fallbackSerial._improv.init();
#  endif
# endif
#endif
  updateActiveTaskUseSerial0();

  if (!Settings.UseSerial) {
    return;
  }

#if !FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  if (activeTaskUseSerial0() || log_to_serial_disabled) {
    return;
  }
#endif // if !FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  begin(Settings.BaudRate);
}

void EspEasy_Console_t::loop()
{
  START_TIMER;

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  const bool consoleUsesSerial0 =
    (static_cast<ESPEasySerialPort>(_console_serial_port) == ESPEasySerialPort::serial0
# ifdef ESP8266
     || static_cast<ESPEasySerialPort>(_console_serial_port) == ESPEasySerialPort::serial0_swap
# endif // ifdef ESP8266
    );

  if (handledByPluginSerialIn())
  {
    // Any serial0 data is already dealt with
    if (!consoleUsesSerial0 && (_mainSerial._serial != nullptr)) {
      readInput(_mainSerial);
    }
    return;
  }
#else // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  if (handledByPluginSerialIn()) {
    return;
  }
#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  readInput(_mainSerial);
#if USES_ESPEASY_CONSOLE_FALLBACK_PORT

  readInput(_fallbackSerial);
#endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT

  STOP_TIMER(CONSOLE_LOOP);
}

void EspEasy_Console_t::addToSerialBuffer(const __FlashStringHelper *line)
{
  addToSerialBuffer(String(line));
}

void EspEasy_Console_t::addToSerialBuffer(char c)
{
  _mainSerial.addToSerialBuffer(c);
#if USES_ESPEASY_CONSOLE_FALLBACK_PORT

  _fallbackSerial.addToSerialBuffer(c);
#endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  process_serialWriteBuffer();
}

void EspEasy_Console_t::addToSerialBuffer(const String& line) {
  process_serialWriteBuffer();

  _mainSerial.addToSerialBuffer(line);
#if USES_ESPEASY_CONSOLE_FALLBACK_PORT

  _fallbackSerial.addToSerialBuffer(line);
#endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  process_serialWriteBuffer();
}

void EspEasy_Console_t::addNewlineToSerialBuffer() {
  _mainSerial.addNewlineToSerialBuffer();
#if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  _fallbackSerial.addNewlineToSerialBuffer();
#endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  process_serialWriteBuffer();
}

bool EspEasy_Console_t::process_serialWriteBuffer() {
  START_TIMER;
  bool res = false;

  if (_mainSerial.process_serialWriteBuffer()) {
    res = true;
  }

#if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  if (_fallbackSerial.process_serialWriteBuffer()) {
    res = true;
  }
#endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT

  STOP_TIMER(CONSOLE_WRITE_SERIAL);
  return res;
}

void EspEasy_Console_t::setDebugOutput(bool enable)
{
  auto port = getPort();

  if (port == nullptr) {
    port->setDebugOutput(enable);
  }
}

String EspEasy_Console_t::getPortDescription() const
{
  return _mainSerial.getPortDescription();
}

#if USES_ESPEASY_CONSOLE_FALLBACK_PORT
String EspEasy_Console_t::getFallbackPortDescription() const
{
  return _fallbackSerial.getPortDescription();
}

#endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT

bool EspEasy_Console_t::handledByPluginSerialIn()
{
#if USES_ESPEASY_CONSOLE_FALLBACK_PORT

  if ((_fallbackSerial._serial != nullptr) &&
      _fallbackSerial._serial->available())
  {
    String dummy;

    return PluginCall(PLUGIN_SERIAL_IN, 0, dummy);
  }
#endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  if ((_mainSerial._serial != nullptr) && _mainSerial._serial->available() &&
      (static_cast<ESPEasySerialPort>(_console_serial_port) == ESPEasySerialPort::serial0
# ifdef ESP8266
       || static_cast<ESPEasySerialPort>(_console_serial_port) == ESPEasySerialPort::serial0_swap
# endif // ifdef ESP8266
      ))
#endif  // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  {
    String dummy;

    return PluginCall(PLUGIN_SERIAL_IN, 0, dummy);
  }
  return false;
}

void EspEasy_Console_t::readInput(EspEasy_Console_Port& port)
{
  if (port._serial == nullptr) return;
  while (port._serial->available())
  {
    delay(0);
    const uint8_t SerialInByte = port._serial->read();

    #if FEATURE_IMPROV
    const bool processingImprov = port._improv.handle(SerialInByte, port._serial);
    #else // if FEATURE_IMPROV
    const bool processingImprov = false;
    #endif // if FEATURE_IMPROV

    // FIXME TD-er: Must check if IMPROV 'ate' some chars and then feed them to the console input if last char was not an IMPROV char.
    if (!processingImprov) {
      if (port.process_consoleInput(SerialInByte))
        return;
    }
  }
}

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
ESPeasySerial * EspEasy_Console_t::getPort()
{
  if (_mainSerial._serial != nullptr) {
    return _mainSerial._serial;
  }
# if USES_ESPEASY_CONSOLE_FALLBACK_PORT

  if (_fallbackSerial._serial != nullptr) {
    return _fallbackSerial._serial;
  }
# endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  return nullptr;
}

#else // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
HardwareSerial * EspEasy_Console_t::getPort()
{
  if (_mainSerial._serial != nullptr) {
    return _mainSerial._serial;
  }
  return nullptr;
}

#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT


void EspEasy_Console_t::endPort()
{
  _mainSerial.endPort();
#if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  _fallbackSerial.endPort();
#endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  delay(10);
}

int EspEasy_Console_t::availableForWrite()
{
  auto serial = getPort();

  if (serial != nullptr) {
    return serial->availableForWrite();
  }
  return 0;
}
