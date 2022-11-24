#include "../PluginStructs/P037_data_struct.h"

#ifdef USES_P037

# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Numerical.h"
# include "../Helpers/RulesMatcher.h"
# include "../WebServer/Markup_Forms.h"
# include "../WebServer/ESPEasy_WebServer.h"
# include "../WebServer/Markup.h"
# include "../WebServer/HTML_wrappers.h"
# include "../ESPEasyCore/ESPEasyRules.h"


P037_data_struct::P037_data_struct(taskIndex_t taskIndex) : _taskIndex(taskIndex) 
{}

P037_data_struct::~P037_data_struct() {
  if (nullptr != root) {
    root->clear();
    delete root;
    root = nullptr;
  }
}

/**
 * Load the settings from file
 */
bool P037_data_struct::loadSettings() {
  if (_taskIndex < TASKS_MAX) {
    size_t offset = 0;
    LoadCustomTaskSettings(_taskIndex, mqttTopics,
                           VARS_PER_TASK, 41, offset);
    offset += VARS_PER_TASK * 41;

    LoadCustomTaskSettings(_taskIndex, jsonAttributes,
                           VARS_PER_TASK, 21, offset);
    offset += VARS_PER_TASK * 21;

    {
      String tmp[1];

      LoadCustomTaskSettings(_taskIndex, tmp,
                             1, 41, offset);
      globalTopicPrefix = std::move(tmp[0]);
      offset           += 41;
    }


    LoadCustomTaskSettings(_taskIndex, valueArray,
                           P037_ARRAY_SIZE, 0, offset + 1);
    return true;
  }
  return false;
}

String P037_data_struct::saveSettings() {
  String res;

  if (_taskIndex < TASKS_MAX) {
    size_t offset = 0;
    res += SaveCustomTaskSettings(_taskIndex, mqttTopics,
                                  VARS_PER_TASK, 41, offset);
    offset += VARS_PER_TASK * 41;

    res += SaveCustomTaskSettings(_taskIndex, jsonAttributes,
                                  VARS_PER_TASK, 21, offset);
    offset += VARS_PER_TASK * 21;

    {
      String tmp[1];
      tmp[0] = globalTopicPrefix;
      res   += SaveCustomTaskSettings(_taskIndex, tmp,
                                      1, 41, offset);
      offset += 41;
    }


    res += SaveCustomTaskSettings(_taskIndex, valueArray,
                                  P037_ARRAY_SIZE, 0, offset + 1);
  }
  return res;
}

String P037_data_struct::getFullMQTTTopic(uint8_t taskValueIndex) const {
  String topic;

  if ((taskValueIndex < VARS_PER_TASK) && (mqttTopics[taskValueIndex].length() > 0)) {
    topic.reserve(globalTopicPrefix.length() + mqttTopics[taskValueIndex].length());
    topic = globalTopicPrefix;
    topic.trim();
    topic += mqttTopics[taskValueIndex];
    topic.trim();
  }
  return topic;
}

bool P037_data_struct::shouldSubscribeToMQTTtopic(const String& topic) const {
  if (topic.length() == 0) { return false; }

  for (uint8_t x = 0; x < VARS_PER_TASK; x++) {
    if (topic.equalsIgnoreCase(getFullMQTTTopic(x))) {
      return true;
    }
  }
  return false;
}

# if P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT

/**
 * Parse the mappings and filters from the settings-string into arrays
 */
