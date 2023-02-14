#include "../Commands/Common.h"

#include <ctype.h>
#include <IPAddress.h>

#include "../../ESPEasy_common.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../DataTypes/EventValueSource.h"

#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../ESPEasyCore/Serial.h"

#include "../Helpers/Networking.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/StringConverter.h"


// Simple function to return "Ok", to avoid flash string duplication in the firmware.
const __FlashStringHelper * return_command_success()
{
  return F("\nOK");
}

const __FlashStringHelper * return_command_failed()
{
  return F("\nFailed");
}

String return_command_success_str()
{
  return return_command_success();
}

String return_command_failed_str()
{
  return return_command_failed();
}

const __FlashStringHelper * return_incorrect_nr_arguments()
{
  return F("Too many arguments, try using quotes!");
}

const __FlashStringHelper * return_incorrect_source()
{
  return F("Command not allowed from this source!");
}

const __FlashStringHelper * return_not_connected()
{
  return F("Not connected to WiFi");
}

String return_result(struct EventStruct *event, const String& result)
{
  serialPrintln();
  serialPrintln(result);

  if (event->Source == EventValueSource::Enum::VALUE_SOURCE_SERIAL) {
    return return_command_success_str();
  }
  return result;
}

const __FlashStringHelper * return_see_serial(struct EventStruct *event)
{
  if (event->Source == EventValueSource::Enum::VALUE_SOURCE_SERIAL) {
    return return_command_success();
  }
  return F("Output sent to serial");
}

String Command_GetORSetIP(struct EventStruct *event,
                          const __FlashStringHelper * targetDescription,
                          const char         *Line,
                          uint8_t               *IP,
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
        return return_result(event, concat(F("Invalid parameter: "), TmpStr1));
      }
    }
  }

  if (!hasArgument) {
    String result = targetDescription;

    if (useStaticIP()) {
      result += formatIP(IP);
    } else {
      result += formatIP(dhcpIP);
      result += F("(DHCP)");
    }
    return return_result(event, result);
  }
  return return_command_success_str();
}

String Command_GetORSetString(struct EventStruct *event,
                              const __FlashStringHelper * targetDescription,
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
        String result = concat(targetDescription, F(" is too large. max size is "));
        result += len;
        return return_result(event, result);
      }
      safe_strncpy(target, TmpStr1, len);
    }
  }

  if (hasArgument) {
    String result = targetDescription;
    result += target;
    return return_result(event, result);
  }
  return return_command_success_str();
}

String Command_GetORSetBool(struct EventStruct *event,
                            const __FlashStringHelper * targetDescription,
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

      int tmp_int = 0;
      if (validIntFromString(TmpStr1, tmp_int)) {
        *value = tmp_int > 0;
      }
      else if (strcmp_P(PSTR("on"), TmpStr1.c_str()) == 0) { *value = true; }
      else if (strcmp_P(PSTR("true"), TmpStr1.c_str()) == 0) { *value = true; }
      else if (strcmp_P(PSTR("off"), TmpStr1.c_str()) == 0) { *value = false; }
      else if (strcmp_P(PSTR("false"), TmpStr1.c_str()) == 0) { *value = false; }
    }
  }

  if (hasArgument) {
    return return_result(event, concat(targetDescription, boolToString(*value)));
  }
  return return_command_success_str();
}

#if FEATURE_ETHERNET
String Command_GetORSetETH(struct EventStruct *event,
                            const __FlashStringHelper * targetDescription,
                            const __FlashStringHelper * valueToString,
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

      int tmp_int = 0;
      if (validIntFromString(TmpStr1, tmp_int)) {
        *value = static_cast<uint8_t>(tmp_int);
      }

      // FIXME TD-er: This should not be in a generic function, but rather pre-processed in the command itself


      // WiFi/Eth mode
      else if (equals(TmpStr1, F("wifi"))) { *value = 0; }
      else if (equals(TmpStr1, F("ethernet"))) { *value = 1; }

      // ETH clockMode
      else if (TmpStr1.startsWith(F("ext"))) { *value = 0; }
      else if (TmpStr1.indexOf(F("gpio0"))  != -1) { *value = 1; }
      else if (TmpStr1.indexOf(F("gpio16")) != -1) { *value = 2; }
      else if (TmpStr1.indexOf(F("gpio17")) != -1) { *value = 3; }
    }
  }

  String result = targetDescription;
  if (hasArgument) {
    result += *value;
  } else {
    result += valueToString;
  }
  return return_result(event, result);
}
#endif

String Command_GetORSetInt8_t(struct EventStruct *event,
                            const __FlashStringHelper * targetDescription,
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

      int tmp_int = 0;
      if (validIntFromString(TmpStr1, tmp_int)) {
        *value = static_cast<int8_t>(tmp_int);
      }
    }
  }

  if (hasArgument) {
    String result = targetDescription;
    result += *value;
    return return_result(event, result);
  }
  return return_command_success_str();
}
