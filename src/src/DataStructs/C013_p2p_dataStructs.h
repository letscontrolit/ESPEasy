#ifndef DATASTRUCTS_C013_P2P_DATASTRUCTS_H
#define DATASTRUCTS_C013_P2P_DATASTRUCTS_H

#include <Arduino.h>

#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataTypes/TaskIndex.h"
#include "../DataTypes/PluginID.h"

// These structs are sent to other nodes, so make sure not to change order or offset in struct.

struct C013_SensorInfoStruct
{
  C013_SensorInfoStruct();

  bool isValid() const;

  uint8_t        header          = 255;
  uint8_t        ID              = 3;
  uint8_t        sourceUnit      = 0;
  uint8_t        destUnit        = 0;
  taskIndex_t sourceTaskIndex = INVALID_TASK_INDEX;
  taskIndex_t destTaskIndex   = INVALID_TASK_INDEX;
  pluginID_t  deviceNumber    = INVALID_PLUGIN_ID;
  char        taskName[26];
  char        ValueNames[VARS_PER_TASK][26];
};

struct C013_SensorDataStruct
{
  C013_SensorDataStruct();

  bool isValid() const;

  uint8_t        header          = 255;
  uint8_t        ID              = 5;
  uint8_t        sourceUnit      = 0;
  uint8_t        destUnit        = 0;
  taskIndex_t sourceTaskIndex = INVALID_TASK_INDEX;
  taskIndex_t destTaskIndex   = INVALID_TASK_INDEX;
  float       Values[VARS_PER_TASK];
};


#endif // DATASTRUCTS_C013_P2P_DATASTRUCTS_H