void P037_data_struct::parseMappings() {
  if (
    #  if P037_MAPPING_SUPPORT
    _maxIdx == -1
    #  endif // if P037_MAPPING_SUPPORT
    #  if P037_MAPPING_SUPPORT && P037_FILTER_SUPPORT
    ||
    #  endif // if P037_MAPPING_SUPPORT && P037_FILTER_SUPPORT
    #  if P037_FILTER_SUPPORT
    _maxFilter == -1
    #  endif // if P037_FILTER_SUPPORT
    ) {
    #  if P037_MAPPING_SUPPORT
    _maxIdx = 0;    // Initialize to empty
    #  endif // if P037_MAPPING_SUPPORT
    #  if P037_FILTER_SUPPORT
    _maxFilter = 0; // Initialize to empty
    #  endif // if P037_FILTER_SUPPORT

    #  if P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT
    int8_t idx;
    #  endif // if P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT
    #  if P037_MAPPING_SUPPORT
    idx = P037_MAX_MAPPINGS;

    for (uint8_t mappingOffset = P037_END_MAPPINGS; mappingOffset >= P037_START_MAPPINGS && _maxIdx == 0; mappingOffset--) {
      if (!valueArray[mappingOffset].isEmpty()) {
        _maxIdx = idx;
      }
      idx--;
    }
    #  endif // if P037_MAPPING_SUPPORT

    #  if P037_FILTER_SUPPORT
    idx = P037_MAX_FILTERS;

    for (uint8_t filterOffset = P037_END_FILTERS; filterOffset >= P037_START_FILTERS && _maxFilter == 0; filterOffset--) {
      if (!valueArray[filterOffset].isEmpty()) {
        _maxFilter = idx;
      }
      idx--;
    }
    #   ifdef P037_FILTER_PER_TOPIC

    if (_maxFilter > 0) { // For Filter-per-topic: Only activate filtering if at least 1 filter is defined
      _maxFilter = VARS_PER_TASK;
    }
    #   endif // ifdef P037_FILTER_PER_TOPIC
    #  endif // if P037_FILTER_SUPPORT
  }
} // parseMappings

# endif // if P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT

