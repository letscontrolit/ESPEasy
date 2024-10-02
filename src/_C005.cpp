#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C005

# include "src/Commands/ExecuteCommand.h"
# include "src/Globals/EventQueue.h"
# include "src/Helpers/_CPlugin_Helper_mqtt.h"
# include "src/Helpers/PeriodicalActions.h"
# include "src/Helpers/StringParser.h"
# include "_Plugin_Helper.h"

// #######################################################################################################
// ################### Controller Plugin 005: Home Assistant (openHAB) MQTT ##############################
// #######################################################################################################

/** Changelog:
 * 2023-08-18 tonhuisman: Clean up source for pull request
 * 2023-03-15 tonhuisman: Add processing of topic endpoint /set to issue a TaskValueSet,taskname,taskvalue,payload command for
 *                        topic %sysname%/#/taskname/valuename/set
 *                        Move /cmd and /set handling to helper file for generic MQTT use
 *                        Reformatted source using Uncrustify
 * 2023-03 Changelog started
 */

# define CPLUGIN_005
# define CPLUGIN_ID_005         5
# define CPLUGIN_NAME_005       "Home Assistant (openHAB) MQTT"

String CPlugin_005_pubname;
bool   CPlugin_005_mqtt_retainFlag = false;

bool C005_parse_command(struct EventStruct *event);

bool CPlugin_005(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      ProtocolStruct& proto = getProtocolStruct(event->idx); //      = CPLUGIN_ID_005;
      proto.usesMQTT     = true;
      proto.usesTemplate = true;
      proto.usesAccount  = true;
      proto.usesPassword = true;
      proto.usesExtCreds = true;
      proto.defaultPort  = 1883;
      proto.usesID       = false;
      #if FEATURE_MQTT_TLS
      proto.usesTLS      = true;
      #endif
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

      if (validControllerIndex(ControllerID)) {
        C005_parse_command(event);
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      if (MQTT_queueFull(event->ControllerIndex)) {
        break;
      }

      // String pubname = CPlugin_005_pubname;

      success = MQTT_protocol_send(event, CPlugin_005_pubname, CPlugin_005_mqtt_retainFlag);

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

bool C005_parse_command(struct EventStruct *event) {
  // FIXME TD-er: Command is not parsed for template arguments.

  // Topic  : event->String1
  // Message: event->String2
  bool validTopic = MQTT_handle_topic_commands(event); // default handling of /cmd and /set topics

  if (!validTopic) {
    String cmd;
    const int lastindex            = event->String1.lastIndexOf('/');
    const String lastPartTopic     = event->String1.substring(lastindex + 1);
    const bool   has_cmd_arg_index = event->String1.lastIndexOf(F("cmd_arg")) != -1;

    if (has_cmd_arg_index) {
      // Example:
      // Topic: ESP_Easy/Bathroom_pir_env/cmd_arg1/GPIO/0
      // Message: 14
      // Full command: gpio,14,0

      uint8_t topic_index  = 1;
      String  topic_folder = parseStringKeepCase(event->String1, topic_index, '/');

      while (!topic_folder.startsWith(F("cmd_arg")) && !topic_folder.isEmpty()) {
        ++topic_index;
        topic_folder = parseStringKeepCase(event->String1, topic_index, '/');
      }

      if (!topic_folder.isEmpty()) {
        int32_t cmd_arg_nr = -1;

        if (validIntFromString(topic_folder.substring(7), cmd_arg_nr)) {
          int constructed_cmd_arg_nr = 0;
          ++topic_index;
          topic_folder = parseStringKeepCase(event->String1, topic_index, '/');
          bool msg_added = false;

          while (!topic_folder.isEmpty()) {
            if (constructed_cmd_arg_nr != 0) {
              cmd += ',';
            }

            if (constructed_cmd_arg_nr == cmd_arg_nr) {
              cmd      += event->String2;
              msg_added = true;
            } else {
              cmd += topic_folder;
              ++topic_index;
              topic_folder = parseStringKeepCase(event->String1, topic_index, '/');
            }
            ++constructed_cmd_arg_nr;
          }

          if (!msg_added) {
            cmd += ',';
            cmd += event->String2;
          }

          // addLog(LOG_LEVEL_INFO, String(F("MQTT cmd: ")) + cmd);

          validTopic = true;
        }
      }
    } else {
      // Example:
      // Topic: ESP_Easy/Bathroom_pir_env/GPIO/14
      // Message: 0 or 1
      // Full command:  gpio,14,0
      if (lastindex > 0) {
        // Topic has at least one separator
        int32_t lastPartTopic_int;
        float   value_f;

        if (validFloatFromString(event->String2, value_f) &&
            validIntFromString(lastPartTopic, lastPartTopic_int)) {
          const int prevLastindex = event->String1.lastIndexOf('/', lastindex - 1);
          cmd = strformat(F("%s,%d,%s"),
                          event->String1.substring(prevLastindex + 1, lastindex).c_str(),
                          lastPartTopic_int,
                          event->String2.c_str()); // Just use the original format
          validTopic = true;
        }
      }
    }

    if (validTopic) {
      MQTT_execute_command(cmd);
    }
  }
  return validTopic;
}

#endif // ifdef USES_C005
