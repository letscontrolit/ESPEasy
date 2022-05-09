#include "../Commands/Provisioning.h"


#ifdef USE_CUSTOM_PROVISIONING

# include "../Commands/Common.h"

# include "../../ESPEasy_common.h"
# include "../DataTypes/ESPEasyFileType.h"
# include "../DataStructs/ESPEasy_EventStruct.h"
# include "../Helpers/ESPEasy_Storage.h"


String Command_Provisioning_Config(struct EventStruct *event, const char *Line)
{
  return downloadFileType(FileType::CONFIG_DAT);
}

String Command_Provisioning_Security(struct EventStruct *event, const char *Line)
{
  return downloadFileType(FileType::SECURITY_DAT);
}

String Command_Provisioning_Notification(struct EventStruct *event, const char *Line)
{
  return downloadFileType(FileType::NOTIFICATION_DAT);
}

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
  const String url = parseStringToEndKeepCase(Line, 2);
  String error;
  if (downloadFirmware(url, error)) {
    // TODO TD-er: send events
  }
  return error;
}


#endif // ifdef USE_CUSTOM_PROVISIONING