bool P037_data_struct::webform_load(
  # if P037_MAPPING_SUPPORT
  bool mappingEnabled
  # endif // if P037_MAPPING_SUPPORT
  # if P037_MAPPING_SUPPORT&& P037_FILTER_SUPPORT
  ,
  # endif // if P037_MAPPING_SUPPORT && P037_FILTER_SUPPORT
  # if P037_FILTER_SUPPORT
  bool filterEnabled
  # endif // if P037_FILTER_SUPPORT
  # if (P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT)&& defined(P037_JSON_SUPPORT)
  ,
  # endif // if (P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT)&& defined(P037_JSON_SUPPORT)
  # ifdef P037_JSON_SUPPORT
  bool jsonEnabled
  # endif // ifdef P037_JSON_SUPPORT
  ) {
  bool success = false;

  addFormSubHeader(F("Topic subscriptions"));

  // Global topic prefix
  addFormTextBox(F("Prefix for all topics"), F("pprefix"), globalTopicPrefix, 40);

  # ifdef P037_JSON_SUPPORT

  if (jsonEnabled) {
    addRowLabel(F("MQTT Topic"));
    html_table(EMPTY_STRING, false); // Sub-table
    html_table_header(F("&nbsp;#&nbsp;"));
    html_table_header(F("Topic"),          500);
    html_table_header(F("JSON Attribute"), 200);
  }
  # endif // ifdef P037_JSON_SUPPORT

  for (int varNr = 0; varNr < VARS_PER_TASK; varNr++) {
    # ifdef P037_JSON_SUPPORT

    if (jsonEnabled) { // Add a column with the json attribute to use for value
      html_TR_TD();
      addHtml(F("&nbsp;"));
      addHtmlInt(varNr + 1);
      html_TD(F("padding-right: 8px"));
      addTextBox(concat(F("template"), varNr + 1),
                 mqttTopics[varNr],
                 40,
                 false, false, EMPTY_STRING, F("xwide"));
      html_TD(F("padding-right: 8px"));
      addTextBox(concat(F("attribute"), varNr + 1),
                 jsonAttributes[varNr],
                 20,
                 false, false, EMPTY_STRING, F("xwide"));
      html_TD();
    } else
    # endif // ifdef P037_JSON_SUPPORT
    {
      addFormTextBox(concat(F("MQTT Topic "), varNr + 1), concat(F("template"), varNr + 1), mqttTopics[varNr], 40);
    }
  }
  # ifdef P037_JSON_SUPPORT

  if (jsonEnabled) {
    html_end_table();
  }
  # endif // ifdef P037_JSON_SUPPORT

  # if P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT
  parseMappings();
  # endif // if P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT

  # if P037_FILTER_SUPPORT

  if (filterEnabled) {
    addFormSubHeader(F("Name - value filters"));

    #  ifdef P037_FILTER_PER_TOPIC
    addRowLabel(F("Filter for MQTT Topic"));
    #  else // ifdef P037_FILTER_PER_TOPIC
    addRowLabel(F("Filter"));
    #  endif // ifdef P037_FILTER_PER_TOPIC
    html_table(F(""), false); // Sub-table
    html_table_header(F("&nbsp;#&nbsp;"));
    html_table_header(F("Name[;Index]"));
    html_table_header(F("Operand"), 180);
    html_table_header(F("Value"));

    const __FlashStringHelper *filterOptions[] = {
      F("equals"), // map name to value
      F("range")   // between 2 values
      #  if P037_FILTER_COUNT >= 3
      , F("list")  // list of multiple values
      #  endif // if P037_FILTER_COUNT >= 3
    };
    const int filterIndices[] = { 0, 1
      #  if P037_FILTER_COUNT >= 3
                                  , 2
      #  endif // if P037_FILTER_COUNT >= 3
    };

    String filters = P037_FILTER_LIST; // Anticipate more filters
    int8_t filterIndex;

    int8_t idx      = 0;
    int8_t filterNr = 1;
    #  ifdef P037_FILTER_PER_TOPIC

    if (_maxFilter <= 0) { _maxFilter = (VARS_PER_TASK * 3); }
    #  endif // ifdef P037_FILTER_PER_TOPIC

    for (uint8_t filterOffset = P037_START_FILTERS; filterOffset <= P037_END_FILTERS && idx < (_maxFilter * 3); filterOffset++) {
      {
        html_TR_TD();
        addHtml(F("&nbsp;"));
        addHtmlInt(filterNr);
        html_TD();
        addTextBox(getPluginCustomArgName(idx + 100 + 0),
                   parseStringKeepCase(valueArray[filterOffset], 1, P037_VALUE_SEPARATOR),
                   32, false, false, EMPTY_STRING, F(""));
      }
      {
        html_TD();
        filterIndex = filters.indexOf(parseString(valueArray[filterOffset], 2, P037_VALUE_SEPARATOR));
        addSelector(getPluginCustomArgName(idx + 100 + 1), P037_FILTER_COUNT, filterOptions, filterIndices, NULL, filterIndex);
        html_TD();

        addTextBox(getPluginCustomArgName(idx + 100 + 2), parseStringKeepCase(valueArray[filterOffset], 3, P037_VALUE_SEPARATOR),
                   32, false, false, EMPTY_STRING, F(""));
        addUnit(F("Range/List: separate values with ; "));
        html_TD();
      }

      filterNr++;
      idx += 3;
    }
    #  ifdef PLUGIN_037_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String info;
      info.reserve(25);
      info += concat(F("P037 maxFilter: "), (int)_maxFilter);
      info += concat(F(" idx: "), (int)idx);
      addLogMove(LOG_LEVEL_INFO, info);
    }
    #  endif // ifdef PLUGIN_037_DEBUG
    #  ifndef P037_FILTER_PER_TOPIC
    filterIndex = 0;
    uint8_t extraFilters = 0;

    while (extraFilters < P037_EXTRA_VALUES && idx < P037_MAX_FILTERS * 3) {
      {
        html_TR_TD();
        addHtml(F("&nbsp;"));
        addHtmlInt(filterNr);
        html_TD();
        addTextBox(getPluginCustomArgName(idx + 100 + 0), EMPTY_STRING,
                   32, false, false, EMPTY_STRING, EMPTY_STRING);
      }
      {
        html_TD();
        addSelector(getPluginCustomArgName(idx + 100 + 1), P037_FILTER_COUNT, filterOptions, filterIndices, NULL, filterIndex);
        html_TD();
        addTextBox(getPluginCustomArgName(idx + 100 + 2), EMPTY_STRING,
                   32, false, false, EMPTY_STRING, EMPTY_STRING);
        addUnit(F("Range/List: separate values with ; "));
        html_TD();
      }
      idx += 3;
      extraFilters++;
      filterNr++;
    }
    #  endif // ifndef P037_FILTER_PER_TOPIC
    html_end_table();
    #  ifndef P037_FILTER_PER_TOPIC
    #   ifdef PLUGIN_037_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      info  = concat(F("P037 extraFilters: "), (int)extraFilters);
      info += concat(F(" idx: "), (int)idx);
      addLogMove(LOG_LEVEL_INFO, info);
    }
    #   endif // ifdef PLUGIN_037_DEBUG
    #  endif  // ifndef P037_FILTER_PER_TOPIC
    addFormNote(F("Both Name and Value must be filled for a valid filter. Filters are case-sensitive."));
    #  ifndef P037_FILTER_PER_TOPIC

    if (extraFilters == P037_EXTRA_VALUES) {
      String moreMessage = concat(F("After filling all filters, submitting this page will make extra filters available (up to "),
                                  P037_MAX_FILTERS);
      moreMessage += F(").");
      addFormNote(moreMessage);
    }
    #  endif // ifndef P037_FILTER_PER_TOPIC
  }

  # endif    // if P037_FILTER_SUPPORT

  # if P037_MAPPING_SUPPORT

  if (mappingEnabled) {
    addFormSubHeader(F("Name - value mappings"));

    addRowLabel(F("Mapping"));
    html_table(F(""), false); // Sub-table
    html_table_header(F("&nbsp;#&nbsp;"));
    html_table_header(F("Name"));
    html_table_header(F("Operand"), 180);
    html_table_header(F("Value"));

    const __FlashStringHelper *operandOptions[] = {
      F("map"),                          // map name to int
      F("percentage") };                 // map attribute value to percentage of provided value
    const int operandIndices[] = { 0, 1 };

    String operands = P037_OPERAND_LIST; // Anticipate more operations
    int8_t operandIndex;

    int8_t idx   = 0;
    int8_t mapNr = 1;

    for (uint8_t mappingOffset = P037_START_MAPPINGS; mappingOffset <= P037_END_MAPPINGS && idx < (_maxIdx * 3); mappingOffset++) {
      {
        html_TR_TD();
        addHtml(F("&nbsp;"));
        addHtmlInt(mapNr);
        html_TD();
        addTextBox(getPluginCustomArgName(idx + 0),
                   parseStringKeepCase(valueArray[mappingOffset], 1, P037_VALUE_SEPARATOR),
                   32, false, false, EMPTY_STRING, F(""));
      }
      {
        html_TD();
        operandIndex = operands.indexOf(parseString(valueArray[mappingOffset], 2, P037_VALUE_SEPARATOR));
        addSelector(getPluginCustomArgName(idx + 1), P037_OPERAND_COUNT, operandOptions, operandIndices, NULL, operandIndex);
        html_TD();
        addTextBox(getPluginCustomArgName(idx + 2),
                   parseStringKeepCase(valueArray[mappingOffset], 3, P037_VALUE_SEPARATOR),
                   32, false, false, EMPTY_STRING, F(""));
        html_TD();
      }
      mapNr++;
      idx += 3;
    }
    #  ifdef PLUGIN_037_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String info;
      info.reserve(25);
      info += concat(F("P037 maxIdx: "), (int)_maxIdx);
      info += concat(F(" idx: "), (int)idx);
      addLogMove(LOG_LEVEL_INFO, info);
    }
    #  endif // ifdef PLUGIN_037_DEBUG
    operandIndex = 0;
    uint8_t extraMappings = 0;

    while (extraMappings < P037_EXTRA_VALUES && idx < P037_MAX_MAPPINGS * 3) {
      {
        html_TR_TD();
        addHtml(F("&nbsp;"));
        addHtmlInt(mapNr);
        html_TD();
        addTextBox(getPluginCustomArgName(idx + 0), EMPTY_STRING,
                   32, false, false, EMPTY_STRING, F(""));
      }
      {
        html_TD();
        addSelector(getPluginCustomArgName(idx + 1), P037_OPERAND_COUNT, operandOptions, operandIndices, NULL, operandIndex);
        html_TD();
        addTextBox(getPluginCustomArgName(idx + 2), EMPTY_STRING,
                   32, false, false, EMPTY_STRING, F(""));
        html_TD();
      }
      idx += 3;
      extraMappings++;
      mapNr++;
    }
    html_end_table();
    #  ifdef PLUGIN_037_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String info;
      info.reserve(35);
      info += concat(F("P037 extraMappings: "), (int)extraMappings);
      info += concat(F(" idx: "), (int)idx);
      addLogMove(LOG_LEVEL_INFO, info);
    }
    #  endif // ifdef PLUGIN_037_DEBUG
    addFormNote(F("Both Name and Value must be filled for a valid mapping. Mappings are case-sensitive."));

    if (extraMappings == P037_EXTRA_VALUES) {
      String moreMessage = concat(F("After filling all mappings, submitting this page will make extra mappings available (up to "),
                                  P037_MAX_MAPPINGS);
      moreMessage += F(").");
      addFormNote(moreMessage);
    }
  }
  # endif // if P037_MAPPING_SUPPORT

  success = true;
  return success;
} // webform_load

