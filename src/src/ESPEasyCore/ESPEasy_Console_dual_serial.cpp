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


EspEasy_Console_t::EspEasy_Console_t() :
  _mainSerial(LOG_TO_SERIAL)
  , _fallbackSerial(LOG_TO_SERIAL_EXTRA)
{
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

  ESPEasySerialConfig config;
  config.port          = static_cast<ESPEasySerialPort>(DEFAULT_CONSOLE_PORT);
  config.baud          = DEFAULT_SERIAL_BAUD;
  config.receivePin    = DEFAULT_CONSOLE_PORT_RXPIN;
  config.transmitPin   = DEFAULT_CONSOLE_PORT_TXPIN;
  config.inverse_logic = false;
  config.rxBuffSize    = 256;
  config.txBuffSize    = ESPEASY_CONSOLE_TX_BUFFSIZE;

  if (config.port != ESPEasySerialPort::serial0) {
    // We only use the _mainSerial for ports other than HW Serial0
    _mainSerial.updateSerialPort(config);
  }

  if (Settings.console_serial0_fallback ||
      (config.port == ESPEasySerialPort::serial0))
  {
    config.port        = ESPEasySerialPort::serial0;
    config.receivePin  = SOC_RX0;
    config.transmitPin = SOC_TX0;
    config.txBuffSize  = ESPEASY_CONSOLE_TX_BUFFSIZE;
    _fallbackSerial.updateSerialPort(config);
  }
}

void EspEasy_Console_t::reInit()
{
  ESPEasySerialConfig config;

  config.port          = static_cast<ESPEasySerialPort>(Settings.console_serial_port);
  config.baud          = Settings.BaudRate;
  config.receivePin    = Settings.console_serial_rxpin;
  config.transmitPin   = Settings.console_serial_txpin;
  config.inverse_logic = false;
  config.rxBuffSize    = 256;
  config.txBuffSize    = ESPEASY_CONSOLE_TX_BUFFSIZE;

  if (config.port != ESPEasySerialPort::serial0) {
    // We only use the _mainSerial for ports other than HW Serial0
    _mainSerial.updateSerialPort(config);
  }

  if (Settings.console_serial0_fallback ||
      (config.port == ESPEasySerialPort::serial0))
  {
    config.port        = ESPEasySerialPort::serial0;
    config.receivePin  = SOC_RX0;
    config.transmitPin = SOC_TX0;
    config.txBuffSize  = ESPEASY_CONSOLE_TX_BUFFSIZE;
    _fallbackSerial.updateSerialPort(config);
  }
}

void EspEasy_Console_t::begin(uint32_t baudrate)
{
  _mainSerial.begin(baudrate);
  _fallbackSerial.begin(baudrate);

#   ifndef BUILD_NO_DEBUG

  if (_mainSerial._serial != nullptr) {
    addLog(LOG_LEVEL_INFO, F("ESPEasy console enabled"));
  }

  if (_fallbackSerial._serial != nullptr) {
    addLog(LOG_LEVEL_INFO,
           (_mainSerial._serial != nullptr)
      ? F("ESPEasy console fallback enabled")
      : F("ESPEasy console enabled"));
  }
#   endif // ifndef BUILD_NO_DEBUG
}

void EspEasy_Console_t::init() {
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

  _mainSerial.readInput();
  _fallbackSerial.readInput();

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
  if (_mainSerial._serial != nullptr) {
    _mainSerial._serial->setDebugOutput(enable);
  }

  if (_fallbackSerial._serial != nullptr) {
    _fallbackSerial._serial->setDebugOutput(enable);
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
