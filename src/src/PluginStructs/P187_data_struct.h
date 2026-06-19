#ifndef PLUGINSTRUCTS_P187_DATA_STRUCT_H
#define PLUGINSTRUCTS_P187_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P187

struct P187_config_struct {
    int param0; // SINUS_AMPLITUDE, TRAPEZ_ON_LEVEL,    RANDOM_MAN
    int param1; // SINUS_OFFSET,    TRAPEZ_OFF_LEVEL,   RANDOM_MIX
    int param2; // SINUS_PERIOD,    TRAPEZ_PERIOD
    int param3; // SINUS_PHASE,     TRAPEZ_ON_TIME
    int param4; //                  TRAPEZ_RISE_TIME
    int param5; //                  TRAPEZ_FALL_TIME
};

struct P187_data_struct : public PluginTaskData_base {
  P187_data_struct() = default;
  
  virtual ~P187_data_struct() = default;

// storage for time counter 
  float P187_time[VARS_PER_TASK];

// volatile storage for working copy of output configuration values, e.g. amplitude, offset, period, phase, ...
  P187_config_struct P187_param[VARS_PER_TASK]; // parameters for VARS_PER_TASK outputs

  bool P187_initialized = false;

};

#endif // ifdef USES_P107
#endif // ifndef PLUGINSTRUCTS_P107_DATA_STRUCT_H
