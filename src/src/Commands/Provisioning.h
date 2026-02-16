#ifndef COMMANDS_PROVISIONING_H
#define COMMANDS_PROVISIONING_H

#include "../../ESPEasy_common.h"

#if FEATURE_CUSTOM_PROVISIONING

class String;

String Command_Provisioning_Dispatcher(struct EventStruct *event,
                                       const char         *Line);
String Command_Provisioning_Config();
String Command_Provisioning_Security();
# if FEATURE_NOTIFIER
String Command_Provisioning_Notification();
# endif // if FEATURE_NOTIFIER
String Command_Provisioning_Provision();

#if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE
String Command_Provisioning_DeviceSecurity();
#endif

String Command_Provisioning_Rules(struct EventStruct *event);

String Command_Provisioning_Firmware(struct EventStruct *event,
                                     const char         *Line);

#endif // if FEATURE_CUSTOM_PROVISIONING

#endif // ifndef COMMANDS_PROVISIONING_H
