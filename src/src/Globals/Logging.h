#ifndef GLOBALS_LOGGING_H
#define GLOBALS_LOGGING_H

#include <stdint.h>
#include <deque>

#include "../Helpers/Log_Helper.h"
#if FEATURE_SYSLOG
#include "../Helpers/SyslogWriter.h"
#endif



extern uint8_t highest_active_log_level;
extern bool log_to_serial_disabled;

extern LogHelper Logging;
#if FEATURE_SYSLOG
extern SyslogWriter syslogWriter;
#endif

#endif // GLOBALS_LOGGING_H