#include "_Plugin_Helper.h"
#ifdef USES_P037

// #######################################################################################################
// #################################### Plugin 037: MQTT Import ##########################################
// #######################################################################################################


// Original plugin created by Namirda

// This task reads data from the MQTT Import input stream and saves the value

/**
 * 2023-03-06, tonhuisman: Fix PLUGIN_INIT behavior to now always return success = true
 * 2022-11-14, tonhuisman: Add support for selecting JSON sub-attributes, using the . notation, like main.sub (1 level only)
 * 2022-11-02, tonhuisman: Enable plugin to generate events initially, like the plugin did before the mapping, filtering and json parsing
 *                         features were added
 * 2022-08-12, tonhuisman: Introduce plugin-specific P037_LIMIT_BUILD_SIZE feature-flag
 * 2022-04-09, tonhuisman: Add features Deduplicate Events, and Max event-queue size
 * 2022-04-09, tonhuisman: Bugfix sending (extra) events only when enabled
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


# define P037_PARSE_JSON          PCONFIG(1) // Parse/process json messages
# define P037_APPLY_MAPPINGS      PCONFIG(2) // Apply mapping strings to numbers
# define P037_APPLY_FILTERS       PCONFIG(3) // Apply filtering on data values
# define P037_SEND_EVENTS         PCONFIG(4) // Send event for each received topic
# define P037_DEDUPLICATE_EVENTS  PCONFIG(5) // Deduplicate events while still in the queue
# define P037_QUEUEDEPTH_EVENTS   PCONFIG(6) // Max. eventqueue-depth to avoid overflow, extra events will be discarded
# define P037_REPLACE_BY_COMMA    PCONFIG(7) // Character in events to replace by a comma

# define P037_MAX_QUEUEDEPTH      150


bool   MQTT_unsubscribe_037(struct EventStruct *event);
bool   MQTTSubscribe_037(struct EventStruct *event);

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

bool P037_addEventToQueue(struct EventStruct *event, String& newEvent) {
  if (newEvent.isEmpty()) { return false; }
  bool result = true;

  if ((P037_QUEUEDEPTH_EVENTS == 0) ||
      (eventQueue.size() <= static_cast<std::size_t>(P037_QUEUEDEPTH_EVENTS))) {
    # if P037_REPLACE_BY_COMMA_SUPPORT

    if (P037_REPLACE_BY_COMMA != 0x0) {
      const String character = String(static_cast<char>(P037_REPLACE_BY_COMMA));
      newEvent.replace(character, F(","));
    }
    # endif // if P037_REPLACE_BY_COMMA_SUPPORT
    eventQueue.add(newEvent, P037_DEDUPLICATE_EVENTS);
  } else {
    result = false;
  }
  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("MQTT: Event added: ");

    if (result) {
      log +=  F("yes");
    } else {
      log += F("NO!");
    }
    addLog(LOG_LEVEL_DEBUG, log);
  }
  # endif // ifndef BUILD_NO_DEBUG
  return result;
}

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

    case PLUGIN_SET_DEFAULTS:
    {
      P037_SEND_EVENTS = 1; // Enable events by default, as the original plugin did...
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      # if P037_JSON_SUPPORT
      addFormSelector_YesNo(F("Parse JSON messages"), F("pjson"),     P037_PARSE_JSON,     true);
      # endif // if P037_JSON_SUPPORT
      # if P037_FILTER_SUPPORT
      addFormSelector_YesNo(F("Apply filters"),       F("pfilters"),  P037_APPLY_FILTERS,  true);
      # endif // if P037_FILTER_SUPPORT
      # if P037_MAPPING_SUPPORT
      addFormSelector_YesNo(F("Apply mappings"),      F("pmappings"), P037_APPLY_MAPPINGS, true);
      # endif // if P037_MAPPING_SUPPORT
      # if P037_MAPPING_SUPPORT || P037_JSON_SUPPORT || P037_FILTER_SUPPORT
      #  if !defined(P037_LIMIT_BUILD_SIZE)
      addFormNote(F("Changing a Yes/No option will reload the page. Changing to No will clear corresponding settings!"));
      #  endif // if !defined(P037_LIMIT_BUILD_SIZE)
      # endif  // if P037_MAPPING_SUPPORT || P037_JSON_SUPPORT || P037_FILTER_SUPPORT

      addFormSubHeader(F("Options"));

      addFormCheckBox(F("Generate events for accepted topics"),
                      F("p037_send_events"), P037_SEND_EVENTS);
      # if !defined(P037_LIMIT_BUILD_SIZE)
      addFormNote(F("Event: &lt;TaskName&gt;#&lt;topic&gt;=&lt;payload&gt;"));
      #  if P037_JSON_SUPPORT
      addFormNote(F("Events when JSON enabled and JSON payload: &lt;Topic&gt;#&lt;json-attribute&gt;=&lt;value&gt;"));
      #  endif // if P037_JSON_SUPPORT
      # endif  // if !defined(P037_LIMIT_BUILD_SIZE)

      {
        addFormCheckBox(F("Deduplicate events"), F("pdedupe"), P037_DEDUPLICATE_EVENTS == 1);
        # if !defined(P037_LIMIT_BUILD_SIZE)
        addFormNote(F("When enabled will not (re-)generate events that are already in the queue."));
        # endif  // if !defined(P037_LIMIT_BUILD_SIZE)
      }

      {
        # if !defined(P037_LIMIT_BUILD_SIZE) && FEATURE_TOOLTIPS
        String toolTip = F("0..");
        toolTip += P037_MAX_QUEUEDEPTH;
        toolTip += F(" entries");
        addFormNumericBox(F("Max. # entries in event queue"), F("pquedepth"), P037_QUEUEDEPTH_EVENTS, 0, P037_MAX_QUEUEDEPTH, toolTip);
        # else // if !defined(P037_LIMIT_BUILD_SIZE) && FEATURE_TOOLTIPS
        addFormNumericBox(F("Max. # entries in event queue"), F("pquedepth"), P037_QUEUEDEPTH_EVENTS, 0, P037_MAX_QUEUEDEPTH);
        # endif // if !defined(P037_LIMIT_BUILD_SIZE) && FEATURE_TOOLTIPS
        addUnit(F("0 = no check"));
        # if !defined(P037_LIMIT_BUILD_SIZE)
        addFormNote(F("New events will be discarded if the event queue has more entries queued."));
        # endif  // if !defined(P037_LIMIT_BUILD_SIZE)
      }
      # if P037_REPLACE_BY_COMMA_SUPPORT
      {
        String character = F(" ");
        character[0] = (P037_REPLACE_BY_COMMA == 0 ? 0x20 : static_cast<uint8_t>(P037_REPLACE_BY_COMMA));
        addRowLabel(F("To replace by comma in event"));
        addTextBox(F("preplch"), character, 1, false, false, F("[!@$%^ &*;:.|/\\]"), F("widenumber"));
        addUnit(F("Single character only, limited to: <b>! @ $ % ^ & * ; : . | / \\</b> is replaced by: <b>,</b> "));
      }
      # endif // if P037_REPLACE_BY_COMMA_SUPPORT

      P037_data_struct *P037_data = new (std::nothrow) P037_data_struct(event->TaskIndex);

      if (nullptr == P037_data) {
        return success;
      }
      success = P037_data->loadSettings() && P037_data->webform_load(
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
      P037_data_struct *P037_data = new (std::nothrow) P037_data_struct(event->TaskIndex);

      if (nullptr == P037_data) {
        return success;
      }
      P037_data->loadSettings(); // FIXME TD-er: Is this loadSettings still needed or even desired?

      # if P037_JSON_SUPPORT
      P037_PARSE_JSON = getFormItemInt(F("pjson"));
      # endif // if P037_JSON_SUPPORT
      # if P037_MAPPING_SUPPORT
      P037_APPLY_MAPPINGS = getFormItemInt(F("pmappings"));
      # endif // if P037_MAPPING_SUPPORT
      # if P037_FILTER_SUPPORT
      P037_APPLY_FILTERS = getFormItemInt(F("pfilters"));
      # endif // if P037_FILTER_SUPPORT
      P037_SEND_EVENTS        = isFormItemChecked(F("p037_send_events")) ? 1 : 0;
      P037_DEDUPLICATE_EVENTS = isFormItemChecked(F("pdedupe")) ? 1 : 0;
      P037_QUEUEDEPTH_EVENTS  = getFormItemInt(F("pquedepth"));
      # if P037_REPLACE_BY_COMMA_SUPPORT
      String character = webArg(F("preplch"));
      P037_REPLACE_BY_COMMA = character[0];

      if (P037_REPLACE_BY_COMMA == 0x20) { // Space -> 0
        P037_REPLACE_BY_COMMA = 0x0;
      }
      # endif // if P037_REPLACE_BY_COMMA_SUPPORT

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
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P037_data_struct(event->TaskIndex));

      P037_data_struct *P037_data = static_cast<P037_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P037_data) && P037_data->loadSettings()) {
        // When we edit the subscription data from the webserver, the plugin is called again with init.
        // In order to resubscribe we have to disconnect and reconnect in order to get rid of any obsolete subscriptions
        if (MQTTclient_connected) {
          // Subscribe to ALL the topics from ALL instance of this import module
          MQTTSubscribe_037(event);
        }
        success = true;
      }
      break;
    }

    case PLUGIN_EXIT:
    {
      MQTT_unsubscribe_037(event);
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

        if (Settings.UseRules) { // No eventQueue guarding for this event
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
      // Resolved tonhuisman: TD-er: It may be useful to generate events with string values.
      // Get the payload and check it out
      String Payload = event->String2;

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

      String unparsedPayload; // To keep an unprocessed copy

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
        if (P037_data->mqttTopics[x].length() == 0) {
          continue; // skip blank subscriptions
        }

        // Now check if the incoming topic matches one of our subscriptions
        subscriptionTopicParsed = P037_data->getFullMQTTTopic(x);
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
        if (P037_data->mqttTopics[x].length() == 0) {
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
      #  ifndef BUILD_NO_DEBUG

      if (matchedTopic && P037_data->hasFilters() && // Single log statement
          loglevelActiveFor(LOG_LEVEL_DEBUG)) {      // Reduce standard logging
        String log = F("IMPT : MQTT filter result: ");
        log += processData ? F("true") : F("false");
        addLogMove(LOG_LEVEL_DEBUG, log);
      }
      #  endif // ifndef BUILD_NO_DEBUG
      # endif // if P037_FILTER_SUPPORT

      if (!processData) { // Nothing to do? then clean up
        Payload.clear();
        unparsedPayload.clear();
      }

      // Get the Topic and see if it matches any of the subscriptions
      for (uint8_t x = 0; x < VARS_PER_TASK && processData; x++)
      {
        if (P037_data->mqttTopics[x].length() == 0) {
          continue; // skip blank subscriptions
        }

        // Now check if the incoming topic matches one of our subscriptions
        subscriptionTopicParsed = P037_data->getFullMQTTTopic(x);
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
                String jsonIndex     = parseString(P037_data->jsonAttributes[x], 2, ';');
                String jsonAttribute = parseStringKeepCase(P037_data->jsonAttributes[x], 1, ';');
                jsonAttribute.trim();

                if (!jsonAttribute.isEmpty()) {
                  key = jsonAttribute;

                  if (key.indexOf('.') > -1) {
                    String part1 = parseStringKeepCase(key, 1, '.');
                    String part2 = parseStringKeepCase(key, 2, '.');
                    Payload = P037_data->doc[part1][part2].as<String>();
                  } else {
                    Payload = P037_data->doc[key].as<String>();
                  }
                  unparsedPayload = Payload;
                  int8_t jIndex = jsonIndex.toInt();

                  if (jIndex > 1) {
                    Payload = parseString(Payload, jIndex, ';');
                  }

                  #  if !defined(P037_LIMIT_BUILD_SIZE) || defined(P037_OVERRIDE)

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
                  #  endif // if !defined(P037_LIMIT_BUILD_SIZE) || defined(P037_OVERRIDE)
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
                    # if !defined(P037_LIMIT_BUILD_SIZE) || defined(P037_OVERRIDE)

                    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                      log.clear();
                      log += F("ERR  : Illegal Payload ");
                      log += Payload;
                      log += ' ';
                      log += getTaskDeviceName(event->TaskIndex);
                      addLogMove(LOG_LEVEL_INFO, log);
                    }
                    # endif // if !defined(P037_LIMIT_BUILD_SIZE) || defined(P037_OVERRIDE)
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
                  RuleEvent += wrapWithQuotesIfContainsParameterSeparatorChar(unparsedPayload);
                  P037_addEventToQueue(event, RuleEvent);
                }

                // Log the event
                # if !defined(P037_LIMIT_BUILD_SIZE) || defined(P037_OVERRIDE)

                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  String log = F("IMPT : [");
                  log += getTaskDeviceName(event->TaskIndex);
                  log += '#';

                  if (checkJson) {
                    log += key;
                  } else {
                    log += getTaskValueName(event->TaskIndex, x);
                  }
                  log += F("] : ");
                  log += doublePayload;
                  addLogMove(LOG_LEVEL_INFO, log);
                }
                # endif // if !defined(P037_LIMIT_BUILD_SIZE) || defined(P037_OVERRIDE)

                // Generate event for rules processing - proposed by TridentTD

                if (Settings.UseRules && P037_SEND_EVENTS) {
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
                      RuleEvent += wrapWithQuotesIfContainsParameterSeparatorChar(Payload);
                      RuleEvent += ',';
                      RuleEvent += wrapWithQuotesIfContainsParameterSeparatorChar(unparsedPayload);
                    } else {
                      RuleEvent += wrapWithQuotesIfContainsParameterSeparatorChar(Payload); // Pass mapped result
                    }
                    P037_addEventToQueue(event, RuleEvent);
                  }

                  // (Always) Generate <Taskname>#<Valuename>=<Payload> event
                  String RuleEvent;
                  RuleEvent.reserve(64);
                  RuleEvent += getTaskDeviceName(event->TaskIndex);
                  RuleEvent += '#';
                  RuleEvent += getTaskValueName(event->TaskIndex, x);
                  RuleEvent += '=';

                  if (numericPayload) {
                    RuleEvent += doublePayload;
                  } else {
                    RuleEvent += wrapWithQuotesIfContainsParameterSeparatorChar(Payload);
                  }
                  P037_addEventToQueue(event, RuleEvent);
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

bool MQTT_unsubscribe_037(struct EventStruct *event)
{
  P037_data_struct *P037_data = static_cast<P037_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P037_data) {
    return false;
  }

  String topic;

  for (uint8_t x = 0; x < VARS_PER_TASK; x++) {
    String tmp = P037_data->getFullMQTTTopic(x);

    if (topic.equalsIgnoreCase(tmp)) {
      // Don't unsubscribe from the same topic twice
      continue;
    }
    topic = std::move(tmp);

    // We must check whether other MQTT import tasks are enabled which may be subscribed to the same topic.
    // Only if we're the only one (left) being subscribed to that topic, unsubscribe
    bool canUnsubscribe = true;

    for (taskIndex_t task = 0; task < INVALID_TASK_INDEX && canUnsubscribe; ++task) {
      if (task != event->TaskIndex) {
        if (Settings.TaskDeviceEnabled[task] &&
            (Settings.TaskDeviceNumber[task] == PLUGIN_ID_037)) {
          P037_data_struct *P037_data_other = static_cast<P037_data_struct *>(getPluginTaskData(task));

          if (nullptr != P037_data_other) {
            if (P037_data_other->shouldSubscribeToMQTTtopic(topic)) {
              canUnsubscribe = false;

              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                String log = F("IMPT : Cannot unsubscribe topic: ");
                log += topic;
                log += F(" used by: [");
                log += getTaskDeviceName(event->TaskIndex);
                log += '#';
                log += getTaskValueName(event->TaskIndex, x);
                log += ']';
                log += topic;
                addLogMove(LOG_LEVEL_INFO, log);
              }
            }
          }
        }
      }
    }

    if (canUnsubscribe && (topic.length() > 0) && MQTTclient.unsubscribe(topic.c_str())) {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("IMPT : [");
        log += getTaskDeviceName(event->TaskIndex);
        log += '#';
        log += getTaskValueName(event->TaskIndex, x);
        log += F("] : Unsubscribe topic: ");
        log += topic;
        addLogMove(LOG_LEVEL_INFO, log);
      }
    }
  }
  return true;
}

bool MQTTSubscribe_037(struct EventStruct *event)
{
  // We must subscribe to the topics.
  P037_data_struct *P037_data = static_cast<P037_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P037_data) {
    return false;
  }

  // FIXME TD-er: Should not be needed to load, as it is loaded when constructing it.
  P037_data->loadSettings();

  // Now loop over all import variables and subscribe to those that are not blank
  for (uint8_t x = 0; x < VARS_PER_TASK; x++) {
    String subscribeTo = P037_data->getFullMQTTTopic(x);

    if (!subscribeTo.isEmpty()) {
      parseSystemVariables(subscribeTo, false);

      if (MQTTclient.subscribe(subscribeTo.c_str())) {
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("IMPT : [");
          log += getTaskDeviceName(event->TaskIndex);
          log += F("#");
          log += getTaskValueName(event->TaskIndex, x);
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

  if (equals(tmpSub, '#')) { return true; } // If the subscription is for '#' then all topics are accepted

  if (tmpSub.endsWith(F("/#"))) {           // A valid MQTT multi-level wildcard is a # at the end of the topic that's preceded by a /
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

    if ((pTopic != pSub) && (!equals(pSub, '+'))) { return false; }

    count = count + 1;
  }
  return false;
}

#endif // USES_P037
