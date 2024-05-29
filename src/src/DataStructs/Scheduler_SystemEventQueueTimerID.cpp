#include "../DataStructs/Scheduler_SystemEventQueueTimerID.h"

#include "../Globals/CPlugins.h"
#include "../Globals/Plugins.h"


SystemEventQueueTimerID::SystemEventQueueTimerID(SchedulerPluginPtrType_e ptr_type, uint8_t Index, uint8_t Function) :
  SchedulerTimerID(SchedulerTimerType_e::SystemEventQueue)
{
  setId((static_cast<uint32_t>(ptr_type) << 16) +
       (Index << 8) +
       Function);
}


#ifndef BUILD_NO_DEBUG
String SystemEventQueueTimerID::decode() const
{
  const SchedulerPluginPtrType_e ptr_type = getPtrType();
  const uint8_t index                     = getIndex();
  String result;

  result += toString(ptr_type);
  result += ',';

  if (ptr_type == SchedulerPluginPtrType_e::ControllerPlugin) {
    result += getCPluginNameFromProtocolIndex(index);
  } else if (ptr_type == SchedulerPluginPtrType_e::TaskPlugin) {
    const deviceIndex_t dev_index = deviceIndex_t::toDeviceIndex(index);
    result += getPluginNameFromDeviceIndex(dev_index);
  } else {
    result += (index + 1);
  }
  result += ',';
  result += getFunction();
  return result;
}

#endif // ifndef BUILD_NO_DEBUG
