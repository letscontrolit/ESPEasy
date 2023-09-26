#include "../DataTypes/SchedulerPluginPtrType.h"

const __FlashStringHelper* toString(SchedulerPluginPtrType_e pluginType) {
  switch (pluginType) {
    case SchedulerPluginPtrType_e::TaskPlugin:         return F("Plugin");
    case SchedulerPluginPtrType_e::ControllerPlugin:   return F("Controller");
#if FEATURE_NOTIFIER
    case SchedulerPluginPtrType_e::NotificationPlugin: return F("Notification");
#endif // if FEATURE_NOTIFIER
  }
  return F("Unknown");
}
