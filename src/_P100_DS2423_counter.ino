#include "_Plugin_Helper.h"
#ifdef USES_P100

// #######################################################################################################
// #################################### Plugin 100: Counter Dallas DS2423  ###############################
// #######################################################################################################

// Maxim Integrated (ex Dallas) DS2423 datasheet : https://datasheets.maximintegrated.com/en/ds/DS2423.pdf

/** Changelog:
 * 2025-06-14 tonhuisman: Add support for Custom Value Type per task value
 * 2025-03-15 tonhuisman: Add option in UI to enable the CountTotal value. When not enabled, still available via [<TaskName>#CountTotal]
 *                        Enable PluginStats feature. Initially set Decimals to 0.
 * 2025-03-06 tonhuisman: Add support for getting the (already fetched and stored) CountTotal value for the selected counter
 * 2025-01-12 tonhuisman: Add support for MQTT AutoDiscovery (not supported (yet?) for DS2423)
 */

# include "src/Helpers/Dallas1WireHelper.h"

# define PLUGIN_100
# define PLUGIN_ID_100         100
# define PLUGIN_NAME_100       "Pulse Counter - DS2423"
# define PLUGIN_VALUENAME1_100 "CountDelta"
# define PLUGIN_VALUENAME2_100 "CountTotal"

boolean Plugin_100(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_100;
      dev.Type           = DEVICE_TYPE_SINGLE;
      dev.VType          = Sensor_VType::SENSOR_TYPE_DUAL;
      dev.FormulaOption  = true;
      dev.ValueCount     = 2;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;
      dev.CustomVTypeVar = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_100);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_100));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_100));
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = 1 == PCONFIG(1) ? 2 : 1;
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = 1 == PCONFIG(1) ? Sensor_VType::SENSOR_TYPE_DUAL : Sensor_VType::SENSOR_TYPE_SINGLE;
      success           = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      ExtraTaskSettings.TaskDeviceValueDecimals[0] = 0; // Counters don't use decimals by default
      ExtraTaskSettings.TaskDeviceValueDecimals[1] = 0;
      break;
    }

    # if FEATURE_MQTT_DISCOVER || FEATURE_CUSTOM_TASKVAR_VTYPE
    case PLUGIN_GET_DISCOVERY_VTYPES:
    {
      #  if FEATURE_CUSTOM_TASKVAR_VTYPE

      for (uint8_t i = 0; i < event->Par5; ++i) {
        event->ParN[i] = ExtraTaskSettings.getTaskVarCustomVType(i);  // Custom/User selection
      }
      #  else // if FEATURE_CUSTOM_TASKVAR_VTYPE
      event->Par1 = static_cast<int>(Sensor_VType::SENSOR_TYPE_NONE); // Not yet supported
      #  endif // if FEATURE_CUSTOM_TASKVAR_VTYPE
      success = true;
      break;
    }
    # endif // if FEATURE_MQTT_DISCOVER || FEATURE_CUSTOM_TASKVAR_VTYPE

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_bidirectional(F("1-Wire"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNote(F("External pull up resistor is needed, see docs!"));

      // Scan the onewire bus and fill dropdown list with devicecount on this GPIO.
      const int8_t Plugin_100_DallasPin = CONFIG_PIN1;

      if (validGpio(Plugin_100_DallasPin)) {
        Dallas_addr_selector_webform_load(event->TaskIndex, Plugin_100_DallasPin, Plugin_100_DallasPin);

        // Counter select
        const __FlashStringHelper *resultsOptions[] = { F("A"), F("B") };
        constexpr size_t optionCount                = NR_ELEMENTS(resultsOptions);
        const FormSelectorOptions selector(optionCount, resultsOptions);
        selector.addFormSelector(F("Counter"), F("counter"), PCONFIG(0));
        addFormNote(F("Counter value is incremental"));
      }
      addFormCheckBox(F("Show CountTotal value"), F("ptot"), PCONFIG(1) == 1);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      // Counter choice
      PCONFIG(0) = getFormItemInt(F("counter"));

      // 1-wire device address
      Dallas_addr_selector_webform_save(event->TaskIndex, CONFIG_PIN1, CONFIG_PIN1);
      PCONFIG(1) = isFormItemChecked(F("ptot")) ? 1 : 0;

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      uint8_t addr[8];
      Dallas_plugin_get_addr(addr, event->TaskIndex);
      string  = Dallas_format_address(addr);
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      UserVar.setFloat(event->TaskIndex, 0, 0);
      UserVar.setFloat(event->TaskIndex, 1, 0);
      UserVar.setFloat(event->TaskIndex, 2, 0);

      if (validGpio(CONFIG_PIN1)) {
        // Explicitly set the pinMode using the "slow" pinMode function
        // This way we know for sure the state of any pull-up or -down resistor is known.
        pinMode(CONFIG_PIN1, INPUT);
        success = true; // Only start with a valid GPIO pin set
      }

      break;
    }

    case PLUGIN_READ:
    {
      uint8_t addr[8];
      Dallas_plugin_get_addr(addr, event->TaskIndex);

      if (addr[0] != 0) {
        if (validGpio(CONFIG_PIN1)) {
          float value = 0.0f;

          if (Dallas_readCounter(addr, &value, CONFIG_PIN1, CONFIG_PIN1, PCONFIG(0)))
          {
            UserVar.setFloat(event->TaskIndex, 0, UserVar[event->BaseVarIndex + 2] != 0
              ? value - UserVar[event->BaseVarIndex + 1]
              : 0);
            UserVar.setFloat(event->TaskIndex, 2, 1);
            UserVar.setFloat(event->TaskIndex, 1, value);
            success = true;
          }
          else
          {
            UserVar.setFloat(event->TaskIndex, 0, NAN);
          }

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = strformat(F("[P100]DS   : Counter %c :"), PCONFIG(0) == 0 ? 'A' : 'B');

            if (success) {
              log += formatUserVarNoCheck(event, 0);
            } else {
              log += F("Error!");
            }
            log += strformat(F(" (%s)"), Dallas_format_address(addr).c_str());
            addLogMove(LOG_LEVEL_INFO, log);
          }
        }
      }
      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      const String cmd = parseString(string, 1);

      if (equals(cmd, F("counttotal"))) {
        string  = formatUserVarNoCheck(event, 1); // Fetch stored value
        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P100
