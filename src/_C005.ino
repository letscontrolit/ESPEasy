#ifdef USES_C005
//#######################################################################################################
//########################### Controller Plugin 005: OpenHAB MQTT #######################################
//#######################################################################################################

#define CPLUGIN_005
#define CPLUGIN_ID_005         5
#define CPLUGIN_NAME_005       "OpenHAB MQTT"

bool CPlugin_005(byte function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_005;
        Protocol[protocolCount].usesMQTT = true;
        Protocol[protocolCount].usesTemplate = true;
        Protocol[protocolCount].usesAccount = true;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].defaultPort = 1883;
        Protocol[protocolCount].usesID = false;
        break;
      }

    case CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_005);
        break;
      }

    case CPLUGIN_INIT:
      {
        MakeControllerSettings(ControllerSettings);
        LoadControllerSettings(event->ControllerIndex, ControllerSettings);
        MQTTDelayHandler.configureControllerSettings(ControllerSettings);
        break;
      }

    case CPLUGIN_PROTOCOL_TEMPLATE:
      {
        event->String1 = F("/%sysname%/#");
        event->String2 = F("/%sysname%/%tskname%/%valname%");
        break;
      }

    case CPLUGIN_PROTOCOL_RECV:
      {
        byte ControllerID = findFirstEnabledControllerWithId(CPLUGIN_ID_005);
        if (ControllerID == CONTROLLER_MAX) {
          // Controller is not enabled.
          break;
        } else {
          String cmd;
          struct EventStruct TempEvent;
          TempEvent.TaskIndex = event->TaskIndex;
          bool validTopic = false;
          const int lastindex = event->String1.lastIndexOf('/');
          const String lastPartTopic = event->String1.substring(lastindex + 1);
          if (lastPartTopic == F("cmd")) {
            cmd = event->String2;
            parseCommandString(&TempEvent, cmd);
            TempEvent.Source = VALUE_SOURCE_MQTT;
            validTopic = true;
          } else {
            if (lastindex > 0) {
              // Topic has at least one separator
              if (isFloat(event->String2) && isInt(lastPartTopic)) {
                int prevLastindex = event->String1.lastIndexOf('/', lastindex - 1);
                cmd = event->String1.substring(prevLastindex + 1, lastindex);
                TempEvent.Par1 = lastPartTopic.toInt();
                TempEvent.Par2 = event->String2.toFloat();
                TempEvent.Par3 = 0;
                validTopic = true;
              }
            }
          }
          if (validTopic) {
            // in case of event, store to buffer and return...
            String command = parseString(cmd, 1);
            if (command == F("event")) {
            eventBuffer = cmd.substring(6);
            } else if (!PluginCall(PLUGIN_WRITE, &TempEvent, cmd)) {
              remoteConfig(&TempEvent, cmd);
            }
          }
        }
        break;
      }

    case CPLUGIN_PROTOCOL_SEND:
      {
        MakeControllerSettings(ControllerSettings);
        LoadControllerSettings(event->ControllerIndex, ControllerSettings);
        if (!ControllerSettings.checkHostReachable(true)) {
            success = false;
            break;
        }
        statusLED(true);

        if (ExtraTaskSettings.TaskIndex != event->TaskIndex)
          PluginCall(PLUGIN_GET_DEVICEVALUENAMES, event, dummyString);

        String pubname = ControllerSettings.Publish;
        parseControllerVariables(pubname, event, false);

        String value = "";
        // byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[event->TaskIndex]);
        byte valueCount = getValueCountFromSensorType(event->sensorType);
        for (byte x = 0; x < valueCount; x++)
        {
          String tmppubname = pubname;
          tmppubname.replace(F("%valname%"), ExtraTaskSettings.TaskDeviceValueNames[x]);
          value = formatUserVarNoCheck(event, x);

          MQTTpublish(event->ControllerIndex, tmppubname.c_str(), value.c_str(), Settings.MQTTRetainFlag);
          String log = F("MQTT : ");
          log += tmppubname;
          log += ' ';
          log += value;
          addLog(LOG_LEVEL_DEBUG, log);
        }
        break;
      }
  }

  return success;
}
#endif
