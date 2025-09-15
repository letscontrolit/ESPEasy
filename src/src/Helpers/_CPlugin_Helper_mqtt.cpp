
#include "../Helpers/_CPlugin_Helper_mqtt.h"

#if FEATURE_MQTT
# include "../Commands/ExecuteCommand.h"

# if FEATURE_MQTT_DISCOVER
#  include "../Globals/MQTT.h"
#  include "../Helpers/StringGenerator_System.h"
# endif // if FEATURE_MQTT_DISCOVER
# include "../Helpers/SystemVariables.h"

# ifdef USES_P001
#  include "../PluginStructs/P001_data_struct.h"
# endif // ifdef USES_P001

/***************************************************************************************
 * Parse MQTT topic for /cmd and /set ending to handle commands or TaskValueSet
 * Special C014 case: handleCmd = false and handleSet is true, so *only* pluginID 1, 9, 19, 33 & 86 are accepted
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

        const taskIndex_t   taskIndex     = findTaskIndexByName(taskName);
        const deviceIndex_t deviceIndex   = getDeviceIndex_from_TaskIndex(taskIndex);
        uint8_t valueNr                   = findDeviceValueIndexByName(valueName, taskIndex);
        const taskVarIndex_t taskVarIndex = static_cast<taskVarIndex_t>(valueNr);

        if (validDeviceIndex(deviceIndex) && validTaskVarIndex(taskVarIndex) && Settings.TaskDeviceEnabled[taskIndex]) {
          # if defined(USES_P001) || defined(USES_P009) || defined(USES_P019) || defined(USES_P033) || defined(USES_P086)
          const int pluginID = Device[deviceIndex].Number;
          # endif // if defined(USES_P001) || defined(USES_P009) || defined(USES_P010) || defined(USES_P033) || defined(USES_P086)
          # ifdef USES_P001

          if (!handled && (pluginID == 1) && validGpio(Settings.TaskDevicePin[0][taskIndex])) { // Plugin 1 Switch, uses 1st GPIO only
            EventStruct   TempEvent(taskIndex);
            const uint8_t switchtype = P001_data_struct::P001_getSwitchType(&TempEvent);        // 0 = Switch
            const bool    inverted   = Settings.TaskDevicePin1Inversed[taskIndex];
            uint32_t value{};
            validUIntFromString(event->String2, value);

            if ((0 == switchtype) && inverted) {
              value = (0 == value) ? 1u : 0u;
            }
            cmd = strformat((0 == switchtype) ? F("gpio,%d,%d") : F("pwm,%d,%d"),
                            Settings.TaskDevicePin[0][taskIndex], value);
            handled = true;
          }
          # endif // ifdef USES_P001
          # if defined(USES_P009) || defined(USES_P019)

          if (!handled && ((pluginID == 9) || (pluginID == 19))) { // Plugin 9 MCP23017, Plugin 19 PCF8574
            EventStruct TempEvent(taskIndex);
            const bool  inverted = Settings.TaskDevicePin1Inversed[taskIndex];
            uint32_t    value{};
            validUIntFromString(event->String2, value);

            if (inverted) {
              value = (0 == value) ? 1u : 0u;
            }
            cmd = strformat((pluginID == 9) ? F("mcpgpio,%d,%d") : F("pcfgpio,%d,%d"),
                            Settings.TaskDevicePort[taskIndex], value);
            handled = true;
          }
          # endif // if defined(USES_P009) || defined(USES_P019)
          # ifdef USES_P033

          if (!handled && ((pluginID == 33) || // Plugin 33 Dummy Device,
                                               // backward compatible behavior: if handleCmd = true then execute TaskValueSet regardless of
                                               // AllowTaskValueSetAllPlugins
                           ((handleCmd || Settings.AllowTaskValueSetAllPlugins()) && (pluginID != 86)))) {
            // TaskValueSet,<task/device nr>,<value nr>,<value/formula (!ToDo) >, works only with new version of P033!

            if (validTaskVarIndex(taskVarIndex)) { // value Name identified, valueNr = uint8_t
              // Set a Dummy Device Value, device Number, var number and argument
              cmd     = strformat(F("TaskValueSet,%d,%d,%s"), taskIndex + 1, valueNr + 1, event->String2.c_str());
              handled = true;
            }
          }
          # endif // ifdef USES_P033
          # ifdef USES_P086

          if (!handled && (pluginID == 86)) { // Plugin 86 Homie receiver. Schedules the event defined in the plugin.
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

  if (handled && !cmd.isEmpty()) {
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
  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(F("MQTT : Executing command: %s"), cmd.c_str()));
  }
  # endif // ifndef BUILD_NO_DEBUG
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

  // Check for %valname%
  const bool contains_valname = pubname.indexOf(F("%valname%")) != -1;

  # ifndef LIMIT_BUILD_SIZE

  // Small speed-up as there are lots of system variables starting with "%sys" and this is used in quite a lot of MQTT topics.
  // Not needed if build size really matters
  if (pubname.indexOf(F("%sysname%")) != -1) {
    pubname.replace(F("%sysname%"), SystemVariables::getSystemVariable(SystemVariables::SYSNAME));
  }

  // %tskname% will not change per taskvalue, so replace now
  if (pubname.indexOf(F("%tskname%")) != -1) {
    pubname.replace(F("%tskname%"), getTaskDeviceName(event->TaskIndex));
  }
  # endif // ifndef LIMIT_BUILD_SIZE

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
      addLog(LOG_LEVEL_DEBUG, strformat(
               F("MQTT %s : %s %s"),
               get_formatted_Controller_number(getCPluginID_from_ControllerIndex(event->ControllerIndex)).c_str(),
               tmppubname.c_str(),
               value.c_str()));
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
  # if FEATURE_STRING_VARIABLES

  if (Settings.SendDerivedTaskValues(event->TaskIndex, event->ControllerIndex)) {
    String taskName = getTaskDeviceName(event->TaskIndex);
    taskName.toLowerCase();
    String postfix;
    const String search = getDerivedValueSearchAndPostfix(taskName, postfix);

    auto it = customStringVar.begin();

    while (it != customStringVar.end()) {
      if (it->first.startsWith(search) && it->first.endsWith(postfix)) {
        String tmppubname = pubname;

        String valueName    = it->first.substring(search.length(), it->first.indexOf('-'));
        const String vname2 = getDerivedValueName(taskName, valueName);

        if (!vname2.isEmpty()) {
          valueName = vname2;
        }

        if (contains_valname) {
          parseValNameVariable(tmppubname, valueName, false);
        }
        parseControllerVariables(tmppubname, event, false);

        if (!it->second.isEmpty()) {
          String value(it->second);
          value = parseTemplateAndCalculate(value);

          if (MQTTpublish(event->ControllerIndex, event->TaskIndex, std::move(tmppubname), std::move(value),
                          retainFlag)) {
            success = true;
          }
        }
      }
      else if (it->first.substring(0, search.length()).compareTo(search) > 0) {
        break;
      }
      ++it;
    }
  }
  # endif // if FEATURE_STRING_VARIABLES

  return success;
}

# if FEATURE_MQTT_DISCOVER

// Support function to fetch the user-configurable VTypes for a plugin
// use like:
// success = getDiscoveryVType(event, Plugin_085_QueryVType, P085_QUERY1_CONFIG_POS, event->Par5);
bool getDiscoveryVType(struct EventStruct *event, QueryVType_ptr func_ptr, uint8_t pConfigOffset, uint8_t nrVars)
{
  bool success           = false;
  constexpr uint8_t _max = PLUGIN_CONFIGVAR_MAX;
  constexpr uint8_t _par = NR_ELEMENTS(event->ParN) - 1;

  for (uint8_t i = 0; i < nrVars && i < _max; ++i) {
    const uint8_t choice = (255 == pConfigOffset) ? i : PCONFIG(i + pConfigOffset);

    if (i < _par) {
      event->ParN[i] = (func_ptr(choice));
    }
    success = true;
  }
  return success;
}

// helper functions to supply a single value VType to be used by getDiscoveryVType
int Plugin_QueryVType_BinarySensor(uint8_t value_nr) {
  return static_cast<int>(Sensor_VType::SENSOR_TYPE_SWITCH) | Sensor_VType_CAN_SET;
}

int Plugin_QueryVType_BinarySensorInv(uint8_t value_nr) {
  return static_cast<int>(Sensor_VType::SENSOR_TYPE_SWITCH_INVERTED) | Sensor_VType_CAN_SET;
}

int Plugin_QueryVType_Analog(uint8_t value_nr) {
  return static_cast<int>(Sensor_VType::SENSOR_TYPE_ANALOG_ONLY);
}

int Plugin_QueryVType_CO2(uint8_t value_nr) {
  return static_cast<int>(Sensor_VType::SENSOR_TYPE_CO2_ONLY);
}

int Plugin_QueryVType_Distance(uint8_t value_nr) {
  return static_cast<int>(Sensor_VType::SENSOR_TYPE_DISTANCE_ONLY);
}

int Plugin_QueryVType_DustPM2_5(uint8_t value_nr) {
  return static_cast<int>(Sensor_VType::SENSOR_TYPE_DUSTPM2_5_ONLY);
}

int Plugin_QueryVType_Lux(uint8_t value_nr) {
  return static_cast<int>(Sensor_VType::SENSOR_TYPE_LUX_ONLY);
}

int Plugin_QueryVType_Temperature(uint8_t value_nr) {
  return static_cast<int>(Sensor_VType::SENSOR_TYPE_TEMP_ONLY);
}

int Plugin_QueryVType_Weight(uint8_t value_nr) {
  return static_cast<int>(Sensor_VType::SENSOR_TYPE_WEIGHT_ONLY);
}

String makeHomeAssistantCompliantName(const String& name) {
  return makeRFCCompliantName(name, '_', '_', 0);
}

#  if FEATURE_MQTT_DEVICECLASS
const char mqtt_binary_deviceclass_names[] PROGMEM =
  "|"                                                               // Default/0 is empty value
  "power|light|plug|door|garage_door|cold|heat|lock|tamper|window|" // Guessed some often used options to be listed first
  "battery|battery_charging|carbon_monoxide|connectivity|gas|"      // power is selected as the default
  "moisture|motion|moving|occupancy|opening|presence|problem|"      // *** DO NOT CHANGE VALUE ORDER!!!
  "running|safety|smoke|sound|update|vibration|";                   // *** Index is stored in task settings!!!

String MQTT_binary_deviceClassName(int devClassIndex) {
  char tmp[17]{};                                                   // length: battery_charging + \0

  String result(GetTextIndexed(tmp, sizeof(tmp), devClassIndex, mqtt_binary_deviceclass_names));

  return result;
}

int MQTT_binary_deviceClassIndex(const String& deviceClassName) {
  return GetCommandCode(deviceClassName.c_str(), mqtt_binary_deviceclass_names);
}

// TwoWay devices are marked with ² in the selector, and disvocered as 'light' instead of 'binary_sensor'
bool MQTT_binary_deviceClassTwoWay(int devClassIndex) {
  switch (devClassIndex) { // Index into mqtt_binary_deviceclass_names
    case 1:                // power
    case 2:                // light
    case 3:                // plug
    case 5:                // garage_door
    case 8:                // lock
    case 26:               // sound
    case 28:               // vibration
      return true;
    default:
      break;
  }
  return false;
}

#  endif // if FEATURE_MQTT_DEVICECLASS

#  if FEATURE_MQTT_STATE_CLASS
const __FlashStringHelper* MQTT_sensor_StateClass(uint8_t index,
                                                  bool    display) {
  switch (index) {
    case 0: return F("");
    case 1: return display ? F("Measurement") : F("measurement");
    case 2: return display ? F("Total") : F("total");
    case 3: return display ? F("Total-increasing") : F("total_increasing");
    case 4: return display ? F("Measurement-angle") : F("measurement_angle");
  }
  return F("");
}

#  endif // if FEATURE_MQTT_STATE_CLASS

bool MQTT_SendAutoDiscovery(controllerIndex_t ControllerIndex, cpluginID_t CPluginID) {
  bool success = true;

  MakeControllerSettings(ControllerSettings); // -V522

  if (!AllocatedControllerSettings()) {
    return false;
  }
  LoadControllerSettings(ControllerIndex, *ControllerSettings);

  if (ControllerSettings->mqtt_autoDiscovery()

      // && (ControllerSettings->MqttAutoDiscoveryTrigger[0] != 0)
      && (ControllerSettings->MqttAutoDiscoveryTopic[0] != 0)) {
    #  ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, concat(strformat(F("MQTT : AutoDiscovery for Controller %d CPlugin %d"),
                                              ControllerIndex + 1, CPluginID),
                                    validTaskIndex(mqttDiscoverOnlyTask) ?
                                    strformat(F(", Task %d"), mqttDiscoverOnlyTask + 1) :
                                    EMPTY_STRING));
    }
    #  endif // ifndef BUILD_NO_DEBUG


    // Dispatch autoDiscovery per supported CPlugin
    switch (CPluginID) {
      case 5: // CPLUGIN_ID_005 : Home assistant/openHAB
        success = MQTT_HomeAssistant_SendAutoDiscovery(ControllerIndex, *ControllerSettings);
        break;
    }
  }

  return success;
}

/********************************************************
 * Send MQTT AutoDiscovery in Home Assistant format
 *******************************************************/
