#include "../ESPEasyCore/ESPEasy_Console.h"

#ifdef ESP32

// Only ESP32 does allow for multiple serial ports

# if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
#  if USES_ESPEASY_CONSOLE_FALLBACK_PORT

#   include "../Commands/ExecuteCommand.h"

#   include "../DataStructs/TimingStats.h"

#   include "../DataTypes/ESPEasy_plugin_functions.h"

#   include "../Globals/Cache.h"
#   include "../Globals/Logging.h"
#   include "../Globals/Plugins.h"
#   include "../Globals/Settings.h"

#   include "../Helpers/Memory.h"

#   include <ESPEasySerialPort.h>

#   ifdef ESP32
#    include <esp32-hal-periman.h>
#   endif


EspEasy_Console_t::EspEasy_Console_t() :
  _mainSerial(LOG_TO_SERIAL)
  , _fallbackSerial(LOG_TO_SERIAL_EXTRA)
{
  const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(_console_serial_port);

#   if USES_USBCDC

  /*
     if (port == ESPEasySerialPort::usb_cdc_0 ||
        port == ESPEasySerialPort::usb_cdc_1)
     {
      USB.manufacturerName(F("ESPEasy"));
      USB.productName()
     }
   */
#   endif // if USES_USBCDC


  // Ideal buffer size is a trade-off between bootspeed
  // and not missing data when the ESP is busy processing stuff.
  // Since we do have a separate buffer in the console,
  // it may just take less time in the background tasks to dump
  // any logs as larger chunks can be transferred at once.
  constexpr size_t buffSize = 512;

  ESPEasySerialConfig config;
  config.port          = port;
  config.baud          = DEFAULT_SERIAL_BAUD;
  config.receivePin    = _console_serial_rxpin;
  config.transmitPin   = _console_serial_txpin;
  config.inverse_logic = false;
  config.rxBuffSize    = 256;
  config.txBuffSize    = buffSize;

  _mainSerial._serial = new (std::nothrow) ESPeasySerial(config);

  if (Settings.console_serial0_fallback && (port != ESPEasySerialPort::serial0)) {
    config.port        = ESPEasySerialPort::serial0;
    config.receivePin  = SOC_RX0;
    config.transmitPin = SOC_TX0;

    _fallbackSerial._serial = new (std::nothrow) ESPeasySerial(config);
  }
}

void EspEasy_Console_t::reInit()
{
  updateActiveTaskUseSerial0();
  bool somethingChanged        = false;
  const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(Settings.console_serial_port);

  const bool consoleUseSerial0 = port == ESPEasySerialPort::serial0;

  const bool canUseSerial0 = !activeTaskUseSerial0() && !log_to_serial_disabled;


  bool mustHaveSerial   = Settings.UseSerial && (!consoleUseSerial0 || canUseSerial0);
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
      delete _fallbackSerial._serial;
      _fallbackSerial._serial = nullptr;
      somethingChanged        = true;
    }
  }

  if ((_console_serial_port != Settings.console_serial_port) ||
      (_console_serial_rxpin != Settings.console_serial_rxpin) ||
      (_console_serial_txpin != Settings.console_serial_txpin) ||
      !mustHaveSerial) {
    if (_mainSerial._serial != nullptr) {
      delete _mainSerial._serial;
      _mainSerial._serial = nullptr;
      somethingChanged    = true;
    }

    _console_serial_port  = Settings.console_serial_port;
    _console_serial_rxpin = Settings.console_serial_rxpin;
    _console_serial_txpin = Settings.console_serial_txpin;
  }

  if ((_mainSerial._serial == nullptr) && mustHaveSerial) {
    unsigned int buffsize = 128;

    const ESPEasySerialPort mainSerialPort = static_cast<ESPEasySerialPort>(_console_serial_port);

#   if USES_HWCDC

    if (mainSerialPort == ESPEasySerialPort::usb_hw_cdc) {
      buffsize = 2048;
    }
#   endif // if USES_HWCDC

    _mainSerial._serial = new (std::nothrow) ESPeasySerial(
      mainSerialPort,
      _console_serial_rxpin,
      _console_serial_txpin,
      false,
      buffsize);
    somethingChanged = true;
  }

  if ((_fallbackSerial._serial == nullptr) && mustHaveFallback) {
    _fallbackSerial._serial = new (std::nothrow) ESPeasySerial(
      ESPEasySerialPort::serial0,
      SOC_RX0,
      SOC_TX0);
    somethingChanged = true;
  }

  if (_fallbackSerial._serial == nullptr) {
    _fallbackSerial._serialWriteBuffer.clear();
  }

  if (_mainSerial._serial == nullptr) {
    _mainSerial._serialWriteBuffer.clear();
  }

  if (somethingChanged) {
    begin(Settings.BaudRate);
  }
}

