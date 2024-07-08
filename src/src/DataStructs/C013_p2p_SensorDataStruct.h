#ifndef DATASTRUCTS_C013_P2P_SENSORDATASTRUCTS_H
#define DATASTRUCTS_C013_P2P_SENSORDATASTRUCTS_H

#include "../../ESPEasy_common.h"

#ifdef USES_C013


# include "../CustomBuild/ESPEasyLimits.h"
# include "../DataStructs/DeviceStruct.h"
# include "../DataStructs/ShortChecksumType.h"
# include "../DataTypes/TaskIndex.h"
# include "../DataTypes/TaskValues_Data.h"
# include "../DataTypes/PluginID.h"


// These structs are sent to other nodes, so make sure not to change order or offset in struct.
struct __attribute__((__packed__)) C013_SensorDataStruct
{
  C013_SensorDataStruct() = default;

  bool       setData(const uint8_t *data,
                     size_t         size);

  bool       prepareForSend();

  bool       matchesPluginID(pluginID_t pluginID) const;

  bool       matchesSensorType(Sensor_VType sensor_type) const;

  pluginID_t getPluginID() const;
  void       setPluginID(pluginID_t pluginID);

  uint8_t     header          = 255;
  uint8_t     ID              = 5;
  uint8_t     sourceUnit      = 0;
  uint8_t     destUnit        = 0;
  taskIndex_t sourceTaskIndex = INVALID_TASK_INDEX;
  taskIndex_t destTaskIndex   = INVALID_TASK_INDEX;

  // deviceNumber and sensorType were not present before build 2023-05-05. (build NR 20460)
  // See:
  // https://github.com/letscontrolit/ESPEasy/commit/cf791527eeaf31ca98b07c45c1b64e2561a7b041#diff-86b42dd78398b103e272503f05f55ee0870ae5fb907d713c2505d63279bb0321
  // Thus should not be checked

  // PluginID least significant byte, to remain compatible with older builds
  // Will be set to 0 (= INVALID_PLUGIN_ID) when a plugin ID > 255 is used
  uint8_t           deviceNumber_lsb = INVALID_PLUGIN_ID.value;
  Sensor_VType      sensorType       = Sensor_VType::SENSOR_TYPE_NONE;
  TaskValues_Data_t values{};

  // Extra info added on 20240619 (build ID 20871)
  ShortChecksumType checksum;
  uint16_t          sourceNodeBuild{};
  uint16_t          timestamp_frac{};
  uint32_t          timestamp_sec{};

  // Optional IDX value to allow receiving remote
  // feed data on a different task index as is used on the sender node.
  uint32_t IDX{};

  // Used for PluginID with values > 255.
  // This way older builds only "see" invalid pluginID
  // Added in build 20240708 (build ID 20890)
  uint16_t deviceNumber_msb{};
};

#endif // ifdef USES_C013

#endif // ifndef DATASTRUCTS_C013_P2P_SENSORDATASTRUCTS_H
