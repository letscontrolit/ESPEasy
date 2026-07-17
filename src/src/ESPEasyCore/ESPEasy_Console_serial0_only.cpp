#include "../ESPEasyCore/ESPEasy_Console.h"

#if !FEATURE_DEFINE_SERIAL_CONSOLE_PORT


# include "../Commands/ExecuteCommand.h"

# include "../DataStructs/TimingStats.h"

# include "../DataTypes/ESPEasy_plugin_functions.h"

# include "../Globals/Cache.h"
# include "../Globals/Logging.h"
# include "../Globals/Plugins.h"
# include "../Globals/Settings.h"

# include "../Helpers/Memory.h"

# include <ESPEasySerialPort.h>

# ifdef ESP32
#  include <esp32-hal-periman.h>
# endif


EspEasy_Console_t::EspEasy_Console_t() :
  _mainSerial(LOG_TO_SERIAL)
{}

void EspEasy_Console_t::reInit()
{
  updateActiveTaskUseSerial0();
  bool somethingChanged = false;

  if (somethingChanged) {
    begin(Settings.BaudRate);
  }
}

void EspEasy_Console_t::begin(uint32_t baudrate)
{
  updateActiveTaskUseSerial0();
  _baudrate = baudrate;

  if (_mainSerial._serial != nullptr) {
# ifdef ESP8266
    _mainSerial._serial->begin(baudrate);
#  ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_INFO, F("ESPEasy console using HW Serial"));
#  endif
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
  }
}

void EspEasy_Console_t::init() {
# if FEATURE_IMPROV
  _mainSerial._improv.init();
# endif // if FEATURE_IMPROV
  updateActiveTaskUseSerial0();

  if (!Settings.UseSerial) {
    return;
  }

  if (activeTaskUseSerial0() || log_to_serial_disabled) {
    return;
  }

  begin(Settings.BaudRate);
}

void EspEasy_Console_t::loop()
{
  if (!Settings.UseSerial) { return; }

  START_TIMER;

  if (handledByPluginSerialIn()) {
    return;
  }

  readInput(_mainSerial);

  STOP_TIMER(CONSOLE_LOOP);
}

bool EspEasy_Console_t::process_serialWriteBuffer() {
  START_TIMER;
  bool res = false;

  if (_mainSerial.process_serialWriteBuffer()) {
    res = true;
  }

# if FEATURE_TIMING_STATS

  if (res) { STOP_TIMER(CONSOLE_WRITE_SERIAL); }
# endif // if FEATURE_TIMING_STATS
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

bool EspEasy_Console_t::handledByPluginSerialIn()
{
  String dummy;

  return PluginCall(PLUGIN_SERIAL_IN, 0, dummy);
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

HardwareSerial * EspEasy_Console_t::getPort()
{
  if (_mainSerial._serial != nullptr) {
    return _mainSerial._serial;
  }
  return nullptr;
}

void EspEasy_Console_t::endPort()
{
  _mainSerial.endPort();
  delay(10);
}

#endif // if !FEATURE_DEFINE_SERIAL_CONSOLE_PORT
