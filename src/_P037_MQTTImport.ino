#include "_Plugin_Helper.h"
#ifdef USES_P037

// #######################################################################################################
// #################################### Plugin 037: MQTT Import ##########################################
// #######################################################################################################


// Original plugin created by Namirda

// This task reads data from the MQTT Import input stream and saves the value

/**
 * 2021-10-23, tonhuisman: Fix stability issues when parsing JSON payloads
 * 2021-10-18, tonhuisman: Add Global topic-prefix to accomodate long topics (with a generic prefix)
 *                         (See forum request: https://www.letscontrolit.com/forum/viewtopic.php?f=6&t=8800)
 * 2021-10, tonhuisman   : Refactoring to reduce memory use so the plugin doesn't crash during saving of settings
 *                         SETTINGS NOW INCOMPATIBLE WITH PREVIOUS PR BUILDS, BUT STILL COMPATIBLE WITH ORIGINAL PLUGIN!
 * 2021-02-13, tonhuisman: Refactoring to reduce memory use and String re-allocations
 * 2020-12-10, tonhuisman: Add name-value mapping, filtering and json parsing
 * 2020-12-17, tonhuisman: Bugfixes, filter per MQTT Topic, reorganized Device page
 */

# include "src/PluginStructs/P037_data_struct.h"

# define PLUGIN_037
# define PLUGIN_ID_037         37
# define PLUGIN_NAME_037       "Generic - MQTT Import"

# define PLUGIN_VALUENAME1_037 "Value1"
# define PLUGIN_VALUENAME2_037 "Value2"
# define PLUGIN_VALUENAME3_037 "Value3"
# define PLUGIN_VALUENAME4_037 "Value4"


# define P037_PARSE_JSON      PCONFIG(1) // Parse/process json messages
# define P037_APPLY_MAPPINGS  PCONFIG(2) // Apply mapping strings to numbers
# define P037_APPLY_FILTERS   PCONFIG(3) // Apply filtering on data values
# define P037_SEND_EVENTS     PCONFIG(4) // Send event for each received topic

# if P037_MAPPING_SUPPORT || P037_JSON_SUPPORT
String P037_getMQTTLastTopicPart(const String& topic) {
  const int16_t lastSlash = topic.lastIndexOf('/');

  if (lastSlash >= static_cast<int16_t>(topic.length() - 1)) {
    return F("");
  }
  String result = topic.substring(lastSlash + 1); // Take last part of the topic

  result.trim();
  return result;
}

# endif // if P037_MAPPING_SUPPORT || P037_JSON_SUPPORT