bool MQTT_HomeAssistant_SendAutoDiscovery(controllerIndex_t         ControllerIndex,
                                          ControllerSettingsStruct& ControllerSettings) {
  bool success = false;

  // Send plugin info
  taskIndex_t fromTask = 0;
  taskIndex_t maxTask  = TASKS_MAX;

  if (validTaskIndex(mqttDiscoverOnlyTask)) {
    fromTask             = mqttDiscoverOnlyTask;
    maxTask              = mqttDiscoverOnlyTask + 1;
    mqttDiscoverOnlyTask = INVALID_TASK_INDEX; // Reset
  }

  for (taskIndex_t x = fromTask; x < maxTask; ++x) {
    const pluginID_t pluginID = Settings.getPluginID_for_task(x);

    if (validPluginID_fullcheck(pluginID)) {
      LoadTaskSettings(x);
      const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(x);

      // Device is enabled so send information
      if (validDeviceIndex(DeviceIndex) &&
          Device[DeviceIndex].SendDataOption &&           // do (can) we send data?
          Settings.TaskDeviceEnabled[x] &&                // task enabled?
          Settings.TaskDeviceSendData[ControllerIndex][x] // selected for this controller?
          ) {
        const String taskName   = getTaskDeviceName(x);
        const int    valueCount = getValueCountForTask(x);
        std::vector<DiscoveryItem> discoveryItems;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, strformat(F("MQTT : Start AutoDiscovery for task %d, %s max. %d value%c"),
                                           x + 1, taskName.c_str(), valueCount, 1 != valueCount ? 's' : ' '));
        }

        String pluginDeviceClass;

        // Translate Device[].VType to usable VType per value, and fetch optional deviceClass, for MQTT AutoDiscovery
        if (MQTT_DiscoveryGetDeviceVType(x, discoveryItems, valueCount, pluginDeviceClass)) {
          const String hostName               = Settings.getHostname();
          const unsigned int groupId          = Settings.TaskDeviceID[ControllerIndex][x] & 0x3FFF; // Remove opt. flag bits
          const protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(ControllerIndex);
          const bool usesControllerIDX        = validProtocolIndex(ProtocolIndex) &&
                                                getProtocolStruct(ProtocolIndex).usesID;
          const bool useGroupId = groupId != 0 && !usesControllerIDX;

          const String elementName = useGroupId ?
                                     (Settings.Unit == groupId ? hostName : strformat(F("Group %u"), groupId)) :
                                     strformat(F("%s %s"),    hostName.c_str(), taskName.c_str());
          const String elementIds = useGroupId ?
                                     strformat(F("group_%u"), groupId) :
                                     strformat(F("%s_%s"),    hostName.c_str(), taskName.c_str());
          const String docLink = makeDocLink(strformat(
                                               F("Plugin/%s.html"),
                                               get_formatted_Plugin_number(Settings.getPluginID_for_task(x)).c_str()), true);

          // dev = device, ids = identifiers, mf = manufacturer, sw = sw_version, o = origin, url = support_url
          const String deviceElement = strformat(F(",\"dev\":{\"name\":\"%s\",\"ids\":[\"%s\"],"
                                                   "\"mf\":\"ESPEasy\",\"sw\":\"%s\"},"
                                                   "\"o\":{\"name\":\"%s\",\"sw\":\"%s\",\"url\":\"%s\"}"),
                                                 elementName.c_str(), elementIds.c_str(),
                                                 getSystemBuildString().c_str(),
                                                 elementName.c_str(), getSystemBuildString().c_str(), docLink.c_str());

          #  if FEATURE_STRING_VARIABLES

          if (Settings.SendDerivedTaskValues(x, ControllerIndex)) {
            String postfix;
            const String search = getDerivedValueSearchAndPostfix(taskName, postfix);

            auto it       = customStringVar.begin();
            uint8_t varNr = VARS_PER_TASK;

            while (it != customStringVar.end()) {
              if (it->first.startsWith(search) && it->first.endsWith(postfix)) {
                String valueName = it->first.substring(search.length(), it->first.indexOf('-'));
                String uom;
                String vTypeStr;
                const String vname2 = getDerivedValueNameUomAndVType(taskName, valueName, uom, vTypeStr);
                Sensor_VType vType  = Sensor_VType::SENSOR_TYPE_NONE;
                vTypeStr.toLowerCase();

                constexpr uint8_t maxVType = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_NOT_SET);

                for (uint8_t i = 0; i < maxVType; ++i) {
                  const Sensor_VType vt = static_cast<Sensor_VType>(i);

                  if ((getValueCountFromSensorType(vt, false) == 1) &&
                      vTypeStr.equalsIgnoreCase(getSensorTypeLabel(vt))) {
                    vType = vt;
                    break;
                  }
                }

                if (Sensor_VType::SENSOR_TYPE_NONE != vType) {
                  if (!vname2.isEmpty()) {
                    valueName = vname2;
                  }
                  discoveryItems.push_back(DiscoveryItem(vType, 1, varNr, valueName, uom, false));

                  ++varNr;
                }
              }
              else if (it->first.substring(0, search.length()).compareTo(search) > 0) {
                break;
              }
              ++it;
            }
          }
          #  endif // if FEATURE_STRING_VARIABLES

          for (size_t s = 0; s < discoveryItems.size(); ++s) {
            struct EventStruct TempEvent(x);
            const uint8_t varCount = discoveryItems[s].varIndex + discoveryItems[s].valueCount;

            switch (discoveryItems[s].VType) {
              // VType values to support, mapped to device classes:
              case Sensor_VType::SENSOR_TYPE_SWITCH:
              case Sensor_VType::SENSOR_TYPE_SWITCH_INVERTED:
              {
                const bool inversedState = Sensor_VType::SENSOR_TYPE_SWITCH_INVERTED == discoveryItems[s].VType;

                for (uint8_t v = discoveryItems[s].varIndex; v < varCount; ++v) {
                  const String valuename  = MQTT_DiscoveryHelperGetValueName(x, v, discoveryItems[s]);
                  String valueDeviceClass = parseStringKeepCase(pluginDeviceClass, v + 1); // Device classes per value

                  if (valueDeviceClass.isEmpty()) { valueDeviceClass = F("power"); } // default
                  #  if FEATURE_MQTT_STATE_CLASS
                  const String stateClass = MQTT_sensor_StateClass(Cache.getTaskVarStateClass(x, v), false);
                  #  else // if FEATURE_MQTT_STATE_CLASS
                  const String stateClass = EMPTY_STRING;
                  #  endif // if FEATURE_MQTT_STATE_CLASS
                  #  if FEATURE_MQTT_DEVICECLASS
                  const bool twoWay = MQTT_binary_deviceClassTwoWay(MQTT_binary_deviceClassIndex(valueDeviceClass));
                  #  else // if FEATURE_MQTT_DEVICECLASS
                  const bool twoWay = true;
                  #  endif // if FEATURE_MQTT_DEVICECLASS

                  // Discover 2-way as Light
                  const __FlashStringHelper*componentClass = twoWay && discoveryItems[s].canSet ? F("light") : F("binary_sensor");
                  const String deviceClass                 = strformat(F("%s\",\"pl_on\":\"%d\",\"pl_off\":\"%d"),
                                                                       valueDeviceClass.c_str(), !inversedState, inversedState);
                  const String uom = MQTT_DiscoveryHelperGetValueUoM(x, v, discoveryItems[s]);

                  if (discoveryItems[s].canSet) {
                    success &= MQTT_DiscoveryPublishWithStatusAndSet(x, v, valuename,
                                                                     ControllerIndex,
                                                                     ControllerSettings,
                                                                     F("device_automation"),
                                                                     EMPTY_STRING, // unused
                                                                     EMPTY_STRING, // No unit of measure used
                                                                     &TempEvent,
                                                                     deviceElement,
                                                                     EMPTY_STRING,
                                                                     success,
                                                                     true, false,
                                                                     useGroupId ? elementName : EMPTY_STRING, elementIds,
                                                                     true); // Send Trigger discovery
                  }
                  success &= MQTT_DiscoveryPublishWithStatusAndSet(x, v, valuename,
                                                                   ControllerIndex,
                                                                   ControllerSettings,
                                                                   componentClass,
                                                                   deviceClass,
                                                                   uom,
                                                                   &TempEvent,
                                                                   deviceElement,
                                                                   stateClass,
                                                                   success,
                                                                   discoveryItems[s].canSet, false,
                                                                   useGroupId ? elementName : EMPTY_STRING, elementIds);
                }
                break;
              }
              case Sensor_VType::SENSOR_TYPE_TEMP_HUM:
              case Sensor_VType::SENSOR_TYPE_TEMP_BARO:
              case Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO:
              case Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO:
              case Sensor_VType::SENSOR_TYPE_TEMP_ONLY:
              case Sensor_VType::SENSOR_TYPE_HUM_ONLY:
              case Sensor_VType::SENSOR_TYPE_BARO_ONLY:
              {
                uint8_t v = discoveryItems[s].varIndex;

                // Temperature
                if ((Sensor_VType::SENSOR_TYPE_HUM_ONLY != discoveryItems[s].VType) &&
                    (Sensor_VType::SENSOR_TYPE_BARO_ONLY != discoveryItems[s].VType)) {
                  uint8_t fromV = v;
                  uint8_t maxV  = v + 1;

                  if (Sensor_VType::SENSOR_TYPE_TEMP_ONLY == discoveryItems[s].VType) {
                    maxV = fromV + discoveryItems[s].valueCount; // SENSOR_TYPE_TEMP_ONLY Can have multiple values
                  }

                  for (v = fromV; v < maxV; ++v) {
                    const String valuename = MQTT_DiscoveryHelperGetValueName(x, v, discoveryItems[s]);
                    const String uom       = MQTT_DiscoveryHelperGetValueUoM(x, v, discoveryItems[s],
                                                                             getValueType2DefaultHAUoM(Sensor_VType::SENSOR_TYPE_TEMP_ONLY));
                    #  if FEATURE_MQTT_STATE_CLASS
                    const String stateClass = MQTT_sensor_StateClass(Cache.getTaskVarStateClass(x, v), false);
                    #  else // if FEATURE_MQTT_STATE_CLASS
                    const String stateClass = EMPTY_STRING;
                    #  endif // if FEATURE_MQTT_STATE_CLASS
                    success &= MQTT_DiscoveryPublishWithStatusAndSet(x, v, valuename,
                                                                     ControllerIndex,
                                                                     ControllerSettings,
                                                                     F("sensor"),
                                                                     getValueType2HADeviceClass(Sensor_VType::SENSOR_TYPE_TEMP_ONLY),
                                                                     uom,
                                                                     &TempEvent,
                                                                     deviceElement,
                                                                     stateClass,
                                                                     success,
                                                                     discoveryItems[s].canSet, false,
                                                                     useGroupId ? elementName : EMPTY_STRING, elementIds);
                  }
                }

                // Humidity
                if ((Sensor_VType::SENSOR_TYPE_TEMP_HUM == discoveryItems[s].VType) ||
                    (Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO == discoveryItems[s].VType) ||
                    (Sensor_VType::SENSOR_TYPE_HUM_ONLY == discoveryItems[s].VType)) {
                  uint8_t fromV = v;
                  uint8_t maxV  = v + 1;

                  if (Sensor_VType::SENSOR_TYPE_HUM_ONLY == discoveryItems[s].VType) {
                    maxV = fromV + discoveryItems[s].valueCount; // SENSOR_TYPE_HUM_ONLY Can have multiple values
                  }

                  for (v = fromV; v < maxV; ++v) {
                    const String valuename = MQTT_DiscoveryHelperGetValueName(x, v, discoveryItems[s]);
                    const String uom       = MQTT_DiscoveryHelperGetValueUoM(x, v, discoveryItems[s],
                                                                             getValueType2DefaultHAUoM(Sensor_VType::SENSOR_TYPE_HUM_ONLY));
                    #  if FEATURE_MQTT_STATE_CLASS
                    const String stateClass = MQTT_sensor_StateClass(Cache.getTaskVarStateClass(x, v), false);
                    #  else // if FEATURE_MQTT_STATE_CLASS
                    const String stateClass = EMPTY_STRING;
                    #  endif // if FEATURE_MQTT_STATE_CLASS
                    success &= MQTT_DiscoveryPublishWithStatusAndSet(x, v, valuename,
                                                                     ControllerIndex,
                                                                     ControllerSettings,
                                                                     F("sensor"),
                                                                     getValueType2HADeviceClass(Sensor_VType::SENSOR_TYPE_HUM_ONLY),
                                                                     uom,
                                                                     &TempEvent,
                                                                     deviceElement,
                                                                     stateClass,
                                                                     success,
                                                                     discoveryItems[s].canSet, false,
                                                                     useGroupId ? elementName : EMPTY_STRING, elementIds);
                  }
                }

                // Barometric pressure
                if ((Sensor_VType::SENSOR_TYPE_TEMP_BARO == discoveryItems[s].VType) ||
                    (Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO == discoveryItems[s].VType) ||
                    (Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO == discoveryItems[s].VType) ||
                    (Sensor_VType::SENSOR_TYPE_BARO_ONLY == discoveryItems[s].VType)) {
                  if (Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO == discoveryItems[s].VType) {
                    v++; // Skip 2nd value = 'EMPTY'
                  }
                  uint8_t fromV = v;
                  uint8_t maxV  = v + 1;

                  if (Sensor_VType::SENSOR_TYPE_BARO_ONLY == discoveryItems[s].VType) {
                    maxV = fromV + discoveryItems[s].valueCount; // SENSOR_TYPE_BARO_ONLY Can have multiple values
                  }

                  for (v = fromV; v < maxV; ++v) {
                    const String valuename = MQTT_DiscoveryHelperGetValueName(x, v, discoveryItems[s]);
                    const String uom       = MQTT_DiscoveryHelperGetValueUoM(x, v, discoveryItems[s],
                                                                             getValueType2DefaultHAUoM(Sensor_VType::SENSOR_TYPE_BARO_ONLY));
                    #  if FEATURE_MQTT_STATE_CLASS
                    const String stateClass = MQTT_sensor_StateClass(Cache.getTaskVarStateClass(x, v), false);
                    #  else // if FEATURE_MQTT_STATE_CLASS
                    const String stateClass = EMPTY_STRING;
                    #  endif // if FEATURE_MQTT_STATE_CLASS
                    success &= MQTT_DiscoveryPublishWithStatusAndSet(x, v, valuename,
                                                                     ControllerIndex,
                                                                     ControllerSettings,
                                                                     F("sensor"),
                                                                     getValueType2HADeviceClass(Sensor_VType::SENSOR_TYPE_BARO_ONLY),
                                                                     uom,
                                                                     &TempEvent,
                                                                     deviceElement,
                                                                     stateClass,
                                                                     success,
                                                                     discoveryItems[s].canSet, false,
                                                                     useGroupId ? elementName : EMPTY_STRING, elementIds);
                    v++;
                  }
                }
                break;
              }
              case Sensor_VType::SENSOR_TYPE_DISTANCE_ONLY:
              case Sensor_VType::SENSOR_TYPE_DUSTPM2_5_ONLY:
              case Sensor_VType::SENSOR_TYPE_DUSTPM1_0_ONLY:
              case Sensor_VType::SENSOR_TYPE_DUSTPM10_ONLY:
              case Sensor_VType::SENSOR_TYPE_CO2_ONLY:
              case Sensor_VType::SENSOR_TYPE_TVOC_ONLY:
              case Sensor_VType::SENSOR_TYPE_AQI_ONLY:
              case Sensor_VType::SENSOR_TYPE_NOX_ONLY:
              case Sensor_VType::SENSOR_TYPE_WEIGHT_ONLY:
              case Sensor_VType::SENSOR_TYPE_MOISTURE_ONLY:
              case Sensor_VType::SENSOR_TYPE_VOLTAGE_ONLY:
              case Sensor_VType::SENSOR_TYPE_CURRENT_ONLY:
              case Sensor_VType::SENSOR_TYPE_POWER_FACT_ONLY:
              case Sensor_VType::SENSOR_TYPE_POWER_USG_ONLY:
              case Sensor_VType::SENSOR_TYPE_REACTIVE_POWER_ONLY:
              case Sensor_VType::SENSOR_TYPE_APPRNT_POWER_USG_ONLY:
              case Sensor_VType::SENSOR_TYPE_LUX_ONLY:
              case Sensor_VType::SENSOR_TYPE_COLOR_RED_ONLY:
              case Sensor_VType::SENSOR_TYPE_COLOR_GREEN_ONLY:
              case Sensor_VType::SENSOR_TYPE_COLOR_BLUE_ONLY:
              case Sensor_VType::SENSOR_TYPE_COLOR_TEMP_ONLY:
              case Sensor_VType::SENSOR_TYPE_UV_ONLY:
              case Sensor_VType::SENSOR_TYPE_UV_INDEX_ONLY:
              case Sensor_VType::SENSOR_TYPE_IR_ONLY:
              case Sensor_VType::SENSOR_TYPE_WIND:
              case Sensor_VType::SENSOR_TYPE_DIRECTION_ONLY:
              case Sensor_VType::SENSOR_TYPE_WIND_SPEED:
              case Sensor_VType::SENSOR_TYPE_DURATION:
              case Sensor_VType::SENSOR_TYPE_DATE:
              case Sensor_VType::SENSOR_TYPE_TIMESTAMP:
              case Sensor_VType::SENSOR_TYPE_DATA_RATE:
              case Sensor_VType::SENSOR_TYPE_DATA_SIZE:
              case Sensor_VType::SENSOR_TYPE_SOUND_PRESSURE:
              case Sensor_VType::SENSOR_TYPE_SIGNAL_STRENGTH:
              case Sensor_VType::SENSOR_TYPE_REACTIVE_ENERGY:
              case Sensor_VType::SENSOR_TYPE_FREQUENCY:
              case Sensor_VType::SENSOR_TYPE_ENERGY:
              case Sensor_VType::SENSOR_TYPE_ENERGY_STORAGE:
              case Sensor_VType::SENSOR_TYPE_ABS_HUMIDITY:
              case Sensor_VType::SENSOR_TYPE_ATMOS_PRESSURE:
              case Sensor_VType::SENSOR_TYPE_BLOOD_GLUCOSE_C:
              case Sensor_VType::SENSOR_TYPE_CO_ONLY:
              case Sensor_VType::SENSOR_TYPE_ENERGY_DISTANCE:
              case Sensor_VType::SENSOR_TYPE_GAS_ONLY:
              case Sensor_VType::SENSOR_TYPE_NITROUS_OXIDE:
              case Sensor_VType::SENSOR_TYPE_OZONE_ONLY:
              case Sensor_VType::SENSOR_TYPE_PRECIPITATION:
              case Sensor_VType::SENSOR_TYPE_PRECIPITATION_INTEN:
              case Sensor_VType::SENSOR_TYPE_SULPHUR_DIOXIDE:
              case Sensor_VType::SENSOR_TYPE_VOC_PARTS:
              case Sensor_VType::SENSOR_TYPE_VOLUME:
              case Sensor_VType::SENSOR_TYPE_VOLUME_FLOW_RATE:
              case Sensor_VType::SENSOR_TYPE_VOLUME_STORAGE:
              case Sensor_VType::SENSOR_TYPE_WATER:
              {
                const String dev    = getValueType2HADeviceClass(discoveryItems[s].VType);
                const String uomDef = getValueType2DefaultHAUoM(discoveryItems[s].VType);

                for (uint8_t v = discoveryItems[s].varIndex; v < varCount; ++v) {
                  const String valuename = MQTT_DiscoveryHelperGetValueName(x, v, discoveryItems[s]);
                  const String uom       = MQTT_DiscoveryHelperGetValueUoM(x, v, discoveryItems[s], uomDef);
                  #  if FEATURE_MQTT_STATE_CLASS
                  const String stateClass = MQTT_sensor_StateClass(Cache.getTaskVarStateClass(x, v), false);
                  #  else // if FEATURE_MQTT_STATE_CLASS
                  const String stateClass = EMPTY_STRING;
                  #  endif // if FEATURE_MQTT_STATE_CLASS
                  success &= MQTT_DiscoveryPublishWithStatusAndSet(x, v, valuename,
                                                                   ControllerIndex,
                                                                   ControllerSettings,
                                                                   F("sensor"),
                                                                   dev,
                                                                   uom,
                                                                   &TempEvent,
                                                                   deviceElement,
                                                                   stateClass,
                                                                   success,
                                                                   discoveryItems[s].canSet, false,
                                                                   useGroupId ? elementName : EMPTY_STRING, elementIds);
                }
                break;
              }


              case Sensor_VType::SENSOR_TYPE_ANALOG_ONLY:
              case Sensor_VType::SENSOR_TYPE_GPS_ONLY:
              case Sensor_VType::SENSOR_TYPE_DIMMER: // Maybe implement?
                // TODO
                break;

              // VType values to ignore, will/should be mapped into something more explicit
              case Sensor_VType::SENSOR_TYPE_NONE:
              case Sensor_VType::SENSOR_TYPE_SINGLE:
              case Sensor_VType::SENSOR_TYPE_DUAL:
              case Sensor_VType::SENSOR_TYPE_TRIPLE:
              case Sensor_VType::SENSOR_TYPE_QUAD:
              case Sensor_VType::SENSOR_TYPE_STRING:
              case Sensor_VType::SENSOR_TYPE_ULONG:
              #  if FEATURE_EXTENDED_TASK_VALUE_TYPES
              case Sensor_VType::SENSOR_TYPE_UINT32_DUAL:
              case Sensor_VType::SENSOR_TYPE_UINT32_TRIPLE:
              case Sensor_VType::SENSOR_TYPE_UINT32_QUAD:
              case Sensor_VType::SENSOR_TYPE_INT32_SINGLE:
              case Sensor_VType::SENSOR_TYPE_INT32_DUAL:
              case Sensor_VType::SENSOR_TYPE_INT32_TRIPLE:
              case Sensor_VType::SENSOR_TYPE_INT32_QUAD:
              case Sensor_VType::SENSOR_TYPE_UINT64_SINGLE:
              case Sensor_VType::SENSOR_TYPE_UINT64_DUAL:
              case Sensor_VType::SENSOR_TYPE_INT64_SINGLE:
              case Sensor_VType::SENSOR_TYPE_INT64_DUAL:
              case Sensor_VType::SENSOR_TYPE_DOUBLE_SINGLE:
              case Sensor_VType::SENSOR_TYPE_DOUBLE_DUAL:
              #  endif // if FEATURE_EXTENDED_TASK_VALUE_TYPES
              case Sensor_VType::SENSOR_TYPE_NOT_SET:
                break;
            }
          }
        }
      }
    }
  }

  return success;
}

