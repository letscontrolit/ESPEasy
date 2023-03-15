#ifndef PLUGINSTRUCTS_P037_DATA_STRUCT_H
#define PLUGINSTRUCTS_P037_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P037

# include "../CustomBuild/StorageLayout.h"
# include "../Globals/EventQueue.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Misc.h"
# include "../Helpers/StringParser.h"
# include "../Globals/MQTT.h"

# include <ArduinoJson.h>

// # define PLUGIN_037_DEBUG     // Additional debugging information

# if defined(PLUGIN_BUILD_CUSTOM) || defined(PLUGIN_BUILD_MAX_ESP32) \
  || (defined(PLUGIN_SET_STABLE) && !(defined(PLUGIN_SET_COLLECTION) || defined(PLUGIN_ENERGY_COLLECTION)))
#  ifndef P037_MAPPING_SUPPORT
#   define P037_MAPPING_SUPPORT 1           // Enable Value mapping support
#  endif // ifndef P037_MAPPING_SUPPORT
#  ifndef P037_FILTER_SUPPORT
#   define P037_FILTER_SUPPORT  1           // Enable filtering support
#  endif // ifndef P037_FILTER_SUPPORT
#  ifndef P037_JSON_SUPPORT
#   define P037_JSON_SUPPORT    1           // Enable Json support
#  endif // ifndef P037_JSON_SUPPORT
#  ifndef P037_REPLACE_BY_COMMA_SUPPORT
#   define P037_REPLACE_BY_COMMA_SUPPORT  1 // Enable Replace by comnma support
#  endif // ifndef P037_REPLACE_BY_COMMA_SUPPORT
# endif // if defined(PLUGIN_BUILD_CUSTOM) || defined(PLUGIN_BUILD_MAX_ESP32)
// || (defined(PLUGIN_SET_STABLE) && !(defined(PLUGIN_SET_COLLECTION) || defined(PLUGIN_ENERGY_COLLECTION)))

// # define P037_OVERRIDE        // When defined, do not limit features because of LIMIT_BUILD_SIZE
// # define P037_LIMIT_BUILD_SIZE // Only limit build size for this plugin (to be defined in Custom.h etc.)

# ifndef PLUGIN_BUILD_MAX_ESP32

// Leave out the fancy stuff if available flash is tight
#  if (defined(LIMIT_BUILD_SIZE) && !defined(P037_OVERRIDE)) || defined(P037_LIMIT_BUILD_SIZE)
#   ifndef P037_LIMIT_BUILD_SIZE
#    define P037_LIMIT_BUILD_SIZE // Use this flag exclusively in P037 sources
#   endif // ifndef P037_LIMIT_BUILD_SIZE
#   ifdef PLUGIN_037_DEBUG
#    undef PLUGIN_037_DEBUG
#   endif // ifdef PLUGIN_037_DEBUG
#   if P037_MAPPING_SUPPORT
#    undef P037_MAPPING_SUPPORT
#    define P037_MAPPING_SUPPORT  0
#   endif // if P037_MAPPING_SUPPORT
#   if (defined(FEATURE_ADC_VCC) || defined(P037_LIMIT_BUILD_SIZE)) && P037_FILTER_SUPPORT
#    undef P037_FILTER_SUPPORT
#    define P037_FILTER_SUPPORT 0
#   endif

// #if P037_JSON_SUPPORT
//   #undef P037_JSON_SUPPORT
//   #define P037_JSON_SUPPORT 0
// #endif
#  endif // if (defined(LIMIT_BUILD_SIZE) && !defined(P037_OVERRIDE)) || defined(P037_LIMIT_BUILD_SIZE)

#  ifdef PLUGIN_DISPLAY_COLLECTION
#   if P037_FILTER_SUPPORT
#    undef P037_FILTER_SUPPORT
#    define P037_FILTER_SUPPORT 0
#   endif // if P037_FILTER_SUPPORT
#   if P037_REPLACE_BY_COMMA_SUPPORT
#    undef P037_REPLACE_BY_COMMA_SUPPORT
#    define P037_REPLACE_BY_COMMA_SUPPORT 0
#   endif // if P037_REPLACE_BY_COMMA_SUPPORT
#  endif  // ifdef PLUGIN_DISPLAY_COLLECTION
# endif // ifndef PLUGIN_BUILD_MAX_ESP32

# define P037_MAX_MAPPINGS  25
# define P037_MAX_FILTERS   VARS_PER_TASK // When VARS_PER_TASK is used, the filter is 1:1 mapped to a MQTT topic
# define P037_EXTRA_VALUES  5             // The number of extra, empty, values to show when adding mappings
                                          // (or filters if not 1:1 with topics is used)

// Only 1 filter per topic
# if P037_FILTER_SUPPORT && P037_MAX_FILTERS == VARS_PER_TASK
#  ifndef P037_FILTER_PER_TOPIC
#   define P037_FILTER_PER_TOPIC
#  endif // ifndef P037_FILTER_PER_TOPIC
# endif  // if P037_FILTER_SUPPORT && P037_MAX_FILTERS == VARS_PER_TASK

