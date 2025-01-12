#include "_Plugin_Helper.h"
#if defined(USES_P089) && defined(ESP8266)

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
 * 2023-03-19 tonhuisman: Show hostname in GPIO column of Devices page
 * 2023-03-14 tonhuisman: Change command handling to not require the taskname as the second argument if no 3rd argument is given.
 *                        Set decimals to 0 whan adding the task.
 * 2023-03 Started changelog, not registered before.
 */


# define PLUGIN_089
# define PLUGIN_NAME_089           "Communication - Ping"
# define PLUGIN_VALUENAME1_089     "Fails"


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
      break;
    }

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
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
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
      break;
    }

    case PLUGIN_READ:
    {
      P089_data_struct *P089_taskdata =
        static_cast<P089_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P089_taskdata) {
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

#endif // if defined(USES_P089) && defined(ESP8266)
