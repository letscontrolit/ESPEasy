#include "../Commands/Provisioning.h"

#if FEATURE_CUSTOM_PROVISIONING

# include "../Commands/Common.h"
# include "../DataTypes/ESPEasyFileType.h"
# include "../DataStructs/ESPEasy_EventStruct.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Networking.h"
# include "../Helpers/StringConverter.h"

String Command_Provisioning_Config(struct EventStruct *event, const char *Line)
{
  return downloadFileType(FileType::CONFIG_DAT);
}

String Command_Provisioning_Security(struct EventStruct *event, const char *Line)
{
  return downloadFileType(FileType::SECURITY_DAT);
}

#if FEATURE_NOTIFIER
String Command_Provisioning_Notification(struct EventStruct *event, const char *Line)
{
  return downloadFileType(FileType::NOTIFICATION_DAT);
}
#endif

String Command_Provisioning_Provision(struct EventStruct *event, const char *Line)
{
  return downloadFileType(FileType::PROVISIONING_DAT);
}

String Command_Provisioning_Rules(struct EventStruct *event, const char *Line)
{
  if ((event->Par1 <= 0) || (event->Par1 > 4)) {
    return F("ProvisionRules: rules index out of range");
  }
  return downloadFileType(FileType::RULES_TXT, event->Par1 - 1);
}

String Command_Provisioning_Firmware(struct EventStruct *event, const char *Line)
{
  // FIXME TD-er: Must only allow to use set prefix in the provisioning settings
  const String url = parseStringToEndKeepCase(Line, 2);
  String error;
  if (downloadFirmware(url, error)) {
    // TODO TD-er: send events
  }
  return error;
}


#endif // if FEATURE_CUSTOM_PROVISIONING