bool P037_data_struct::webform_save(
  # if P037_FILTER_SUPPORT
  bool filterEnabled
  # endif // if P037_FILTER_SUPPORT
  # if P037_FILTER_SUPPORT&& defined(P037_JSON_SUPPORT)
  ,
  # endif // if P037_FILTER_SUPPORT && defined(P037_JSON_SUPPORT)
  # ifdef P037_JSON_SUPPORT
  bool jsonEnabled
  # endif // ifdef P037_JSON_SUPPORT
  ) {
  bool success = false;

  String error;

  error.reserve(80); // Estimated

  for (int varNr = 0; varNr < VARS_PER_TASK; varNr++) {
    mqttTopics[varNr] = webArg(concat(F("template"), varNr + 1));

    # ifdef P037_JSON_SUPPORT

    if (jsonEnabled) {
      jsonAttributes[varNr] = webArg(concat(F("attribute"), varNr + 1));
    }
    # endif // P037_JSON_SUPPORT
  }

  globalTopicPrefix = webArg(F("pprefix"));

  # if P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT
  String left, right;
  bool   firstError;
  int8_t idx = 0;
  # endif // if P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT

  // Mappings are processed first
  # if P037_MAPPING_SUPPORT
  firstError = true;
  String  operands = P037_OPERAND_LIST;
  uint8_t mapNr    = 1;
  left.reserve(32);
  right.reserve(32);

  for (uint8_t mappingOffset = P037_START_MAPPINGS; mappingOffset <= P037_END_MAPPINGS; mappingOffset++) {
    left.clear();
    left +=  webArg(getPluginCustomArgName(idx + 0));
    left.trim();
    right.clear();
    right += webArg(getPluginCustomArgName(idx + 2));
    right.trim();

    if (!left.isEmpty() || !right.isEmpty()) {
      valueArray[mappingOffset]  = wrapWithQuotes(left);
      valueArray[mappingOffset] += P037_VALUE_SEPARATOR;
      uint8_t oper = getFormItemInt(getPluginCustomArgName(idx + 1));
      valueArray[mappingOffset] += operands.substring(oper, oper + 1);
      valueArray[mappingOffset] += P037_VALUE_SEPARATOR;
      valueArray[mappingOffset] += wrapWithQuotes(right);
    } else {
      valueArray[mappingOffset] = EMPTY_STRING;
    }

    if (left.isEmpty() != right.isEmpty()) {
      if (firstError) {
        error     += F("Name and value should both be filled for mapping ");
        firstError = false;
      } else {
        error += ',';
      }
      error += mapNr;
    }
    mapNr++;
    idx += 3;
    delay(0); // leave some yield
  }

  if (!firstError) {
    error += '\n';
  }
  # endif // if P037_MAPPING_SUPPORT

  # if P037_FILTER_SUPPORT
  String filters = P037_FILTER_LIST;
  firstError = true;
  uint8_t filterNr = 1;
  idx = 0;

  for (uint8_t filterOffset = P037_START_FILTERS; filterOffset <= P037_END_FILTERS; filterOffset++) {
    left =  webArg(getPluginCustomArgName(idx + 100 + 0));
    left.trim();
    right = webArg(getPluginCustomArgName(idx + 100 + 2));
    right.trim();

    if (!left.isEmpty() || !right.isEmpty()
        #  ifdef P037_FILTER_PER_TOPIC
        || true // Store all filters and in the same order, including empty filters
        #  endif // ifdef P037_FILTER_PER_TOPIC
        ) {
      valueArray[filterOffset]  = wrapWithQuotes(left);
      valueArray[filterOffset] += P037_VALUE_SEPARATOR;
      uint8_t oper = getFormItemInt(getPluginCustomArgName(idx + 100 + 1));
      valueArray[filterOffset] += filters.substring(oper, oper + 1);
      valueArray[filterOffset] += P037_VALUE_SEPARATOR;
      valueArray[filterOffset] += wrapWithQuotes(right);
    } else {
      valueArray[filterOffset] = EMPTY_STRING;
    }

    if (left.isEmpty() != right.isEmpty()) {
      if (firstError) {
        error     += F("Name and value should both be filled for filter ");
        firstError = false;
      } else {
        error += ',';
      }
      error += filterNr;
    }
    filterNr++;
    idx += 3;
    delay(0); // leave some yield
  }
  #  ifndef P037_FILTER_PER_TOPIC

  if (!firstError) {
    error += '\n';
  }
  #  endif // ifndef P037_FILTER_PER_TOPIC
  # endif  // if P037_FILTER_SUPPORT

  error += saveSettings();

  if (!error.isEmpty()) {
    addHtmlError(error);
  }
  # if P037_MAPPING_SUPPORT
  _maxIdx = -1; // Invalidate current mappings and filters
  # endif // if P037_MAPPING_SUPPORT
  # if P037_FILTER_SUPPORT
  _maxFilter = -1;
  # endif // if P037_FILTER_SUPPORT

  success = true;

  return success;
} // webform_save

