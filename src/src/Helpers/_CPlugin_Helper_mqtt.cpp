
#include "../Helpers/_CPlugin_Helper_mqtt.h"

#if FEATURE_MQTT
# include "../Commands/InternalCommands.h"

/***************************************************************************************
 * Parse MQTT topic for /cmd and /set ending to handle commands or TaskValueSet
 **************************************************************************************/
bool MQTT_handle_topic_commands(struct EventStruct *event,
                                bool                handleCmd,
                                bool                handleSet,
                                bool                tryRemoteConfig) {
  bool handled = false;

  // Topic  : event->String1
  // Message: event->String2
  String cmd;
  int    lastindex           = event->String1.lastIndexOf('/');
  const String lastPartTopic = event->String1.substring(lastindex + 1);

  if (!handled && handleCmd && equals(lastPartTopic, F("cmd"))) {
    // Example:
    // Topic: ESP_Easy/Bathroom_pir_env/cmd
    // Message: gpio,14,0
    // Full command:  gpio,14,0

    cmd = event->String2;

    // SP_C005a: string= ;cmd=gpio,12,0 ;taskIndex=12 ;string1=ESPT12/cmd ;string2=gpio,12,0
    handled = true;
  }

  if (!handled && handleSet && equals(lastPartTopic, F("set"))) {
    // Example:
    // Topic: ESP_Easy/DummyTask/DummyVar/set
    // Message: 14
    // Full command: TaskValueSet,DummyTask,DummyVar,14
    const String topic = event->String1.substring(0, lastindex);
    lastindex = topic.lastIndexOf('/');

    if (lastindex > -1) {
      String taskName  = topic.substring(0, lastindex);
      String valueName = topic.substring(lastindex + 1);
      lastindex = taskName.lastIndexOf('/');

      if (lastindex > -1) {
        taskName = taskName.substring(lastindex + 1);

        if (!taskName.isEmpty() && !valueName.isEmpty() && !event->String2.isEmpty() &&
            cmd.reserve(12 + 3 + taskName.length() + valueName.length() + event->String2.length())) {
          cmd     = F("TaskValueSet");
          cmd    += ',';
          cmd    += taskName;
          cmd    += ',';
          cmd    += valueName;
          cmd    += ',';
          cmd    += event->String2;
          handled = true;
        }
      }
    }
  }

  if (handled) {
    MQTT_execute_command(cmd, tryRemoteConfig);
  }
  return handled;
}

/*****************************************************************************************
 * Execute commands received via MQTT, sanitize event arguments with regard to commas vs =
 * event/asyncevent are added to queue, other commands executed immediately
 ****************************************************************************************/
void MQTT_execute_command(String& cmd,
                          bool    tryRemoteConfig) {
  // in case of event, store to buffer and return...
  const String command = parseString(cmd, 1);

  if (equals(command, F("event")) || equals(command, F("asyncevent"))) {
    if (Settings.UseRules) {
      // Need to sanitize the event a bit to allow for sending event values as MQTT messages.
      // For example:
      // Publish topic: espeasy_node/cmd_arg2/event/myevent/2
      // Message: 1
      // Actual event:  myevent=1,2

      // Strip out the "event" or "asyncevent" part, leaving the actual event string
      String args = parseStringToEndKeepCase(cmd, 2);

      {
        // Get the first part upto a parameter separator
        // Example: "myEvent,1,2,3", which needs to be converted to "myEvent=1,2,3"
        // N.B. This may contain the first eventvalue too
        // e.g. "myEvent=1,2,3" => "myEvent=1"
        String eventName    = parseStringKeepCase(args, 1);
        String eventValues  = parseStringToEndKeepCase(args, 2);
        const int equal_pos = eventName.indexOf('=');

        if (equal_pos != -1) {
          // We found an '=' character, so the actual event name is everything before that char.
          eventName   = args.substring(0, equal_pos);
          eventValues = args.substring(equal_pos + 1); // Rest of the event, after the '=' char
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
        args = eventName;

        if (eventValues.length() > 0) {
          // Only append an = if there are eventvalues.
          args += '=';
          args += eventValues;
        }
      }

      // Check for duplicates, as sometimes a node may have multiple subscriptions to the same topic.
      // Then it may add several of the same events in a burst.
      eventQueue.addMove(std::move(args), true);
    }
  } else {
    ExecuteCommand(INVALID_TASK_INDEX, EventValueSource::Enum::VALUE_SOURCE_MQTT, cmd.c_str(), true, true, tryRemoteConfig);
  }
}

bool MQTT_protocol_send(EventStruct *event,
                        String       pubname,
                        bool         retainFlag) {
  bool success = false;

  parseControllerVariables(pubname, event, false);

  const uint8_t valueCount = getValueCountForTask(event->TaskIndex);

  for (uint8_t x = 0; x < valueCount; x++)
  {
    // MFD: skip publishing for values with empty labels (removes unnecessary publishing of unwanted values)
    if (getTaskValueName(event->TaskIndex, x).isEmpty()) {
      continue; // we skip values with empty labels
    }
    String tmppubname = pubname;
    parseSingleControllerVariable(tmppubname, event, x, false);
    String value;

    if (event->sensorType == Sensor_VType::SENSOR_TYPE_STRING) {
      value = event->String2.substring(0, 20); // For the log
    } else {
      value = formatUserVarNoCheck(event, x);
    }
        # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = strformat(F("MQTT C%03d : "), event->ControllerIndex);
      log += tmppubname;
      log += ' ';
      log += value;
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
        # endif // ifndef BUILD_NO_DEBUG

    // Small optimization so we don't try to copy potentially large strings
    if (event->sensorType == Sensor_VType::SENSOR_TYPE_STRING) {
      if (MQTTpublish(event->ControllerIndex, event->TaskIndex, tmppubname.c_str(), event->String2.c_str(),
                      retainFlag)) {
        success = true;
      }
    } else {
      // Publish using move operator, thus tmppubname and value are empty after this call
      if (MQTTpublish(event->ControllerIndex, event->TaskIndex, std::move(tmppubname), std::move(value),
                      retainFlag)) {
        success = true;
      }
    }
  }
  return success;
}

#endif // if FEATURE_MQTT
