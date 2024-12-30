#ifdef USES_P152

// #######################################################################################################
// #################################### Plugin 152: ESP32 DAC ############################################
// ### DAC (Digital Analog converter) for ESP32 and ESP32-S2                                           ###
// #######################################################################################################

/** Changelog:
 * 2023-04-10 tonhuisman: Minor corrections
 * 2023-04-09 tonhuisman: Adopt to latest mega branch changes as the plugin is last changed some years ago
 * 2023-04-09 tonhuisman: Migrate plugin from ESPEasyPluginPlayground to mega branch
 */

#include "_Plugin_Helper.h"

#if defined(SOC_DAC_SUPPORTED) && SOC_DAC_SUPPORTED

#define PLUGIN_152
#define PLUGIN_ID_152         152
#define PLUGIN_NAME_152       "Output - ESP32 DAC"
#define PLUGIN_VALUENAME1_152 "Output"

#define P152_DAC_VALUE        UserVar[event->BaseVarIndex]
#define P152_SET_DAC_VALUE(x) UserVar.setFloat(event->TaskIndex, 0, x)


boolean Plugin_152(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_152;
      Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Custom             = true;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 1;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_152);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_152));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addRowLabel(F("Analog Output"));
      addDAC_PinSelect(F("taskdevicepin1"), CONFIG_PIN1);

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
      int dac;

      success = getDAC_gpio_info(CONFIG_PIN1, dac);
      break;
    }

    case PLUGIN_READ:
    {
      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      const String command = parseString(string, 1);

      // Command: dac,<dac>,<value> : <dac>: 1 or 2, <value>: 0..255
      if (equals(command, F("dac"))) {
        int dac;

        if (getDAC_gpio_info(CONFIG_PIN1, dac) && (dac == event->Par1)) {
          int value = min(255, max(0, event->Par2)); // Limit value
          P152_SET_DAC_VALUE(value);
          dacWrite(CONFIG_PIN1, value);              // Set output value
          addLog(LOG_LEVEL_INFO,
                 formatGpioName_DAC(CONFIG_PIN1) +
                 concat(F(" : Output: "), value));

          success = true;
        }
      }
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      int dac;

      if (getDAC_gpio_info(CONFIG_PIN1, dac)) {
        string += formatGpioName_DAC(CONFIG_PIN1);
        success = true;
      }
      break;
    }
  }

  return success;
}

#endif

#endif // USES_P152
