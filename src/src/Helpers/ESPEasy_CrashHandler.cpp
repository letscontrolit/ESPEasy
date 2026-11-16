#include "../Helpers/ESPEasy_CrashHandler.h"

#include "../Globals/Settings.h"
#include "../Globals/RTC.h"

#define CRASHHANDLER_MIN_BOOTLOOP_COUNT  10

/********************************************************************************************\
   Disable Plugin, based on bootFailedCount
 \*********************************************************************************************/
static uint8_t disablePlugin(uint8_t bootFailedCount) {
  for (taskIndex_t i = 0; i < TASKS_MAX && bootFailedCount > 0; ++i) {
    if (Settings.TaskDeviceEnabled[i]) {
      --bootFailedCount;

      if (bootFailedCount == 0) {
        // Disable temporarily as unit crashed
        // FIXME TD-er: Should this be stored?
        Settings.TaskDeviceEnabled[i] = false;
      }
    }
  }
  return bootFailedCount;
}

static uint8_t disableAllPlugins(uint8_t bootFailedCount) {
  if (bootFailedCount > 0) {
    --bootFailedCount;

    for (taskIndex_t i = 0; i < TASKS_MAX; ++i) {
      // Disable temporarily as unit crashed
      // FIXME TD-er: Should this be stored?
      Settings.TaskDeviceEnabled[i] = false;
    }
  }
  return bootFailedCount;
}

/********************************************************************************************\
   Disable Controller, based on bootFailedCount
 \*********************************************************************************************/
static uint8_t disableController(uint8_t bootFailedCount) {
  for (controllerIndex_t i = 0; i < CONTROLLER_MAX && bootFailedCount > 0; ++i) {
    if (Settings.ControllerEnabled[i]) {
      --bootFailedCount;

      if (bootFailedCount == 0) {
        Settings.ControllerEnabled[i] = false;
      }
    }
  }
  return bootFailedCount;
}

static uint8_t disableAllControllers(uint8_t bootFailedCount) {
  if (bootFailedCount > 0) {
    --bootFailedCount;

    for (controllerIndex_t i = 0; i < CONTROLLER_MAX; ++i) {
      Settings.ControllerEnabled[i] = false;
    }
  }
  return bootFailedCount;
}

/********************************************************************************************\
   Disable Notification, based on bootFailedCount
 \*********************************************************************************************/
#if FEATURE_NOTIFIER

static uint8_t disableNotification(uint8_t bootFailedCount) {
  for (uint8_t i = 0; i < NOTIFICATION_MAX && bootFailedCount > 0; ++i) {
    if (Settings.NotificationEnabled[i]) {
      --bootFailedCount;

      if (bootFailedCount == 0) {
        Settings.NotificationEnabled[i] = false;
      }
    }
  }
  return bootFailedCount;
}

static uint8_t disableAllNotifications(uint8_t bootFailedCount) {
  if (bootFailedCount > 0) {
    --bootFailedCount;

    for (uint8_t i = 0; i < NOTIFICATION_MAX; ++i) {
      Settings.NotificationEnabled[i] = false;
    }
  }
  return bootFailedCount;
}

#endif // if FEATURE_NOTIFIER

/********************************************************************************************\
   Disable Network Interfaces, based on bootFailedCount
 \*********************************************************************************************/
static uint8_t disableNetwork(uint8_t bootFailedCount)
{
  for (ESPEasy::net::networkIndex_t i = 0; i < NETWORK_MAX && bootFailedCount > 0; ++i) {
    if (Settings.getNetworkEnabled(i)) {
      --bootFailedCount;

      if (bootFailedCount == 0) {
        Settings.setNetworkEnabled(i, false);
      }
    }
  }
  return bootFailedCount;
}

static uint8_t disableAllNetworks(uint8_t bootFailedCount)
{
  if (bootFailedCount > 0) {
    --bootFailedCount;

    for (ESPEasy::net::networkIndex_t i = 0; i < NETWORK_MAX; ++i) {
      Settings.setNetworkEnabled(i, false);
    }
  }
  return bootFailedCount;
}

/********************************************************************************************\
   Disable Rules, based on bootFailedCount
 \*********************************************************************************************/
static uint8_t disableRules(uint8_t bootFailedCount) {
  if (bootFailedCount > 0) {
    --bootFailedCount;
    Settings.UseRules = false;
  }
  return bootFailedCount;
}

void disableFailingComponent()
{
  if ((RTC.bootFailedCount > CRASHHANDLER_MIN_BOOTLOOP_COUNT) && (RTC.bootCounter > CRASHHANDLER_MIN_BOOTLOOP_COUNT)) {
    uint8_t toDisable = RTC.bootFailedCount - CRASHHANDLER_MIN_BOOTLOOP_COUNT;
    toDisable = disablePlugin(toDisable);

    if (toDisable != 0) {
      toDisable = disableController(toDisable);
    }
    #if FEATURE_NOTIFIER

    if (toDisable != 0) {
      toDisable = disableNotification(toDisable);
    }
    #endif // if FEATURE_NOTIFIER

    if (toDisable != 0) {
      toDisable = disableNetwork(toDisable);
    }

    if (toDisable != 0) {
      toDisable = disableRules(toDisable);
    }

    if (toDisable != 0) {
      toDisable = disableAllPlugins(toDisable);
    }

    if (toDisable != 0) {
      toDisable = disableAllControllers(toDisable);
    }
#if FEATURE_NOTIFIER

    if (toDisable != 0) {
      toDisable = disableAllNotifications(toDisable);
    }
#endif // if FEATURE_NOTIFIER

    if (toDisable != 0) {
      toDisable = disableAllNetworks(toDisable);
    }
  }

}
