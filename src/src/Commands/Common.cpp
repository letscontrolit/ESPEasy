#include "../Commands/Common.h"

#include <ctype.h>
#include <IPAddress.h>

#include "../../ESPEasyWifi.h"
#include "../../ESPEasy_common.h"
#include "../../ESPEasy_fdwdecl.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../DataStructs/EventValueSource.h"

#include "../Helpers/Numerical.h"
#include "../Helpers/StringConverter.h"


// Simple function to return "Ok", to avoid flash string duplication in the firmware.
String return_command_success()
{
  return F("\nOk");
}

String return_command_failed()
{
  return F("\nFailed");
}

String return_incorrect_nr_arguments()
{
  return F("Too many arguments, try using quotes!");
}

String return_not_connected()
{
  return F("Not connected to WiFi");
}

String return_result(struct EventStruct *event, const String& result)
{
  serialPrintln(result);

  if (event->Source == EventValueSource::Enum::VALUE_SOURCE_SERIAL) {
    return return_command_success();
  }
  return result;
}

String return_see_serial(struct EventStruct *event)
{
  if (event->Source == EventValueSource::Enum::VALUE_SOURCE_SERIAL) {
    return return_command_success();
  }
  return F("Output sent to serial");
}

String Command_GetORSetIP(struct EventStruct *event,
                          const String      & targetDescription,
                          const char         *Line,
                          byte               *IP,
                          const IPAddress   & dhcpIP,
                          int                 arg)
{
  bool hasArgument = false;
  {
    // Check if command is valid. Leave in separate scope to delete the TmpStr1
    String TmpStr1;

    if (GetArgv(Line, TmpStr1, arg + 1)) {
      hasArgument = true;

      if (!str2ip(TmpStr1, IP)) {
        String result = F("Invalid parameter: ");
        result += TmpStr1;
        return return_result(event, result);
      }
    }
  }

  if (!hasArgument) {
    serialPrintln();
    String result = targetDescription;

    if (useStaticIP()) {
      result += formatIP(IP);
    } else {
      result += formatIP(dhcpIP);
      result += F("(DHCP)");
    }
    return return_result(event, result);
  }
  return return_command_success();
}

String Command_GetORSetString(struct EventStruct *event,
                              const String      & targetDescription,
                              const char         *Line,
                              char               *target,
                              size_t              len,
                              int                 arg
                              )
{
  bool hasArgument = false;
  {
    // Check if command is valid. Leave in separate scope to delete the TmpStr1
    String TmpStr1;

    if (GetArgv(Line, TmpStr1, arg + 1)) {
      hasArgument = true;

      if (TmpStr1.length() > len) {
        String result = targetDescription;
        result += F(" is too large. max size is ");
        result += len;
        serialPrintln();
        return return_result(event, result);
      }
      strcpy(target, TmpStr1.c_str());
    }
  }

  if (hasArgument) {
    serialPrintln();
    String result = targetDescription;
    result += target;
    return return_result(event, result);
  }
  return return_command_success();
}

String Command_GetORSetBool(struct EventStruct *event,
                            const String      & targetDescription,
                            const char         *Line,
                            bool               *value,
                            int                 arg)
{
  bool hasArgument = false;
  {
    // Check if command is valid. Leave in separate scope to delete the TmpStr1
    String TmpStr1;

    if (GetArgv(Line, TmpStr1, arg + 1)) {
      hasArgument = true;
      TmpStr1.toLowerCase();

      if (isInt(TmpStr1)) {
        *value = atoi(TmpStr1.c_str()) > 0;
      }
      else if (strcmp_P(PSTR("on"), TmpStr1.c_str()) == 0) { *value = true; }
      else if (strcmp_P(PSTR("true"), TmpStr1.c_str()) == 0) { *value = true; }
      else if (strcmp_P(PSTR("off"), TmpStr1.c_str()) == 0) { *value = false; }
      else if (strcmp_P(PSTR("false"), TmpStr1.c_str()) == 0) { *value = false; }
    }
  }

  if (hasArgument) {
    String result = targetDescription;
    result += boolToString(*value);
    return return_result(event, result);
  }
  return return_command_success();
}

String Command_GetORSetUint8_t(struct EventStruct *event,
                            const String      & targetDescription,
                            const char         *Line,
                            uint8_t            *value,
                            int                 arg)
{
  bool hasArgument = false;
  {
    // Check if command is valid. Leave in separate scope to delete the TmpStr1
    String TmpStr1;

    if (GetArgv(Line, TmpStr1, arg + 1)) {
      hasArgument = true;
      TmpStr1.toLowerCase();

      if (isInt(TmpStr1)) {
        *value = (uint8_t)atoi(TmpStr1.c_str());
      }
      else if (strcmp_P(PSTR("WIFI"), TmpStr1.c_str()) == 0) { *value = 0; }
      else if (strcmp_P(PSTR("ETHERNET"), TmpStr1.c_str()) == 0) { *value = 1; }
    }
  }

  if (hasArgument) {
    String result = targetDescription;
    result += *value;
    return return_result(event, result);
  }
  return return_command_success();
}

String Command_GetORSetInt8_t(struct EventStruct *event,
                            const String      & targetDescription,
                            const char         *Line,
                            int8_t             *value,
                            int                 arg)
{
  bool hasArgument = false;
  {
    // Check if command is valid. Leave in separate scope to delete the TmpStr1
    String TmpStr1;

    if (GetArgv(Line, TmpStr1, arg + 1)) {
      hasArgument = true;
      TmpStr1.toLowerCase();

      if (isInt(TmpStr1)) {
        *value = (int8_t)atoi(TmpStr1.c_str());
      }
    }
  }

  if (hasArgument) {
    String result = targetDescription;
    result += *value;
    return return_result(event, result);
  }
  return return_command_success();
}
