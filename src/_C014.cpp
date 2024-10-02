#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C014

# include "src/Commands/ExecuteCommand.h"
# include "src/DataTypes/NodeTypeID.h"
# include "src/Globals/Device.h"
# include "src/Globals/MQTT.h"
# include "src/Globals/Plugins.h"
# include "src/Globals/Statistics.h"
# include "src/Helpers/_CPlugin_Helper_mqtt.h"
# include "src/Helpers/PeriodicalActions.h"
# include "_Plugin_Helper.h"

// #######################################################################################################
// ################################# Controller Plugin 014: Homie 3/4 ####################################
// #######################################################################################################

/** Changelog:
 * 2024-03-02 tonhuisman: Fix using parseSystemVariables() for processing %sysname%. Might still break the same configurations,
 *                        logging improvements
 * 2023-10-30 tonhuisman: Fix using getHostname() instead of getName() for %sysname%. This might break some configurations!
 *                        minor improvements
 * 2023-08-18 tonhuisman: Clean up source to improve resource usage
 * 2023-03-15 tonhuisman: Replace use of deprecated DummyValueSet with TaskValueSet
 * 2023-03 Changelog started
 */

# define CPLUGIN_014
# define CPLUGIN_ID_014              14

// Define which Homie version to use
# if !defined(CPLUGIN_014_V3) && !defined(CPLUGIN_014_V4)

// #define CPLUGIN_014_V3
#  define CPLUGIN_014_V4
# endif // if !defined(CPLUGIN_014_V3) && !defined(CPLUGIN_014_V4)

# ifdef CPLUGIN_014_V3
  #  define CPLUGIN_014_HOMIE_VERSION   "3.0.0"
  #  define CPLUGIN_NAME_014            "Homie MQTT (Version 3.0.1)"
# endif // ifdef CPLUGIN_014_V3
# ifdef CPLUGIN_014_V4
  #  define CPLUGIN_014_HOMIE_VERSION   "4.0.0"
  #  define CPLUGIN_NAME_014            "Homie MQTT (Version 4.0.0 dev)"
# endif // ifdef CPLUGIN_014_V4

// subscribe and publish schemes should not be changed by the user. This will probably break the homie convention. Do @ your own risk;)
# define CPLUGIN_014_SUBSCRIBE       "homie/%sysname%/+/+/set" // only subscribe to /set topics to reduce load by receiving all retained
                                                               // messages
# define CPLUGIN_014_PUBLISH         "homie/%sysname%/%tskname%/%valname%"

# define CPLUGIN_014_BASE_TOPIC      "homie/%sysname%/#"
# define CPLUGIN_014_BASE_VALUE      "homie/%sysname%/%device%/%node%/%property%"
# define CPLUGIN_014_INTERVAL        "90"                  // to prevent timeout !ToDo set by lowest plugin interval
# define CPLUGIN_014_SYSTEM_DEVICE   "SYSTEM"              // name for system device Plugin for cmd and GIO values
# define CPLUGIN_014_CMD_VALUE       "cmd"                 // name for command value
# define CPLUGIN_014_GPIO_VALUE      "gpio"                // name for gpio value i.e. "gpio1"
# define CPLUGIN_014_GPIO_VALUE_LEN  4                     // length of GPIO to avoid creating a String to get the length
# define CPLUGIN_014_CMD_VALUE_NAME  "Command"             // human readabele name for command value

# define CPLUGIN_014_GPIO_COMMAND          "gpio"          // name for gpio command
# define CPLUGIN_014_TASKVALUESET_COMMAND  "taskvalueset"  // name for taskvalueset command
# define CPLUGIN_014_HOMIEVALUESET_COMMAND "homievalueset" // name for homievalueset command

uint8_t msgCounter = 0;                                    // counter for send Messages (currently for information / log only!

String CPlugin_014_pubname;
bool   CPlugin_014_mqtt_retainFlag = false;

void C014_replaceSysname(String& var) {
  parseSystemVariables(var, false); // Used to be getName(), but that doesn't include the UnitNr when configured
}

bool CPlugin_014_sendMQTTdevice(String                     tmppubname,
                                taskIndex_t                taskIndex,
                                const __FlashStringHelper *topic,
                                const String             & payload,
                                int                      & errorCounter) {
  tmppubname.replace(F("#"), topic);
  bool mqttReturn = MQTTpublish(CPLUGIN_ID_014, taskIndex, tmppubname.c_str(), payload.c_str(), true);

  if (mqttReturn) {
    msgCounter++;
  } else {
    errorCounter++;
  }

  String log;

  if (loglevelActiveFor(LOG_LEVEL_ERROR)) { // Also true for LOG_LEVEL_DEBUG_MORE
    log = strformat(F("C014 : T:%s P: %s"), String(topic).c_str(), payload.c_str());
  }
  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE) && mqttReturn) {
    log += F(" success!");
    addLogMove(LOG_LEVEL_DEBUG_MORE, log);
  }
  # endif // ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_ERROR) && !mqttReturn) {
    log += F(" ERROR!");
    addLogMove(LOG_LEVEL_ERROR, log);
  }
  processMQTTdelayQueue();
  return mqttReturn;
}

