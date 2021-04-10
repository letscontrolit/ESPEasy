#ifndef PLUGINSTRUCTS_P110_DATA_STRUCT_H
#define PLUGINSTRUCTS_P110_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P110

#define P110_DEBUG        // Enable debugging output (INFO loglevel)
#define P110_DEBUG_DEBUG  // Enable extended debugging output (DEBUG loglevel)

#ifdef LIMIT_BUILD_SIZE
  #ifdef P110_DEBUG_DEBUG
  #undef P110_DEBUG_DEBUG
  #endif
#endif

#include <Wire.h>
#include <VL53L0X.h>

struct P110_data_struct : public PluginTaskData_base {
public:

  P110_data_struct(uint8_t i2c_addr, int timing, bool range);

  bool  begin();
  long  readDistance();
  bool  isReadSuccessful();

private:

  VL53L0X sensor;

  uint8_t i2cAddress;
  bool    initState = false;
  int     timing;
  bool    range;
  bool    success = false;
};
#endif // ifdef USES_P110
#endif // ifndef PLUGINSTRUCTS_P110_DATA_STRUCT_H
