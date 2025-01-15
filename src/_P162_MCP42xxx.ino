#include "_Plugin_Helper.h"
#ifdef USES_P162

// #######################################################################################################
// ################################ Plugin-162: MCP42xxx/MCP41xxx Digipot ################################
// #######################################################################################################

/** Changelog:
 * 2024-04-16 tonhuisman: Add Send values on change option, so Interval can be set to 0, and the data will be sent when changed
 * 2024-04-10 tonhuisman: Initial version. Support for Digipot MCP42xxx (dual channel) and MCP41xxx (single channel).
 *                        No support for daisy-chaining (MCP42xxx can do that, but not implemented)
 *                        RST and SHDN pins are not available on all boards, so should be set to none when not available.
 * 2024-04-08 tonhuisman: Start plugin development
 */

# define PLUGIN_162
# define PLUGIN_ID_162         162
# define PLUGIN_NAME_162       "Output - MCP42xxx Digipot"
# define PLUGIN_VALUENAME1_162 "W0"
# define PLUGIN_VALUENAME2_162 "W1"

# include "src/PluginStructs/P162_data_struct.h"

boolean Plugin_162(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_162;
      dev.Type           = DEVICE_TYPE_SPI3;
      dev.VType          = Sensor_VType::SENSOR_TYPE_DUAL;
      dev.ValueCount     = 2;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.TimerOptional  = true;
      dev.FormulaOption  = true;
      dev.PluginStats    = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_162);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_162));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_162));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:                                   // define 'GPIO 1st' name in webserver
    {
      event->String1 = formatGpioName_output(F("CS PIN"));             // P162_CS_PIN
      event->String2 = formatGpioName_output_optional(F("RST PIN "));  // P162_RST_PIN
      event->String3 = formatGpioName_output_optional(F("SHDN PIN ")); // P162_SHD_PIN
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P162_INIT_W0        = P162_RESET_VALUE;
      P162_INIT_W1        = P162_RESET_VALUE;
      P162_SHUTDOWN_VALUE = -1;

      ExtraTaskSettings.TaskDeviceValueDecimals[0] = 0;
      ExtraTaskSettings.TaskDeviceValueDecimals[1] = 0;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Initial value W0"), F("iw0"), P162_INIT_W0, 0, 255);
      addUnit(F("0..255"));
      addFormCheckBox(F("Initial shutdown W0"), F("sw0"), P162_SHUTDOWN_W0);

      addFormSeparator(2);

      addFormNumericBox(F("Initial value W1"), F("iw1"), P162_INIT_W1, 0, 255);
      addUnit(F("0..255"));
      addFormCheckBox(F("Initial shutdown W1"), F("sw1"), P162_SHUTDOWN_W0);

      addFormSeparator(2);

      addFormNumericBox(F("Value at Shutdown"), F("shd"), P162_SHUTDOWN_VALUE, -1, 256);
      addUnit(F("-1..256"));

      addFormCheckBox(F("Send values on change"), F("chg"), P162_CHANGED_EVENTS);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P162_INIT_W0        = getFormItemInt(F("iw0"));
      P162_INIT_W1        = getFormItemInt(F("iw1"));
      P162_SHUTDOWN_W0    = isFormItemChecked(F("sw0"));
      P162_SHUTDOWN_W1    = isFormItemChecked(F("sw1"));
      P162_SHUTDOWN_VALUE = getFormItemInt(F("shd"));
      P162_CHANGED_EVENTS = isFormItemChecked(F("chg"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P162_data_struct(P162_CS_PIN, P162_RST_PIN, P162_SHD_PIN));
      P162_data_struct *P162_data = static_cast<P162_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P162_data) {
        success = P162_data->plugin_init(event);
      }

      break;
    }

    case PLUGIN_READ:
    {
      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      P162_data_struct *P162_data = static_cast<P162_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P162_data) {
        success = P162_data->plugin_write(event, string);
      }

      break;
    }
  }
  return success;
}

#endif // USES_P162
