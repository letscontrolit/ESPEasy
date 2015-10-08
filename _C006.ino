//#######################################################################################################
//########################### Controller Plugin 006: PiDome MQTT ########################################
//#######################################################################################################

#define CPLUGIN_006
#define CPLUGIN_ID_006         6
#define CPLUGIN_NAME_006       "PiDome MQTT"

boolean CPlugin_006(byte function, struct EventStruct *event)
{
  boolean success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_006;
        strcpy_P(Protocol[protocolCount].Name, PSTR(CPLUGIN_NAME_006));
        Protocol[protocolCount].usesMQTT = true;
        Protocol[protocolCount].usesAccount = false;
        Protocol[protocolCount].usesPassword = false;
        break;
      }

    case CPLUGIN_PROTOCOL_TEMPLATE:
      {
        strcpy_P(Settings.MQTTsubscribe, PSTR("/Home/#"));
        strcpy_P(Settings.MQTTpublish, PSTR("/hooks/devices/%id%/SensorData/%valname%"));        
        break;
      }
      
    case CPLUGIN_PROTOCOL_RECV:
      {
        // Split topic into array
        String tmpTopic = event->String1.substring(1);
        String topicSplit[10];
        int SlashIndex = tmpTopic.indexOf('/');
        byte count = 0;
        while (SlashIndex > 0 && count < 10 - 1)
        {
          topicSplit[count] = tmpTopic.substring(0, SlashIndex);
          tmpTopic = tmpTopic.substring(SlashIndex + 1);
          SlashIndex = tmpTopic.indexOf('/');
          count++;
        }
        topicSplit[count] = tmpTopic;

        String name = topicSplit[4];
        String cmd = topicSplit[5];
        struct EventStruct TempEvent;
        TempEvent.Par1 = topicSplit[2].toInt();
        TempEvent.Par2 = event->String2.toFloat();
        if (name == Settings.Name)
        {
          PluginCall(PLUGIN_WRITE, &TempEvent, cmd);
        }
        break;
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        // MQTT publish structure:
        // /hooks/devices/idx/groupid/value name
        if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0)
          PluginCall(PLUGIN_GET_DEVICEVALUENAMES, event, dummyString);

        String pubname = Settings.MQTTpublish;
        pubname.replace("%sysname%", Settings.Name);
        pubname.replace("%tskname%", ExtraTaskSettings.TaskDeviceName);
        pubname.replace("%id%", String(event->idx));

        String value = "";
        byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[event->TaskIndex]);

        switch (event->sensorType)
        {
          case SENSOR_TYPE_SINGLE:                      // single value sensor, used for Dallas, BH1750, etc
          case SENSOR_TYPE_SWITCH:
          case SENSOR_TYPE_DIMMER:
            pubname.replace("%valname%", ExtraTaskSettings.TaskDeviceValueNames[0]);
            value = String(UserVar[event->BaseVarIndex]);
            MQTTclient.publish(pubname, value);
            break;
          case SENSOR_TYPE_TEMP_HUM:
          case SENSOR_TYPE_TEMP_BARO:
            String tmppubname = pubname;
            tmppubname.replace("%valname%", ExtraTaskSettings.TaskDeviceValueNames[0]);
            value = String(UserVar[event->BaseVarIndex]);
            MQTTclient.publish(tmppubname, value);
            tmppubname = pubname;
            tmppubname.replace("%valname%", ExtraTaskSettings.TaskDeviceValueNames[1]);
            value = String(UserVar[event->BaseVarIndex + 1]);
            MQTTclient.publish(tmppubname, value);
            break;
        }

      }
      return success;
  }
}
