#include "src/Globals/Logging.h"

#include "src/DataStructs/LogStruct.h"


LogStruct Logging;


uint8_t highest_active_log_level(0);
bool log_to_serial_disabled(false);
