#ifndef DATASTRUCTS_SCHEDULER_PLUGINDEVICETIMERID_H
#define DATASTRUCTS_SCHEDULER_PLUGINDEVICETIMERID_H

#include "../DataStructs/SchedulerTimerID.h"

#include "../DataTypes/DeviceIndex.h"
#include "../DataTypes/PluginID.h"

/*********************************************************************************************\
* Plugin Timer
* Essentially calling PLUGIN_DEVICETIMER_IN
* Similar to PLUGIN_TASKTIMER_IN, addressed to a plugin instead of a task.
\*********************************************************************************************/
struct PluginDeviceTimerID : SchedulerTimerID {
  PluginDeviceTimerID(pluginID_t pluginID,
                      int        Par1);

  deviceIndex_t get_deviceIndex() const;

#ifndef BUILD_NO_DEBUG
  String        decode() const;
#endif // ifndef BUILD_NO_DEBUG
};

#endif // ifndef DATASTRUCTS_SCHEDULER_PLUGINDEVICETIMERID_H
