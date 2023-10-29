#ifndef PLUGINSTRUCTS_P026_DATA_STRUCT_H
#define PLUGINSTRUCTS_P026_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P026

// place sensor type selector right after the output value settings
# define P026_QUERY1_CONFIG_POS  0
# define P026_SENSOR_TYPE_INDEX  (P026_QUERY1_CONFIG_POS + VARS_PER_TASK)
# define P026_NR_OUTPUT_VALUES   getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P026_SENSOR_TYPE_INDEX)))

struct P026_data_struct {
  static bool GetDeviceValueNames(struct EventStruct *event);

  static bool WebformLoadOutputSelector(struct EventStruct *event);

  static bool WebformSave(struct EventStruct *event);

  static bool Plugin_Read(struct EventStruct *event);

# ifndef PLUGIN_BUILD_MINIMAL_OTA
  static bool Plugin_GetConfigValue(struct EventStruct *event,
                                    String            & string);
# endif // ifndef PLUGIN_BUILD_MINIMAL_OTA

# if FEATURE_PACKED_RAW_DATA
  static bool Plugin_GetPackedRawData(struct EventStruct *event,
                                      String            & string);
# endif // if FEATURE_PACKED_RAW_DATA
};

#endif  // ifdef USES_P026
#endif  // ifndef PLUGINSTRUCTS_P026_DATA_STRUCT_H
