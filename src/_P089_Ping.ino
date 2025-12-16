#include "_Plugin_Helper.h"
#ifdef USES_P089

# include "src/PluginStructs/P089_data_struct.h"

// FIXME TD-er: Support Ping in ESP32
// Also remove check for ESP8266 in Helpers/_Plugin_init.h and .cpp


// #######################################################################################################
// #################### Plugin 089 ICMP Ping probing ##############
// #######################################################################################################

/*
   Ping tests for hostnames and ips
   written by https://github.com/nuclearcat
   Useful to detect strange wifi failures (when it stays connected but not able to reach any ip)
   and to test other devices for reachability (this is why SendDataOption is enabled)
   Maintainer: Denys Fedoryshchenko, denys AT nuclearcat.com
 */
/** Changelog:
 * 2025-08-15 tonhuisman: Move actual ping action to a service, instantiated by the first task instance, and deleted by the last stopped
 *                        task instance. Service is looped in 10 per second function.
 *                        Enable PluginStats for ESP32, so the trend of average ping-time can be displayed.
 * 2025-08-15 tonhuisman: Reduce max. ping count from 100 to 25
 * 2025-08-14 tonhuisman: Add code to plugin to use dvarrel/ESPping library that works for ESP32.
 *                        ESP32 handles the ping in an independent RTOS task, to make it non-blocking.
 * 2025-01-12 tonhuisman: Add support for MQTT AutoDiscovery (not supported for Ping)
 * 2023-03-19 tonhuisman: Show hostname in GPIO column of Devices page
 * 2023-03-14 tonhuisman: Change command handling to not require the taskname as the second argument if no 3rd argument is given.
 *                        Set decimals to 0 whan adding the task.
 * 2023-03 Started changelog, not registered before.
 */


# define PLUGIN_089
# define PLUGIN_NAME_089           "Communication - Ping"
# define PLUGIN_VALUENAME1_089     "Fails"
# define PLUGIN_VALUENAME2_089     "Avg_ms"


boolean Plugin_089(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_089;
      dev.Type           = DEVICE_TYPE_CUSTOM0;
      dev.VType          = Sensor_VType::SENSOR_TYPE_SINGLE;
      dev.ValueCount     = 1;
      dev.DecimalsOnly   = true;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      # ifdef ESP32
      dev.PluginStats = true;
      # endif // ifdef ESP32
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      // return the device name
      string = F(PLUGIN_NAME_089);
      break;
    }
    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_089));
      # ifdef ESP32
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_089));
      # endif // ifdef ESP32
      break;
    }

    # ifdef ESP32
    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      int vCount = P089_VALUE_COUNT;

      if (0 == vCount) {
        vCount = 1;
      }
      event->Par1 = vCount;
      success     = true;
      break;
    }
    # endif // ifdef ESP32

    # if FEATURE_MQTT_DISCOVER
    case PLUGIN_GET_DISCOVERY_VTYPES:
    {
      event->Par1 = static_cast<int>(Sensor_VType::SENSOR_TYPE_NONE); // Not yet supported
      #  ifdef ESP32
      event->Par2 = static_cast<int>(Sensor_VType::SENSOR_TYPE_DURATION);
      #  endif // ifdef ESP32
      success = true;
      break;
    }
    # endif // if FEATURE_MQTT_DISCOVER

    case PLUGIN_SET_DEFAULTS:
    {
      ExtraTaskSettings.TaskDeviceValueDecimals[0] = 0; // Count doesn't include decimals
      break;
    }

    case PLUGIN_WEBFORM_SHOW_GPIO_DESCR:
    {
      char hostname[PLUGIN_089_HOSTNAME_SIZE]{};
      LoadCustomTaskSettings(event->TaskIndex, (uint8_t *)&hostname, PLUGIN_089_HOSTNAME_SIZE);
      string  = hostname;
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      char hostname[PLUGIN_089_HOSTNAME_SIZE]{};
      LoadCustomTaskSettings(event->TaskIndex, (uint8_t *)&hostname, PLUGIN_089_HOSTNAME_SIZE);
      addFormTextBox(F("Hostname"), F("host"), hostname, PLUGIN_089_HOSTNAME_SIZE - 2);

      # ifdef ESP32

      if (P089_PING_COUNT < 1) {
        P089_PING_COUNT = 5;
      }
      addFormNumericBox(F("Ping count"), F("pcount"), P089_PING_COUNT, 1, P089_MAX_PING_COUNT);

      const __FlashStringHelper*countOptions[] = {
        F("Fails"),
        F("Fails, Avg_ms"),
      };
      const int countValues[] = { 1, 2 };
      FormSelectorOptions selector(2, countOptions, countValues);
      selector.reloadonchange = true;
      selector.addFormSelector(F("Available Values"), F("pvals"), P089_VALUE_COUNT);
      # endif // ifdef ESP32

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      # ifdef ESP32
      P089_PING_COUNT  = getFormItemInt(F("pcount"), 5);
      P089_VALUE_COUNT = getFormItemInt(F("pvals"), 1);
      # endif // ifdef ESP32

      char hostname[PLUGIN_089_HOSTNAME_SIZE]{};

      // Reset "Fails" if settings updated
      UserVar.setFloat(event->TaskIndex, 0, 0);
      strncpy(hostname, webArg(F("host")).c_str(), sizeof(hostname));
      SaveCustomTaskSettings(event->TaskIndex, (uint8_t *)&hostname, PLUGIN_089_HOSTNAME_SIZE);
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      success = initPluginTaskData(event->TaskIndex, new (std::nothrow) P089_data_struct());
      UserVar.setFloat(event->TaskIndex, 0, 0);
      # ifdef ESP32
      UserVar.setFloat(event->TaskIndex, 1, 0);
      # endif // ifdef ESP32
      break;
    }

    # ifdef ESP32
    case PLUGIN_TEN_PER_SECOND:
    {
      P089_data_struct *P089_taskdata =
        static_cast<P089_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P089_taskdata) && P089_taskdata->isInitialized()) {
        success = P089_taskdata->loop();
      }
      break;
    }
    # endif // ifdef ESP32
    case PLUGIN_READ:
    {
      P089_data_struct *P089_taskdata =
        static_cast<P089_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr == P089_taskdata) || !P089_taskdata->isInitialized()) {
        break;
      }

      if (P089_taskdata->send_ping(event)) {
        UserVar.setFloat(event->TaskIndex, 0, UserVar.getFloat(event->TaskIndex, 0) + 1);
      }

      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      String command = parseString(string, 1);

      if (equals(command, F("pingset")))
      {
        const String taskName = parseString(string, 2);
        String param1         = parseString(string, 3);
        taskIndex_t taskIndex = findTaskIndexByName(taskName);

        if (param1.isEmpty() ||
            (!param1.isEmpty() && (taskIndex != TASKS_MAX) && (taskIndex == event->TaskIndex))) {
          int32_t val_new{};

          if (param1.isEmpty()) {
            param1 = taskName;
          }

          if (validIntFromString(param1, val_new)) {
            // Avoid overflow and weird values
            if ((val_new > -1024) && (val_new < 1024)) {
              UserVar.setFloat(event->TaskIndex, 0, val_new);
              success = true;
            }
          }
        }
      }
      break;
    }
  }
  return success;
}

#endif // ifdef USES_P089
