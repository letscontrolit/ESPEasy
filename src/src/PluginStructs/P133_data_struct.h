#ifndef PLUGINSTRUCTS_P133_DATA_STRUCT_H
#define PLUGINSTRUCTS_P133_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P133
# include <UVlight_LTR390.h>

# define PLUGIN_133_DEBUG true // Set to true to enable extra log output (info)

# define P133_SELECT_MODE   PCONFIG(0)
# define P133_UVGAIN        PCONFIG(1)
# define P133_UVRESOLUTION  PCONFIG(2)
# define P133_ALSGAIN       PCONFIG(3)
# define P133_ALSRESOLUTION PCONFIG(4)
# define P133_INITRESET     PCONFIG(5)

# define P133_LOOP_INTERVAL 1u // 0u = 0.1 sec., 30u = 30x ten per second = 3 seconds per mode

enum class P133_selectMode_e {
  DualMode = 0,
  UVMode   = 1,
  ALSMode  = 2
};

struct P133_data_struct : public PluginTaskData_base {
public:

  P133_data_struct(P133_selectMode_e   selectMode,
                   ltr390_gain_t       uvGain,
                   ltr390_resolution_t uvResolution,
                   ltr390_gain_t       alsGain,
                   ltr390_resolution_t alsResolution,
                   bool                initReset);

  P133_data_struct() = delete;
  virtual ~P133_data_struct();

  bool plugin_init(struct EventStruct *event);
  bool plugin_read(struct EventStruct *event);
  bool plugin_ten_per_second(struct EventStruct *event);

private:

  UVlight_LTR390 *ltr390 = nullptr;

  bool init_sensor();
  bool initialised = false;

  uint32_t loopCounter = P133_LOOP_INTERVAL;
  uint32_t uvValue     = 0;
  uint32_t alsValue    = 0;
  float    uviValue    = 0.0f;
  float    luxValue    = 0.0f;
  bool     sensorRead  = false;

  ltr390_mode_t mode = LTR390_MODE_UVS;

  const P133_selectMode_e   _selectMode;
  const ltr390_gain_t       _uvGain;
  const ltr390_resolution_t _uvResolution;
  const ltr390_gain_t       _alsGain;
  const ltr390_resolution_t _alsResolution;
  const bool                _initReset;
};

#endif // ifdef USES_P133
#endif // ifndef PLUGINSTRUCTS_P133_DATA_STRUCT_H
