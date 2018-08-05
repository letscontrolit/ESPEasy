#ifndef COMMAND_RULES_H
#define COMMAND_RTC_H


#include "../ESPEasy-Globals.h"

String Command_Rules_Execute(struct EventStruct *event, const char* Line)
{
	char TmpStr1[INPUT_COMMAND_SIZE];
	if (GetArgv(Line, TmpStr1, 2)) {
		String fileName = TmpStr1;
		String event = "";
		rulesProcessingFile(fileName, event);
	}
	return return_command_success();
}


String Command_Rules_UseRules(struct EventStruct *event, const char* Line)
{
	return Command_GetORSetBool(event, F("Rules:"),
				    Line,
				    (bool*)&Settings.UseRules,
				    1);
}

String Command_Rules_Events(struct EventStruct *event, const char* Line)
{
	String eventName = Line;
	eventName = eventName.substring(6);
	eventName.replace('$', '#');
	if (Settings.UseRules)
		rulesProcessing(eventName);
	return return_command_success();
}

#endif // COMMAND_RULES_H
