#ifndef COMMANDS_ONE_WIRE_H
#include "../Helpers/Dallas1WireHelper.h"

#if FEATURE_DALLAS_HELPER && FEATURE_COMMAND_OWSCAN
String Command_OneWire_Owscan(struct EventStruct *event,
                              const char         *Line);

#endif // if FEATURE_DALLAS_HELPER && FEATURE_COMMAND_OWSCAN

#endif // ifndef COMMANDS_ONE_WIRE_H
