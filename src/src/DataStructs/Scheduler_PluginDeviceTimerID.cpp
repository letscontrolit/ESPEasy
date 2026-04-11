#include "../DataStructs/Scheduler_PluginDeviceTimerID.h"

#include "../Globals/Plugins.h"
#include "../Helpers/_Plugin_init.h"

PluginDeviceTimerID::PluginDeviceTimerID(pluginID_t pluginID, int Par1) :
  SchedulerTimerID(SchedulerTimerType_e::PLUGIN_DEVICETIMER_IN_e)
{
  // plugin number and par1 form a unique key that can be used to restart a timer
  // Use deviceIndex instead of pluginID, since the deviceIndex uses less bits.
  const deviceIndex_t deviceIndex = getDeviceIndex(pluginID);

  if (validDeviceIndex(deviceIndex)) {
    // FIXME TD-er: Must add a constexpr function with nr of included plugins.
    const unsigned nrBits = getNrBitsDeviceIndex();
    const unsigned mask   = MASK_BITS(nrBits);
    setId((deviceIndex.value & mask) | (Par1 << nrBits));
  }
}

deviceIndex_t PluginDeviceTimerID::get_deviceIndex() const {
  const unsigned nrBits = getNrBitsDeviceIndex();
  const unsigned mask   = MASK_BITS(nrBits);

  return deviceIndex_t::toDeviceIndex(getId() & mask);
}

#ifndef BUILD_NO_DEBUG
String PluginDeviceTimerID::decode() const
{
  const deviceIndex_t deviceIndex = get_deviceIndex();

  if (validDeviceIndex(deviceIndex)) {
    return getPluginNameFromDeviceIndex(deviceIndex);
  }
  return String(getId());
}

#endif // ifndef BUILD_NO_DEBUG
