#ifndef COMMAND_MQTT_H
#define COMMAND_MQTT_H


bool Command_MQTT_Retain(struct EventStruct *event, const char* Line)
{
  return Command_GetORSetBool(F("MQTT Retain:"),
   Line,
   (bool *)&Settings.MQTTRetainFlag,
   1);
}

bool Command_MQTT_UseUnitNameAsClientId(struct EventStruct *event, const char* Line)
{
  return Command_GetORSetBool(F("MQTT Use Unit Name as ClientId:"),
   Line,
   (bool *)&Settings.MQTTUseUnitNameAsClientId,
   1);
}

bool Command_MQTT_messageDelay(struct EventStruct *event, const char* Line)
{
  char TmpStr1[INPUT_COMMAND_SIZE];
  if (GetArgv(Line, TmpStr1, 2)) {
    Settings.MessageDelay = event->Par1;
  }
  else{
    Serial.println();
    Serial.print(F("MQTT message delay:"));
    Serial.println(Settings.MessageDelay);
  }
  return true;
}

bool Command_MQTT_Publish(struct EventStruct *event, const char* Line)
{
  bool success = false;
  if (wifiStatus == ESPEASY_WIFI_SERVICES_INITIALIZED) {
    // ToDo TD-er: Not sure about this function, but at least it sends to an existing MQTTclient
    int enabledMqttController = firstEnabledMQTTController();
    if (enabledMqttController >= 0) {
      success = true;
      String eventName = Line;
      eventName = eventName.substring(8);
      int index = eventName.indexOf(',');
      if (index > 0)
      {
        String topic = eventName.substring(0, index);
        String value = eventName.substring(index + 1);
        MQTTpublish(enabledMqttController, topic.c_str(), value.c_str(), Settings.MQTTRetainFlag);
      }
    }
  }
  return success;
}

#endif // COMMAND_MQTT_H