bool MQTT_DiscoveryGetDeviceVType(taskIndex_t                 TaskIndex,
                                  std::vector<DiscoveryItem>& discoveryItems,
                                  int                         valueCount,
                                  String                    & deviceClass) {
  const size_t orgLen = discoveryItems.size();
  struct EventStruct TempEvent(TaskIndex);
  String dummy;

  PluginCall(PLUGIN_GET_DEVICEVTYPE, &TempEvent, dummy);
  Sensor_VType VType = TempEvent.sensorType;

  deviceClass.clear();

  TempEvent.Par5 = valueCount; // Pass in the provided value count, so we don't have to determine that again

  // Get value VTypes and deviceClass from plugin
  if (PluginCall(PLUGIN_GET_DISCOVERY_VTYPES, &TempEvent, deviceClass)) {
    #  ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLog(LOG_LEVEL_DEBUG, strformat(F("MQTT : AutoDiscovery using Plugin-defined config (%d)"), discoveryItems.size()));
    }
    #  endif // ifndef BUILD_NO_DEBUG

    uint8_t maxVar = VARS_PER_TASK;

    for (; maxVar > 0; --maxVar) { // Only include minimal used values
      if (TempEvent.ParN[maxVar - 1] != 0) {
        break;
      }
    }

    for (uint8_t v = 0; v < maxVar; ++v) {
      const Sensor_VType VType = static_cast<Sensor_VType>(TempEvent.ParN[v] & 0xff);
      const bool _canSet       = (TempEvent.ParN[v] & Sensor_VType_CAN_SET) == Sensor_VType_CAN_SET;

      if (Sensor_VType::SENSOR_TYPE_NONE != VType) {
        discoveryItems.push_back(DiscoveryItem(VType, 1, v, _canSet));
      }
    }

    #  ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLog(LOG_LEVEL_DEBUG, strformat(F("MQTT : AutoDiscovery Plugin specific config added (%d)"), discoveryItems.size()));
    }
    #  endif // ifndef BUILD_NO_DEBUG
  } else {
    // Use Device VType setting
    if (Sensor_VType::SENSOR_TYPE_NONE != VType) {
      // Expand combined VTypes into separate single-value VTypes
      if (Sensor_VType::SENSOR_TYPE_TEMP_HUM == VType) {
        discoveryItems.push_back(DiscoveryItem(Sensor_VType::SENSOR_TYPE_TEMP_ONLY, 1, 0));
        discoveryItems.push_back(DiscoveryItem(Sensor_VType::SENSOR_TYPE_HUM_ONLY, 1, 1));
      } else
      if (Sensor_VType::SENSOR_TYPE_TEMP_BARO == VType) {
        discoveryItems.push_back(DiscoveryItem(Sensor_VType::SENSOR_TYPE_TEMP_ONLY, 1, 0));
        discoveryItems.push_back(DiscoveryItem(Sensor_VType::SENSOR_TYPE_BARO_ONLY, 1, 1));
      } else
      if (Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO == VType) {
        discoveryItems.push_back(DiscoveryItem(Sensor_VType::SENSOR_TYPE_TEMP_ONLY, 1, 0));
        discoveryItems.push_back(DiscoveryItem(Sensor_VType::SENSOR_TYPE_HUM_ONLY, 1, 1));
        discoveryItems.push_back(DiscoveryItem(Sensor_VType::SENSOR_TYPE_BARO_ONLY, 1, 2));
      } else
      if (Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO == VType) {
        discoveryItems.push_back(DiscoveryItem(Sensor_VType::SENSOR_TYPE_TEMP_ONLY, 1, 0));
        discoveryItems.push_back(DiscoveryItem(Sensor_VType::SENSOR_TYPE_BARO_ONLY, 1, 2));
      } else {
        discoveryItems.push_back(DiscoveryItem(VType, valueCount, 0));
      }

      #  ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        addLog(LOG_LEVEL_DEBUG, strformat(F("MQTT : AutoDiscovery Default Plugin config (%d)"), discoveryItems.size()));
      }
      #  endif // ifndef BUILD_NO_DEBUG
    }
  }

  return discoveryItems.size() != orgLen; // Something added?
}

