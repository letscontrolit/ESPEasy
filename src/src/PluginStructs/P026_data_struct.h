#ifndef PLUGINSTRUCTS_P026_DATA_STRUCT_H
#define PLUGINSTRUCTS_P026_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P026

# include "src/DataStructs/ESPEasy_packed_raw_data.h"
# include "src/ESPEasyCore/ESPEasyNetwork.h"
# include "src/Globals/ESPEasyWiFiEvent.h"
# include "src/Helpers/Memory.h"

# include "ESPEasy-Globals.h"

// place sensor type selector right after the output value settings
# define P026_QUERY1_CONFIG_POS  0
# define P026_SENSOR_TYPE_INDEX  (P026_QUERY1_CONFIG_POS + VARS_PER_TASK)
# define P026_NR_OUTPUT_VALUES   getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P026_SENSOR_TYPE_INDEX)))

# if FEATURE_INTERNAL_TEMPERATURE
#  define P026_NR_OUTPUT_OPTIONS  15
# else // if FEATURE_INTERNAL_TEMPERATURE
#  define P026_NR_OUTPUT_OPTIONS  14
# endif // if FEATURE_INTERNAL_TEMPERATURE

#endif // ifdef USES_P026
#endif // ifndef PLUGINSTRUCTS_P026_DATA_STRUCT_H
