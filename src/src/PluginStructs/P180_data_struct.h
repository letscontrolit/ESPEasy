#ifndef PLUGINSTRUCTS_P180_DATA_STRUCT_H
#define PLUGINSTRUCTS_P180_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P180

# include "../Helpers/BusCmd_Handler_I2C.h"
# include "../Helpers/BusCmd_Helper.h"

# include <map>

# define P180_MAX_NAME_LENGTH     20   // Max name length
# if FEATURE_EXTENDED_CUSTOM_SETTINGS
#  define P180_MAX_COMMAND_LENGTH  600 // Max I2C command sequence length
# else // if FEATURE_EXTENDED_CUSTOM_SETTINGS
#  define P180_MAX_COMMAND_LENGTH  175 // Max I2C command sequence length
# endif // if FEATURE_EXTENDED_CUSTOM_SETTINGS

// Buffer layout:
// - VARS_PER_TASK entries for cache-name
// - VARS_PER_TASK entries for I2C commands per value
// - 1 entry for 1-time initialization
// - 1 entry for de-initialization on shutdown
constexpr uint8_t P180_CUSTOM_BUFFER_SIZE    = ((VARS_PER_TASK * 2) + 2);
constexpr uint8_t P180_BUFFER_START_CACHE    = 0;
constexpr uint8_t P180_BUFFER_START_COMMANDS = VARS_PER_TASK;
constexpr uint8_t P180_BUFFER_ENTRY_INIT     = (VARS_PER_TASK * 2);
constexpr uint8_t P180_BUFFER_ENTRY_EXIT     = ((VARS_PER_TASK * 2) + 1);

# define P180_ENABLE_PIN          CONFIG_PIN1
# define P180_RST_PIN             CONFIG_PIN2

# define P180_I2C_ADDRESS         PCONFIG(0)

# define P180_SENSOR_TYPE_INDEX   1 // PCONFIG(1)
# define P180_NR_OUTPUT_VALUES    getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P180_SENSOR_TYPE_INDEX)))
# define P180_VALUE_OFFSET        2 // PCONFIG(2) .. PCONFIG(5)
# define P180_LOG_DEBUG           PCONFIG(6)

// The default set of single-value VType options
constexpr uint8_t P180_START_VTYPE = 0;

struct P180_data_struct : public PluginTaskData_base {
  P180_data_struct(struct EventStruct *event);
  P180_data_struct() = delete;
  virtual ~P180_data_struct();

  bool plugin_init(struct EventStruct *event);
  void plugin_exit(struct EventStruct *event);
  bool plugin_read(struct EventStruct *event);
  bool plugin_once_a_second(struct EventStruct *event);
  bool plugin_ten_per_second(struct EventStruct *event);
  bool plugin_fifty_per_second(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    String            & string);
  # ifndef LIMIT_BUILD_SIZE
  bool plugin_get_config(struct EventStruct *event,
                         String            & string);
  # endif // ifndef LIMIT_BUILD_SIZE

private:

  BusCmd_Helper_struct*busCmd_Helper = nullptr;

  void loadStrings(struct EventStruct *event);

  int16_t     _enPin;
  int16_t     _rstPin;
  taskIndex_t _taskIndex     = INVALID_TASK_INDEX;
  bool        _initialized   = false;
  bool        _stringsLoaded = false;
  bool        _showLog       = false;

  String _strings[P180_CUSTOM_BUFFER_SIZE];
};

#endif // ifdef USES_P180
#endif // ifndef PLUGINSTRUCTS_P180_DATA_STRUCT_H
