#ifndef COMMANDS_PROVISIONING_H
#define COMMANDS_PROVISIONING_H

#include "../../ESPEasy_common.h"

#if FEATURE_CUSTOM_PROVISIONING

class String;

String Command_Provisioning_Config(struct EventStruct *event,
                                   const char         *Line);
String Command_Provisioning_Security(struct EventStruct *event,
                                     const char         *Line);
#if FEATURE_NOTIFIER
String Command_Provisioning_Notification(struct EventStruct *event,
                                         const char         *Line);
#endif
String Command_Provisioning_Provision(struct EventStruct *event,
                                      const char         *Line);
String Command_Provisioning_Rules(struct EventStruct *event,
                                  const char         *Line);

String Command_Provisioning_Firmware(struct EventStruct *event,
                                     const char         *Line);

#endif // if FEATURE_CUSTOM_PROVISIONING

#endif // ifndef COMMANDS_PROVISIONING_H
