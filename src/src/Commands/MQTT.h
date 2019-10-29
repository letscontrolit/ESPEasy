#ifndef COMMAND_MQTT_H
#define COMMAND_MQTT_H

class String;

String Command_MQTT_Retain(struct EventStruct *event, const char *Line);
String Command_MQTT_UseUnitNameAsClientId(struct EventStruct *event, const char *Line);
String Command_MQTT_messageDelay(struct EventStruct *event, const char *Line);
String Command_MQTT_Publish(struct EventStruct *event, const char *Line);


#endif // COMMAND_MQTT_H