// send MQTT Message with CPLUGIN_014_BASE_TOPIC Topic scheme / Payload
bool CPlugin_014_sendMQTTdevice(const String             & tmppubname,
                                taskIndex_t                taskIndex,
                                const __FlashStringHelper *topic,
                                const __FlashStringHelper *payload,
                                int                      & errorCounter) {
  return CPlugin_014_sendMQTTdevice(tmppubname, taskIndex, topic, String(payload), errorCounter);
}

// send MQTT Message with CPLUGIN_014_BASE_VALUE Topic scheme / Payload
bool CPlugin_014_sendMQTTnode(String        tmppubname,
                              const String& node,
                              const String& value,
                              const String& topic,
                              const String& payload,
                              int         & errorCounter) {
  tmppubname.replace(F("%device%"),    node);
  tmppubname.replace(F("%node%"),      value);
  tmppubname.replace(F("/%property%"), topic); // leading forward slash required to send "homie/device/value" topics
  bool mqttReturn = MQTTpublish(CPLUGIN_ID_014, INVALID_TASK_INDEX, tmppubname.c_str(), payload.c_str(), true);

  if (mqttReturn) { msgCounter++; }
  else { errorCounter++; }

  String log;

  if (loglevelActiveFor(LOG_LEVEL_ERROR)) { // Also true for LOG_LEVEL_DEBUG_MORE
    log = strformat(F("C014 : V:%s T: %s P: %s"), value.c_str(), topic.c_str(), payload.c_str());
  }
  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE) && mqttReturn) {
    log += F(" success!");
    addLogMove(LOG_LEVEL_DEBUG_MORE, log);
  }
  # endif // ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_ERROR) && !mqttReturn) {
    log += F(" ERROR!");
    addLogMove(LOG_LEVEL_ERROR, log);
  }
  processMQTTdelayQueue();
  return mqttReturn;
}

// and String to a comma separated list
void C014_addToList(String& valuesList, const String& node) {
  if (valuesList.length() > 0) {
    valuesList += ',';
  }
  valuesList += node;
}

