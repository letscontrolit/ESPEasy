#ifndef PLUGINSTRUCTS_P168_DATA_STRUCT_H
#define PLUGINSTRUCTS_P168_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P168

# include <Adafruit_VEML7700.h>

# define P168_I2C_ADDRESS             PCONFIG(0)
# define P168_ALS_GAIN                PCONFIG(1)
# define P168_ALS_INTEGRATION         PCONFIG(2)
# define P168_PSM_MODE                PCONFIG(3)
# define P168_READLUX_MODE            PCONFIG(4)

enum class P168_power_save_mode_e : uint8_t {
  Disabled = 4,
  Mode1    = 0,
  Mode2    = 1,
  Mode3    = 2,
  Mode4    = 3,
};

struct P168_data_struct : public PluginTaskData_base {
public:

  P168_data_struct(uint8_t alsGain,
                   uint8_t alsIntegration,
                   uint8_t psmMode,
                   uint8_t readMethod);

  P168_data_struct() = delete;
  virtual ~P168_data_struct();

  bool init(struct EventStruct *event);
  bool plugin_read(struct EventStruct *event);
  bool plugin_get_config_value(struct EventStruct *event,
                               String            & string);
  bool isInitialized() const {
    return initialized;
  }

private:

  Adafruit_VEML7700 *veml = nullptr;

  uint8_t _als_gain;
  uint8_t _als_integration;
  uint8_t _psm_mode;
  uint8_t _readMethod;

  bool initialized = false;
};

#endif // ifdef USES_P168
#endif // ifndef PLUGINSTRUCTS_P168_DATA_STRUCT_H
