#include "_Plugin_Helper.h"
#ifdef USES_P163

// #######################################################################################################
// ########################## Plugin 163: Counter - RadSens I2C radiation counter ########################
// #######################################################################################################

/** Changelog:
 * 2024-08-12 tonhuisman: Start plugin for RadSens I2C radiation counter using RadSens library
 * (Newest changes on top)
 **/

/** Commands:
 * radsens,calibration,<calibrationvalue> : Set new Calibration value in impulses per millirad. Default 105 imp/uR.
 */

# define PLUGIN_163
# define PLUGIN_ID_163          163
# define PLUGIN_NAME_163        "Counter - RadSens I2C radiation counter"
# define PLUGIN_VALUENAME1_163  "Count"
# define PLUGIN_VALUENAME2_163  "iDynamic"
# define PLUGIN_VALUENAME3_163  "iStatic"

# include "./src/PluginStructs/P163_data_struct.h"

boolean Plugin_163(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number         = PLUGIN_ID_163;
      Device[deviceCount].Type             = DEVICE_TYPE_I2C;
      Device[deviceCount].VType            = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports            = 0;
      Device[deviceCount].FormulaOption    = true;
      Device[deviceCount].ValueCount       = 3;
      Device[deviceCount].SendDataOption   = true;
      Device[deviceCount].TimerOption      = true;
      Device[deviceCount].TimerOptional    = true;
      Device[deviceCount].GlobalSyncOption = true;
      Device[deviceCount].PluginStats      = true;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_163);

      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_163));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_163));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_163));

      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = (RS_DEFAULT_I2C_ADDRESS == event->Par1);

      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = RS_DEFAULT_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P163_SET_LED_STATE(true);                                    // Device defaults
      P163_SET_LOW_POWER(false);
      P163_CFG_THRESHOLD                         = -1;             // Threshold disabled
      Settings.TaskDeviceTimer[event->TaskIndex] = Settings.Delay; // Set default like non-TimerOptional

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormCheckBox(F("Use Low Power mode"), F("lpmode"), P163_GET_LOW_POWER);
      addFormCheckBox(F("Enable onboard Led"), F("led"),    P163_GET_LED_STATE);
      addFormNumericBox(F("Events on Count-threshold"), F("chg"), P163_CFG_THRESHOLD, -1);
      addUnit(F("-1 = disabled"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P163_SET_LOW_POWER(isFormItemChecked(F("lpmode")));
      P163_SET_LED_STATE(isFormItemChecked(F("led")));
      P163_CFG_THRESHOLD = getFormItemInt(F("chg"));

      success = true;

      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P163_data_struct(event));
      P163_data_struct *P163_data = static_cast<P163_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P163_data) && P163_data->init(event);

      break;
    }

    case PLUGIN_READ:
    {
      P163_data_struct *P163_data = static_cast<P163_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P163_data) {
        success = P163_data->plugin_read(event);
      }

      break;
    }

    case PLUGIN_WRITE:
    {
      P163_data_struct *P163_data = static_cast<P163_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P163_data) {
        success = P163_data->plugin_write(event, string);
      }

      break;
    }
  }

  return success;
}

#endif // USES_P163