bool CPlugin_014(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool   success      = false;
  int    errorCounter = 0;
  String pubname;
  String tmppubname;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      ProtocolStruct& proto = getProtocolStruct(event->idx); //      = CPLUGIN_ID_014;
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
      string = F(CPLUGIN_NAME_014);
      break;
    }

    case CPlugin::Function::CPLUGIN_INIT:
    {
      success = init_mqtt_delay_queue(event->ControllerIndex, CPlugin_014_pubname, CPlugin_014_mqtt_retainFlag);
      break;
    }

    case CPlugin::Function::CPLUGIN_EXIT:
    {
      exit_mqtt_delay_queue();
      break;
    }

    case CPlugin::Function::CPLUGIN_INTERVAL:
    {
      if (MQTTclient.connected()) {
        errorCounter = 0;

        pubname = F(CPLUGIN_014_BASE_TOPIC); // Scheme to form device messages
        C014_replaceSysname(pubname);

        # ifdef CPLUGIN_014_V3

        // $stats/uptime	Device → Controller	Time elapsed in seconds since the boot of the device	Yes	Yes
        CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex, F("$stats/uptime"), toString(getUptimeMinutes() * 60, 0), errorCounter);

        // $stats/signal	Device → Controller	Signal strength in %	Yes	No
        float RssI = WiFi.RSSI();
        RssI = isnan(RssI) ? -100.0f : RssI;
        RssI = min(max(2 * (RssI + 100.0f), 0.0f), 100.0f);

        CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex, F("$stats/signal"), toString(RssI, 1), errorCounter);
        # endif // ifdef CPLUGIN_014_V3

        if (errorCounter > 0) {
          // alert: this is the state the device is when connected to the MQTT broker, but something wrong is happening. E.g. a sensor is
          // not providing data and needs human intervention. You have to send this message when something is wrong.
          CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex, F("$state"), F("alert"), errorCounter);
        } else {
          // ready: this is the state the device is in when it is connected to the MQTT broker, has sent all Homie messages and is ready to
          // operate. You have to send this message after all other announcements message have been sent.
          CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex, F("$state"), F("ready"), errorCounter);
          success = true;
        }

        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          addLog(LOG_LEVEL_DEBUG,
                 strformat(F("C014 : $stats information sent with %s errors! (%d messages)"),
                           errorCounter > 0 ? String(errorCounter).c_str() : "no",
                           msgCounter));
        }
        # endif // ifndef BUILD_NO_DEBUG
        msgCounter = 0;
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_GOT_CONNECTED: //// call after connected to mqtt server to publich device autodicover features
    {
      statusLED(true);

      // send autodiscover header
      pubname = F(CPLUGIN_014_BASE_TOPIC);           // Scheme to form device messages
      C014_replaceSysname(pubname);
      int deviceCount = 1;                           // minimum the SYSTEM device exists
      int nodeCount   = 1;                           // minimum the cmd node exists
      errorCounter = 0;

      if (lastBootCause != BOOT_CAUSE_DEEP_SLEEP) {  // skip sending autodiscover data when returning from deep sleep
        String nodename = F(CPLUGIN_014_BASE_VALUE); // Scheme to form node messages
        C014_replaceSysname(nodename);
        String nodesList;                            // build comma separated List for nodes
        String valuesList;                           // build comma separated List for values
        String deviceName;                           // current Device Name nr:name
        String valueName;                            // current Value Name
        String unitName;                             // estimate Units

        // init: this is the state the device is in when it is connected to the MQTT broker, but has not yet sent all Homie messages and is
        // not yet ready to operate. This is the first message that must that must be sent.
        CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex, F("$state"), F("init"),                    errorCounter);

        // $homie	Device → Controller	Version of the Homie convention the device conforms to	Yes	Yes
        CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex, F("$homie"), F(CPLUGIN_014_HOMIE_VERSION), errorCounter);

        // $name	Device → Controller	Friendly name of the device	Yes	Yes
        CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex, F("$name"),  Settings.getName(),           errorCounter);

        // $localip	Device → Controller	IP of the device on the local network	Yes	Yes
        # ifdef CPLUGIN_014_V3
        CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex, F("$localip"), formatIP(NetworkLocalIP()), errorCounter);

        // $mac	Device → Controller	Mac address of the device network interface. The format MUST be of the type A1:B2:C3:D4:E5:F6	Yes	Yes
        CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex, F("$mac"),     NetworkMacAddress(),        errorCounter);

        // $implementation	Device → Controller	An identifier for the Homie implementation (example esp8266)	Yes	Yes
        CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex, F("$implementation"),
                                   #  if defined(ESP8266)
                                   F("ESP8266"),
                                   #  endif // if defined(ESP8266)
                                   #  if defined(ESP32)
                                   F("ESP32"),
                                   #  endif // if defined(ESP32)
                                   errorCounter);

        // $fw/version	Device → Controller	Version of the firmware running on the device	Yes	Yes
        CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex, F("$fw/version"), toString(Settings.Build, 0),
                                   errorCounter);

        #  if FEATURE_ESPEASY_P2P

        // $fw/name	Device → Controller	Name of the firmware running on the device. Allowed characters are the same as the device ID	Yes	Yes
        CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex, F("$fw/name"), getNodeTypeDisplayString(NODE_TYPE_ID),
                                   errorCounter);
        #  endif // if FEATURE_ESPEASY_P2P

        // $stats/interval	Device → Controller	Interval in seconds at which the device refreshes its $stats/+: See next section for
        // details about statistical attributes	Yes	Yes
        CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex, F("$stats/interval"), F(CPLUGIN_014_INTERVAL),
                                   errorCounter);
        # endif // ifdef CPLUGIN_014_V3

        // always send the SYSTEM device with the cmd node
        C014_addToList(nodesList,  F(CPLUGIN_014_SYSTEM_DEVICE));
        C014_addToList(valuesList, F(CPLUGIN_014_CMD_VALUE));

        // $name	Device → Controller	Friendly name of the Node	Yes	Yes
        CPlugin_014_sendMQTTnode(nodename,
                                 F(CPLUGIN_014_SYSTEM_DEVICE),
                                 F("$name"),
                                 F(""),
                                 F(CPLUGIN_014_SYSTEM_DEVICE),
                                 errorCounter);

        // $name	Device → Controller	Friendly name of the property.	Any String	Yes	No ("")
        CPlugin_014_sendMQTTnode(nodename,
                                 F(CPLUGIN_014_SYSTEM_DEVICE),
                                 F(CPLUGIN_014_CMD_VALUE),
                                 F("/$name"),
                                 F(CPLUGIN_014_CMD_VALUE_NAME),
                                 errorCounter);

        // $datatype	The data type. See Payloads.	Enum: [integer, float, boolean, string, enum, color]
        CPlugin_014_sendMQTTnode(nodename,
                                 F(CPLUGIN_014_SYSTEM_DEVICE),
                                 F(CPLUGIN_014_CMD_VALUE),
                                 F("/$datatype"),
                                 F("string"),
                                 errorCounter);

        // $settable	Device → Controller	Specifies whether the property is settable (true) or readonly (false)	true or false	Yes	No
        // (false)
        CPlugin_014_sendMQTTnode(nodename,
                                 F(CPLUGIN_014_SYSTEM_DEVICE),
                                 F(CPLUGIN_014_CMD_VALUE),
                                 F("/$settable"),
                                 F("true"),
                                 errorCounter);

        // enum all devices

        // FIRST Standard GPIO tasks
        int gpio = 0;

        while (gpio <= MAX_GPIO) {
          const PinBootState pinBootState = Settings.getPinBootState(gpio);

          if (pinBootState != PinBootState::Default_state) { // anything but default
            nodeCount++;
            valueName = concat(F(CPLUGIN_014_GPIO_VALUE), gpio);
            C014_addToList(valuesList, valueName);

            // $name	Device → Controller	Friendly name of the property.	Any String	Yes	No ("")
            CPlugin_014_sendMQTTnode(nodename, F(CPLUGIN_014_SYSTEM_DEVICE), valueName, F("/$name"),     valueName,    errorCounter);

            // $datatype	The data type. See Payloads.	Enum: [integer, float, boolean,string, enum, color]
            CPlugin_014_sendMQTTnode(nodename, F(CPLUGIN_014_SYSTEM_DEVICE), valueName, F("/$datatype"), F("boolean"), errorCounter);

            if (pinBootState != PinBootState::Input) { // defined as output
              // $settable	Device → Controller	Specifies whether the property is settable (true) or readonly (false)	true or
              // false	Yes	No (false)
              CPlugin_014_sendMQTTnode(nodename, F(CPLUGIN_014_SYSTEM_DEVICE), valueName, F("/$settable"), F("true"), errorCounter);
            }
          }
          ++gpio;
        }

        // $properties	Device → Controller	Properties the node exposes, with format id separated by a , if there are multiple nodes.	Yes	Yes
        CPlugin_014_sendMQTTnode(nodename, F(CPLUGIN_014_SYSTEM_DEVICE), F("$properties"), EMPTY_STRING, valuesList, errorCounter);
        valuesList = EMPTY_STRING;
        deviceCount++;

        // SECOND Plugins
        for (taskIndex_t x = 0; x < TASKS_MAX; ++x) {
          const pluginID_t pluginID = Settings.getPluginID_for_task(x);

          if (validPluginID_fullcheck(pluginID)) {
            LoadTaskSettings(x);
            const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(x);

            deviceName = getTaskDeviceName(x);

            if (validDeviceIndex(DeviceIndex) && Settings.TaskDeviceEnabled[x]) { // Device is enabled so send information
                                                                                  // device enabled
              valuesList = EMPTY_STRING;

              const uint8_t valueCount = getValueCountForTask(x);

              if (!Device[DeviceIndex].SendDataOption) { // check if device is not sending data = assume that it can receive.
                constexpr pluginID_t HOMIE_RECEIVER_PLUGIN_ID(86);

                if (pluginID == HOMIE_RECEIVER_PLUGIN_ID) {
                  for (uint8_t varNr = 0; varNr < valueCount; ++varNr) {
                    if (validPluginID_fullcheck(Settings.getPluginID_for_task(x))) {
                      if (ExtraTaskSettings.TaskDeviceValueNames[varNr][0] != 0) { // do not send if Value Name is empty!
                        C014_addToList(valuesList, ExtraTaskSettings.TaskDeviceValueNames[varNr]);

                        // $settable	Device → Controller	Specifies whether the property is settable (true) or readonly (false)	true
                        // or false	Yes	No (false)
                        CPlugin_014_sendMQTTnode(nodename,
                                                 deviceName,
                                                 ExtraTaskSettings.TaskDeviceValueNames[varNr],
                                                 F("/$settable"),
                                                 F("true"),
                                                 errorCounter);

                        // $name	Device → Controller	Friendly name of the property.	Any String	Yes	No ("")
                        valueName  = F("Homie Receiver: ");
                        valueName += ExtraTaskSettings.TaskDeviceValueNames[varNr];
                        CPlugin_014_sendMQTTnode(nodename,
                                                 deviceName,
                                                 ExtraTaskSettings.TaskDeviceValueNames[varNr],
                                                 F("/$name"),
                                                 valueName,
                                                 errorCounter);

                        // $datatype	The data type. See Payloads.	Enum: [integer, float, boolean,string, enum, color]
                        unitName = EMPTY_STRING;

                        switch (Settings.TaskDevicePluginConfig[x][varNr]) {
                          case 0:
                            valueName = F("integer");

                            if ((ExtraTaskSettings.TaskDevicePluginConfig[varNr] != 0) ||
                                (ExtraTaskSettings.TaskDevicePluginConfig[varNr + 5] != 0)) {
                              unitName  = ExtraTaskSettings.TaskDevicePluginConfig[varNr];
                              unitName += ':';
                              unitName += ExtraTaskSettings.TaskDevicePluginConfig[varNr + valueCount];
                            }
                            break;
                          case 1:
                            valueName = F("float");

                            if ((ExtraTaskSettings.TaskDevicePluginConfig[varNr] != 0) ||
                                (ExtraTaskSettings.TaskDevicePluginConfig[varNr + 5] != 0)) {
                              unitName = strformat(F("%d:%d"),
                                                   ExtraTaskSettings.TaskDevicePluginConfig[varNr],
                                                   ExtraTaskSettings.TaskDevicePluginConfig[varNr + valueCount]);
                            }
                            break;
                          case 2: valueName = F("boolean"); break;
                          case 3: valueName = F("string"); break;
                          case 4:
                            valueName = F("enum");
                            unitName  = ExtraTaskSettings.TaskDeviceFormula[varNr];
                            break;
                          case 5:
                            valueName = F("color");
                            unitName  = F("rgb");
                            break;
                          case 6:
                            valueName = F("color");
                            unitName  = F("hsv");
                            break;
                        }
                        CPlugin_014_sendMQTTnode(nodename,
                                                 deviceName,
                                                 ExtraTaskSettings.TaskDeviceValueNames[varNr],
                                                 F("/$datatype"),
                                                 valueName,
                                                 errorCounter);

                        if (!unitName.isEmpty()) {
                          CPlugin_014_sendMQTTnode(nodename,
                                                   deviceName,
                                                   ExtraTaskSettings.TaskDeviceValueNames[varNr],
                                                   F("/$format"),
                                                   unitName,
                                                   errorCounter);
                        }
                        nodeCount++;
                      }
                    }
                  }
                }
              } else {
                // ignore cutom values for now! Assume all Values are standard float.
                // String customValuesStr;
                // customValues = PluginCall(PLUGIN_WEBFORM_SHOW_VALUES, &TempEvent, customValuesStr);
                uint8_t customValues = false;

                if (!customValues) { // standard Values
                  for (uint8_t varNr = 0; varNr < valueCount; ++varNr) {
                    const pluginID_t pluginID = Settings.getPluginID_for_task(x);

                    if (validPluginID_fullcheck(pluginID)) {
                      if (ExtraTaskSettings.TaskDeviceValueNames[varNr][0] != 0) { // do not send if Value Name is empty!
                        C014_addToList(valuesList, ExtraTaskSettings.TaskDeviceValueNames[varNr]);

                        // $name	Device → Controller	Friendly name of the property.	Any String	Yes	No ("")
                        CPlugin_014_sendMQTTnode(nodename,
                                                 deviceName,
                                                 ExtraTaskSettings.TaskDeviceValueNames[varNr],
                                                 F("/$name"),
                                                 ExtraTaskSettings.TaskDeviceValueNames[varNr],
                                                 errorCounter);

                        // $datatype	The data type. See Payloads.	Enum: [integer, float, boolean,string, enum, color]
                        CPlugin_014_sendMQTTnode(nodename,
                                                 deviceName,
                                                 ExtraTaskSettings.TaskDeviceValueNames[varNr],
                                                 F("/$datatype"),
                                                 F("float"),
                                                 errorCounter);

                        constexpr pluginID_t DUMMY_PLUGIN_ID(33);

                        if (pluginID == DUMMY_PLUGIN_ID) { // Dummy Device can send AND receive Data
                          CPlugin_014_sendMQTTnode(nodename,
                                                   deviceName,
                                                   ExtraTaskSettings.TaskDeviceValueNames[varNr],
                                                   F("/$settable"),
                                                   F("true"),
                                                   errorCounter);
                        }

                        nodeCount++;

                        /* TODO Fix units?
                           // because values in ESPEasy are unitless lets assume some units by the value name (still case sensitive)

                           if (strstr(ExtraTaskSettings.TaskDeviceValueNames[varNr], "temp") != nullptr)
                           {
                           unitName = F("°C");
                           } else if (strstr(ExtraTaskSettings.TaskDeviceValueNames[varNr], "humi") != nullptr)
                           {
                           unitName = F("%");
                           } else if (strstr(ExtraTaskSettings.TaskDeviceValueNames[varNr], "press") != nullptr)
                           {
                           unitName = F("Pa");
                           }                        // ToDo: .... and more

                           if (!unitName.isEmpty()) // found a unit match
                           {
                           // $unit	Device → Controller	A string containing the unit of this property. You
                           are not limited to the recommended values, although they are the only well known ones
                           that will have to be recognized by any Homie consumer.Recommended: Yes No
                            ("")
                           CPlugin_014_sendMQTTnode(nodename, deviceName,
                                                   ExtraTaskSettings.TaskDeviceValueNames[varNr], F("/$unit"), unitName,
                                                   errorCounter);
                           }
                           unitName = F("");
                         */
                      }
                    }
                  }      // end loop throug values
                } else { // Device has custom Values
                  # ifndef BUILD_NO_DEBUG

                  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
                    addLog(LOG_LEVEL_DEBUG, strformat(F("C014 : Device has custom values: %s not implemented!"),
                                                      getPluginNameFromDeviceIndex(getDeviceIndex_from_TaskIndex(x)).c_str()));
                  }
                  # endif // ifndef BUILD_NO_DEBUG
                }
              }

              if (!valuesList.isEmpty()) {
                // only add device to list if it has nodes!
                // $name	Device → Controller	Friendly name of the Node	Yes	Yes
                CPlugin_014_sendMQTTnode(nodename,
                                         deviceName,
                                         F("$name"),
                                         F(""),
                                         getTaskDeviceName(x),
                                         errorCounter);

                // $type	Device → Controller	Type of the node	Yes	Yes
                CPlugin_014_sendMQTTnode(nodename,
                                         deviceName,
                                         F("$type"),
                                         F(""),
                                         getPluginNameFromDeviceIndex(DeviceIndex),
                                         errorCounter);

                // add device to device list
                C014_addToList(nodesList, deviceName);
                deviceCount++;

                // $properties	Device → Controller	Properties the node exposes, with format id separated by a , if there are multiple
                // nodes.	Yes	Yes
                CPlugin_014_sendMQTTnode(nodename,
                                         deviceName,
                                         F("$properties"),
                                         F(""),
                                         valuesList,
                                         errorCounter);
                valuesList = EMPTY_STRING;
              }
            } else { // device not enabeled
              # ifndef BUILD_NO_DEBUG

              if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
                addLog(LOG_LEVEL_DEBUG, strformat(F("C014 : Device Disabled: %s not propagated!"),
                                                  getPluginNameFromDeviceIndex(getDeviceIndex_from_TaskIndex(x)).c_str()));
              }
              # endif // ifndef BUILD_NO_DEBUG
            }
          } // device configured
        }   // loop through devices

        // and finally ...
        // $nodes	Device → Controller	Nodes the device exposes, with format id separated by a , if there are multiple nodes. To
        // make a node an array, append [] to the ID.	Yes	Yes
        CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex, F("$nodes"), nodesList, errorCounter);
      }

      if (errorCounter > 0) {
        // alert: this is the state the device is when connected to the MQTT broker, but something wrong is happening. E.g. a sensor is not
        // providing data and needs human intervention. You have to send this message when something is wrong.
        CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex, F("$state"), F("alert"), errorCounter);
      } else {
        // ready: this is the state the device is in when it is connected to the MQTT broker, has sent all Homie messages and is ready to
        // operate. You have to send this message after all other announcements message have been sent.
        CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex, F("$state"), F("ready"), errorCounter);
        success = true;
      }

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLog(LOG_LEVEL_INFO,
               strformat(F("C014 : autodiscover information of %d Devices and %d Nodes sent with %s errors! (%d messages)"),
                         deviceCount,
                         nodeCount,
                         errorCounter > 0 ? String(errorCounter).c_str() : "no",
                         msgCounter)
               );
      }
      msgCounter   = 0;
      errorCounter = 0;
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_TEMPLATE:
    {
      event->String1 = F(CPLUGIN_014_SUBSCRIBE);
      event->String2 = F(CPLUGIN_014_PUBLISH);
      break;
    }

    case CPlugin::Function::CPLUGIN_GOT_INVALID:
    {
      pubname = F(CPLUGIN_014_BASE_TOPIC); // Scheme to form device messages
      C014_replaceSysname(pubname);

      // disconnected: this is the state the device is in when it is cleanly disconnected from the MQTT broker. You must send this message
      // before cleanly disconnecting
      success = CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex, F("$state"), F("disconnected"), errorCounter);

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = strformat(F("C014 : Device: %s got invalid (disconnect%s"),
                               Settings.getHostname().c_str(), String(success ? F("ed).") : F(") failed!")).c_str());
        addLogMove(LOG_LEVEL_INFO, log);
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_FLUSH:
    {
      pubname = F(CPLUGIN_014_BASE_TOPIC); // Scheme to form device messages
      C014_replaceSysname(pubname);

      // sleeping: this is the state the device is in when the device is sleeping. You have to send this message before sleeping.
      success = CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex, F("$state"), F("sleeping"), errorCounter);
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_RECV:
    {
      const controllerIndex_t ControllerID = findFirstEnabledControllerWithId(CPLUGIN_ID_014);
      bool validTopic                      = false;
      bool cmdExecuted                     = false;

      if (!validControllerIndex(ControllerID)) {
        // Controller is not enabled.
        break;
      } else {
        String cmd;
        int    lastindex = event->String1.lastIndexOf('/');
        errorCounter = 0;

        if (equals(event->String1.substring(lastindex + 1), F("set"))) {
          pubname   = event->String1.substring(0, lastindex);
          lastindex = pubname.lastIndexOf('/');
          String nodeName        = pubname.substring(0, lastindex);
          const String valueName = pubname.substring(lastindex + 1);
          lastindex = nodeName.lastIndexOf('/');
          nodeName  = nodeName.substring(lastindex + 1);

          String log;

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            log = strformat(F("C014 : MQTT received: /set: N: %s V: %s"), nodeName.c_str(), valueName.c_str());
          }

          if (equals(nodeName, F(CPLUGIN_014_SYSTEM_DEVICE))) {                    // msg to a system device
            if (valueName.startsWith(F(CPLUGIN_014_GPIO_VALUE))) {                 // msg to to set gpio values
              constexpr size_t gpio_value_tag_length = CPLUGIN_014_GPIO_VALUE_LEN; // now uses fixed length or constexpr

              // get the GPIO
              // Homie spec says state should be 'true' or 'false'...
              cmd = strformat(F("GPIO,%d,%c"),
                              valueName.substring(gpio_value_tag_length).toInt(),
                              (equals(event->String2, F("true")) || equals(event->String2, '1')) ? '1' : '0');
              validTopic = true;
            } else if (equals(valueName, F(CPLUGIN_014_CMD_VALUE))) { // msg to send a command
              cmd        = event->String2;
              validTopic = true;
            } else {
              cmd = strformat(F("SYSTEM/%s unknown!"), valueName.c_str());
            }
          } else {              // msg to a receiving plugin
            // Only handle /set case that supports P033 Dummy device and P086 Homie receiver
            cmdExecuted = MQTT_handle_topic_commands(event, false, true);
            validTopic  = true; // Avoid error messages
          }

          if (validTopic) {
            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              log += strformat(F(" cmd: %s OK"), cmd.c_str());
              addLog(LOG_LEVEL_INFO, log);
            }
          } else if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            log += F(" INVALID MSG");
            addLog(LOG_LEVEL_INFO, log);
          }
        }

        if (validTopic && !cmdExecuted) { // Don't execute twice
          MQTT_execute_command(cmd, true);
        }
      }
      success = validTopic;
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      if (MQTT_queueFull(event->ControllerIndex)) {
        break;
      }

      success = MQTT_protocol_send(event, CPlugin_014_pubname, CPlugin_014_mqtt_retainFlag);

      break;
    }

    case CPlugin::Function::CPLUGIN_ACKNOWLEDGE:
    {
      /* if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
         String log = F("CPLUGIN_ACKNOWLEDGE: ");
         log += string;
         log += F(" / ");
         log += getTaskDeviceName(event->TaskIndex);
         log += F(" / ");
         log += ExtraTaskSettings.TaskDeviceValueNames[event->Par2-1];
         log += F(" sensorType:");
         log += event->sensorType;
         log += F(" Source:");
         log += event->Source;
         log += F(" idx:");
         log += event->idx;
         log += F(" S1:");
         log += event->String1;
         log += F(" S2:");
         log += event->String2;
         log += F(" S3:");
         log += event->String3;
         log += F(" S4:");
         log += event->String4;
         log += F(" S5:");
         log += event->String5;
         log += F(" P1:");
         log += event->Par1;
         log += F(" P2:");
         log += event->Par2;
         log += F(" P3:");
         log += event->Par3;
         log += F(" P4:");
         log += event->Par4;
         log += F(" P5:");
         log += event->Par5;
         addLog(LOG_LEVEL_DEBUG, log);
         } */

      if (!string.isEmpty()) {
        const String commandName = parseString(string, 1);    // could not find a way to get the command out of the event structure.

        if (equals(commandName, F(CPLUGIN_014_GPIO_COMMAND))) // !ToDo : As gpio is like any other plugin commands should be integrated
                                                              // below!
        {
          const int port         = event->Par1;               // parseString(string, 2).toInt();
          const int valueInt     = event->Par2;               // parseString(string, 3).toInt();
          const String valueBool = boolToString(valueInt == 1);

          String topic = F(CPLUGIN_014_PUBLISH);              // ControllerSettings.Publish not used because it can be modified by the user!
          C014_replaceSysname(topic);
          topic.replace(F("%tskname%"), F(CPLUGIN_014_SYSTEM_DEVICE));
          topic.replace(F("%valname%"), concat(F(CPLUGIN_014_GPIO_VALUE), port));

          success = MQTTpublish(CPLUGIN_ID_014, INVALID_TASK_INDEX, topic.c_str(), valueBool.c_str(), false);

          String log = strformat(F("C014 : Acknowledged GPIO%d value:%s (%d)"),
                                 port, valueBool.c_str(), valueInt);

          if (loglevelActiveFor(LOG_LEVEL_INFO) && success) {
            log += F(" success!");
            addLogMove(LOG_LEVEL_INFO, log);
          }

          if (loglevelActiveFor(LOG_LEVEL_ERROR) && !success) {
            log += F(" ERROR!");
            addLogMove(LOG_LEVEL_ERROR, log);
          }
        } else { // not gpio
          const taskVarIndex_t taskVarIndex = event->Par2 - 1;

          if (validTaskVarIndex(taskVarIndex)) {
            userVarIndex_t userVarIndex = event->BaseVarIndex + taskVarIndex;
            String topic                = F(CPLUGIN_014_PUBLISH);
            C014_replaceSysname(topic);
            const int deviceIndex = event->Par1; // parseString(string, 2).toInt();
            LoadTaskSettings(deviceIndex - 1);
            const String deviceName = getTaskDeviceName(event->TaskIndex);
            topic.replace(F("%tskname%"), deviceName);
            const String valueName = ExtraTaskSettings.TaskDeviceValueNames[taskVarIndex]; // parseString(string, 3).toInt()-1];
            topic.replace(F("%valname%"), valueName);
            String valueStr;
            int    valueInt = 0;

            if (equals(commandName, F(CPLUGIN_014_TASKVALUESET_COMMAND))) { // removed dummyvalueset command some time ago...
              valueStr = formatUserVarNoCheck(event, taskVarIndex);         // parseString(string, 4);
              success  = MQTTpublish(CPLUGIN_ID_014, INVALID_TASK_INDEX, topic.c_str(), valueStr.c_str(), false);

              String log = strformat(F("C014 : Acknowledged: %s var: %s topic: %s value: %s"),
                                     deviceName.c_str(), valueName.c_str(), topic.c_str(), valueStr.c_str());

              if (loglevelActiveFor(LOG_LEVEL_INFO) && success) {
                log += F(" success!");
                addLogMove(LOG_LEVEL_INFO, log);
              }

              if (loglevelActiveFor(LOG_LEVEL_ERROR) && !success) {
                log += F(" ERROR!");
                addLogMove(LOG_LEVEL_ERROR, log);
              }
            } else if (equals(commandName, F(CPLUGIN_014_HOMIEVALUESET_COMMAND))) { // acknowledges value form P086 Homie Receiver
              switch (Settings.TaskDevicePluginConfig[deviceIndex - 1][taskVarIndex]) {
                case 0:                                                             // PLUGIN_085_VALUE_INTEGER
                  valueInt = static_cast<int>(UserVar[userVarIndex]);
                  valueStr = toString(UserVar[userVarIndex], 0);
                  break;
                case 1: // PLUGIN_085_VALUE_FLOAT
                  valueStr = formatUserVarNoCheck(event, taskVarIndex);
                  break;
                case 2: // PLUGIN_085_VALUE_BOOLEAN

                  valueStr = boolToString(UserVar[userVarIndex] == 1);
                  break;
                case 3: // PLUGIN_085_VALUE_STRING
                  // valueStr = ExtraTaskSettings.TaskDeviceFormula[taskVarIndex];
                  valueStr = parseStringToEndKeepCase(string, 4);
                  break;
                case 4: // PLUGIN_085_VALUE_ENUM
                  valueInt = static_cast<int>(UserVar[userVarIndex]);
                  valueStr = parseStringKeepCase(ExtraTaskSettings.TaskDeviceFormula[taskVarIndex], valueInt);
                  break;
                case 5: // PLUGIN_085_VALUE_RGB
                  // valueStr = ExtraTaskSettings.TaskDeviceFormula[taskVarIndex];
                  valueStr = parseStringToEnd(string, 4);
                  break;
                case 6: // PLUGIN_085_VALUE_HSV
                  // valueStr = ExtraTaskSettings.TaskDeviceFormula[taskVarIndex];
                  valueStr = parseStringToEnd(string, 4);
                  break;
              }
              success = MQTTpublish(CPLUGIN_ID_014, INVALID_TASK_INDEX, topic.c_str(), valueStr.c_str(), false);

              String log = concat(F("C014 : homie acknowledge: "), deviceName);

              if (loglevelActiveFor(LOG_LEVEL_INFO) && success) {
                log += strformat(F(" taskIndex:%d valueNr:%d valueName:%s valueType:%d topic:%s valueInt:%d valueStr:%s success!"),
                                 deviceIndex,
                                 event->Par2,
                                 valueName.c_str(),
                                 Settings.TaskDevicePluginConfig[deviceIndex - 1][taskVarIndex],
                                 topic.c_str(),
                                 valueInt,
                                 valueStr.c_str());
                addLogMove(LOG_LEVEL_INFO, log);
              }

              if (loglevelActiveFor(LOG_LEVEL_ERROR) && !success) {
                log += strformat(F(" var: %s topic: %s value: %s ERROR!"), // was: failed!
                                 valueName.c_str(), topic.c_str(), valueStr.c_str());
                addLogMove(LOG_LEVEL_ERROR, log);
              }
            } else { // Acknowledge not implemented yet
              /* if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                 String log = F("C014 : Plugin acknowledged: ");
                 log+=function;
                 log+=F(" / ");
                 log+=commandName;
                 log+=F(" cmd: ");
                 log+=string;
                 log+=F(" not implemented!");
                 addLog(LOG_LEVEL_ERROR, log);
                 } */
              success = false;
            }
          }
        }
      }
      break;
    }

    default:
      break;
  }

  return success;
}

#endif // ifdef USES_C014
