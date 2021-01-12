#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C005

# include "src/Commands/InternalCommands.h"
# include "src/Globals/EventQueue.h"
# include "src/Globals/ExtraTaskSettings.h"
# include "src/Helpers/PeriodicalActions.h"
# include "src/Helpers/StringParser.h"
# include "_Plugin_Helper.h"

// #######################################################################################################
// ################### Controller Plugin 005: Home Assistant (openHAB) MQTT ##############################
// #######################################################################################################

# define CPLUGIN_005
# define CPLUGIN_ID_005         5
# define CPLUGIN_NAME_005       "Home Assistant (openHAB) MQTT"

String CPlugin_005_pubname;
bool   CPlugin_005_mqtt_retainFlag = false;


bool CPlugin_005(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      Protocol[++protocolCount].Number     = CPLUGIN_ID_005;
      Protocol[protocolCount].usesMQTT     = true;
      Protocol[protocolCount].usesTemplate = true;
      Protocol[protocolCount].usesAccount  = true;
      Protocol[protocolCount].usesPassword = true;
      Protocol[protocolCount].usesExtCreds = true;
      Protocol[protocolCount].defaultPort  = 1883;
      Protocol[protocolCount].usesID       = false;
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

        // Topic  : event->String1
        // Message: event->String2
        String cmd;
        bool   validTopic          = false;
        const int lastindex        = event->String1.lastIndexOf('/');
        const String lastPartTopic = event->String1.substring(lastindex + 1);

        if (lastPartTopic == F("cmd")) {
          // Example:
          // topic: ESP_Easy/Bathroom_pir_env/cmd
          // data: gpio,14,0
          // Full command:  gpio,14,0

          cmd = event->String2;

          // SP_C005a: string= ;cmd=gpio,12,0 ;taskIndex=12 ;string1=ESPT12/cmd ;string2=gpio,12,0
          validTopic = true;
        } else {
          // Example:
          // topic: ESP_Easy/Bathroom_pir_env/GPIO/14
          // data: 0 or 1
          // Full command:  gpio,14,0
          if (lastindex > 0) {
            // Topic has at least one separator
            int   lastPartTopic_int;
            float value_f;

            if (validFloatFromString(event->String2, value_f) &&
                validIntFromString(lastPartTopic, lastPartTopic_int)) {
              int prevLastindex = event->String1.lastIndexOf('/', lastindex - 1);
              cmd        = event->String1.substring(prevLastindex + 1, lastindex);
              cmd       += ',';
              cmd       += lastPartTopic_int;
              cmd       += ',';
              cmd       += event->String2; // Just use the original format
              validTopic = true;
            }
          }
        }

        if (validTopic) {
          // in case of event, store to buffer and return...
          String command = parseString(cmd, 1);

          if ((command == F("event")) || (command == F("asyncevent"))) {
            if (Settings.UseRules) {
              eventQueue.add(parseStringToEnd(cmd, 2));
            }
          } else {
            ExecuteCommand(event->TaskIndex, EventValueSource::Enum::VALUE_SOURCE_MQTT, cmd.c_str(), true, true, true);
          }
        }
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      String pubname         = CPlugin_005_pubname;
      bool   mqtt_retainFlag = CPlugin_005_mqtt_retainFlag;

      LoadTaskSettings(event->TaskIndex);
      parseControllerVariables(pubname, event, false);

      byte valueCount = getValueCountForTask(event->TaskIndex);

      for (byte x = 0; x < valueCount; x++)
      {
        // MFD: skip publishing for values with empty labels (removes unnecessary publishing of unwanted values)
        if (ExtraTaskSettings.TaskDeviceValueNames[x][0] == 0) {
          continue; // we skip values with empty labels
        }
        String tmppubname = pubname;
        parseSingleControllerVariable(tmppubname, event, x, false);
        String value;

        // Small optimization so we don't try to copy potentially large strings
        if (event->sensorType == Sensor_VType::SENSOR_TYPE_STRING) {
          MQTTpublish(event->ControllerIndex, tmppubname.c_str(), event->String2.c_str(), mqtt_retainFlag);
          value = event->String2.substring(0, 20); // For the log
        } else {
          value = formatUserVarNoCheck(event, x);
          MQTTpublish(event->ControllerIndex, tmppubname.c_str(), value.c_str(), mqtt_retainFlag);
        }
# ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = F("MQTT : ");
          log += tmppubname;
          log += ' ';
          log += value;
          addLog(LOG_LEVEL_DEBUG, log);
        }
# endif // ifndef BUILD_NO_DEBUG
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

#endif // ifdef USES_C005
