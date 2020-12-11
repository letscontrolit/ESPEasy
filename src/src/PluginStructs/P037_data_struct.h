#ifndef PLUGINSTRUCTS_P037_DATA_STRUCT_H
#define PLUGINSTRUCTS_P037_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#include "../../ESPEasy_common.h"
#include "../CustomBuild/StorageLayout.h"

#ifdef USES_P037

# define PLUGIN_037_DEBUG     // Additional debugging information

# define P037_MAPPING_SUPPORT // Enable Value mapping support
# define P037_FILTER_SUPPORT  // Enable filtering support
# define P037_JSON_SUPPORT    // Enable Json support

// #ifdef LIMIT_BUILD_SIZE       // Leave out the fancy stuff is available flash is tight
// #  ifdef P037_MAPPING_SUPPORT
//      #undef P037_MAPPING_SUPPORT
// #  endif
// #  ifdef P037_FILTER_SUPPORT
//      #undef P037_FILTER_SUPPORT
// #  endif
// #  ifdef P037_JSON_SUPPORT
//      #undef P037_JSON_SUPPORT
// #  endif
// #endif

# define P037_MAX_MAPPINGS  25
# define P037_MAX_FILTERS   25
# define P037_EXTRA_VALUES  5

# define P037_OPERAND_COUNT 2
# define P037_OPERAND_LIST  F("=%")

# define P037_FILTER_COUNT  2 // 3rd option not implemented yet
# define P037_FILTER_LIST   F("=-:")

// Data structure
struct P037_data_struct : public PluginTaskData_base
{
  P037_data_struct(taskIndex_t taskIndex);
  ~P037_data_struct();

  bool   webform_load(
#ifdef P037_MAPPING_SUPPORT
                      bool mappingEnabled
#endif
#if defined(P037_MAPPING_SUPPORT) && defined(P037_FILTER_SUPPORT)
                      ,
#endif
#ifdef P037_FILTER_SUPPORT
                      bool filterEnabled
#endif
#if (defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)) && defined(P037_JSON_SUPPORT)
                      ,
#endif
#ifdef P037_JSON_SUPPORT
                      bool jsonEnabled
#endif
                     );
  bool   webform_save(
#ifdef P037_FILTER_SUPPORT
                      bool filterEnabled
#endif
#if defined(P037_FILTER_SUPPORT) && defined(P037_JSON_SUPPORT)
                      ,
#endif
#ifdef P037_JSON_SUPPORT
                      bool jsonEnabled
#endif
                     );
#ifdef P037_MAPPING_SUPPORT
  String mapValue(String input);
  void   logMapValue(String input, String result);
#endif // P037_MAPPING_SUPPORT
#ifdef P037_FILTER_SUPPORT
  bool   hasFilters();
  bool   checkFilters(String key, String value);
#ifdef PLUGIN_037_DEBUG
  void   logFilterValue(String text, String key, String value, String match);
#endif // PLUGIN_037_DEBUG
#endif // P037_FILTER_SUPPORT

  // The settings structures
  // The stuff we want to save between settings
  struct tP037_StoredSettings_struct {
    char deviceTemplate[VARS_PER_TASK][41];		// variable for saving the subscription topics, leave as first element for backward compatibility
    char jsonAttributes[VARS_PER_TASK][21];		// variable for saving the json attribute to use
#if defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)
    // All saved in a single string for most efficient storage
    char valueMappings[DAT_TASKS_CUSTOM_SIZE - ((VARS_PER_TASK * 41) + (VARS_PER_TASK * 21))];      // name=num,name2=num,name3%num,... mappings + | + name=value,name2=value2,... Json filters
#endif
  };

  // int maxMappings = DAT_TASKS_CUSTOM_SIZE - ((VARS_PER_TASK * 41) + (VARS_PER_TASK * 21));
  // Stored settings data:
  tP037_StoredSettings_struct StoredSettings;

private:
  // Private methods and vars
  bool   loadSettings();
#if defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)
  void parseMappings();
#endif
  taskIndex_t _taskIndex = TASKS_MAX;
#ifdef P037_MAPPING_SUPPORT
  String _mapping[P037_MAX_MAPPINGS * 3];
  int8_t _maxIdx = -1;
#endif
#ifdef P037_FILTER_SUPPORT
  String _filter[P037_MAX_FILTERS * 3];
  int8_t _maxFilter = -1;
#endif
};

#endif  // ifdef USED_P037
#endif  // ifndef PLUGINSTRUCTS_P037_DATA_STRUCT_H