boolean Plugin_037(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_037;
      Device[deviceCount].Type               = DEVICE_TYPE_DUMMY;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE; // This means it has a single pin
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true; // Need this in order to get the decimals option
      Device[deviceCount].ValueCount         = VARS_PER_TASK;
      Device[deviceCount].SendDataOption     = false;
      Device[deviceCount].TimerOption        = false;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_037);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_037));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_037));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_037));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_037));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormSubHeader(F("Options"));
      # if P037_MAPPING_SUPPORT || P037_JSON_SUPPORT || P037_FILTER_SUPPORT
      const __FlashStringHelper *optionsNoYes[2] = { F("No"), F("Yes") };
      int optionValuesNoYes[2]                   = { 0, 1 };
      # endif // if P037_MAPPING_SUPPORT || P037_JSON_SUPPORT || P037_FILTER_SUPPORT

      # if P037_JSON_SUPPORT
      addFormSelector(F("Parse JSON messages"), F("p037_parse_json"),     2, optionsNoYes, optionValuesNoYes, P037_PARSE_JSON,     true);
      # endif // if P037_JSON_SUPPORT
      # if P037_FILTER_SUPPORT
      addFormSelector(F("Apply filters"),       F("p037_apply_filters"),  2, optionsNoYes, optionValuesNoYes, P037_APPLY_FILTERS,  true);
      # endif // if P037_FILTER_SUPPORT
      # if P037_MAPPING_SUPPORT
      addFormSelector(F("Apply mappings"),      F("p037_apply_mappings"), 2, optionsNoYes, optionValuesNoYes, P037_APPLY_MAPPINGS, true);
      # endif // if P037_MAPPING_SUPPORT
      # if P037_MAPPING_SUPPORT || P037_JSON_SUPPORT || P037_FILTER_SUPPORT
      #  if !defined(LIMIT_BUILD_SIZE)
      addFormNote(F("Changing a Yes/No option will reload the page. Changing to No will clear corresponding settings!"));
      #  endif // if !defined(LIMIT_BUILD_SIZE)
      # endif  // if P037_MAPPING_SUPPORT || P037_JSON_SUPPORT || P037_FILTER_SUPPORT
      addFormCheckBox(F("Generate events for accepted topics"),
                      F("p037_send_events"), P037_SEND_EVENTS);
      # if !defined(LIMIT_BUILD_SIZE)
      addFormNote(F("Event: &lt;TaskName&gt;#&lt;topic&gt;=&lt;payload&gt;"));
      #  if P037_JSON_SUPPORT
      addFormNote(F("Events when JSON enabled and JSON payload: &lt;Topic&gt;#&lt;json-attribute&gt;=&lt;value&gt;"));
      #  endif // if P037_JSON_SUPPORT
      # endif  // if !defined(LIMIT_BUILD_SIZE)

      P037_data_struct *P037_data = new (std::nothrow) P037_data_struct(event->TaskIndex);

      if (nullptr == P037_data) {
        return success;
      }
      success = P037_data->webform_load(
        # if P037_MAPPING_SUPPORT
        P037_APPLY_MAPPINGS
        # endif // if P037_MAPPING_SUPPORT
        # if P037_MAPPING_SUPPORT && P037_FILTER_SUPPORT
        ,
        # endif // if P037_MAPPING_SUPPORT && P037_FILTER_SUPPORT
        # if P037_FILTER_SUPPORT
        P037_APPLY_FILTERS
        # endif // if P037_FILTER_SUPPORT
        # if (P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT) && P037_JSON_SUPPORT
        ,
        # endif // if (P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT) && P037_JSON_SUPPORT
        # if P037_JSON_SUPPORT
        P037_PARSE_JSON
        # endif // if P037_JSON_SUPPORT
        );
      delete P037_data;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      # if P037_JSON_SUPPORT
      P037_PARSE_JSON = getFormItemInt(F("p037_parse_json"));
      # endif // if P037_JSON_SUPPORT
      # if P037_MAPPING_SUPPORT
      P037_APPLY_MAPPINGS = getFormItemInt(F("p037_apply_mappings"));
      # endif // if P037_MAPPING_SUPPORT
      # if P037_FILTER_SUPPORT
      P037_APPLY_FILTERS = getFormItemInt(F("p037_apply_filters"));
      # endif // if P037_FILTER_SUPPORT
      P037_SEND_EVENTS = isFormItemChecked(F("p037_send_events")) ? 1 : 0;

      P037_data_struct *P037_data = new (std::nothrow) P037_data_struct(event->TaskIndex);

      if (nullptr == P037_data) {
        return success;
      }
      success = P037_data->webform_save(
        # if P037_FILTER_SUPPORT
        P037_APPLY_FILTERS
        # endif // if P037_FILTER_SUPPORT
        # if P037_FILTER_SUPPORT && P037_JSON_SUPPORT
        ,
        # endif // if P037_FILTER_SUPPORT && P037_JSON_SUPPORT
        # if P037_JSON_SUPPORT
        P037_PARSE_JSON
        # endif // if P037_JSON_SUPPORT
        );
      delete P037_data;

      break;
    }

    case PLUGIN_INIT:
    {
      success = false;
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P037_data_struct(event->TaskIndex));

      // When we edit the subscription data from the webserver, the plugin is called again with init.
      // In order to resubscribe we have to disconnect and reconnect in order to get rid of any obsolete subscriptions
      if (MQTTclient_connected) {
        // Subscribe to ALL the topics from ALL instance of this import module
        MQTTSubscribe_037(event);
        success = true;
      }
      break;
    }

    case PLUGIN_EXIT:
    {
      P037_data_struct *P037_data = static_cast<P037_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P037_data) {
        return success;
      }

      delete P037_data;

      break;
    }


    case PLUGIN_READ:
    {
      // This routine does not output any data and so we do not need to respond to regular read requests

      success = false;
      break;
    }

    case PLUGIN_MQTT_CONNECTION_STATE:
    {
      const bool currentConnectedState = event->Par1 == 1;

      if (P037_MQTTImport_connected != currentConnectedState) {
        P037_MQTTImport_connected = currentConnectedState;

        if (Settings.UseRules) {
          eventQueue.add(currentConnectedState ? F("MQTTimport#Connected") : F("MQTTimport#Disconnected"));
        }
      }

      if (currentConnectedState) {
        success = MQTTSubscribe_037(event);
      }
      break;
    }

    case PLUGIN_MQTT_IMPORT:
    {
      LoadTaskSettings(event->TaskIndex);

      // Resolved tonhuisman: TD-er: It may be useful to generate events with string values.
      // Get the payload and check it out
      String Payload = event->String2;
      String unparsedPayload; // To keep an unprocessed copy

      # ifdef PLUGIN_037_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String info = F("P037 : topic: ");
        info += event->String1;
        info += F(" value: ");
        info += Payload;
        addLog(LOG_LEVEL_INFO, info);
      }
      # endif // ifdef PLUGIN_037_DEBUG

      P037_data_struct *P037_data = static_cast<P037_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P037_data) {
        return success;
      }

      bool checkJson = false;

      String subscriptionTopicParsed;
      # if P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT || P037_JSON_SUPPORT
      bool processData = false;  // Don't do the for loop again if we're not going to match
      // As we can receive quite a lot of topics not intended for this plugin,
      // first do a quick check if the topic matches here, to try and avoid a bunch of unneeded mapping, filtering and logging
      bool matchedTopic = false; // Ignore by default
      subscriptionTopicParsed.reserve(80);

      for (uint8_t x = 0; x < VARS_PER_TASK; x++)
      {
        if (P037_data->deviceTemplate[x].length() == 0) {
          continue; // skip blank subscriptions
        }

        // Now check if the incoming topic matches one of our subscriptions
        subscriptionTopicParsed += P037_data->StoredSettings.globalTopicPrefix;
        subscriptionTopicParsed.trim();
        subscriptionTopicParsed += P037_data->deviceTemplate[x];
        parseSystemVariables(subscriptionTopicParsed, false);

        if (MQTTCheckSubscription_037(event->String1, subscriptionTopicParsed)) {
          matchedTopic = true; // Yes we should process it here
          processData  = true; // Allow going into second for loop
        }
      }
      # else // if P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT || P037_JSON_SUPPORT
      bool processData = true;
      # endif // if P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT || P037_JSON_SUPPORT
      # if P037_JSON_SUPPORT

      if (matchedTopic &&
          P037_PARSE_JSON &&
          Payload.startsWith(F("{"))) { // With JSON enabled and rudimentary check for JSon content
        #  ifdef PLUGIN_037_DEBUG
        addLog(LOG_LEVEL_INFO, F("IMPT : MQTT JSON data detected."));
        #  endif // ifdef PLUGIN_037_DEBUG
        checkJson = true;
      }
      # endif           // if P037_JSON_SUPPORT

      if (!checkJson) { // Avoid storing any json in an extra copy in memory
        unparsedPayload = event->String2;
      }

      bool   continueProcessing = false;
      String key;

      # if P037_MAPPING_SUPPORT

      if (matchedTopic && !checkJson && P037_APPLY_MAPPINGS) { // Apply mappings?
        key     = P037_getMQTTLastTopicPart(event->String1);
        Payload = P037_data->mapValue(Payload, key);
      }
      # endif // if P037_MAPPING_SUPPORT

      # if P037_JSON_SUPPORT

      if (checkJson) {
        continueProcessing = P037_data->parseJSONMessage(event->String2);
      }
      # endif // if P037_JSON_SUPPORT

      # if P037_FILTER_SUPPORT
      #  ifdef P037_FILTER_PER_TOPIC

      for (uint8_t x = 0; x < VARS_PER_TASK && matchedTopic; x++) {
        if (P037_data->deviceTemplate[x].length() == 0) {
          continue; // skip blank subscriptions
        }
      #  else // ifdef P037_FILTER_PER_TOPIC
      int8_t x = -1;

      if (matchedTopic) {
      #  endif // P037_FILTER_PER_TOPIC

        // non-json filter check
        if (!checkJson && P037_data->hasFilters()) { // See if we pass the filters
          key = P037_getMQTTLastTopicPart(event->String1);
          #  if P037_MAPPING_SUPPORT

          if (P037_APPLY_MAPPINGS) {
            Payload = P037_data->mapValue(Payload, key);
          }
          #  endif // if P037_MAPPING_SUPPORT
          processData = P037_data->checkFilters(key, Payload, x + 1); // Will return true unless key matches *and* Payload doesn't
        }
        #  if P037_JSON_SUPPORT

        #   ifndef P037_FILTER_PER_TOPIC

        // json filter check
        if (checkJson && P037_data->hasFilters()) { // See if we pass the filters for all json attributes
          do {
            key     = P037_data->iter->key().c_str();
            Payload = P037_data->iter->value().as<String>();
            #    if P037_MAPPING_SUPPORT

            if (P037_APPLY_MAPPINGS) {
              Payload = P037_data->mapValue(Payload, key);
            }
            #    endif // if P037_MAPPING_SUPPORT
            processData = P037_data->checkFilters(key, Payload, x + 1); // Will return true unless key matches *and* Payload doesn't
            ++P037_data->iter;
          } while (processData && P037_data->iter != P037_data->doc.end());
        }
        #   endif // P037_FILTER_PER_TOPIC
        #  endif  // if P037_JSON_SUPPORT
      }

      if (matchedTopic && P037_data->hasFilters() && // Single log statement
          loglevelActiveFor(LOG_LEVEL_DEBUG)) {      // Reduce standard logging
        String log = F("IMPT : MQTT filter result: ");
        log += processData ? F("true") : F("false");
        addLogMove(LOG_LEVEL_DEBUG, log);
      }
      # endif // if P037_FILTER_SUPPORT

      if (!processData) { // Nothing to do? then clean up
        Payload.clear();
        unparsedPayload.clear();
      }

      // Get the Topic and see if it matches any of the subscriptions
      for (uint8_t x = 0; x < VARS_PER_TASK && processData; x++)
      {
        if (P037_data->deviceTemplate[x].length() == 0) {
          continue; // skip blank subscriptions
        }

        // Now check if the incoming topic matches one of our subscriptions
        subscriptionTopicParsed.reserve(80);
        subscriptionTopicParsed = P037_data->StoredSettings.globalTopicPrefix;
        subscriptionTopicParsed.trim();
        subscriptionTopicParsed += P037_data->deviceTemplate[x];
        parseSystemVariables(subscriptionTopicParsed, false);

        if (MQTTCheckSubscription_037(event->String1, subscriptionTopicParsed)) {
          # if P037_JSON_SUPPORT
          #  ifdef P037_FILTER_PER_TOPIC

          // json filter check
          bool passFilter = true;

          if (checkJson && P037_data->hasFilters()) { // See if we pass the filters for all json attributes
            P037_data->iter = P037_data->doc.begin();

            do {
              key     = P037_data->iter->key().c_str();
              Payload = P037_data->iter->value().as<String>();
              #   if P037_MAPPING_SUPPORT

              if (P037_APPLY_MAPPINGS) {
                Payload = P037_data->mapValue(Payload, key);
              }
              #   endif // if P037_MAPPING_SUPPORT
              passFilter = P037_data->checkFilters(key, Payload, x + 1); // Will return true unless key matches *and* Payload doesn't

              ++P037_data->iter;
            } while (passFilter && P037_data->iter != P037_data->doc.end());
            P037_data->iter = P037_data->doc.begin();
          }

          if (passFilter) // Watch it!
          #  endif // P037_FILTER_PER_TOPIC
          # endif // if P037_JSON_SUPPORT
          {
            do {
              # if P037_JSON_SUPPORT

              if (checkJson && (P037_data->iter != P037_data->doc.end())) {
                String jsonIndex     = parseString(P037_data->StoredSettings.jsonAttributes[x], 2, ';');
                String jsonAttribute = parseStringKeepCase(P037_data->StoredSettings.jsonAttributes[x], 1, ';');
                jsonAttribute.trim();

                if (!jsonAttribute.isEmpty()) {
                  key             = jsonAttribute;
                  Payload         = P037_data->doc[key].as<String>();
                  unparsedPayload = Payload;
                  int8_t jIndex = jsonIndex.toInt();

                  if (jIndex > 1) {
                    Payload = parseString(Payload, jIndex, ';');
                  }

                  #  if !defined(LIMIT_BUILD_SIZE) || defined(P037_OVERRIDE)

                  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                    String log = F("IMPT : MQTT fetched json attribute: ");
                    log.reserve(48);
                    log += key;
                    log += F(" payload: ");
                    log += Payload;

                    if (!jsonIndex.isEmpty()) {
                      log += F(" index: ");
                      log += jsonIndex;
                    }
                    addLogMove(LOG_LEVEL_INFO, log);
                  }
                  #  endif // if !defined(LIMIT_BUILD_SIZE) || defined(P037_OVERRIDE)
                  continueProcessing = false; // no need to loop over all attributes, the configured one is found
                } else {
                  key             = P037_data->iter->key().c_str();
                  Payload         = P037_data->iter->value().as<String>();
                  unparsedPayload = Payload;
                }
                #  ifdef PLUGIN_037_DEBUG

                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  String log = F("P037 json key: ");
                  log.reserve(48);
                  log += key;
                  log += F(" payload: ");
                  #   if P037_MAPPING_SUPPORT
                  log += (P037_APPLY_MAPPINGS ? P037_data->mapValue(Payload, key) : Payload);
                  #   else // if P037_MAPPING_SUPPORT
                  log += Payload;
                  #   endif // if P037_MAPPING_SUPPORT
                  addLogMove(LOG_LEVEL_INFO, log);
                }
                #  endif // ifdef PLUGIN_037_DEBUG
                ++P037_data->iter;
              }
              #  if P037_MAPPING_SUPPORT

              if (P037_APPLY_MAPPINGS) {
                Payload = P037_data->mapValue(Payload, key);
              }
              #  endif // if P037_MAPPING_SUPPORT
              # endif  // if P037_JSON_SUPPORT
              bool numericPayload = true; // Unless it's not

              if (!checkJson || (checkJson && (!key.isEmpty()))) {
                double doublePayload;

                if (!validDoubleFromString(Payload, doublePayload)) {
                  if (!checkJson && (P037_SEND_EVENTS == 0)) { // If we want all values as events, then no error logged and don't stop here
                    String log = F("IMPT : Bad Import MQTT Command ");
                    log.reserve(64);
                    log += event->String1;
                    addLog(LOG_LEVEL_ERROR, log);
                    # if !defined(LIMIT_BUILD_SIZE) || defined(P037_OVERRIDE)

                    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                      log.clear();
                      log += F("ERR  : Illegal Payload ");
                      log += Payload;
                      log += ' ';
                      log += getTaskDeviceName(event->TaskIndex);
                      addLogMove(LOG_LEVEL_INFO, log);
                    }
                    # endif // if !defined(LIMIT_BUILD_SIZE) || defined(P037_OVERRIDE)
                    success = false;
                    break;
                  }
                  numericPayload = false;                                  // No, it isn't numeric
                  doublePayload  = NAN;                                    // Invalid value
                }
                UserVar[event->BaseVarIndex + x] = doublePayload;          // Save the new value

                if (!checkJson && P037_SEND_EVENTS && Settings.UseRules) { // Generate event of all non-json topic/payloads
                  String RuleEvent;
                  RuleEvent.reserve(64);
                  RuleEvent += getTaskDeviceName(event->TaskIndex);
                  RuleEvent += '#';
                  RuleEvent += event->String1;
                  RuleEvent += '=';
                  RuleEvent += unparsedPayload;
                  eventQueue.addMove(std::move(RuleEvent));
                }

                // Log the event
                # if !defined(LIMIT_BUILD_SIZE) || defined(P037_OVERRIDE)

                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  String log = F("IMPT : [");
                  log += getTaskDeviceName(event->TaskIndex);
                  log += '#';

                  if (checkJson) {
                    log += key;
                  } else {
                    log += ExtraTaskSettings.TaskDeviceValueNames[x];
                  }
                  log += F("] : ");
                  log += doublePayload;
                  addLogMove(LOG_LEVEL_INFO, log);
                }
                # endif // if !defined(LIMIT_BUILD_SIZE) || defined(P037_OVERRIDE)

                // Generate event for rules processing - proposed by TridentTD

                if (Settings.UseRules) {
                  if (checkJson) {
                    // For JSON payloads generate <Topic>#<Attribute>=<Payload> event
                    String RuleEvent;
                    RuleEvent.reserve(64);
                    RuleEvent += event->String1;
                    # if P037_FILTER_SUPPORT && defined(P037_FILTER_PER_TOPIC)
                    RuleEvent += P037_data->getFilterAsTopic(x + 1);
                    # endif // if P037_FILTER_SUPPORT && defined(P037_FILTER_PER_TOPIC)
                    RuleEvent += '#';
                    RuleEvent += key;
                    RuleEvent += '=';
                    bool hasSemicolon = unparsedPayload.indexOf(';') > -1;

                    if (numericPayload && !hasSemicolon) {
                      RuleEvent += doublePayload;
                    } else if (numericPayload && hasSemicolon) { // semicolon separated list, pass unparsed
                      RuleEvent += Payload;
                      RuleEvent += ',';
                      RuleEvent += unparsedPayload;
                    } else {
                      RuleEvent += Payload; // Pass mapped result
                    }
                    eventQueue.addMove(std::move(RuleEvent));
                  }

                  // (Always) Generate <Taskname>#<Valuename>=<Payload> event
                  String RuleEvent;
                  RuleEvent.reserve(64);
                  RuleEvent += getTaskDeviceName(event->TaskIndex);
                  RuleEvent += '#';
                  RuleEvent += ExtraTaskSettings.TaskDeviceValueNames[x];
                  RuleEvent += '=';

                  if (numericPayload) {
                    RuleEvent += doublePayload;
                  } else {
                    RuleEvent += Payload;
                  }
                  eventQueue.addMove(std::move(RuleEvent));
                }
                # if P037_JSON_SUPPORT

                if (checkJson && (P037_data->iter == P037_data->doc.end())) {
                  continueProcessing = false;
                }
                # endif // if P037_JSON_SUPPORT
              }
            } while (continueProcessing);
          }

          success = true;
        }
      }
      # if P037_JSON_SUPPORT

      if (checkJson) {
        P037_data->cleanupJSON(); // Free/cleanup memory
      }
      # endif // if P037_JSON_SUPPORT

      break;
    }
  }

  return success;
}

