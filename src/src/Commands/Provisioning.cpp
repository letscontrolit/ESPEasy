#include "../Commands/Provisioning.h"

#if FEATURE_CUSTOM_PROVISIONING

# include "../Commands/Common.h"
# include "../DataTypes/ESPEasyFileType.h"
# include "../DataStructs/ESPEasy_EventStruct.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Networking.h"
# include "../Helpers/StringConverter.h"

String Command_Provisioning_Dispatcher(struct EventStruct *event,
                                       const char         *Line)
{
  const String cmd = parseString(Line, 2);

  if (equals(cmd, F("config"))) {
    return Command_Provisioning_Config();
  } else
  if (equals(cmd, F("firmware"))) {
    return Command_Provisioning_Firmware(event, Line);
  } else
  # if FEATURE_NOTIFIER
  if (equals(cmd, F("notification"))) {
    return Command_Provisioning_Notification();
  } else
  # endif // if FEATURE_NOTIFIER
  if (equals(cmd, F("provision"))) {
    return Command_Provisioning_Provision();
  } else
  if (equals(cmd, F("rules"))) {
    return Command_Provisioning_Rules(event);
  } else
  if (equals(cmd, F("security"))) {
    return Command_Provisioning_Security();
  }
  return return_command_failed_flashstr();
}

String Command_Provisioning_Config()
{
  return downloadFileType(FileType::CONFIG_DAT);
}

String Command_Provisioning_Security()
{
  return downloadFileType(FileType::SECURITY_DAT);
}

# if FEATURE_NOTIFIER
String Command_Provisioning_Notification()
{
  return downloadFileType(FileType::NOTIFICATION_DAT);
}

# endif // if FEATURE_NOTIFIER

String Command_Provisioning_Provision()
{
  return downloadFileType(FileType::PROVISIONING_DAT);
}

String Command_Provisioning_Rules(struct EventStruct *event)
{
  if ((event->Par2 <= 0) || (event->Par2 > 4)) {
    return F("Provision,Rules: rules index out of range");
  }
  return downloadFileType(FileType::RULES_TXT, event->Par2 - 1);
}

String Command_Provisioning_Firmware(struct EventStruct *event, const char *Line)
{
  // FIXME TD-er: Must only allow to use set prefix in the provisioning settings
  const String url = parseStringToEndKeepCase(Line, 3);
  String error;

  downloadFirmware(url, error); // Events are sent from download handler
  return error;
}

# ifdef PLUGIN_BUILD_MAX_ESP32
void Command_Provisioning_DeprecatedMessage(const String& param) {
  addLog(LOG_LEVEL_ERROR, strformat(F("WARNING: 'Provision%s' is deprecated, change to 'Provision,%s'"), param.c_str(), param.c_str()));
}

String Command_Provisioning_ConfigFallback(struct EventStruct *event, const char *Line)
{
  Command_Provisioning_DeprecatedMessage(F("Config"));
  return Command_Provisioning_Config();
}

String Command_Provisioning_SecurityFallback(struct EventStruct *event, const char *Line)
{
  Command_Provisioning_DeprecatedMessage(F("Security"));
  return Command_Provisioning_Security();
}

#  if FEATURE_NOTIFIER
String Command_Provisioning_NotificationFallback(struct EventStruct *event, const char *Line)
{
  Command_Provisioning_DeprecatedMessage(F("Notification"));
  return Command_Provisioning_Notification();
}

String Command_Provisioning_ProvisionFallback(struct EventStruct *event, const char *Line)
{
  Command_Provisioning_DeprecatedMessage(F("Provision"));
  return Command_Provisioning_Provision();
}

String Command_Provisioning_RulesFallback(struct EventStruct *event, const char *Line)
{
  Command_Provisioning_DeprecatedMessage(F("Rules,<n>"));

  if ((event->Par1 <= 0) || (event->Par1 > 4)) {
    return F("ProvisionRules: rules index out of range");
  }
  return downloadFileType(FileType::RULES_TXT, event->Par1 - 1);
}

String Command_Provisioning_FirmwareFallback(struct EventStruct *event, const char *Line)
{
  Command_Provisioning_DeprecatedMessage(F("Firmware,<Firmware.bin>"));

  // FIXME TD-er: Must only allow to use set prefix in the provisioning settings
  const String url = parseStringToEndKeepCase(Line, 2);
  String error;

  downloadFirmware(url, error);
  return error;
}

#  endif // if FEATURE_NOTIFIER

# endif // ifdef PLUGIN_BUILD_MAX_ESP32

#endif // if FEATURE_CUSTOM_PROVISIONING
