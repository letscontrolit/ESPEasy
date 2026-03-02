#pragma once

#include "../DataStructs/SchedulerTimerID.h"

#include "../DataTypes/ESPEasy_plugin_functions.h"
#include "../../ESPEasy/net/DataTypes/NetworkIndex.h"


/*********************************************************************************************\
* Plugin Task Timer  (PLUGIN_TASKTIMER_IN)
* Can be scheduled per combo taskIndex & Par1 (20 least significant bits)
\*********************************************************************************************/
struct NWPluginTimerID : SchedulerTimerID {
  // taskIndex and par1 form a unique key that can be used to restart a timer
  NWPluginTimerID(ESPEasy::net::networkIndex_t taskIndex,
                  int                          Par1,
                  NWPlugin::Function           function = NWPlugin::Function::NWPLUGIN_TIMER_IN);

  ESPEasy::net::networkIndex_t getNetworkIndex() const;

  NWPlugin::Function           getFunction() const;

#ifndef BUILD_NO_DEBUG
  String                       decode() const;
#endif // ifndef BUILD_NO_DEBUG

};
