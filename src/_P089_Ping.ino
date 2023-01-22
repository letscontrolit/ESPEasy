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
      Device[++deviceCount].Number           = PLUGIN_ID_089;
      Device[deviceCount].Type               = DEVICE_TYPE_DUMMY;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].DecimalsOnly       = true;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
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

    case PLUGIN_WEBFORM_LOAD:
    {
      char hostname[PLUGIN_089_HOSTNAME_SIZE];
      LoadCustomTaskSettings(event->TaskIndex, (uint8_t *)&hostname, PLUGIN_089_HOSTNAME_SIZE);
      addFormTextBox(F("Hostname"), F("host"), hostname, PLUGIN_089_HOSTNAME_SIZE - 2);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      char hostname[PLUGIN_089_HOSTNAME_SIZE] = {};

      // Reset "Fails" if settings updated
      UserVar[event->BaseVarIndex] = 0;
      strncpy(hostname, webArg(F("host")).c_str(), sizeof(hostname));
      SaveCustomTaskSettings(event->TaskIndex, (uint8_t *)&hostname, PLUGIN_089_HOSTNAME_SIZE);
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P089_data_struct());
      UserVar[event->BaseVarIndex] = 0;
      success                      = true;
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
        UserVar[event->BaseVarIndex]++;
      }

      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      String command = parseString(string, 1);

      if (command.equals(F("pingset")))
      {
        String taskName       = parseString(string, 2);
        taskIndex_t taskIndex = findTaskIndexByName(taskName);

        if ((taskIndex != TASKS_MAX) && (taskIndex == event->TaskIndex)) {
          success = true;
          String param1 = parseString(string, 3);
          int    val_new;

          if (validIntFromString(param1, val_new)) {
            // Avoid overflow and weird values
            if ((val_new > -1024) && (val_new < 1024)) {
              UserVar[event->BaseVarIndex] = val_new;
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
