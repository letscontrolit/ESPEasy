#include "../DataStructs/C013_p2p_SensorInfoStruct.h"

#ifdef USES_C013

# include "../DataStructs/NodeStruct.h"
# include "../Globals/ExtraTaskSettings.h"
# include "../Globals/Nodes.h"
# include "../Globals/Plugins.h"
# include "../Globals/Settings.h"

# include "../CustomBuild/CompiletimeDefines.h"

# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/StringConverter.h"

bool C013_SensorInfoStruct::prepareForSend(size_t& sizeToSend)
{
  if (!(validTaskIndex(sourceTaskIndex) &&
        validTaskIndex(destTaskIndex) &&
        validPluginID(getPluginID()))) {
    return false;
  }

  sizeToSend = sizeof(C013_SensorInfoStruct);

  sourceNodeBuild = get_build_nr();
  checksum.clear();

  ZERO_FILL(taskName);
  safe_strncpy(taskName, getTaskDeviceName(sourceTaskIndex), sizeof(taskName));

  for (uint8_t x = 0; x < VARS_PER_TASK; x++) {
    ZERO_FILL(ValueNames[x]);
    safe_strncpy(ValueNames[x], getTaskValueName(sourceTaskIndex, x), sizeof(ValueNames[x]));
  }


  if (sourceNodeBuild >= 20871) {
    LoadTaskSettings(sourceTaskIndex);

    ExtraTaskSettings_version = ExtraTaskSettings.version;

    for (uint8_t x = 0; x < VARS_PER_TASK; x++) {
      TaskDeviceValueDecimals[x] = ExtraTaskSettings.TaskDeviceValueDecimals[x];
      TaskDeviceMinValue[x]      = ExtraTaskSettings.TaskDeviceMinValue[x];
      TaskDeviceMaxValue[x]      = ExtraTaskSettings.TaskDeviceMaxValue[x];
      TaskDeviceErrorValue[x]    = ExtraTaskSettings.TaskDeviceErrorValue[x];
      VariousBits[x]             = ExtraTaskSettings.VariousBits[x];

      /*
            ZERO_FILL(TaskDeviceFormula[x]);

            if (ExtraTaskSettings.TaskDeviceFormula[x][0] != 0) {
              safe_strncpy(TaskDeviceFormula[x], ExtraTaskSettings.TaskDeviceFormula[x], sizeof(TaskDeviceFormula[x]));
            }
       */
    }

    for (uint8_t x = 0; x < PLUGIN_CONFIGVAR_MAX; ++x) {
      TaskDevicePluginConfig[x] = Settings.TaskDevicePluginConfig[sourceTaskIndex][x];
    }
  }

  // Check to see if last bytes are all zero, so we can simply not send them
  bool doneShrinking                          = false;
  constexpr unsigned len_upto_sourceNodeBuild = offsetof(C013_SensorInfoStruct, sourceNodeBuild);

  const uint8_t *data = reinterpret_cast<const uint8_t *>(this);

  while (!doneShrinking) {
    if (sizeToSend < len_upto_sourceNodeBuild) {
      doneShrinking = true;
    }
    else {
      if (data[sizeToSend - 1] == 0) {
        --sizeToSend;
      } else {
        doneShrinking = true;
      }
    }
  }

  if (sourceNodeBuild >= 20871) {
    // Make sure to add checksum as last step
    constexpr unsigned len_upto_checksum = offsetof(C013_SensorInfoStruct, checksum);
    const ShortChecksumType tmpChecksum(
      reinterpret_cast<const uint8_t *>(this),
      sizeToSend,
      len_upto_checksum);

    checksum = tmpChecksum;
  }

  return true;
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

  // Before copying the data, compute the checksum of the entire packet
  constexpr unsigned len_upto_checksum = offsetof(C013_SensorInfoStruct, checksum);
  const ShortChecksumType tmpChecksum(
    data,
    size,
    len_upto_checksum);

  // Need to keep track of different possible versions of data which still need to be supported.
  if (size > sizeof(C013_SensorInfoStruct)) {
    size = sizeof(C013_SensorInfoStruct);
  }

  if (size <= 138) {
    setPluginID(INVALID_PLUGIN_ID);
    sensorType = Sensor_VType::SENSOR_TYPE_NONE;

    NodeStruct *sourceNode = Nodes.getNode(data[2]); // sourceUnit

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
         validTaskIndex(destTaskIndex) &&
         validPluginID(getPluginID());
}

pluginID_t C013_SensorInfoStruct::getPluginID() const
{
  // Support for pluginID > 255 added in build 20240708 (build ID 20890)
  if (deviceNumber_lsb == 0 && 
      deviceNumber_msb != 0 && 
      sourceNodeBuild >= 20890) {
    return pluginID_t::toPluginID(deviceNumber_msb);
  }
  return pluginID_t::toPluginID(deviceNumber_lsb);
}

void C013_SensorInfoStruct::setPluginID(pluginID_t pluginID)
{
  deviceNumber_lsb = 0u;
  deviceNumber_msb = 0u;

  if (validPluginID(pluginID)) {
    if (pluginID.value <= 255) {
      // Compatible with older builds
      deviceNumber_lsb = (pluginID.value & 0xFF);
    } else {
      // Store in new 16bit member and set deviceNumber_lsb to invalid for older builds
      deviceNumber_msb = pluginID.value;
    }
  }
}

#endif // ifdef USES_C013
