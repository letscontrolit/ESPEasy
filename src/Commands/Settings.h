#ifndef COMMAND_SETTINGS_H
#define COMMAND_SETTINGS_H


String Command_Settings_Build(struct EventStruct *event, const char* Line)
{
	if (HasArgv(Line, 2)) {
		Settings.Build = event->Par1;
	} else {
		serialPrintln();
		String result = F("Build:");
		result += Settings.Build;
    return return_result(event, result);
	}
	return return_command_success();
}

String Command_Settings_Unit(struct EventStruct *event, const char* Line)
{
	if (HasArgv(Line, 2)) {
		Settings.Unit = event->Par1;
	}else  {
		serialPrintln();
		String result = F("Unit:");
		result += Settings.Unit;
    return return_result(event, result);
	}
	return return_command_success();
}

String Command_Settings_Name(struct EventStruct *event, const char* Line)
{
	return Command_GetORSetString(event, F("Name:"),
				      Line,
				      Settings.Name,
				      sizeof(Settings.Name),
				      1);
}

String Command_Settings_Password(struct EventStruct *event, const char* Line)
{
	return Command_GetORSetString(event, F("Password:"),
				      Line,
				      SecuritySettings.Password,
				      sizeof(SecuritySettings.Password),
				      1
				      );
}

String Command_Settings_Save(struct EventStruct *event, const char* Line)
{
	SaveSettings();
	return return_command_success();
}

String Command_Settings_Load(struct EventStruct *event, const char* Line)
{
	LoadSettings();
	return return_command_success();
}

String Command_Settings_Print(struct EventStruct *event, const char* Line)
{
	serialPrintln();

	serialPrintln(F("System Info"));
	serialPrint(F("  IP Address    : ")); serialPrintln(WiFi.localIP().toString());
	serialPrint(F("  Build         : ")); serialPrintln(String((int)BUILD));
	serialPrint(F("  Name          : ")); serialPrintln(Settings.Name);
	serialPrint(F("  Unit          : ")); serialPrintln(String((int)Settings.Unit));
	serialPrint(F("  WifiSSID      : ")); serialPrintln(SecuritySettings.WifiSSID);
	serialPrint(F("  WifiKey       : ")); serialPrintln(SecuritySettings.WifiKey);
	serialPrint(F("  WifiSSID2     : ")); serialPrintln(SecuritySettings.WifiSSID2);
	serialPrint(F("  WifiKey2      : ")); serialPrintln(SecuritySettings.WifiKey2);
	serialPrint(F("  Free mem      : ")); serialPrintln(String(FreeMem()));
	return return_see_serial(event);
}

String Command_Settings_Reset(struct EventStruct *event, const char* Line)
{
	ResetFactory();
	reboot();
	return return_command_success();
}


#endif // COMMAND_SETTINGS_H
