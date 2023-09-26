#ifndef COMMAND_COMMON_H
#define COMMAND_COMMON_H

#include <ctype.h>

#include "../../ESPEasy_common.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../DataTypes/EventValueSource.h"


#include <IPAddress.h>

// Simple function to return "Ok", to avoid flash string duplication in the firmware.
const __FlashStringHelper * return_command_success_flashstr();
const __FlashStringHelper * return_command_failed_flashstr();

const __FlashStringHelper * return_command_boolean_result_flashstr(bool success);

String return_command_success();
String return_command_failed();

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
