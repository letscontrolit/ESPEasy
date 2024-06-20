#include "../DataStructs/C013_p2p_SensorDataStruct.h"

#ifdef USES_C013

# include "../DataStructs/NodeStruct.h"
# include "../Globals/Nodes.h"
# include "../Globals/ESPEasy_time.h"
# include "../Globals/Plugins.h"

# include "../CustomBuild/CompiletimeDefines.h"

bool C013_SensorDataStruct::prepareForSend()
{
  sourceNodeBuild = get_build_nr();
  checksum.clear();

  if (sourceNodeBuild >= 20871) {
    if (node_time.systemTimePresent()) {
      uint32_t unix_time_frac{};
      timestamp_sec  = node_time.getUnixTime(unix_time_frac);
      timestamp_frac = unix_time_frac >> 16;
    }


    // Make sure to add checksum as last step
    constexpr unsigned len_upto_checksum = offsetof(C013_SensorDataStruct, checksum);

    const ShortChecksumType tmpChecksum(
      reinterpret_cast<const uint8_t *>(this),
      sizeof(C013_SensorDataStruct),
      len_upto_checksum);

    checksum = tmpChecksum;
  }

  // FIXME TD-er: Checksum is now computed twice
  return isValid();
}

bool C013_SensorDataStruct::setData(const uint8_t *data, size_t size)
{
  // First clear entire struct
  memset(this, 0, sizeof(C013_SensorDataStruct));

  if (size < 6) {
    return false;
  }

  if ((data[0] != 255) || // header
      (data[1] != 5)) {   // ID
    return false;
  }

  // Need to keep track of different possible versions of data which still need to be supported.
  // Really old versions of ESPEasy might send upto 80 bytes of uninitialized data
  // meaning for sizes > 24 bytes we may need to check the version of ESPEasy running on the node.
  if (size > sizeof(C013_SensorDataStruct)) {
    size = sizeof(C013_SensorDataStruct);
  }
  NodeStruct *sourceNode = Nodes.getNode(data[2]); // sourceUnit

  if (sourceNode != nullptr) {
    if (sourceNode->build < 20871) {
      if (size > 24) {
        size = 24;
      }
    }
  }

  if (size <= 24) {
    deviceNumber = INVALID_PLUGIN_ID;
    sensorType   = Sensor_VType::SENSOR_TYPE_NONE;

    if (sourceNode != nullptr) {
      sourceNodeBuild = sourceNode->build;
    }
  }
  memcpy(this, data, size);

  return isValid();
}

bool C013_SensorDataStruct::isValid() const
{
  if ((header != 255) || (ID != 5)) { return false; }

  if (checksum.isSet()) {
    constexpr unsigned len_upto_checksum = offsetof(C013_SensorDataStruct, checksum);
    const ShortChecksumType tmpChecksum(
      reinterpret_cast<const uint8_t *>(this),
      sizeof(C013_SensorDataStruct),
      len_upto_checksum);

    if (!(tmpChecksum == checksum)) {
      return false;
    }
  }
  return validTaskIndex(sourceTaskIndex) &&
         validTaskIndex(destTaskIndex);
}

bool C013_SensorDataStruct::matchesPluginID(pluginID_t pluginID) const
{
  if ((deviceNumber.value == 255) || !validPluginID(deviceNumber) || !validPluginID(pluginID)) {
    // Was never set, so probably received data from older node.
    return true;
  }
  return pluginID == deviceNumber;
}

bool C013_SensorDataStruct::matchesSensorType(Sensor_VType sensor_type) const
{
  if ((deviceNumber.value == 255) || (sensorType == Sensor_VType::SENSOR_TYPE_NONE)) {
    // Was never set, so probably received data from older node.
    return true;
  }
  return sensorType == sensor_type;
}

#endif // ifdef USES_C013
