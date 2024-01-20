#include "../DataStructs/FactoryDefault_LogConsoleSettings_NVS.h"


#ifdef ESP32

# include "../Globals/Settings.h"
# include "../Helpers/StringConverter.h"

// Max. 15 char keys for ESPEasy Factory Default marked keys
# define FACTORY_DEFAULT_NVS_LOG_SETTINGS_KEY  "Log"
# define FACTORY_DEFAULT_NVS_CONSOLE_SETTINGS_KEY  "Console"


bool FactoryDefault_LogConsoleSettings_NVS::applyToSettings_from_NVS(ESPEasy_NVS_Helper& preferences)
{
  bool updated = false;

  if (preferences.getPreference(F(FACTORY_DEFAULT_NVS_LOG_SETTINGS_KEY), LogSettings)) {
    updated                 = true;
    Settings.SyslogLevel    = LogSettings_bits.SyslogLevel;
    Settings.SerialLogLevel = LogSettings_bits.SerialLogLevel;
    Settings.WebLogLevel    = LogSettings_bits.WebLogLevel;
    Settings.SDLogLevel     = LogSettings_bits.SDLogLevel;
    Settings.SyslogFacility = LogSettings_bits.SyslogFacility;
    Settings.SyslogPort     = LogSettings_bits.SyslogPort;

    for (size_t i = 0; i < 4; ++i) {
      Settings.Syslog_IP[i] = LogSettings_bits.Syslog_IP[i];
    }
  }

  if (preferences.getPreference(F(FACTORY_DEFAULT_NVS_CONSOLE_SETTINGS_KEY), ConsoleSettings)) {
    updated                           = true;
    Settings.console_serial_port      = ConsoleSettings_bits.console_serial_port;
    Settings.UseSerial                = ConsoleSettings_bits.UseSerial;
    Settings.console_serial_rxpin     = ConsoleSettings_bits.console_serial_rxpin;
    Settings.console_serial_txpin     = ConsoleSettings_bits.console_serial_txpin;
    Settings.console_serial0_fallback = ConsoleSettings_bits.console_serial0_fallback;
    Settings.BaudRate                 = ConsoleSettings_bits.BaudRate;
  }
  return updated;
}

void FactoryDefault_LogConsoleSettings_NVS::fromSettings_to_NVS(ESPEasy_NVS_Helper& preferences)
{
  {
    LogSettings_bits.SyslogLevel    = Settings.SyslogLevel;
    LogSettings_bits.SerialLogLevel = Settings.SerialLogLevel;
    LogSettings_bits.WebLogLevel    = Settings.WebLogLevel;
    LogSettings_bits.SDLogLevel     = Settings.SDLogLevel;
    LogSettings_bits.SyslogFacility = Settings.SyslogFacility;
    LogSettings_bits.SyslogPort     = Settings.SyslogPort;

    for (size_t i = 0; i < 4; ++i) {
      LogSettings_bits.Syslog_IP[i] = Settings.Syslog_IP[i];
    }
    preferences.setPreference(F(FACTORY_DEFAULT_NVS_LOG_SETTINGS_KEY), LogSettings);
  }
  {
    ConsoleSettings_bits.console_serial_port      = Settings.console_serial_port;
    ConsoleSettings_bits.UseSerial                = Settings.UseSerial;
    ConsoleSettings_bits.console_serial_rxpin     = Settings.console_serial_rxpin;
    ConsoleSettings_bits.console_serial_txpin     = Settings.console_serial_txpin;
    ConsoleSettings_bits.console_serial0_fallback = Settings.console_serial0_fallback;
    ConsoleSettings_bits.BaudRate                 = Settings.BaudRate;

    preferences.setPreference(F(FACTORY_DEFAULT_NVS_CONSOLE_SETTINGS_KEY), ConsoleSettings);
  }
}

void FactoryDefault_LogConsoleSettings_NVS::clear_from_NVS(ESPEasy_NVS_Helper& preferences)
{
  preferences.remove(F(FACTORY_DEFAULT_NVS_LOG_SETTINGS_KEY));
  preferences.remove(F(FACTORY_DEFAULT_NVS_CONSOLE_SETTINGS_KEY));
}

#endif // ifdef ESP32