bool MQTTSubscribe_037(struct EventStruct *event)
{
  // We must subscribe to the topics.
  P037_data_struct *P037_data = static_cast<P037_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P037_data) {
    return false;
  }

  // char deviceTemplate[VARS_PER_TASK][41];		// variable for saving the subscription topics
  LoadCustomTaskSettings(event->TaskIndex, reinterpret_cast<uint8_t *>(&P037_data->StoredSettings), sizeof(P037_data->StoredSettings));

  String topicPrefix = P037_data->StoredSettings.globalTopicPrefix;

  topicPrefix.trim();

  // Now loop over all import variables and subscribe to those that are not blank
  for (uint8_t x = 0; x < VARS_PER_TASK; x++)
  {
    String subscribeTo = P037_data->StoredSettings.deviceTemplate[x];
    subscribeTo.trim();

    if (!subscribeTo.isEmpty())
    {
      if (!topicPrefix.isEmpty()) {
        subscribeTo.reserve(topicPrefix.length() + subscribeTo.length());
        subscribeTo = topicPrefix + subscribeTo;
      }
      parseSystemVariables(subscribeTo, false);

      if (MQTTclient.subscribe(subscribeTo.c_str())) {
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("IMPT : [");
          LoadTaskSettings(event->TaskIndex);
          log += getTaskDeviceName(event->TaskIndex);
          log += F("#");
          log += ExtraTaskSettings.TaskDeviceValueNames[x];
          log += F("] subscribed to ");
          log += subscribeTo;
          addLogMove(LOG_LEVEL_INFO, log);
        }
      } else {
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log = F("IMPT : Error subscribing to ");
          log += subscribeTo;
          addLogMove(LOG_LEVEL_ERROR, log);
        }
        return false;
      }
    }
  }
  return true;
}

