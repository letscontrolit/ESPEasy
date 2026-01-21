#include "../DataStructs/Scheduler_NWPluginTimerID.h"

#include "../Globals/Plugins.h"
#include "../Helpers/Misc.h"

NWPluginTimerID::NWPluginTimerID(ESPEasy::net::networkIndex_t networkIndex,
                                 int                          Par1,
                                 NWPlugin::Function           function) :
  SchedulerTimerID(SchedulerTimerType_e::NWPLUGIN_TIMER_IN_e)
{
  constexpr unsigned nrBitsNetworkIndex   = NR_BITS(NETWORK_MAX);
  constexpr unsigned mask_networkIndex    = MASK_BITS(nrBitsNetworkIndex);
  constexpr unsigned nrBitsPluginFunction = NrBitsNWPluginFunctions;
  constexpr unsigned mask_function        = MASK_BITS(nrBitsPluginFunction);

  if (validTaskIndex(networkIndex)) {
    setId((networkIndex & mask_networkIndex) |
          ((static_cast<unsigned>(function) & mask_function) << nrBitsNetworkIndex) |
          (Par1 << (nrBitsNetworkIndex + nrBitsPluginFunction)));
  }
}

ESPEasy::net::networkIndex_t NWPluginTimerID::getNetworkIndex() const
{
  constexpr unsigned nrBitsNetworkIndex = NR_BITS(NETWORK_MAX);
  constexpr unsigned mask_networkIndex  = MASK_BITS(nrBitsNetworkIndex);

  return static_cast<ESPEasy::net::networkIndex_t>(getId() & mask_networkIndex);
}

NWPlugin::Function NWPluginTimerID::getFunction() const
{
  constexpr unsigned nrBitsNetworkIndex   = NR_BITS(NETWORK_MAX);
  constexpr unsigned nrBitsPluginFunction = NrBitsNWPluginFunctions;
  constexpr unsigned mask_function        = MASK_BITS(nrBitsPluginFunction);

  return static_cast<NWPlugin::Function>((getId() >> nrBitsNetworkIndex) & mask_function);
}

#ifndef BUILD_NO_DEBUG

String NWPluginTimerID::decode() const
{
  const ESPEasy::net::networkIndex_t networkIndex = getNetworkIndex();

  if (validTaskIndex(networkIndex)) {
    return getTaskDeviceName(networkIndex);
  }
  return String(getId());
}

#endif // ifndef BUILD_NO_DEBUG
