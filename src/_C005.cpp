#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C005

# include "src/Commands/ExecuteCommand.h"
# include "src/Globals/EventQueue.h"
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


      String pubname              = CPlugin_005_pubname;
      const bool contains_valname = pubname.indexOf(F("%valname%")) != -1;
      bool mqtt_retainFlag        = CPlugin_005_mqtt_retainFlag;

      uint8_t valueCount = getValueCountForTask(event->TaskIndex);

      for (uint8_t x = 0; x < valueCount; x++)
      {
        // MFD: skip publishing for values with empty labels (removes unnecessary publishing of unwanted values)
        if (Cache.getTaskDeviceValueName(event->TaskIndex, x).isEmpty()) {
          continue; // we skip values with empty labels
        }

        String tmppubname = pubname;

        if (contains_valname) {
          parseSingleControllerVariable(tmppubname, event, x, false);
        }
        parseControllerVariables(tmppubname, event, false);
        String value;

        if (event->sensorType == Sensor_VType::SENSOR_TYPE_STRING) {
# ifndef BUILD_NO_DEBUG
          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            value = event->String2.substring(0, 20); // For the log
          }
# endif
        } else {
          value = formatUserVarNoCheck(event, x);
        }
# ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          addLogMove(LOG_LEVEL_DEBUG,
                     strformat(
                       F("MQTT : %s %s"),
                       tmppubname.c_str(),
                       value.c_str()));
        }
# endif // ifndef BUILD_NO_DEBUG

        // Small optimization so we don't try to copy potentially large strings
        if (event->sensorType == Sensor_VType::SENSOR_TYPE_STRING) {
          if (MQTTpublish(event->ControllerIndex, event->TaskIndex, tmppubname.c_str(), event->String2.c_str(), mqtt_retainFlag)) {
            success = true;
          }
        } else {
          // Publish using move operator, thus tmppubname and value are empty after this call
          if (MQTTpublish(event->ControllerIndex, event->TaskIndex, std::move(tmppubname), std::move(value), mqtt_retainFlag)) {
            success = true;
          }
        }
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

bool C005_parse_command(struct EventStruct *event) {
  // FIXME TD-er: Command is not parsed for template arguments.

  // Topic  : event->String1
  // Message: event->String2
  String cmd;
  bool   validTopic              = false;
  const int lastindex            = event->String1.lastIndexOf('/');
  const String lastPartTopic     = event->String1.substring(lastindex + 1);
  const bool   has_cmd_arg_index = event->String1.lastIndexOf(F("cmd_arg")) != -1;

  if (equals(lastPartTopic, F("cmd"))) {
    // Example:
    // Topic: ESP_Easy/Bathroom_pir_env/cmd
    // Message: gpio,14,0
    // Full command:  gpio,14,0

    move_special(cmd, String(event->String2));

    // SP_C005a: string= ;cmd=gpio,12,0 ;taskIndex=12 ;string1=ESPT12/cmd ;string2=gpio,12,0
    validTopic = true;
  } else if (has_cmd_arg_index) {
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

        // addLog(LOG_LEVEL_INFO, concat(F("MQTT cmd: "), cmd));

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

        cmd = strformat(
          F("%s,%d,%s"),
          event->String1.substring(prevLastindex + 1, lastindex).c_str(),
          lastPartTopic_int,
          event->String2.c_str() // Just use the original format
          );
        validTopic = true;
      }
    }
  }

  if (validTopic) {
    // in case of event, store to buffer and return...
    const String command = parseString(cmd, 1);

    if ((equals(command, F("event"))) || (equals(command, F("asyncevent")))) {
      if (Settings.UseRules) {
        // Need to sanitize the event a bit to allow for sending event values as MQTT messages.
        // For example:
        // Publish topic: espeasy_node/cmd_arg2/event/myevent/2
        // Message: 1
        // Actual event:  myevent=1,2

        // Strip out the "event" or "asyncevent" part, leaving the actual event string
        cmd = parseStringToEndKeepCase(cmd, 2);

        {
          // Get the first part upto a parameter separator
          // Example: "myEvent,1,2,3", which needs to be converted to "myEvent=1,2,3"
          // N.B. This may contain the first eventvalue too
          // e.g. "myEvent=1,2,3" => "myEvent=1"
          String eventName    = parseStringKeepCase(cmd, 1);
          String eventValues  = parseStringToEndKeepCase(cmd, 2);
          const int equal_pos = eventName.indexOf('=');

          if (equal_pos != -1) {
            // We found an '=' character, so the actual event name is everything before that char.
            eventName   = cmd.substring(0, equal_pos);
            eventValues = cmd.substring(equal_pos + 1); // Rest of the event, after the '=' char
          }

          if (eventValues.startsWith(F(","))) {
            // Need to reconstruct the event to get rid of calls like these:
            // myevent=,1,2
            eventValues = eventValues.substring(1);
          }

          // Now reconstruct the complete event
          // Without event values: "myEvent" (no '=' char)
          // With event values: "myEvent=1,2,3"

          // Re-using the 'cmd' String as that has pre-allocated memory which is
          // known to be large enough to hold the entire event.
          cmd = eventName;

          if (eventValues.length() > 0) {
            // Only append an = if there are eventvalues.
            cmd += '=';
            cmd += eventValues;
          }
        }

        // Check for duplicates, as sometimes a node may have multiple subscriptions to the same topic.
        // Then it may add several of the same events in a burst.
        eventQueue.addMove(std::move(cmd), true);
      }
    } else {
      ExecuteCommand_all({ EventValueSource::Enum::VALUE_SOURCE_MQTT, std::move(cmd) }, true);
    }
  }
  return validTopic;
}

#endif // ifdef USES_C005
