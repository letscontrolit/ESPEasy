#ifndef PLUGINSTRUCTS_P187_DATA_STRUCT_H
#define PLUGINSTRUCTS_P187_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P187

// storage for type of output to generate, e.g. sinus or trapezoid
# define P187_OUTPUT_OPTION_CONFIG_POS    0
# define P187_OUTPUT_OPTION0_CONFIG       PCONFIG(P187_OUTPUT_OPTION_CONFIG_POS + 0)
# define P187_OUTPUT_OPTION1_CONFIG       PCONFIG(P187_OUTPUT_OPTION_CONFIG_POS + 1)
# define P187_OUTPUT_OPTION2_CONFIG       PCONFIG(P187_OUTPUT_OPTION_CONFIG_POS + 2)
# define P187_OUTPUT_OPTION3_CONFIG       PCONFIG(P187_OUTPUT_OPTION_CONFIG_POS + 3)
# define P187_OUTPUT_OPTIONx_CONFIG(x)    PCONFIG(P187_OUTPUT_OPTION_CONFIG_POS + x)

// storage for the type of output to generate, e.g. SENSOR_TYPE_SINGLE, ...
# define P187_OUTPUT_TYPE_INDEX           4
# define P187_OUTPUT_TYPE                 PCONFIG(P187_OUTPUT_TYPE_INDEX)
struct P187_config_struct {
    int param0; // SINUS_AMPLITUDE, TRAPEZ_ON_LEVEL,    RANDOM_MAN
    int param1; // SINUS_OFFSET,    TRAPEZ_OFF_LEVEL,   RANDOM_MIX
    int param2; // SINUS_PERIOD,    TRAPEZ_PERIOD
    int param3; // SINUS_PHASE,     TRAPEZ_ON_TIME
    int param4; //                  TRAPEZ_RISE_TIME
    int param5; //                  TRAPEZ_FALL_TIME
};

enum P187_output_options {
  // do not modify order of these, as they are used in the code to determine which output is selected
  P187_OUTPUT_SINUS = 0,
  P187_OUTPUT_TRAPEZ,
  P187_OUTPUT_RANDOM,

  // keep as last:
  P187_NR_OUTPUT_OPTIONS

};


struct P187_data_struct : public PluginTaskData_base {
  P187_data_struct() = default;
  
  virtual ~P187_data_struct() = default;

// storage for time counter 
  float P187_time[VARS_PER_TASK];

// volatile storage for working copy of output configuration values, e.g. amplitude, offset, period, phase, ...
  P187_config_struct P187_param[VARS_PER_TASK]; // parameters for VARS_PER_TASK outputs

  bool init(struct EventStruct *event);
  bool plugin_read(struct EventStruct *event);
  bool loop(struct EventStruct *event);

  bool isInitialized() const {
    return initialized;
  }

  private:
    bool  initialized = false;
};

#endif // ifdef USES_P107
#endif // ifndef PLUGINSTRUCTS_P107_DATA_STRUCT_H
