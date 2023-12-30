#ifndef DATASTRUCTS_FACTORYDEFAULT_LOG_CONSOLE_SETTINGS_NVS_H
#define DATASTRUCTS_FACTORYDEFAULT_LOG_CONSOLE_SETTINGS_NVS_H

#include "../../ESPEasy_common.h"

#ifdef ESP32

# include "../Helpers/ESPEasy_NVS_Helper.h"

class FactoryDefault_LogConsoleSettings_NVS {
public:

  bool applyToSettings_from_NVS(ESPEasy_NVS_Helper& preferences);

  void fromSettings_to_NVS(ESPEasy_NVS_Helper& preferences);

  void clear_from_NVS(ESPEasy_NVS_Helper& preferences);

private:

  union {
    struct {
      uint64_t SyslogLevel    : 3;
      uint64_t SerialLogLevel : 3;
      uint64_t WebLogLevel    : 3;
      uint64_t SDLogLevel     : 3;
      uint64_t SyslogFacility : 4;
      uint64_t SyslogPort     : 16;
      uint8_t Syslog_IP[4];
    } LogSettings_bits;

    uint64_t LogSettings{};
  };

  union {
    struct {
      uint8_t  console_serial_port : 7;
      uint8_t  UseSerial           : 1;
      int8_t   console_serial_rxpin;
      int8_t   console_serial_txpin;
      uint8_t  console_serial0_fallback;
      uint32_t BaudRate;
    } ConsoleSettings_bits;

    uint64_t ConsoleSettings{};
  };
};

#endif // ifdef ESP32
#endif // ifndef DATASTRUCTS_FACTORYDEFAULT_LOG_CONSOLE_SETTINGS_NVS_H
