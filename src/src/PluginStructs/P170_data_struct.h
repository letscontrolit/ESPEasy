#ifndef PLUGINSTRUCTS_P170_DATA_STRUCT_H
#define PLUGINSTRUCTS_P170_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P170

# define P170_I2C_ADDRESS             0x77
# define P170_I2C_ADDRESS_HIGH        0x78

# define P170_LOW_STEPS               8
# define P170_HIGH_STEPS              12
# define P170_TOTAL_STEPS             (P170_LOW_STEPS + P170_HIGH_STEPS)
# define P170_MM_PER_STEP             5
# define P170_MM_PER_STEP_STR         "5"  // String form of above value

# define P170_STEP_ACTIVE_LEVEL_DEF   100
# define P170_DEFAULT_INTERVAL        1000 // 1 second default interval

# define P170_TRIGGER_LOW_LEVEL       PCONFIG(0)
# define P170_TRIGGER_HIGH_LEVEL      PCONFIG(1)
# define P170_TRIGGER_ONCE            PCONFIG(2)
# define P170_STEP_ACTIVE_LEVEL       PCONFIG(3)

struct P170_data_struct : public PluginTaskData_base {
public:

  P170_data_struct(uint8_t level);

  P170_data_struct() = delete;
  virtual ~P170_data_struct() {}

  bool init(struct EventStruct *event);

  bool plugin_read(struct EventStruct *event);
  bool isInitialized() const {
    return initialized;
  }

private:

  uint8_t getSteps();
  bool    readHighSteps();
  bool    readLowSteps();

  uint8_t _level;

  uint8_t data[P170_TOTAL_STEPS]{};
  uint8_t level;
  uint8_t steps;
  bool    lowlevel  = false;
  bool    highlevel = false;

  bool initialized = false;
};

#endif // ifdef USES_P170
#endif // ifndef PLUGINSTRUCTS_P170_DATA_STRUCT_H
