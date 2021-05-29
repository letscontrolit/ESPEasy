#ifndef COMMAND_COMMON_H
#define COMMAND_COMMON_H

#include <ctype.h>
#include <Arduino.h>

#include "../../ESPEasy_common.h"

class IPAddress;

const __FlashStringHelper * return_command_success();
const __FlashStringHelper * return_command_failed();
const __FlashStringHelper * return_incorrect_nr_arguments();
const __FlashStringHelper * return_incorrect_source();
const __FlashStringHelper * return_not_connected();
String return_result(struct EventStruct *event,
                     const String      & result);
const __FlashStringHelper * return_see_serial(struct EventStruct *event);

bool   IsNumeric(const char *source);

String Command_GetORSetIP(struct EventStruct *event,
                          const String      & targetDescription,
                          const char         *Line,
                          byte               *IP,
                          const IPAddress&    dhcpIP,
                          int                 arg);

String Command_GetORSetString(struct EventStruct *event,
                              const String      & targetDescription,
                              const char         *Line,
                              char               *target,
                              size_t              len,
                              int                 arg
                              );

String Command_GetORSetBool(struct EventStruct *event,
                            const String      & targetDescription,
                            const char         *Line,
                            bool               *value,
                            int                 arg);

String Command_GetORSetUint8_t(struct EventStruct *event,
                            const String      & targetDescription,
                            const char         *Line,
                            uint8_t            *value,
                            int                 arg);

String Command_GetORSetInt8_t(struct EventStruct *event,
                            const String      & targetDescription,
                            const char         *Line,
                            int8_t             *value,
                            int                 arg);

#endif // COMMAND_COMMON_H
