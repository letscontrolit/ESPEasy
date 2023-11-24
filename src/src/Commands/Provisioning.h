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

String Command_Provisioning_Rules(struct EventStruct *event);

String Command_Provisioning_Firmware(struct EventStruct *event,
                                     const char         *Line);

// FIXME DEPRECATED!
# ifdef PLUGIN_BUILD_MAX_ESP32
String Command_Provisioning_ConfigFallback(struct EventStruct *event,
                                           const char         *Line);
String Command_Provisioning_SecurityFallback(struct EventStruct *event,
                                             const char         *Line);
#  if FEATURE_NOTIFIER
String Command_Provisioning_NotificationFallback(struct EventStruct *event,
                                                 const char         *Line);
#  endif // if FEATURE_NOTIFIER
String Command_Provisioning_ProvisionFallback(struct EventStruct *event,
                                              const char         *Line);
String Command_Provisioning_RulesFallback(struct EventStruct *event,
                                          const char         *Line);

String Command_Provisioning_FirmwareFallback(struct EventStruct *event,
                                             const char         *Line);
# endif // ifdef PLUGIN_BUILD_MAX_ESP32

#endif // if FEATURE_CUSTOM_PROVISIONING

#endif // ifndef COMMANDS_PROVISIONING_H