String MQTT_TaskValueUniqueName(const String& taskName,
                                const String& valueName) {
  String uniqueId = strformat(F("%s_%s_%s"), Settings.getHostname().c_str(), taskName.c_str(), valueName.c_str());

  uniqueId.toLowerCase();
  return uniqueId;
}

String MQTT_DiscoveryBuildValueTopic(const String            & topic,
                                     struct EventStruct       *event,
                                     uint8_t                   taskValueIndex,
                                     const __FlashStringHelper*deviceClass,
                                     const String            & uniqueId,
                                     const String            & elementId,
                                     const String            & valueName) {
  String tmpTopic(topic);

  #  if FEATURE_STRING_VARIABLES

  if (taskValueIndex >= VARS_PER_TASK) {
    parseValNameVariable(tmpTopic, valueName, false);
  } else
  #  endif // if FEATURE_STRING_VARIABLES
  {
    parseSingleControllerVariable(tmpTopic, event, taskValueIndex, false);
  }
  parseDeviceClassVariable(tmpTopic, deviceClass, false);
  parseUniqueIdVariable(tmpTopic, uniqueId, false);
  parseElementIdVariable(tmpTopic, elementId, false);
  parseControllerVariables(tmpTopic, event, false); // Replace this last

  while (tmpTopic.endsWith(F("/"))) {
    tmpTopic = tmpTopic.substring(0, tmpTopic.length() - 1);
  }

  return tmpTopic;
}