# if P037_MAPPING_SUPPORT
#  ifdef PLUGIN_037_DEBUG
void P037_data_struct::logMapValue(const String& input, const String& result) {
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String info;
    info.reserve(45);
    info += concat(F("IMPT : MQTT mapped value '"), input);
    info += concat(F("' to '"), result);
    info += '\'';
    addLogMove(LOG_LEVEL_INFO, info);
  }
} // logMapValue

#  endif // ifdef PLUGIN_037_DEBUG

/**
 * Map a string to a (numeric) value, unchanged if no mapping found
 */
String P037_data_struct::mapValue(const String& input, const String& attribute) {
  String result = String(input); // clone

  if (!input.isEmpty()) {
    parseMappings();
    const String operands = P037_OPERAND_LIST;

    int8_t idx = 0;

    for (uint8_t mappingOffset = P037_START_MAPPINGS; mappingOffset <= P037_END_MAPPINGS && idx <= _maxIdx; mappingOffset++) {
      const String name = parseStringKeepCase(valueArray[mappingOffset], 1, P037_VALUE_SEPARATOR);
      const String oper = parseString(valueArray[mappingOffset], 2, P037_VALUE_SEPARATOR);
      const String valu = parseStringKeepCase(valueArray[mappingOffset], 3, P037_VALUE_SEPARATOR);

      if ((name == input) || ((!attribute.isEmpty()) && (name == attribute))) {
        int8_t operandIndex = operands.indexOf(oper);

        switch (operandIndex) {
          case 0: // = => 1:1 mapping
          {
            if (!valu.isEmpty()) {
              result = valu;
              #  ifdef PLUGIN_037_DEBUG
              logMapValue(input, result);
              #  endif // ifdef PLUGIN_037_DEBUG
            }
            break;
          }
          case 1: // % => percentage of mapping
          {
            double inputDouble;
            double mappingDouble;

            if (validDoubleFromString(input, inputDouble) &&
                validDoubleFromString(valu, mappingDouble)) {
              if (compareDoubleValues('>', mappingDouble, 0.0)) {
                double resultDouble = (100.0 / mappingDouble) * inputDouble; // Simple calculation to percentage
                int8_t decimals     = 0;
                int8_t dotPos       = input.indexOf('.');

                if (dotPos > -1) {
                  String decPart = input.substring(dotPos + 1);
                  decimals = decPart.length();             // Take the number of decimals to the output value
                }
                result = toString(resultDouble, decimals); // Percentage with same decimals as input
                #  ifdef PLUGIN_037_DEBUG
                logMapValue(input, result);
                #  endif // ifdef PLUGIN_037_DEBUG
              }
            }
            break;
          }
          default:
            break;
        }
      }
      idx++;
    }
  }

  return result;
} // mapValue

