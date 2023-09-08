#include "../DataStructs/Scheduler_SystemEventQueueTimerID.h"

#include "../Globals/CPlugins.h"
#include "../Globals/Plugins.h"


SystemEventQueueTimerID::SystemEventQueueTimerID(SchedulerPluginPtrType_e ptr_type, uint8_t Index, uint8_t Function) :
  SchedulerTimerID(SchedulerTimerType_e::SystemEventQueue)
{
  id = (static_cast<uint32_t>(ptr_type) << 16) +
       (Index << 8) +
       Function;
}

uint8_t SystemEventQueueTimerID::getFunction() const {
  return static_cast<uint8_t>((id) & 0xFF);
}

uint8_t SystemEventQueueTimerID::getIndex() const {
  return static_cast<uint8_t>((id >> 8) & 0xFF);
}

SchedulerPluginPtrType_e SystemEventQueueTimerID::getPtrType() const {
  return static_cast<SchedulerPluginPtrType_e>((id >> 16) & 0xFF);
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
    result += getPluginNameFromDeviceIndex(index);
  } else {
    result += (index + 1);
  }
  result += ',';
  result += getFunction();
  return result;
}

#endif // ifndef BUILD_NO_DEBUG
