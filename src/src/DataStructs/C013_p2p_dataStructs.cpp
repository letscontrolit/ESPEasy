#include "C013_p2p_dataStructs.h"

#include "../Globals/Plugins.h"
#include "../../ESPEasy_common.h"


C013_SensorInfoStruct::C013_SensorInfoStruct()
{
  ZERO_FILL(taskName);

  for (int i = 0; i < VARS_PER_TASK; ++i) {
    ZERO_FILL(ValueNames[i]);
  }
}

bool C013_SensorInfoStruct::isValid() const
{
  if ((header != 255) || (ID != 3)) { return false; }

  if (!validTaskIndex(sourceTaskIndex) ||
      !validTaskIndex(destTaskIndex) ||
      !validPluginID(deviceNumber)) {
    return false;
  }


  return true;
}

C013_SensorDataStruct::C013_SensorDataStruct()
{
  for (int i = 0; i < VARS_PER_TASK; ++i) {
    Values[i] = 0.0f;
  }
}

bool C013_SensorDataStruct::isValid() const
{
  if ((header != 255) || (ID != 5)) { return false; }

  if (!validTaskIndex(sourceTaskIndex) ||
      !validTaskIndex(destTaskIndex)) {
    return false;
  }
  return true;
}
