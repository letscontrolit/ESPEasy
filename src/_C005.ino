#include "_CPlugin_Helper.h"
#ifdef USES_C005
//#######################################################################################################
//################### Controller Plugin 005: Home Assistant (openHAB) MQTT ##############################
//#######################################################################################################

#define CPLUGIN_005
#define CPLUGIN_ID_005         5
#define CPLUGIN_NAME_005       "Home Assistant (openHAB) MQTT"

String CPlugin_005_pubname;
bool CPlugin_005_mqtt_retainFlag = false;


bool CPlugin_005(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_005;
        Protocol[protocolCount].usesMQTT = true;
        Protocol[protocolCount].usesTemplate = true;
        Protocol[protocolCount].usesAccount = true;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].usesExtCreds = true;
        Protocol[protocolCount].defaultPort = 1883;
        Protocol[protocolCount].usesID = false;
        break;
      }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_005);
        break;
      }

    case CPlugin::Function::CPLUGIN_INIT:
      {
        success = init_mqtt_delay_queue(event->ControllerIndex, CPlugin_005_pubname, CPlugin_005_mqtt_retainFlag);
        break;
      }

    case CPlugin::Function::CPLUGIN_EXIT:
      {
        exit_mqtt_delay_queue();
        break;
      }

    case CPlugin::Function::CPLUGIN_PROTOCOL_TEMPLATE:
      {
        event->String1 = F("%sysname%/#");
        event->String2 = F("%sysname%/%tskname%/%valname%");
        break;
      }

    case CPlugin::Function::CPLUGIN_PROTOCOL_RECV:
      {
        controllerIndex_t ControllerID = findFirstEnabledControllerWithId(CPLUGIN_ID_005);
        if (!validControllerIndex(ControllerID)) {
          // Controller is not enabled.
          break;
        } else {
          // FIXME TD-er: Command is not parsed for template arguments.
          String cmd;
          struct EventStruct TempEvent;
          TempEvent.TaskIndex = event->TaskIndex;
          bool validTopic = false;
          const int lastindex = event->String1.lastIndexOf('/');
          const String lastPartTopic = event->String1.substring(lastindex + 1);
          if (lastPartTopic == F("cmd")) {
            cmd = event->String2;
            parseCommandString(&TempEvent, cmd);
            TempEvent.Source = EventValueSource::Enum::VALUE_SOURCE_MQTT;
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
            if (command == F("event") || command == F("asyncevent")) {
              eventQueue.add(parseStringToEnd(cmd, 2));
            } else if (!PluginCall(PLUGIN_WRITE, &TempEvent, cmd)) {
              remoteConfig(&TempEvent, cmd);
            }
          }
        }
        break;
      }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
      {
        String pubname = CPlugin_005_pubname;
        bool mqtt_retainFlag = CPlugin_005_mqtt_retainFlag;

        if (ExtraTaskSettings.TaskIndex != event->TaskIndex) {
          String dummy;
          PluginCall(PLUGIN_GET_DEVICEVALUENAMES, event, dummy);
        }

        parseControllerVariables(pubname, event, false);

        byte valueCount = getValueCountFromSensorType(event->sensorType);
        for (byte x = 0; x < valueCount; x++)
        {
          //MFD: skip publishing for values with empty labels (removes unnecessary publishing of unwanted values)
          if (ExtraTaskSettings.TaskDeviceValueNames[x][0]==0)
             continue; //we skip values with empty labels
             
          String tmppubname = pubname;
          tmppubname.replace(F("%valname%"), ExtraTaskSettings.TaskDeviceValueNames[x]);
          String value = "";
          // Small optimization so we don't try to copy potentially large strings
          if (event->sensorType == SENSOR_TYPE_STRING) {
            MQTTpublish(event->ControllerIndex, tmppubname.c_str(), event->String2.c_str(), mqtt_retainFlag);
            value = event->String2.substring(0, 20); // For the log
          } else {
            value = formatUserVarNoCheck(event, x);
            MQTTpublish(event->ControllerIndex, tmppubname.c_str(), value.c_str(), mqtt_retainFlag);
          }
#ifndef BUILD_NO_DEBUG
          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            String log = F("MQTT : ");
            log += tmppubname;
            log += ' ';
            log += value;
            addLog(LOG_LEVEL_DEBUG, log);
          }
#endif
        }
        break;
      }

    case CPlugin::Function::CPLUGIN_FLUSH:
      {
        processMQTTdelayQueue();
        delay(0);
        break;
      }

    default:
      break;

  }

  return success;
}
#endif
