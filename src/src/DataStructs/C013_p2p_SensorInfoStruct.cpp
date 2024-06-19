#include "../DataStructs/C013_p2p_SensorInfoStruct.h"

#ifdef USES_C013

# include "../DataStructs/NodeStruct.h"
# include "../Globals/ExtraTaskSettings.h"
# include "../Globals/Nodes.h"
# include "../Globals/Plugins.h"
# include "../Globals/Settings.h"

# include "../CustomBuild/CompiletimeDefines.h"

# include "../Helpers/ESPEasy_Storage.h"

bool C013_SensorInfoStruct::validate()
{
  sourceNodeBuild = get_build_nr();
  checksum.clear();

  if (sourceNodeBuild >= 20871) {
    LoadTaskSettings(sourceTaskIndex);

    for (uint8_t x = 0; x < VARS_PER_TASK; x++) {
      TaskDeviceValueDecimals[x] = ExtraTaskSettings.TaskDeviceValueDecimals[x];
      TaskDeviceMinValue[x]      = ExtraTaskSettings.TaskDeviceMinValue[x];
      TaskDeviceMaxValue[x]      = ExtraTaskSettings.TaskDeviceMaxValue[x];
      TaskDeviceErrorValue[x]    = ExtraTaskSettings.TaskDeviceErrorValue[x];
      VariousBits[x]             = ExtraTaskSettings.VariousBits[x];
    }

    for (uint8_t x = 0; x < PLUGIN_CONFIGVAR_MAX; ++x) {
      TaskDevicePluginConfig[x] = Settings.TaskDevicePluginConfig[sourceTaskIndex][x];
    }

    constexpr unsigned len_upto_checksum = offsetof(C013_SensorInfoStruct, checksum);

    const ShortChecksumType tmpChecksum(
      reinterpret_cast<const uint8_t *>(this),
      sizeof(C013_SensorInfoStruct),
      len_upto_checksum);

    checksum = tmpChecksum;
  }

  // FIXME TD-er: Checksum is now computed twice
  return isValid();
}

bool C013_SensorInfoStruct::setData(const uint8_t *data, size_t size)
{
  // First clear entire struct
  memset(this, 0, sizeof(C013_SensorInfoStruct));

  if (size < 6) {
    return false;
  }

  if ((data[0] != 255) || // header
      (data[1] != 3)) {   // ID
    return false;
  }

  // Need to keep track of different possible versions of data which still need to be supported.
  if (size > sizeof(C013_SensorInfoStruct)) {
    size = sizeof(C013_SensorInfoStruct);
  }

  if (size <= 138) {
    deviceNumber = INVALID_PLUGIN_ID;
    sensorType   = Sensor_VType::SENSOR_TYPE_NONE;

    NodeStruct *sourceNode = Nodes.getNode(data[2]); // sourceUnit

    if (sourceNode != nullptr) {
      sourceNodeBuild = sourceNode->build;
    }
  }
  memcpy(this, data, size);

  return isValid();
}

bool C013_SensorInfoStruct::isValid() const
{
  if ((header != 255) || (ID != 3)) { return false; }

  if (checksum.isSet()) {
    constexpr unsigned len_upto_checksum = offsetof(C013_SensorInfoStruct, checksum);
    const ShortChecksumType tmpChecksum(
      reinterpret_cast<const uint8_t *>(this),
      sizeof(C013_SensorInfoStruct),
      len_upto_checksum);

    if (!(tmpChecksum == checksum)) {
      return false;
    }
  }

  return validTaskIndex(sourceTaskIndex) &&
         validTaskIndex(destTaskIndex) &&
         validPluginID(deviceNumber);
}

#endif // ifdef USES_C013
