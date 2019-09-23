#include "../Globals/Logging.h"

#include "../DataStructs/LogStruct.h"


LogStruct Logging;

uint8_t highest_active_log_level = 0;
bool log_to_serial_disabled = false;

std::deque<char> serialWriteBuffer;