# endif // if P037_MAPPING_SUPPORT

# if P037_FILTER_SUPPORT

/**
 * do we have filter values?
 */
bool P037_data_struct::hasFilters() {
  parseMappings(); // When not parsed yet
  #  ifdef PLUGIN_037_DEBUG
  addLogMove(LOG_LEVEL_INFO, concat(F("p037 hasFilter: "), (int)_maxFilter));
  #  endif // ifdef PLUGIN_037_DEBUG
  return _maxFilter > 0;
} // hasFilters

#  ifdef P037_FILTER_PER_TOPIC
String P037_data_struct::getFilterAsTopic(uint8_t topicId) {
  String result;

  if (hasFilters() &&
      (topicId > 0) &&
      (topicId <= VARS_PER_TASK)) {
    result.reserve(32);
    uint8_t fltBase = (topicId - 1) + P037_START_FILTERS;
    String  name    = parseStringKeepCase(valueArray[fltBase], 1, P037_VALUE_SEPARATOR);
    String  valu    = parseStringKeepCase(valueArray[fltBase], 3, P037_VALUE_SEPARATOR);

    if ((!name.isEmpty()) &&
        (!valu.isEmpty())) {
      result += '/';
      result += name;
      result += '/';

      if (!_filterListItem.isEmpty()) {
        result += _filterListItem;
      } else {
        result += valu;
      }
    }
  }
  return result;
}

