#ifdef USES_P098

// #######################################################################################################
// #################################### Plugin 098: ESPEasy-Now Receiver #################################
// #######################################################################################################

#include "_Plugin_Helper.h"

#include "src/Globals/ESPEasy_now_handler.h"


#define PLUGIN_098
#define PLUGIN_ID_098         98
#define PLUGIN_NAME_098       "Generic - ESPEasy-Now Receiver"
#define PLUGIN_VALUENAME1_098 "Value"


struct P098_data_struct : public PluginTaskData_base {
  P098_data_struct()  {}

  ~P098_data_struct() {}
};

boolean Plugin_098(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_098;
      Device[deviceCount].Type               = DEVICE_TYPE_DUMMY;
      Device[deviceCount].VType              = SENSOR_TYPE_STRING; // FIXME TD-er: Must make this the same as the sender.
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].DecimalsOnly       = true;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = false;
      Device[deviceCount].GlobalSyncOption   = false;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_098);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_098));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      // Do not set the sensor type, or else it will be set for all instances of the Dummy plugin.
      // sensorTypeHelper_setSensorType(event, 0);

      plugin_EspEasy_now_enabled = true;
      success                    = true;
      break;
    }

    case PLUGIN_EXIT:
    {
      plugin_EspEasy_now_enabled = false;
      success                    = true;
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      if (ESPEasy_now_handler.loop()) {
        // Some packet was handled, check if it is something for this plugin
      }
      break;
    }

    case PLUGIN_READ:
    {
      event->sensorType = PCONFIG(0);
      success           = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      // FIXME TD-er: Create commands for ESPEasy_now receiver
      String command = parseString(string, 1);

      if (command == F("espeasynow")) {
        String subcommand = parseString(string, 2);

        if (subcommand == F("")) {}
      }
      break;
    }
  }
  return success;
}

#endif // USES_P098
