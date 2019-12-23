#include "../Commands/Time.h"

#include "../../ESPEasy_common.h"
#include "../Commands/Common.h"
#include "../Globals/Settings.h"

#include "../../ESPEasy_fdwdecl.h"


String Command_NTPHost(struct EventStruct *event, const char* Line)
{
	return Command_GetORSetString(event, F("NTPHost:"),
				      Line,
				      Settings.NTPHost,
				      sizeof(Settings.NTPHost),
				      1);
}

String Command_useNTP(struct EventStruct *event, const char* Line)
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

String Command_TimeZone(struct EventStruct *event, const char* Line)
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

String Command_DST(struct EventStruct *event, const char* Line)
{
	if (HasArgv(Line, 2)) {
		Settings.DST = event->Par1;
	}else  {
		serialPrintln();
		String result = F("DST:");
		result += boolToString(Settings.DST);
		return return_result(event, result);
	}
	return return_command_success();
}
