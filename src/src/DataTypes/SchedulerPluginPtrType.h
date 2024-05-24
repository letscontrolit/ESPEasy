#ifndef DATATYPES_SCHEDULERSchedulerPluginPtrType_e_H
#define DATATYPES_SCHEDULERSchedulerPluginPtrType_e_H

#include "../../ESPEasy_common.h"


enum class SchedulerPluginPtrType_e : uint8_t {
  TaskPlugin,
  ControllerPlugin
#if FEATURE_NOTIFIER
  , NotificationPlugin
#endif // if FEATURE_NOTIFIER
};

const __FlashStringHelper* toString(SchedulerPluginPtrType_e pluginType);


#endif // ifndef DATATYPES_SCHEDULERSchedulerPluginPtrType_e_H
