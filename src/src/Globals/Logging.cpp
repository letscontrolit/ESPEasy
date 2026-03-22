#include "../Globals/Logging.h"


LogHelper Logging;
#if FEATURE_SYSLOG
SyslogWriter syslogWriter(LOG_TO_SYSLOG);
#endif
uint8_t highest_active_log_level = 0;
bool log_to_serial_disabled = false;

