#ifndef PLUGINSTRUCTS_P163_DATA_STRUCT_H
#define PLUGINSTRUCTS_P163_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P163

# include <CG_RadSens.h>

# define P163_CFG_THRESHOLD             PCONFIG(0)
# define P163_CFG_COUNT_AVG             PCONFIG(1)

# define P163_CONFIG_FLAGS              PCONFIG_ULONG(0) // All flags
# define P163_CONFIG_LOW_POWER          0                // Flag indexes
# define P163_CONFIG_LED_STATE          1
# define P163_CONFIG_READ_INCREMENT     2
# define P163_CONFIG_RESET_ON_READ      3

# define P163_GET_LOW_POWER    (bitRead(P163_CONFIG_FLAGS, P163_CONFIG_LOW_POWER))
# define P163_SET_LOW_POWER(T) (bitWrite(P163_CONFIG_FLAGS, P163_CONFIG_LOW_POWER, T))
# define P163_GET_LED_STATE    (bitRead(P163_CONFIG_FLAGS, P163_CONFIG_LED_STATE))
# define P163_SET_LED_STATE(T) (bitWrite(P163_CONFIG_FLAGS, P163_CONFIG_LED_STATE, T))
# define P163_GET_READ_INCREMENT    (bitRead(P163_CONFIG_FLAGS, P163_CONFIG_READ_INCREMENT))
# define P163_SET_READ_INCREMENT(T) (bitWrite(P163_CONFIG_FLAGS, P163_CONFIG_READ_INCREMENT, T))
# define P163_GET_RESET_ON_READ    (bitRead(P163_CONFIG_FLAGS, P163_CONFIG_RESET_ON_READ))
# define P163_SET_RESET_ON_READ(T) (bitWrite(P163_CONFIG_FLAGS, P163_CONFIG_RESET_ON_READ, T))

struct P163_data_struct : public PluginTaskData_base {
public:

  P163_data_struct(struct EventStruct *event);

  P163_data_struct() = delete;
  virtual ~P163_data_struct();

  bool init(struct EventStruct *event);

  bool plugin_read(struct EventStruct *event);
  bool plugin_ten_per_second(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    String            & string);
  bool isInitialized() const {
    return initialized;
  }

private:

  bool setOutputValues(struct EventStruct *event);

  CG_RadSens *sensor = nullptr;

  int _threshold = 0;
  # if FEATURE_PLUGIN_STATS
  uint8_t _countAvg = 1;
  # endif // if FEATURE_PLUGIN_STATS
  bool _readIncrement = false;
  bool _resetOnRead   = false;
  bool _lowPowerMode  = false;
  bool _ledState      = true;
  bool _changeOnly    = false;

  bool initialized = false;
};

#endif // ifdef USES_P163
#endif // ifndef PLUGINSTRUCTS_P163_DATA_STRUCT_H