#  endif // P037_FILTER_PER_TOPIC

#  ifdef PLUGIN_037_DEBUG
void P037_data_struct::logFilterValue(const String& text, const String& key, const String& value, const String& match) {
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(text.length() + key.length() + value.length() + match.length() + 16);
    log += text;
    log += key;
    log += concat(F(" value: "), value);
    log += concat(F(" match: "), match);
    addLogMove(LOG_LEVEL_INFO, log);
  }
} // logFilterValue

#  endif // PLUGIN_037_DEBUG

/**
 * checkFilters
 * Algorithm: (all comparisons are case-sensitive)
 * - If key is not found in the list, return true
 * - If key is found and value matches, return true
 * - if key is found but value doesn't match, return false
 * key can be in the list multiple times
 */
bool P037_data_struct::checkFilters(const String& key, const String& value, int8_t topicId) {
  bool result = true;

  if ((!key.isEmpty()) &&
      (!value.isEmpty())) { // Ignore empty input(s)
    String  filters = P037_FILTER_LIST;
    String  valueData = value;
    String  fltKey, fltIndex, filterData, fltOper;
    double  from, to, doubleValue;
    int8_t  rangeSeparator;
    bool    accept       = true;
    bool    matchTopicId = true;
    uint8_t fltFrom      = P037_START_FILTERS;
    uint8_t fltMax       = P037_START_FILTERS + _maxFilter;
    #  ifdef P037_FILTER_PER_TOPIC

    if (topicId > 0) {
      fltFrom = (topicId - 1) + P037_START_FILTERS;
      fltMax  = topicId + P037_START_FILTERS;
    }
    #  endif // ifdef P037_FILTER_PER_TOPIC

    for (uint8_t flt = fltFrom; flt < fltMax; flt++) {
      fltOper        = parseStringKeepCase(valueArray[flt], 2, P037_VALUE_SEPARATOR);
      fltKey         = parseStringKeepCase(valueArray[flt], 1, P037_VALUE_SEPARATOR);
      rangeSeparator = parseString(fltKey, 2).toInt();

      if (rangeSeparator > 0) {
        valueData.replace(';', ',');
        valueData = parseString(valueData, rangeSeparator);
      }
      fltKey = parseString(fltKey, 1);
      fltKey.trim();

      if (fltKey == key) {
        result = false;                          // Matched key, so now we are looking for matching value
        int8_t filterIndex = filters.indexOf(fltOper);
        filterData = parseStringKeepCase(valueArray[flt], 3, P037_VALUE_SEPARATOR);
        parseSystemVariables(filterData, false); // Replace system variables

        switch (filterIndex) {
          case 0:                                // = => equals
          {
            _filterListItem = EMPTY_STRING;

            if (filterData == valueData) {
              #  ifdef PLUGIN_037_DEBUG
              String match;
              match.reserve(30);
              match += parseStringKeepCase(valueArray[flt], 3, P037_VALUE_SEPARATOR); // re-parse

              if (topicId > 0) {
                match += F(" topic match: ");
                match += matchTopicId ? F("yes") : F("no");
              }
              logFilterValue(F("P037 filter equals key: "), key, valueData, match);
              #  endif // ifdef PLUGIN_037_DEBUG
              return matchTopicId; // Match, don't look any further
            }
            break;
          }
          case 1:                                       // - => range x-y (inside) or y-x (outside)
          {
            _filterListItem = EMPTY_STRING;
            rangeSeparator  = filterData.indexOf(';');  // Semicolons

            if (rangeSeparator == -1) {
              rangeSeparator = filterData.indexOf('-'); // Fall-back test for dash
            }

            if (rangeSeparator > -1) {
              accept = false;

              if (validDoubleFromString(filterData.substring(0, rangeSeparator),  from) &&
                  validDoubleFromString(filterData.substring(rangeSeparator + 1), to) &&
                  validDoubleFromString(valueData,                                doubleValue)) {
                if (compareDoubleValues('>' + '=', to, from)) { // Normal low - high range: between low and high
                  if (compareDoubleValues('>' + '=', doubleValue, from) &&
                      compareDoubleValues('<' + '=', doubleValue, to)) {
                    accept = true;
                  }
                } else { // Alternative high - low range: outside low and high values
                  if (compareDoubleValues('>' + '=', doubleValue, from) ||
                      compareDoubleValues('<' + '=', doubleValue, to)) {
                    accept = true;
                  }
                }

                #  ifdef PLUGIN_037_DEBUG

                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  String match;
                  match.reserve(30);
                  match += F("P037 filter ");
                  match += accept ? EMPTY_STRING : F("NOT ");
                  match += F("in range key: ");
                  logFilterValue(match, key, valueData, parseStringKeepCase(valueArray[flt], 3, P037_VALUE_SEPARATOR));
                }
                #  endif // ifdef PLUGIN_037_DEBUG

                if (accept) {
                  return matchTopicId; // bail out, we're done
                }
              }
            }
            break;
          }
          #  if P037_FILTER_COUNT >= 3
          case 2: // : => Match against a semicolon-separated list
          {
            _filterListItem = EMPTY_STRING;
            String item;
            rangeSeparator = filterData.indexOf(';');

            if ((rangeSeparator > -1) &&
                validDoubleFromString(valueData, doubleValue)) {
              accept = false;

              do {
                item = filterData.substring(0, rangeSeparator);
                item.trim();
                filterData = filterData.substring(rangeSeparator + 1);
                filterData.trim();
                rangeSeparator = filterData.indexOf(';');

                if (rangeSeparator == -1) {
                  rangeSeparator = filterData.length(); // Last value
                }

                if (validDoubleFromString(item, from) &&
                    compareDoubleValues('=', doubleValue, from)) {
                  accept          = true;
                  _filterListItem = item;
                }
              } while (!filterData.isEmpty() && !accept);

              #   ifdef PLUGIN_037_DEBUG

              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                String match;
                match.reserve(30);
                match += F("P037 filter ");
                match += accept ? EMPTY_STRING : F("NOT ");
                match += F("in list key: ");
                logFilterValue(match, key, valueData, parseStringKeepCase(valueArray[flt], 3, P037_VALUE_SEPARATOR));
              }
              #   endif // ifdef PLUGIN_037_DEBUG

              if (accept) {
                return matchTopicId; // bail out, we're done
              }
            }
            break;
          }
          #  endif // if P037_FILTER_COUNT >= 3
          default:
            break;
        }
      }
      delay(0); // Allow some yield
    }
  }
  return result;
}

