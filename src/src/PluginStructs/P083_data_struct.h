#ifndef PLUGINSTRUCTS_P083_DATA_STRUCT_H
#define PLUGINSTRUCTS_P083_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#include "../../ESPEasy_common.h"

#ifdef USES_P083

# include "Adafruit_SGP30.h"

struct P083_data_struct : public PluginTaskData_base {
public:

  P083_data_struct();

  Adafruit_SGP30 sgp;
  unsigned long  init_time   = 0;
  bool           initialized = false;
  bool           newValues   = false;
};

#endif // ifdef USES_P083
#endif // ifndef PLUGINSTRUCTS_P083_DATA_STRUCT_H
