#ifndef COMMAND_MQTT_H
#define COMMAND_MQTT_H


String Command_MQTT_Retain(struct EventStruct *event, const char* Line)
{
  return Command_GetORSetBool(event, F("MQTT Retain:"),
   Line,
   (bool *)&Settings.MQTTRetainFlag,
   1);
}

String Command_MQTT_UseUnitNameAsClientId(struct EventStruct *event, const char* Line)
{
  return Command_GetORSetBool(event, F("MQTT Use Unit Name as ClientId:"),
   Line,
   (bool *)&Settings.MQTTUseUnitNameAsClientId,
   1);
}

String Command_MQTT_messageDelay(struct EventStruct *event, const char* Line)
{
  char TmpStr1[INPUT_COMMAND_SIZE];
  if (GetArgv(Line, TmpStr1, 2)) {
    Settings.MessageDelay = event->Par1;
  }
  else{
    String result = F("MQTT message delay:");
    result += Settings.MessageDelay;
    Serial.println();
    Serial.println(result);
    return result;
  }
  return return_command_success();
}

String Command_MQTT_Publish(struct EventStruct *event, const char* Line)
{
  if (wifiStatus == ESPEASY_WIFI_SERVICES_INITIALIZED) {
    // ToDo TD-er: Not sure about this function, but at least it sends to an existing MQTTclient
    int enabledMqttController = firstEnabledMQTTController();
    if (enabledMqttController >= 0) {
      String eventName = Line;
      eventName = eventName.substring(8);
      int index = eventName.indexOf(',');
      if (index > 0)
      {
        String topic = eventName.substring(0, index);
        String value = eventName.substring(index + 1);
        MQTTpublish(enabledMqttController, topic.c_str(), value.c_str(), Settings.MQTTRetainFlag);
      }
      return return_command_success();
    }
    return F("No MQTT controller enabled");
  }
  return return_not_connected();
}

#endif // COMMAND_MQTT_H
