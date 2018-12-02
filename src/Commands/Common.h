#ifndef COMMAND_COMMON_H
#define COMMAND_COMMON_H

#include <ctype.h>
#include <Arduino.h>

bool IsNumeric(const char *source)
{
  bool result = false;

  if (source) {
    int len = strlen(source);

    if (len != 0) {
      int i;

      for (i = 0; i < len && isdigit(source[i]); i++) {}
      result = i == len;
    }
  }
  return result;
}

String Command_GetORSetIP(struct EventStruct        *event,
                          const __FlashStringHelper *targetDescription,
                          const char                *Line,
                          byte                      *IP,
                          IPAddress                  dhcpIP,
                          int                        arg)
{
  bool hasArgument = false;
  {
    // Check if command is valid. Leave in separate scope to delete the TmpStr1
    String TmpStr1;

    if (GetArgv(Line, TmpStr1, arg + 1)) {
      hasArgument = true;

      if (!str2ip(TmpStr1.c_str(), IP)) {
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

String Command_GetORSetString(struct EventStruct        *event,
                              const __FlashStringHelper *targetDescription,
                              const char                *Line,
                              char                      *target,
                              size_t                     len,
                              int                        arg
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
      } else {
        strcpy(target, TmpStr1.c_str());
      }
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

String Command_GetORSetBool(struct EventStruct        *event,
                            const __FlashStringHelper *targetDescription,
                            const char                *Line,
                            bool                      *value,
                            int                        arg)
{
  bool hasArgument = false;
  {
    // Check if command is valid. Leave in separate scope to delete the TmpStr1
    String TmpStr1;

    if (GetArgv(Line, TmpStr1, arg + 1)) {
      hasArgument = true;
      TmpStr1.toLowerCase();

      if (IsNumeric(TmpStr1.c_str())) {
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
    result += toString(*value);
    return return_result(event, result);
  }
  return return_command_success();
}

#endif // COMMAND_COMMON_H
