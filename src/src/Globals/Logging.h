#ifndef GLOBALS_LOGGING_H
#define GLOBALS_LOGGING_H

#include <stdint.h>
#include <deque>

extern uint8_t highest_active_log_level;
extern bool log_to_serial_disabled;

struct LogStruct;
extern LogStruct Logging;

/*********************************************************************************************\
 * Buffer for outputting logs via serial port.
\*********************************************************************************************/
extern std::deque<char> serialWriteBuffer;


#endif // GLOBALS_LOGGING_H