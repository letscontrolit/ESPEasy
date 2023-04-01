#include "../Commands/Settings.h"

#include "../../ESPEasy_common.h"

#include "../Commands/Common.h"

#include "../CustomBuild/CompiletimeDefines.h"

#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/Serial.h"

#include "../Globals/SecuritySettings.h"
#include "../Globals/Settings.h"

#include "../Helpers/ESPEasy_FactoryDefault.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Memory.h"
#include "../Helpers/MDNS_Helper.h"
#include "../Helpers/Misc.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringGenerator_System.h"


String Command_Settings_Build(struct EventStruct *event, const char* Line)
{
	if (HasArgv(Line, 2)) {
	  Settings.Build = event->Par1;
	} else {
      return return_result(event, concat(F("Build:"), static_cast<int>(Settings.Build)));
	}
	return return_command_success_str();
}

String Command_Settings_Unit(struct EventStruct *event, const char* Line)
{
	if (HasArgv(Line, 2)) {
	  Settings.Unit = event->Par1;
	  update_mDNS();
	} else {
      return return_result(event, concat(F("Unit:"), static_cast<int>(Settings.Unit)));
	}
	return return_command_success_str();
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

const __FlashStringHelper *  Command_Settings_Password_Clear(struct EventStruct *event, const char* Line)
{
	const String storedPassword = SecuritySettings.getPassword();
	if (storedPassword.length() > 0) {
		// There is a password set, so we must check it.
		const String password = parseStringKeepCase(Line, 2);
		if (!storedPassword.equals(password)) {
			return return_command_failed();
		}
        ZERO_FILL(SecuritySettings.Password);
	}
	return return_command_success();
}

const __FlashStringHelper * Command_Settings_Save(struct EventStruct *event, const char* Line)
{
	SaveSettings();
	return return_command_success();
}

const __FlashStringHelper * Command_Settings_Load(struct EventStruct *event, const char* Line)
{
	LoadSettings();
	return return_command_success();
}

const __FlashStringHelper * Command_Settings_Print(struct EventStruct *event, const char* Line)
{
	serialPrintln();

	serialPrintln(F("System Info"));
	serialPrint(F("  IP Address    : ")); serialPrintln(NetworkLocalIP().toString());
	serialPrint(F("  Build         : ")); serialPrintln(String(get_build_nr()) + '/' + getSystemBuildString());
	serialPrint(F("  Name          : ")); serialPrintln(Settings.getName());
	serialPrint(F("  Unit          : ")); serialPrintln(String(static_cast<int>(Settings.Unit)));
	serialPrint(F("  WifiSSID      : ")); serialPrintln(SecuritySettings.WifiSSID);
	serialPrint(F("  WifiKey       : ")); serialPrintln(SecuritySettings.WifiKey);
	serialPrint(F("  WifiSSID2     : ")); serialPrintln(SecuritySettings.WifiSSID2);
	serialPrint(F("  WifiKey2      : ")); serialPrintln(SecuritySettings.WifiKey2);
	serialPrint(F("  Free mem      : ")); serialPrintln(String(FreeMem()));
	return return_see_serial(event);
}

const __FlashStringHelper * Command_Settings_Reset(struct EventStruct *event, const char* Line)
{
	ResetFactory();
	reboot(ESPEasy_Scheduler::IntendedRebootReason_e::ResetFactoryCommand);
	return return_command_success();
}
