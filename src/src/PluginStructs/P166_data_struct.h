#ifndef PLUGINSTRUCTS_P166_DATA_STRUCT_H
#define PLUGINSTRUCTS_P166_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P166

# include <DFRobot_GP8403.h>

# define P166_I2C_ADDRESS      PCONFIG(0)
# define P166_MAX_VOLTAGE      PCONFIG(1) // 0-5V or 0-10V
# define P166_RESTORE_VALUES   PCONFIG(2)
# define P166_PRESET_OUTPUT(N) PCONFIG_FLOAT((N))

# define P166_PresetEntries           25      // Should be enough for now
# define P166_MAX_OUTPUTS             2       // Technical limit
# define P166_RANGE_5V                5000    // mV
# define P166_RANGE_10V               10000   // mV
# define P166_FACTOR_mV               1000.0f // V -> mV

struct P166_data_struct : public PluginTaskData_base {
public:

  P166_data_struct(uint8_t                        address,
                   DFRobot_GP8403::eOutPutRange_t range);

  P166_data_struct() = delete;
  virtual ~P166_data_struct();

  bool init(struct EventStruct *event);

  bool plugin_read(struct EventStruct *event);
  bool plugin_write(struct EventStruct *event,
                    String            & string);
  bool plugin_get_config_value(struct EventStruct *event,
                               String            & string);
  bool isInitialized() const {
    return initialized;
  }

private:

  bool validPresetValue(const String& name,
                        float       & value);
  void setUserVarAndLog(struct EventStruct *event,
                        taskVarIndex_t      varNr,
                        const bool          voltValue,
                        const float         fValue,
                        const String      & subcommand);

  DFRobot_GP8403                *gp8403 = nullptr;
  uint8_t                        _address;
  DFRobot_GP8403::eOutPutRange_t _range;
  bool                           initialized = false;

  String  presets[P166_PresetEntries]{};
  uint8_t maxPreset = 0;
};

#endif // ifdef USES_P166
#endif // ifndef PLUGINSTRUCTS_P166_DATA_STRUCT_H
