#ifndef COMMAND_TIME_H
#define COMMAND_TIME_H


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
	char TmpStr1[INPUT_COMMAND_SIZE];
	if (GetArgv(Line, TmpStr1, 2)) {
		Settings.UseNTP = event->Par1;
	}else  {
		Serial.println();
		String result = F("UseNTP:");
		result += toString(Settings.UseNTP);
		return return_result(event, result);
	}
	return return_command_success();
}

String Command_TimeZone(struct EventStruct *event, const char* Line)
{
	char TmpStr1[INPUT_COMMAND_SIZE];
	if (GetArgv(Line, TmpStr1, 2)) {
		Settings.TimeZone = event->Par1;
	}else  {
		Serial.println();
		String result = F("TimeZone:");
		result += Settings.TimeZone;
		return return_result(event, result);
	}
	return return_command_success();
}

String Command_DST(struct EventStruct *event, const char* Line)
{
	char TmpStr1[INPUT_COMMAND_SIZE];
	if (GetArgv(Line, TmpStr1, 2)) {
		Settings.DST = event->Par1;
	}else  {
		Serial.println();
		String result = F("DST:");
		result += toString(Settings.DST);
		return return_result(event, result);
	}
	return return_command_success();
}

#endif // COMMAND_TIME_H
