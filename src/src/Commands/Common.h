#ifndef COMMAND_COMMON_H
#define COMMAND_COMMON_H

#include <ctype.h>
#include <Arduino.h>

#include "../../ESPEasy_common.h"

#include <IPAddress.h>

const __FlashStringHelper * return_command_success();
const __FlashStringHelper * return_command_failed();
String return_command_success_str();
String return_command_failed_str();
const __FlashStringHelper * return_incorrect_nr_arguments();
const __FlashStringHelper * return_incorrect_source();
const __FlashStringHelper * return_not_connected();
String return_result(struct EventStruct *event,
                     const String      & result);
const __FlashStringHelper * return_see_serial(struct EventStruct *event);

String Command_GetORSetIP(struct EventStruct *event,
                          const __FlashStringHelper * targetDescription,
                          const char         *Line,
                          uint8_t               *IP,
                          const IPAddress&    dhcpIP,
                          int                 arg);

String Command_GetORSetString(struct EventStruct *event,
                              const __FlashStringHelper * targetDescription,
                              const char         *Line,
                              char               *target,
                              size_t              len,
                              int                 arg
                              );

String Command_GetORSetBool(struct EventStruct *event,
                            const __FlashStringHelper * targetDescription,
                            const char         *Line,
                            bool               *value,
                            int                 arg);

#if FEATURE_ETHERNET
String Command_GetORSetETH(struct EventStruct *event,
                            const __FlashStringHelper * targetDescription,
                            const __FlashStringHelper * valueToString,
                            const char         *Line,
                            uint8_t            *value,
                            int                 arg);
#endif

String Command_GetORSetInt8_t(struct EventStruct *event,
                            const __FlashStringHelper * targetDescription,
                            const char         *Line,
                            int8_t             *value,
                            int                 arg);

#endif // COMMAND_COMMON_H
