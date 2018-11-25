#ifndef COMMAND_DIAGNOSTIC_H
#define COMMAND_DIAGNOSTIC_H


String Command_Lowmem(struct EventStruct *event, const char* Line)
{
	String result;
	result += lowestRAM;
	result += F(" : ");
	result += lowestRAMfunction;
	return return_result(event, result);
}

String Command_Malloc(struct EventStruct *event, const char* Line)
{
	char* ramtest;
	ramtest = (char*)malloc(event->Par1);
	if (ramtest == NULL) return F("failed");
	free(ramtest);
	return return_command_success();
}

String Command_SysLoad(struct EventStruct *event, const char* Line)
{
	String result = toString(getCPUload());
	result += F("% (LC=");
	result += getLoopCountPerSec();
	result += ')';
	return return_result(event, result);
}

String Command_SerialFloat(struct EventStruct *event, const char* Line)
{
	pinMode(1, INPUT);
	pinMode(3, INPUT);
	delay(60000);
	return return_command_success();
}

String Command_MemInfo(struct EventStruct *event, const char* Line)
{
	serialPrint(F("SecurityStruct         | "));
	serialPrintln(String(sizeof(SecuritySettings)));
	serialPrint(F("SettingsStruct         | "));
	serialPrintln(String(sizeof(Settings)));
	serialPrint(F("ExtraTaskSettingsStruct| "));
	serialPrintln(String(sizeof(ExtraTaskSettings)));
	serialPrint(F("DeviceStruct           | "));
	serialPrintln(String(sizeof(Device)));
	return return_see_serial(event);
}

String Command_MemInfo_detail(struct EventStruct *event, const char* Line)
{
	showSettingsFileLayout = true;
	Command_MemInfo(event, Line);
	for (int st = 0; st < SettingsType_MAX; ++st) {
		SettingsType settingsType = static_cast<SettingsType>(st);
		int max_index, offset, max_size;
		int struct_size = 0;
		serialPrintln();
		serialPrint(getSettingsTypeString(settingsType));
		serialPrintln(F(" | start | end | max_size | struct_size"));
		serialPrintln(F("--- | --- | --- | --- | ---"));
		getSettingsParameters(settingsType, 0, max_index, offset, max_size, struct_size);
		for (int i = 0; i < max_index; ++i) {
			getSettingsParameters(settingsType, i, offset, max_size);
			serialPrint(String(i));
			serialPrint("|");
			serialPrint(String(offset));
			serialPrint("|");
			serialPrint(String(offset + max_size - 1));
			serialPrint("|");
			serialPrint(String(max_size));
			serialPrint("|");
			serialPrintln(String(struct_size));
		}
	}
	return return_see_serial(event);
}

String Command_Background(struct EventStruct *event, const char* Line)
{
	unsigned long timer = millis() + event->Par1;
	serialPrintln(F("start"));
	while (!timeOutReached(timer))
		backgroundtasks();
	serialPrintln(F("end"));
	return return_see_serial(event);
}

String Command_Debug(struct EventStruct *event, const char* Line)
{
	char TmpStr1[INPUT_COMMAND_SIZE];
	if (GetArgv(Line, TmpStr1, 2)) {
		setLogLevelFor(LOG_TO_SERIAL, event->Par1);
	}else  {
		serialPrintln();
		serialPrint(F("Serial debug level: "));
		serialPrintln(String(Settings.SerialLogLevel));
	}
	return return_see_serial(event);
}

String Command_logentry(struct EventStruct *event, const char* Line)
{
	addLog(LOG_LEVEL_INFO,Line);
	return return_command_success();
}

#endif // COMMAND_DIAGNOSTIC_H
