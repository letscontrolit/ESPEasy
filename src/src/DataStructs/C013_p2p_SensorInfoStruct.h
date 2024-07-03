#ifndef DATASTRUCTS_C013_P2P_SENSORINFOSTRUCTS_H
#define DATASTRUCTS_C013_P2P_SENSORINFOSTRUCTS_H

#include "../../ESPEasy_common.h"

#ifdef USES_C013


# include "../CustomBuild/ESPEasyLimits.h"
# include "../DataStructs/DeviceStruct.h"
# include "../DataStructs/ShortChecksumType.h"
# include "../DataTypes/TaskIndex.h"
# include "../DataTypes/TaskValues_Data.h"
# include "../DataTypes/PluginID.h"


// These structs are sent to other nodes, so make sure not to change order or offset in struct.
struct __attribute__((__packed__)) C013_SensorInfoStruct
{
  C013_SensorInfoStruct() = default;

  bool setData(const uint8_t *data,
               size_t         size);

  bool prepareForSend(size_t& sizeToSend);

  uint8_t      header          = 255;
  uint8_t      ID              = 3;
  uint8_t      sourceUnit      = 0;
  uint8_t      destUnit        = 0;
  taskIndex_t  sourceTaskIndex = INVALID_TASK_INDEX;
  taskIndex_t  destTaskIndex   = INVALID_TASK_INDEX;
  pluginID_t   deviceNumber    = INVALID_PLUGIN_ID;
  char         taskName[26]{};
  char         ValueNames[VARS_PER_TASK][26]{};
  Sensor_VType sensorType = Sensor_VType::SENSOR_TYPE_NONE;

  // Extra info added on 20240619 (build ID 20871)
  ShortChecksumType checksum;
  uint16_t          sourceNodeBuild = 0;

  // Optional IDX value to allow receiving remote 
  // feed data on a different task index as is used on the sender node.
  uint32_t          IDX             = 0;

  // Settings PCONFIG values
  int16_t TaskDevicePluginConfig[PLUGIN_CONFIGVAR_MAX]{};

  // Some info from ExtraTaskSettings Sorted so the most likely member to be 0 is at the end.
  uint8_t  ExtraTaskSettings_version = 0;
  uint8_t  TaskDeviceValueDecimals[VARS_PER_TASK]{};
  uint32_t VariousBits[VARS_PER_TASK]{};
  float    TaskDeviceErrorValue[VARS_PER_TASK]{};
  float    TaskDeviceMinValue[VARS_PER_TASK]{};
  float    TaskDeviceMaxValue[VARS_PER_TASK]{};

  // Put these as last as they are most likely to be empty
  // FIXME TD-er: Sending formula over is not working well on the receiving end.
//  char TaskDeviceFormula[VARS_PER_TASK][NAME_FORMULA_LENGTH_MAX + 1]{};
};


#endif // ifdef USES_C013

#endif // ifndef DATASTRUCTS_C013_P2P_SENSORINFOSTRUCTS_H
