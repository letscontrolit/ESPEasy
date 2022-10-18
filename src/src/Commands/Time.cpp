#include "../Commands/Time.h"


#include "../../ESPEasy_common.h"


#include "../Commands/Common.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../ESPEasyCore/Serial.h"

#include "../Globals/ESPEasy_time.h"
#include "../Globals/Settings.h"

#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/StringConverter.h"


String Command_NTPHost(struct EventStruct *event, const char *Line)
{
  return Command_GetORSetString(event, F("NTPHost:"),
                                Line,
                                Settings.NTPHost,
                                sizeof(Settings.NTPHost),
                                1);
}

String Command_useNTP(struct EventStruct *event, const char *Line)
{
  if (HasArgv(Line, 2)) {
    Settings.UseNTP(event->Par1);
  } else {
    return return_result(event, concat(F("UseNTP:"), boolToString(Settings.UseNTP())));
  }
  return return_command_success_str();
}

String Command_TimeZone(struct EventStruct *event, const char *Line)
{
  if (HasArgv(Line, 2)) {
    Settings.TimeZone = event->Par1;
  } else {
    return return_result(event, concat(F("TimeZone:"), static_cast<int>(Settings.TimeZone)));
  }
  return return_command_success_str();
}

String Command_DST(struct EventStruct *event, const char *Line)
{
  if (HasArgv(Line, 2)) {
    Settings.DST = event->Par1;
  } else  {
    return return_result(event, concat(F("DST:"),  boolToString(Settings.DST)));
  }
  return return_command_success_str();
}

String Command_DateTime(struct EventStruct *event, const char *Line)
{
  String TmpStr1;

  if (GetArgv(Line, TmpStr1, 2)) {
    struct tm newtime;
    int yr = 1970;
    int mnth = 1;
    int d = 1;
    sscanf(TmpStr1.c_str(), "%4d-%2d-%2d", &yr, &mnth, &d);
    newtime.tm_year = yr - 1900;
    newtime.tm_mon  = mnth - 1; // tm_mon starts at 0
    newtime.tm_mday = d;

    int h = 0;
    int m = 0;
    int s = 0;

    if (GetArgv(Line, TmpStr1, 3)) {
      sscanf(TmpStr1.c_str(), "%2d:%2d:%2d", &h, &m, &s);
    }
    newtime.tm_hour = h;
    newtime.tm_min  = m;
    newtime.tm_sec  = s;

    // Please note the time set in this command is in UTC time, not local time.
    node_time.setExternalTimeSource(makeTime(newtime), timeSource_t::Manual_set);
  } else  {
    // serialPrintln();
    return return_result(event, concat(F("Datetime:"), node_time.getDateTimeString('-', ':', ' ')));
  }
  return return_command_success_str();
}