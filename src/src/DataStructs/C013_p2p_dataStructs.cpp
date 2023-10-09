#include "../DataStructs/C013_p2p_dataStructs.h"

#ifdef USES_C013

# include "../Globals/Plugins.h"



bool C013_SensorInfoStruct::isValid() const
{
  if ((header != 255) || (ID != 3)) { return false; }

  return validTaskIndex(sourceTaskIndex) &&
         validTaskIndex(destTaskIndex) &&
         validPluginID(deviceNumber);
}

bool C013_SensorDataStruct::isValid() const
{
  if ((header != 255) || (ID != 5)) { return false; }

  return validTaskIndex(sourceTaskIndex) &&
         validTaskIndex(destTaskIndex);
}

bool C013_SensorDataStruct::matchesPluginID(pluginID_t pluginID) const
{
  if (deviceNumber.value == 255 || !validPluginID(deviceNumber) || !validPluginID(pluginID)) {
    // Was never set, so probably received data from older node.
    return true;
  }
  return pluginID == deviceNumber;
}

bool C013_SensorDataStruct::matchesSensorType(Sensor_VType sensor_type) const
{
  if (deviceNumber.value == 255 || sensorType == Sensor_VType::SENSOR_TYPE_NONE) {
    // Was never set, so probably received data from older node.
    return true;
  }
  return sensorType == sensor_type;
}

#endif // ifdef USES_C013
