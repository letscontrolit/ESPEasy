#ifndef DATASTRUCTS_C016_BINARY_ELEMENT_H
#define DATASTRUCTS_C016_BINARY_ELEMENT_H

#include "../../ESPEasy_common.h"
#ifdef USES_C016

#include "../DataTypes/PluginID.h"
#include "../DataTypes/TaskValues_Data.h"
#include "../DataTypes/TaskIndex.h"
#include "../DataTypes/SensorVType.h"

// The binary format to store the samples using the Cache Controller
// Do NOT change order of members!
struct C016_binary_element {

  pluginID_t getPluginID() const;
  void       setPluginID(pluginID_t pluginID);

  uint8_t getValueCount() const;
  void setValueCount(uint8_t valueCount);


  TaskValues_Data_t values{};
  unsigned long unixTime{};
  taskIndex_t   TaskIndex{ INVALID_TASK_INDEX };
  uint8_t       pluginID_lsb{ 0u }; // INVALID_PLUGIN_ID.value
  Sensor_VType  sensorType{ Sensor_VType::SENSOR_TYPE_NONE };
  uint8_t       valueCount_and_pluginID_msb{}; // msb 4 bits for pluginID, lsb 4 bits for valueCount
};

#endif

#endif