# define P037_ARRAY_SIZE      (P037_MAX_MAPPINGS + P037_MAX_FILTERS) // Storage layout definitions
# define P037_START_MAPPINGS  0
# define P037_END_MAPPINGS    (P037_MAX_MAPPINGS - 1)
# define P037_START_FILTERS   P037_MAX_MAPPINGS
# define P037_END_FILTERS     (P037_MAX_MAPPINGS + P037_MAX_FILTERS - 1)

# define P037_OPERAND_COUNT   2
# define P037_OPERAND_LIST    F("=%")

# define P037_FILTER_COUNT    3
# define P037_FILTER_LIST     F("=-:") // Length should at least match P037_FILTER_COUNT

# define P037_VALUE_SEPARATOR '\x02'   // Separator outside of the normal ascii character values

// Data structure
struct P037_data_struct : public PluginTaskData_base
{
  P037_data_struct(taskIndex_t taskIndex);
  P037_data_struct() = delete;
  virtual ~P037_data_struct();

  bool webform_load(
    # if P037_MAPPING_SUPPORT
    bool mappingEnabled
    # endif // if P037_MAPPING_SUPPORT
    # if P037_MAPPING_SUPPORT && P037_FILTER_SUPPORT
    ,
    # endif // if P037_MAPPING_SUPPORT && P037_FILTER_SUPPORT
    # if P037_FILTER_SUPPORT
    bool filterEnabled
    # endif // if P037_FILTER_SUPPORT
    # if (P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT) && P037_JSON_SUPPORT
    ,
    # endif // if (P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT) && P037_JSON_SUPPORT
    # if P037_JSON_SUPPORT
    bool jsonEnabled
    # endif // if P037_JSON_SUPPORT
    );
  bool webform_save(
    # if P037_FILTER_SUPPORT
    bool filterEnabled
    # endif // if P037_FILTER_SUPPORT
    # if P037_FILTER_SUPPORT && P037_JSON_SUPPORT
    ,
    # endif // if P037_FILTER_SUPPORT && P037_JSON_SUPPORT
    # if P037_JSON_SUPPORT
    bool jsonEnabled
    # endif // if P037_JSON_SUPPORT
    );
  # if P037_MAPPING_SUPPORT
  String mapValue(const String& input,
                  const String& attribute);
  #  ifdef PLUGIN_037_DEBUG
  void   logMapValue(const String& input,
                     const String& result);
  #  endif // ifdef PLUGIN_037_DEBUG
  # endif  // if P037_MAPPING_SUPPORT
  # if P037_FILTER_SUPPORT
  bool hasFilters();
  bool checkFilters(const String& key,
                    const String& value,
                    int8_t        topicId);
  #  ifdef P037_FILTER_PER_TOPIC
  String getFilterAsTopic(uint8_t topicId);
  #  endif // P037_FILTER_PER_TOPIC
  #  ifdef PLUGIN_037_DEBUG
  void   logFilterValue(const String& text,
                        const String& key,
                        const String& value,
                        const String& match);
  #  endif // PLUGIN_037_DEBUG
  # endif  // if P037_FILTER_SUPPORT


  # if P037_JSON_SUPPORT
  bool parseJSONMessage(const String& message);
  void cleanupJSON();
  JsonObject           doc;
  JsonObject::iterator iter;
  # endif // if P037_JSON_SUPPORT

  // The settings structures
  // The stuff we want to save between settings
  String mqttTopics[VARS_PER_TASK] = {};
  String jsonAttributes[VARS_PER_TASK] = {};
  String globalTopicPrefix = {};
  String valueArray[P037_ARRAY_SIZE] = {}; // Layout: P037_START_MAPPINGS..P037_END_MAPPINGS = mappings,
                                           // P037_START_FILTERS..P037_END_FILTERS = filters

  String getFullMQTTTopic(uint8_t taskValueIndex) const;

  bool   shouldSubscribeToMQTTtopic(const String& topic) const;

  bool   loadSettings();

private:

  String saveSettings();

  # if P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT
  void   parseMappings();
  # endif // if P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT
  taskIndex_t _taskIndex = TASKS_MAX;
  # if P037_MAPPING_SUPPORT
  int8_t _maxIdx = -1;
  # endif // if P037_MAPPING_SUPPORT
  # if P037_FILTER_SUPPORT
  int8_t _maxFilter = -1;
  String _filterListItem;
  # endif // if P037_FILTER_SUPPORT
  # if P037_JSON_SUPPORT
  DynamicJsonDocument *root                  = nullptr;
  uint16_t             lastJsonMessageLength = 512;
  # endif // if P037_JSON_SUPPORT
};

#endif    // ifdef USED_P037
#endif // ifndef PLUGINSTRUCTS_P037_DATA_STRUCT_H
