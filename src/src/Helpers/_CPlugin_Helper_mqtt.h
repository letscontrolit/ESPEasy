#ifndef CPLUGIN_HELPER_MQTT_H
#define CPLUGIN_HELPER_MQTT_H

#if FEATURE_MQTT
# include "../Helpers/_CPlugin_Helper.h"

bool MQTT_handle_topic_commands(struct EventStruct *event,
                                bool                handleCmd       = true,
                                bool                handleSet       = true,
                                bool                tryRemoteConfig = false);
void MQTT_execute_command(String& command,
                          bool    tryRemoteConfig = false);
bool MQTT_protocol_send(EventStruct *event,
                        String       pubname,
                        bool         retainFlag);

#endif // if FEATURE_MQTT
#endif // ifndef CPLUGIN_HELPER_MQTT_H
