#include "_Plugin_Helper.h"
#ifdef USES_P029

// #######################################################################################################
// #################################### Plugin 029: Output ###############################################
// #######################################################################################################

/** Changelog:
 * 2024-03-24 tonhuisman: Reformat source (Uncrustify), add option 'Invert On/Off value'
 * 2024-03-24 tonhuisman: Start Changelog (newest on top)
 */

# define PLUGIN_029
# define PLUGIN_ID_029         29
# define PLUGIN_NAME_029       "Output - Domoticz MQTT Helper"
# define PLUGIN_VALUENAME1_029 "Output"

# define P029_INVERTED PCONFIG(0)

boolean Plugin_029(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number             = PLUGIN_ID_029;
      dev.Type               = DEVICE_TYPE_SINGLE;
      dev.VType              = Sensor_VType::SENSOR_TYPE_SWITCH;
      dev.Ports              = 0;
      dev.ValueCount         = 1;
      dev.setPin1Direction(gpio_direction::gpio_output);
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_029);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_029));
      ExtraTaskSettings.TaskDeviceValueDecimals[0] = 0;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      // We need the index of the controller we are: 0-CONTROLLER_MAX
      uint8_t controllerNr = 0;

      for (controllerIndex_t i = 0; i < CONTROLLER_MAX; i++)
      {
        //            if (Settings.Protocol[i] == CPLUGIN_ID_002) { controllerNr = i; }   -> error: 'CPLUGIN_ID_002' was not declared in
        // this scope
        if (Settings.Protocol[i] == 2) { controllerNr = i; }
      }

      addRowLabel(F("IDX"));
      addNumericBox(
        concat(F("TDID"), controllerNr + 1), // ="taskdeviceid"
        Settings.TaskDeviceID[controllerNr][event->TaskIndex],
        0,
        DOMOTICZ_MAX_IDX);

      addFormCheckBox(F("Invert On/Off value"), F("inverted"), P029_INVERTED == 1);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P029_INVERTED = isFormItemChecked(F("inverted")) ? 1 : 0;
      success       = true;
      break;
    }
    case PLUGIN_INIT:
    {
      success = true;
      break;
    }
  }
  return success;
}

#endif // USES_P029
