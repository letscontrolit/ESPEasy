#ifndef PLUGINSTRUCTS_P003_DATA_STRUCT_H
#define PLUGINSTRUCTS_P003_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P003

# ifndef P003_USE_EXTRA_COUNTERTYPES
#  ifdef LIMIT_BUILD_SIZE
#   define P003_USE_EXTRA_COUNTERTYPES  0 // Don't include for limited builds
#  else // ifdef LIMIT_BUILD_SIZE
#   define P003_USE_EXTRA_COUNTERTYPES  1
#  endif // ifdef LIMIT_BUILD_SIZE
# endif // ifndef P003_USE_EXTRA_COUNTERTYPES

// ... their index in UserVar and TaskDeviceValueNames
# define P003_IDX_pulseCounter           0
# define P003_IDX_pulseTotalCounter      1
# define P003_IDX_pulseTime              2

// ... and the following index into UserVar for storing the persisted TotalCounter
# define P003_IDX_persistedTotalCounter  3

// indexes for config parameters
# define P003_IDX_DEBOUNCETIME   0
# define P003_IDX_COUNTERTYPE    1
# define P003_IDX_MODETYPE       2
# define P003_IDX_IGNORE_ZERO    3

// values for WEBFORM Counter Types
# define P003_CT_INDEX_COUNTER              0
# define P003_CT_INDEX_COUNTER_TOTAL_TIME   1
# define P003_CT_INDEX_TOTAL                2
# define P003_CT_INDEX_COUNTER_TOTAL        3
# if P003_USE_EXTRA_COUNTERTYPES
#  define P003_CT_INDEX_TIME                4
#  define P003_CT_INDEX_TOTAL_TIME          5
#  define P003_CT_INDEX_TIME_COUNTER        6
# endif // if P003_USE_EXTRA_COUNTERTYPES


# include "../Helpers/_Internal_GPIO_pulseHelper.h"

struct P003_data_struct : public PluginTaskData_base {
  P003_data_struct(const Internal_GPIO_pulseHelper::pulseCounterConfig& config);
  P003_data_struct() = delete;

  virtual ~P003_data_struct() = default;

  bool plugin_read(struct EventStruct *event);

  Internal_GPIO_pulseHelper pulseHelper;

  bool lastDeltaZero = false;
};


#endif // ifdef USES_P003
#endif // ifndef PLUGINSTRUCTS_P003_DATA_STRUCT_H
