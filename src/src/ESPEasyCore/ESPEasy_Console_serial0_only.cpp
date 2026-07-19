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

  // TODO TD-er: Must check whether a task uses serial 0 and then stop serial console
  // activeTaskUseSerial0()
  bool somethingChanged = false;

  if (somethingChanged) {
    begin(Settings.BaudRate);
  }
}

void EspEasy_Console_t::begin(uint32_t baudrate)
{
  _mainSerial.begin(baudrate);
# ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_INFO, F("ESPEasy console enabled"));
# endif
}

void EspEasy_Console_t::init() {
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

  _mainSerial.readInput();

  STOP_TIMER(CONSOLE_LOOP);
}

bool EspEasy_Console_t::process_serialWriteBuffer() {

# if FEATURE_TIMING_STATS
  START_TIMER;
  bool res = false;

  if (_mainSerial.process_serialWriteBuffer()) {
    res = true;
  }

  if (res) { STOP_TIMER(CONSOLE_WRITE_SERIAL); }
  return res;

# else // if FEATURE_TIMING_STATS
  return _mainSerial.process_serialWriteBuffer();
# endif // if FEATURE_TIMING_STATS
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

HardwareSerial * EspEasy_Console_t::getPort() { return _mainSerial._serial; }

void             EspEasy_Console_t::endPort()
{
  _mainSerial.endPort();
  delay(10);
}

#endif // if !FEATURE_DEFINE_SERIAL_CONSOLE_PORT
