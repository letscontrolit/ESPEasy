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

  return validTaskIndex(sourceTaskIndex) &&
         validTaskIndex(destTaskIndex);
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

  constexpr unsigned len_upto_checksum = offsetof(C013_SensorDataStruct, checksum);
  const ShortChecksumType tmpChecksum(
    data,
    size,
    len_upto_checksum);


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
    setPluginID(INVALID_PLUGIN_ID);
    sensorType = Sensor_VType::SENSOR_TYPE_NONE;

    if (sourceNode != nullptr) {
      sourceNodeBuild = sourceNode->build;
    }
  }

  memcpy(this, data, size);

  if (checksum.isSet()) {
    if (!(tmpChecksum == checksum)) {
      return false;
    }
  }

  return validTaskIndex(sourceTaskIndex) &&
         validTaskIndex(destTaskIndex);
}

bool C013_SensorDataStruct::matchesPluginID(pluginID_t pluginID) const
{
  const pluginID_t deviceNumber = getPluginID();

  if ((deviceNumber.value == 255) || !validPluginID(deviceNumber) || !validPluginID(pluginID)) {
    // Was never set, so probably received data from older node.
    return true;
  }
  return pluginID == deviceNumber;
}

bool C013_SensorDataStruct::matchesSensorType(Sensor_VType sensor_type) const
{
  if ((getPluginID().value == 255) || (sensorType == Sensor_VType::SENSOR_TYPE_NONE)) {
    // Was never set, so probably received data from older node.
    return true;
  }
  return sensorType == sensor_type;
}

pluginID_t C013_SensorDataStruct::getPluginID() const
{
# if FEATURE_SUPPORT_OVER_255_PLUGINS

  // Support for pluginID > 255 added in build 20240708 (build ID 20890)
  if ((deviceNumber_lsb == 0) &&
      (deviceNumber_msb != 0) &&
      (sourceNodeBuild >= 20890)) {
    return pluginID_t::toPluginID(deviceNumber_msb);
  }
# endif // if FEATURE_SUPPORT_OVER_255_PLUGINS
  return pluginID_t::toPluginID(deviceNumber_lsb);
}

void C013_SensorDataStruct::setPluginID(pluginID_t pluginID)
{
  deviceNumber_lsb = 0u;
  deviceNumber_msb = 0u;

  if (validPluginID(pluginID)) {
# if FEATURE_SUPPORT_OVER_255_PLUGINS

    if (pluginID.value <= 255) {
      // Compatible with older builds
      deviceNumber_lsb = (pluginID.value & 0xFF);
    } else {
      // Store in new 16bit member and set deviceNumber_lsb to invalid for older builds
      deviceNumber_msb = pluginID.value;
    }
# else // if FEATURE_SUPPORT_OVER_255_PLUGINS
    deviceNumber_lsb = pluginID.value;
# endif // if FEATURE_SUPPORT_OVER_255_PLUGINS
  }
}

#endif // ifdef USES_C013
