
#include "../Helpers/_CPlugin_Helper_mqtt.h"

#if FEATURE_MQTT
# include "../Commands/ExecuteCommand.h"

/***************************************************************************************
 * Parse MQTT topic for /cmd and /set ending to handle commands or TaskValueSet
 * Special C014 case: handleCmd = false and handleSet is true, so *only* pluginID 33 & 86 are accepted
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
      String taskName        = topic.substring(0, lastindex);
      const String valueName = topic.substring(lastindex + 1);
      lastindex = taskName.lastIndexOf('/');

      if (lastindex > -1) {
        taskName = taskName.substring(lastindex + 1);

        const taskIndex_t    taskIndex    = findTaskIndexByName(taskName);
        const deviceIndex_t  deviceIndex  = getDeviceIndex_from_TaskIndex(taskIndex);
        const taskVarIndex_t taskVarIndex = event->Par2 - 1;
        uint8_t valueNr;

        if (validDeviceIndex(deviceIndex) && validTaskVarIndex(taskVarIndex)) {
          const int pluginID = Device[deviceIndex].Number;

          # ifdef USES_P033

          if ((pluginID == 33) || // Plugin 33 Dummy Device,
              // backward compatible behavior: if handleCmd = true then execute TaskValueSet regardless of AllowTaskValueSetAllPlugins
              ((handleCmd || Settings.AllowTaskValueSetAllPlugins()) && (pluginID != 86))) {
            // TaskValueSet,<task/device nr>,<value nr>,<value/formula (!ToDo) >, works only with new version of P033!
            valueNr = findDeviceValueIndexByName(valueName, taskIndex);

            if (validTaskVarIndex(valueNr)) { // value Name identified
              // Set a Dummy Device Value, device Number, var number and argument
              cmd     = strformat(F("TaskValueSet,%d,%d,%s"), taskIndex + 1, valueNr + 1, event->String2.c_str());
              handled = true;
            }
          }
          # endif // ifdef USES_P033
          # if defined(USES_P033) && defined(USES_P086)
          else
          # endif // if defined(USES_P033) && defined(USES_P086)
          # ifdef USES_P086

          if (pluginID == 86) { // Plugin 86 Homie receiver. Schedules the event defined in the plugin.
            // Does NOT store the value.
            // Use HomieValueSet to save the value. This will acknowledge back to the controller too.
            valueNr = findDeviceValueIndexByName(valueName, taskIndex);

            if (validTaskVarIndex(valueNr)) {
              cmd = strformat(F("event,%s="), valueName.c_str());

              if (Settings.TaskDevicePluginConfig[taskIndex][valueNr] == 3) {   // Quote String parameters. PLUGIN_086_VALUE_STRING
                cmd += wrapWithQuotes(event->String2);
              } else {
                if (Settings.TaskDevicePluginConfig[taskIndex][valueNr] == 4) { // Enumeration parameter, find Number of item.
                                                                                // PLUGIN_086_VALUE_ENUM
                  const String enumList = ExtraTaskSettings.TaskDeviceFormula[taskVarIndex];
                  int i                 = 1;
                  String part           = parseStringKeepCase(enumList, i);

                  while (!part.isEmpty()) {                      // lookup result in enum List, keep it backward compatible, but
                    if (part.equalsIgnoreCase(event->String2)) { // Homie spec says it should be case-sensitive...
                      break;
                    }
                    i++;
                    part = parseStringKeepCase(enumList, i);
                  }
                  cmd += i;
                  cmd += ',';
                }
                cmd += event->String2;
              }
              handled = true;
            }
          }
          # endif // ifdef USES_P086
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
  bool success                = false;
  const bool contains_valname = pubname.indexOf(F("%valname%")) != -1;

  const uint8_t valueCount = getValueCountForTask(event->TaskIndex);

  for (uint8_t x = 0; x < valueCount; ++x) {
    // MFD: skip publishing for values with empty labels (removes unnecessary publishing of unwanted values)
    if (getTaskValueName(event->TaskIndex, x).isEmpty()) {
      continue; // we skip values with empty labels
    }
    String tmppubname = pubname;

    if (contains_valname) {
      parseSingleControllerVariable(tmppubname, event, x, false);
    }
    parseControllerVariables(tmppubname, event, false);
    String value;

    if (event->sensorType == Sensor_VType::SENSOR_TYPE_STRING) {
      value = event->String2.substring(0, 20); // For the log
    } else {
      value = formatUserVarNoCheck(event, x);
    }
    # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLog(LOG_LEVEL_DEBUG, strformat(F("MQTT C%03d : %s %s"), event->ControllerIndex, tmppubname.c_str(), value.c_str()));
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
