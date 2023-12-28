#ifndef DATASTRUCTS_C013_P2P_DATASTRUCTS_H
#define DATASTRUCTS_C013_P2P_DATASTRUCTS_H

#include "../../ESPEasy_common.h"

#ifdef USES_C013


# include "../CustomBuild/ESPEasyLimits.h"
# include "../DataStructs/DeviceStruct.h"
# include "../DataTypes/TaskIndex.h"
# include "../DataTypes/TaskValues_Data.h"
# include "../DataTypes/PluginID.h"

// These structs are sent to other nodes, so make sure not to change order or offset in struct.

struct __attribute__((__packed__)) C013_SensorInfoStruct
{
  C013_SensorInfoStruct() = default;

  bool isValid() const;

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
};

struct C013_SensorDataStruct
{
  C013_SensorDataStruct() = default;

  bool isValid() const;

  bool matchesPluginID(pluginID_t pluginID) const;

  bool matchesSensorType(Sensor_VType sensor_type) const;

  uint8_t           header          = 255;
  uint8_t           ID              = 5;
  uint8_t           sourceUnit      = 0;
  uint8_t           destUnit        = 0;
  taskIndex_t       sourceTaskIndex = INVALID_TASK_INDEX;
  taskIndex_t       destTaskIndex   = INVALID_TASK_INDEX;

  // deviceNumber and sensorType were not present before build 2023-05-05. (build NR 20460)
  // See: https://github.com/letscontrolit/ESPEasy/commit/cf791527eeaf31ca98b07c45c1b64e2561a7b041#diff-86b42dd78398b103e272503f05f55ee0870ae5fb907d713c2505d63279bb0321
  // Thus should not be checked
  pluginID_t        deviceNumber    = INVALID_PLUGIN_ID;
  Sensor_VType      sensorType      = Sensor_VType::SENSOR_TYPE_NONE;
  TaskValues_Data_t values{};
};

constexpr unsigned int size = sizeof(C013_SensorDataStruct);

#endif // ifdef USES_C013

#endif // DATASTRUCTS_C013_P2P_DATASTRUCTS_H
