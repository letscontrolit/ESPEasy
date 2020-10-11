#include "../Commands/Time.h"


#include "../../ESPEasy_common.h"


#include "../Commands/Common.h"

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
    Settings.UseNTP = event->Par1;
  } else {
    serialPrintln();
    String result = F("UseNTP:");
    result += boolToString(Settings.UseNTP);
    return return_result(event, result);
  }
  return return_command_success();
}

String Command_TimeZone(struct EventStruct *event, const char *Line)
{
  if (HasArgv(Line, 2)) {
    Settings.TimeZone = event->Par1;
  } else {
    serialPrintln();
    String result = F("TimeZone:");
    result += Settings.TimeZone;
    return return_result(event, result);
  }
  return return_command_success();
}

String Command_DST(struct EventStruct *event, const char *Line)
{
  if (HasArgv(Line, 2)) {
    Settings.DST = event->Par1;
  } else  {
    serialPrintln();
    String result = F("DST:");
    result += boolToString(Settings.DST);
    return return_result(event, result);
  }
  return return_command_success();
}

String Command_DateTime(struct EventStruct *event, const char *Line)
{
  String TmpStr1;

  if (GetArgv(Line, TmpStr1, 2)) {
    struct tm tm;
    int yr, mnth, d;
    sscanf(TmpStr1.c_str(), "%4d-%2d-%2d", &yr, &mnth, &d);
    tm.tm_year = yr - 1970;
    tm.tm_mon  = mnth;
    tm.tm_mday = d;

    if (GetArgv(Line, TmpStr1, 3)) {
      int h, m, s;
      sscanf(TmpStr1.c_str(), "%2d:%2d:%2d", &h, &m, &s);
      tm.tm_hour = h;
      tm.tm_min  = m;
      tm.tm_sec  = s;
    } else {
      tm.tm_hour = 0;
      tm.tm_min  = 0;
      tm.tm_sec  = 0;
    }

    node_time.sysTime    = makeTime(tm);
    node_time.timeSource = Manual_set;
  } else  {
    // serialPrintln();
    String result = F("Datetime:");
    result += node_time.getDateTimeString('-', ':', ' ');
    return return_result(event, result);
  }
  return return_command_success();
}