void EspEasy_Console_t::begin(uint32_t baudrate)
{
  updateActiveTaskUseSerial0();
  _baudrate = baudrate;

  if (_mainSerial._serial != nullptr) {
    _mainSerial._serial->begin(baudrate);
    addLog(LOG_LEVEL_INFO, F("ESPEasy console using ESPEasySerial"));
  }

  if (_fallbackSerial._serial != nullptr) {
    _fallbackSerial._serial->begin(baudrate);

    // Need to have this string as C-string, not F-string
    perimanSetPinBusExtraType(SOC_RX0, "Console");
    perimanSetPinBusExtraType(SOC_TX0, "Console");

    addLog(LOG_LEVEL_INFO, F("ESPEasy console fallback enabled"));
  }
}

void EspEasy_Console_t::init() {
#   if FEATURE_IMPROV
  _mainSerial._improv.init();
  _fallbackSerial._improv.init();
#   endif // if FEATURE_IMPROV
  updateActiveTaskUseSerial0();

  if (!Settings.UseSerial) {
    return;
  }

  begin(Settings.BaudRate);
}

void EspEasy_Console_t::loop()
{
  if (!Settings.UseSerial) { return; }

  START_TIMER;

  const bool consoleUsesSerial0 =
    (static_cast<ESPEasySerialPort>(_console_serial_port) == ESPEasySerialPort::serial0
    );

  if (handledByPluginSerialIn())
  {
    // Any serial0 data is already dealt with
    if (!consoleUsesSerial0 && (_mainSerial._serial != nullptr)) {
      readInput(_mainSerial);
    }
    return;
  }

  readInput(_mainSerial);
  readInput(_fallbackSerial);

  STOP_TIMER(CONSOLE_LOOP);
}

bool EspEasy_Console_t::process_serialWriteBuffer() {
  START_TIMER;
  bool res = false;

  if (_mainSerial.process_serialWriteBuffer()) {
    res = true;
  }

  if (_fallbackSerial.process_serialWriteBuffer()) {
    res = true;
  }
#   if FEATURE_TIMING_STATS

  if (res) { STOP_TIMER(CONSOLE_WRITE_SERIAL); }
#   endif // if FEATURE_TIMING_STATS
  return res;
}

void EspEasy_Console_t::setDebugOutput(bool enable)
{
  auto port = getPort();

  if (port != nullptr) {
    port->setDebugOutput(enable);
  }
}

String EspEasy_Console_t::getPortDescription() const
{
  return _mainSerial.getPortDescription();
}

String EspEasy_Console_t::getFallbackPortDescription() const
{
  return _fallbackSerial.getPortDescription();
}

bool EspEasy_Console_t::handledByPluginSerialIn()
{
  if ((_fallbackSerial._serial != nullptr) &&
      _fallbackSerial._serial->available())
  {
    String dummy;

    return PluginCall(PLUGIN_SERIAL_IN, 0, dummy);
  }

  if ((_mainSerial._serial != nullptr) && _mainSerial._serial->available() &&
      (static_cast<ESPEasySerialPort>(_console_serial_port) == ESPEasySerialPort::serial0
      ))
  {
    String dummy;

    return PluginCall(PLUGIN_SERIAL_IN, 0, dummy);
  }
  return false;
}

void EspEasy_Console_t::readInput(EspEasy_Console_Port& port)
{
  size_t bytesToRead = port.available();

  while (bytesToRead > 0)
  {
    --bytesToRead;
    delay(0);
    const int SerialInByte = port.read();

    if (SerialInByte >= 0) {
      if (port.process_consoleInput(SerialInByte)) {
        // Processed a full line
        return;
      }
    }
  }
}

ESPeasySerial * EspEasy_Console_t::getPort()
{
  if (_mainSerial._serial != nullptr) {
    return _mainSerial._serial;
  }

  if (_fallbackSerial._serial != nullptr) {
    return _fallbackSerial._serial;
  }
  return nullptr;
}

void EspEasy_Console_t::endPort()
{
  _mainSerial.endPort();
  _fallbackSerial.endPort();
  delay(10);
}

#  endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
# endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
#endif // ifdef ESP32