bool MQTT_DiscoveryPublish(controllerIndex_t ControllerIndex,
                           const String    & topic,
                           const String    & discoveryMessage,
                           taskIndex_t       x,
                           uint8_t           v,
                           bool              retainDsc) {
  bool result = false;

  // Send each discovery message separate
  result = MQTTpublish(ControllerIndex, INVALID_TASK_INDEX, topic.c_str(), discoveryMessage.c_str(), retainDsc);
  #  ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(F("MQTT : Discovery %d Task %d '%s': disc.topic: %s"),
                                     result, x + 1, getTaskDeviceName(x).c_str(), topic.c_str()));
    addLog(LOG_LEVEL_INFO, strformat(F("MQTT : Discovery payload: %s"),
                                     discoveryMessage.c_str()));
  }
  #  endif // ifndef BUILD_NO_DEBUG
  return result;
}

bool MQTT_DiscoveryPublishWithStatusAndSet(taskIndex_t               taskIndex,
                                           uint8_t                   taskValue,
                                           const String            & valueName,
                                           controllerIndex_t         ControllerIndex,
                                           ControllerSettingsStruct& ControllerSettings,
                                           const __FlashStringHelper*componentClass,
                                           String                    deviceClass,
                                           String                    unitOfMeasure,
                                           struct EventStruct       *event,
                                           const String              deviceElement,
                                           const String              stateClass,
                                           bool                      success,
                                           bool                      hasSet,
                                           bool                      hasIcon,
                                           const String            & elementName,
                                           const String            & elementId,
                                           bool                      sendTrigger) {
  if (!valueName.isEmpty()) {
    const String withSet   = hasSet ? F(",\"cmd_t\":\"~/set\"") : EMPTY_STRING;
    const String schema    = hasSet ? EMPTY_STRING : "\"schema\":\"basic\",";
    const String devOrIcon = hasIcon ? F("ic") : F("dev_cla");
    const String withUoM   = unitOfMeasure.isEmpty() ? EMPTY_STRING :
                             strformat(F(",\"unit_of_meas\":\"%s\""), unitOfMeasure.c_str());
    const String stateJson = stateClass.isEmpty() ? EMPTY_STRING :
                             strformat(F(",\"stat_cla\":\"%s\""),     stateClass.c_str());
    const String taskName  = makeHomeAssistantCompliantName(getTaskDeviceName(taskIndex));
    const String valName   = makeHomeAssistantCompliantName(valueName);
    const bool   retainDsc = ControllerSettings.mqtt_retainDiscovery();
    const String discoveryTopic(ControllerSettings.MqttAutoDiscoveryTopic);
    const String publishTopic(ControllerSettings.Publish);
    const String discoveryConfig(parseStringKeepCase(ControllerSettings.MqttAutoDiscoveryConfig, 1, '|'));

    const String uniqueId = elementName.isEmpty() ? MQTT_TaskValueUniqueName(taskName, valName)
                                                  : strformat(F("%s_%s"), elementId.c_str(), valName.c_str());
    const String publish = MQTT_DiscoveryBuildValueTopic(publishTopic,
                                                         event,
                                                         taskValue,
                                                         componentClass,
                                                         uniqueId,
                                                         elementId,
                                                         valName);
    const String discoveryUrl = MQTT_DiscoveryBuildValueTopic(discoveryTopic,
                                                              event,
                                                              taskValue,
                                                              componentClass,
                                                              uniqueId,
                                                              elementId,
                                                              valName);
    const String discoveryMessage = strformat(F("{\"~\":\"%s\",\"name\":\"%s %s\",\"uniq_id\":\"%s\",%s"
                                                "\"%s\":\"%s\"%s%s%s,\"stat_t\":\"~\""
                                                "%s}"), // deviceElement last
                                              publish.c_str(), taskName.c_str(), valName.c_str(), uniqueId.c_str(), schema.c_str(),
                                              devOrIcon.c_str(), deviceClass.c_str(), withUoM.c_str(), stateJson.c_str(), withSet.c_str(),
                                              deviceElement.c_str());
    const String triggerMessage = strformat(F("{\"atype\":\"trigger\",\"t\":\"%s\","
                                              "\"p\":\"device_automation\","
                                              "\"type\":\"button_short_press\"," // FIXME ?
                                              "\"stype\":\"switch_1\""           // FIXME ?
                                              "%s}"),                            // deviceElement is used to pass in the TriggerState
                                            publish.c_str(), deviceElement.c_str());

    return MQTT_DiscoveryPublish(ControllerIndex,
                                 discoveryConfig.isEmpty() ? concat(discoveryUrl, F("/config")) : concat(discoveryUrl, discoveryConfig),
                                 sendTrigger ? triggerMessage : discoveryMessage,
                                 taskIndex,
                                 taskValue,
                                 retainDsc);
  }
  return success;
}

