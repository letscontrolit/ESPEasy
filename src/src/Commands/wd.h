#ifndef COMMAND_WD_H
#define COMMAND_WD_H

#include "../Commands/Common.h"

#ifndef LIMIT_BUILD_SIZE

class String;
struct EventStruct;

String Command_WD_Config(EventStruct *event, const char* Line);
String Command_WD_Read(EventStruct *event, const char* Line);

#endif

#endif // COMMAND_WD_H
