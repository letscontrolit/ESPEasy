#ifndef COMMAND_SYSTEM_H
#define COMMAND_SYSTEM_H


String Command_System_NoSleep(struct EventStruct *event, const char* Line)
{
	Settings.deepSleep = 0;
	return return_command_success();
}

String Command_System_deepSleep(struct EventStruct *event, const char* Line)
{
	if (event->Par1 > 0)
		deepSleepStart(event->Par1); // call the second part of the function to avoid check and enable one-shot operation
	return return_command_success();
}

String Command_System_Reboot(struct EventStruct *event, const char* Line)
{
	pinMode(0, INPUT);
	pinMode(2, INPUT);
	pinMode(15, INPUT);
#if defined(ESP8266)
	ESP.reset();
#endif
#if defined(ESP32)
	ESP.restart();
#endif
	return return_command_success();
}

String Command_System_Restart(struct EventStruct *event, const char* Line)
{
	ESP.restart();
	return return_command_success();
}

#endif // COMMAND_SYSTEM_H