# endif // if P037_FILTER_SUPPORT

# ifdef P037_JSON_SUPPORT

/**
 * Allocate a DynamicJsonDocument and parse the message.
 * Returns true if the operation succeeded, and doc and iter can be used, when n ot successful the state of those variables is undefined.
 */
bool P037_data_struct::parseJSONMessage(const String& message) {
  bool result = false;

  if ((nullptr != root) &&
      (message.length() * 1.5 > lastJsonMessageLength)) {
    cleanupJSON();
  }

  if (message.length() * 1.5 > lastJsonMessageLength) {
    lastJsonMessageLength = message.length() * 1.5;
    #  ifdef PLUGIN_037_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLogMove(LOG_LEVEL_INFO, concat(F("IMPT : JSON buffer increased to "), (int)lastJsonMessageLength));
    }
    #  endif // ifdef PLUGIN_037_DEBUG
  }

  if (nullptr == root) {
    root = new (std::nothrow) DynamicJsonDocument(lastJsonMessageLength); // Dynamic allocation
  }

  if (nullptr != root) {
    deserializeJson(*root, message);

    if (!root->isNull()) {
      result = true;
      doc    = root->as<JsonObject>();
      iter   = doc.begin();
    }
  }
  return result;
}

/**
 * Release the created DynamicJsonDocument (if it was allocated)
 */
void P037_data_struct::cleanupJSON() {
  if (nullptr != root) {
    root->clear();
    delete root;
    root = nullptr;
  }
}

# endif // P037_JSON_SUPPORT

#endif  // ifdef USES_P037
