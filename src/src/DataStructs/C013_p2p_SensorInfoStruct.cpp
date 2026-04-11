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
        validPluginID(deviceNumber))) {
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

UP_C013_SensorInfoStruct C013_SensorInfoStruct::create(const uint8_t *data, size_t size)
{
  {
    UP_C013_SensorInfoStruct invalid_res{};

    if (size < 6) {
      return invalid_res;
    }

    if ((data[0] != 255) || // header
        (data[1] != 3)) {   // ID
      return invalid_res;
    }

    // Before copying the data, compute the checksum of the entire packet
    constexpr unsigned len_upto_checksum = offsetof(C013_SensorInfoStruct, checksum);
    const ShortChecksumType tmpChecksum(
      data,
      size,
      len_upto_checksum);

    if (size >= (len_upto_checksum + 4)) {
      // Data could have checksum, see if it is valid.
      uint8_t buf[4];
      memcpy(buf, data + len_upto_checksum, sizeof(buf));
      const ShortChecksumType checksum_data(buf);

      if (checksum_data.isSet()) {
        if (!(tmpChecksum == checksum_data)) {
          return invalid_res;
        }
      }
    }
  }

  // Need to keep track of different possible versions of data which still need to be supported.
  if (size > sizeof(C013_SensorInfoStruct)) {
    size = sizeof(C013_SensorInfoStruct);
  }

  MakeC013_SensorInfo(res);

  if (!AllocatedC013_SensorInfo(res)) { return res; }

  memcpy((uint8_t *)res.get(), data, size);

  if (size <= 138) {
    res->deviceNumber = INVALID_PLUGIN_ID;
    res->sensorType   = Sensor_VType::SENSOR_TYPE_NONE;

    NodeStruct *sourceNode = Nodes.getNode(data[2]); // sourceUnit

    if (sourceNode != nullptr) {
      res->sourceNodeBuild = sourceNode->build;
    }
  }

  if (!(validTaskIndex(res->sourceTaskIndex) &&
        validTaskIndex(res->destTaskIndex) &&
        validPluginID(res->deviceNumber)))
  {
    res.reset();
  }
  return res;
}

#endif // ifdef USES_C013
