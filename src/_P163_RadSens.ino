#include "_Plugin_Helper.h"
#ifdef USES_P163

// #######################################################################################################
// ######################## Plugin 163: Environment - RadSens I2C radiation counter ######################
// #######################################################################################################

/** Changelog:
 * 2024-08-23 tonhuisman: Add options to read new pulses only (default) instead of incrementing pulse count,
 *                        and reset on read, to clear the incrementing pulxe count
 * 2024-08-13 tonhuisman: Use pluginstats to get average over last n samples for determining event threshold
 *                        Add highvoltage subcommand to switch the high voltage off or on
 * 2024-08-12 tonhuisman: Start plugin for RadSens I2C radiation counter using RadSens library
 * (Newest changes on top)
 **/

/** Commands:
 * radsens,calibration,<calibrationvalue> : Set new Calibration value in impulses per millirad. Default 105 imp/uR.
 * radsens,highvoltage,<0|1>              : Switch the high voltage for the geiger tube off or on to reduce power consumption.
 */

# define PLUGIN_163
# define PLUGIN_ID_163          163
# define PLUGIN_NAME_163        "Environment - RadSens I2C radiation counter"
# define PLUGIN_VALUENAME1_163  "Count"
# define PLUGIN_VALUENAME2_163  "iDynamic"
# define PLUGIN_VALUENAME3_163  "iStatic"
# define PLUGIN_VALUENAME4_163  "IncrCount"

# include "./src/PluginStructs/P163_data_struct.h"

boolean Plugin_163(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_163;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_SINGLE;
      dev.FormulaOption  = true;
      dev.ValueCount     = 4;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.TimerOptional  = true;
      dev.PluginStats    = true;

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
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_163));

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
      P163_CFG_THRESHOLD = -1;                                     // Threshold disabled
      # if FEATURE_PLUGIN_STATS
      P163_CFG_COUNT_AVG = 10;                                     // Take average over last 10 values
      # endif // if FEATURE_PLUGIN_STATS

      Settings.TaskDeviceTimer[event->TaskIndex] = Settings.Delay; // Set default like non-TimerOptional

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormCheckBox(F("Read incremental count"), F("rinc"),   P163_GET_READ_INCREMENT);
      addFormCheckBox(F("Reset after read"),       F("rst"),    P163_GET_RESET_ON_READ);

      addFormCheckBox(F("Use Low Power mode"),     F("lpmode"), P163_GET_LOW_POWER);
      addFormCheckBox(F("Enable onboard Led"),     F("led"),    P163_GET_LED_STATE);

      addFormNumericBox(F("Events on Count-threshold"), F("chg"), P163_CFG_THRESHOLD, -1);
      addUnit(F("-1 = disabled"));

      # if FEATURE_PLUGIN_STATS
      addFormNumericBox(F("Use Count-average of values"), F("avg"), P163_CFG_COUNT_AVG, 1, PLUGIN_STATS_NR_ELEMENTS);
      addUnit(concat(F("1.."), PLUGIN_STATS_NR_ELEMENTS));
      addFormNote(F("Stats for the Count value must be enabled."));
      # endif // if FEATURE_PLUGIN_STATS

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P163_SET_READ_INCREMENT(isFormItemChecked(F("rinc")));
      P163_SET_RESET_ON_READ(isFormItemChecked(F("rst")));
      P163_SET_LOW_POWER(isFormItemChecked(F("lpmode")));
      P163_SET_LED_STATE(isFormItemChecked(F("led")));
      P163_CFG_THRESHOLD = getFormItemInt(F("chg"));
      # if FEATURE_PLUGIN_STATS
      P163_CFG_COUNT_AVG = getFormItemInt(F("avg"));
      # endif // if FEATURE_PLUGIN_STATS

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
