#ifndef COMMAND_SETTINGS_H
#define COMMAND_SETTINGS_H


String Command_Settings_Build(struct EventStruct *event, const char* Line)
{
	char TmpStr1[INPUT_COMMAND_SIZE];
	if (GetArgv(Line, TmpStr1, 2)) {
		Settings.Build = event->Par1;
	}else  {
		Serial.println();
		String result = F("Build:");
		result += Settings.Build;
    return return_result(event, result);
	}
	return return_command_success();
}

String Command_Settings_Unit(struct EventStruct *event, const char* Line)
{
	char TmpStr1[INPUT_COMMAND_SIZE];
	if (GetArgv(Line, TmpStr1, 2)) {
		Settings.Unit = event->Par1;
	}else  {
		Serial.println();
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
	char str[20];
	Serial.println();

	Serial.println(F("System Info"));
	IPAddress ip = WiFi.localIP();
	sprintf_P(str, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
	Serial.print(F("  IP Address    : ")); Serial.println(str);
	Serial.print(F("  Build         : ")); Serial.println((int)BUILD);
	Serial.print(F("  Name          : ")); Serial.println(Settings.Name);
	Serial.print(F("  Unit          : ")); Serial.println((int)Settings.Unit);
	Serial.print(F("  WifiSSID      : ")); Serial.println(SecuritySettings.WifiSSID);
	Serial.print(F("  WifiKey       : ")); Serial.println(SecuritySettings.WifiKey);
	Serial.print(F("  WifiSSID2     : ")); Serial.println(SecuritySettings.WifiSSID2);
	Serial.print(F("  WifiKey2      : ")); Serial.println(SecuritySettings.WifiKey2);
	Serial.print(F("  Free mem      : ")); Serial.println(FreeMem());
	return return_see_serial(event);
}

String Command_Settings_Reset(struct EventStruct *event, const char* Line)
{
	ResetFactory();
  #if defined(ESP8266)
	ESP.reset();
  #endif
  #if defined(ESP32)
	ESP.restart();
  #endif
	return return_command_success();
}


#endif // COMMAND_SETTINGS_H