String MQTT_DiscoveryHelperGetValueName(taskIndex_t   taskIndex,
                                        uint8_t       taskVarIndex,
                                        DiscoveryItem discoveryItem) {
  return
    #  if FEATURE_STRING_VARIABLES
    discoveryItem.varIndex >= VARS_PER_TASK ? discoveryItem.varName :
    #  endif // if FEATURE_STRING_VARIABLES
    getTaskValueName(taskIndex, taskVarIndex);
}

String MQTT_DiscoveryHelperGetValueUoM(taskIndex_t   taskIndex,
                                       uint8_t       taskVarIndex,
                                       DiscoveryItem discoveryItem,
                                       const String& defaultUoM) {
  return
  #  if FEATURE_TASKVALUE_UNIT_OF_MEASURE
    #   if FEATURE_STRING_VARIABLES
    discoveryItem.varIndex >= VARS_PER_TASK ? discoveryItem.uom :
    #   endif // if FEATURE_STRING_VARIABLES
    toUnitOfMeasureName(Cache.getTaskVarUnitOfMeasure(taskIndex, taskVarIndex), defaultUoM);
  #  else // FEATURE_TASKVALUE_UNIT_OF_MEASURE
    #   if FEATURE_STRING_VARIABLES
    discoveryItem.varIndex >= VARS_PER_TASK ? discoveryItem.uom :
    #   endif // if FEATURE_STRING_VARIABLES
    defaultUoM;
  #  endif // FEATURE_TASKVALUE_UNIT_OF_MEASURE
}

# endif // if FEATURE_MQTT_DISCOVER

#endif // if FEATURE_MQTT