//
// Check to see if Topic matches the MQTT subscription
//
bool MQTTCheckSubscription_037(const String& Topic, const String& Subscription) {
  if (Topic.isEmpty() || Subscription.isEmpty())  {
    return false;
  }

  String tmpTopic = Topic;
  String tmpSub   = Subscription;

  tmpTopic.trim();
  tmpSub.trim();

  // Get rid of leading '/'
  if (tmpTopic[0] == '/') { tmpTopic = tmpTopic.substring(1); }

  if (tmpSub[0] == '/') { tmpSub = tmpSub.substring(1); }

  // Test for multi-level wildcard (#) see: http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718107 (for MQTT 3 and
  // MQTT 5)

  if (tmpSub == F("#")) { return true; // If the subscription is for '#' then all topics are accepted
  }

  if (tmpSub.endsWith(F("/#"))) {      // A valid MQTT multi-level wildcard is a # at the end of the topic that's preceded by a /
    bool multiLevelWildcard = tmpTopic.startsWith(tmpSub.substring(0, tmpSub.length() - 1));

    if (tmpSub.indexOf('+') == -1) {
      return multiLevelWildcard;                   // It matched, or not
    }
  } else {
    if (tmpSub.indexOf('#') != -1) { return false; // Invalid topic
    }
  }

  // Add trailing / if required

  int lenTopic = tmpTopic.length();

  if (tmpTopic.substring(lenTopic - 1, lenTopic) != "/") { tmpTopic += '/'; }

  int lenSub = tmpSub.length();

  if (tmpSub.substring(lenSub - 1, lenSub) != "/") { tmpSub += '/'; }

  // Now get first part

  int SlashTopic;
  int SlashSub;
  int count = 0;

  String pTopic;
  String pSub;

  while (count < 10) {
    //  Get locations of the first /

    SlashTopic = tmpTopic.indexOf('/');
    SlashSub   = tmpSub.indexOf('/');

    //  If no slashes found then match is OK
    //  If only one slash found then not OK

    if ((SlashTopic == -1) && (SlashSub == -1)) { return true; }

    if ((SlashTopic == -1) && (SlashSub != -1)) { return false; }

    if ((SlashTopic != -1) && (SlashSub == -1)) { return false; }

    //  Get the values for the current subtopic

    pTopic = tmpTopic.substring(0, SlashTopic);
    pSub   = tmpSub.substring(0, SlashSub);

    //  And strip the subtopic from the topic

    tmpTopic = tmpTopic.substring(SlashTopic + 1);
    tmpSub   = tmpSub.substring(SlashSub + 1);

    //  If the subtopics match then OK - otherwise fail
    if (pSub == "#") { return true; }

    if ((pTopic != pSub) && (pSub != F("+"))) { return false; }

    count = count + 1;
  }
  return false;
}

#endif // USES_P037
