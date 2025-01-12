#include "_Plugin_Helper.h"
#ifdef USES_P170

// #######################################################################################################
// ############################### Plugin 170: Input - I2C Liquid level sensor ###########################
// #######################################################################################################

/**
 * 2024-05-25 tonhuisman: Add optional logging at info level with received data
 * 2024-05-20 tonhuisman: Add low- and high-level trigger checks (0 = disabled), and trigger-once option with auto-reset
 *                        Trigger is checked at Interval setting, or once per second if Interval = 0.
 *                        Add sensitivity setting, to compensate for different liquids.
 * 2024-05-19 tonhuisman: Start plugin for Seeed studio I2C Liquid level sensor
 *                        Using direct I2C communication
 **/

# define PLUGIN_170
# define PLUGIN_ID_170          170
# define PLUGIN_NAME_170        "Input - I2C Liquid level sensor"
# define PLUGIN_VALUENAME1_170  "Level"
# define PLUGIN_VALUENAME2_170  "Steps"

# include "./src/PluginStructs/P170_data_struct.h"

boolean Plugin_170(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_170;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_QUAD;
      dev.FormulaOption  = true;
      dev.ValueCount     = 2;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.TimerOptional  = true;
      dev.PluginStats    = true;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_170);

      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_170));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_170));

      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      const uint8_t i2cAddressValues[] = { P170_I2C_ADDRESS, P170_I2C_ADDRESS_HIGH };

      success = intArrayContains(2, i2cAddressValues, event->Par1);

      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P170_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P170_STEP_ACTIVE_LEVEL = P170_STEP_ACTIVE_LEVEL_DEF;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Sensitivity"), F("sens"), P170_STEP_ACTIVE_LEVEL, 50, 254);
      addUnit(F("50..254"));

      addFormSubHeader(F("Events"));

      addFormNumericBox(F("Trigger on Low level"), F("low"), P170_TRIGGER_LOW_LEVEL, 0, 100);
      addUnit(F("0..100mm"));
      addFormNumericBox(F("Trigger on High level"), F("high"), P170_TRIGGER_HIGH_LEVEL, 0, 100);
      addUnit(F("0..100mm"));
      addFormNote(F("Trigger level 0 = Disabled, step size: " P170_MM_PER_STEP_STR "mm, (rounded down)"));
      addFormCheckBox(F("Trigger only once"), F("once"), P170_TRIGGER_ONCE);
      addFormNote(F("Auto-reset when level is correct again, separate for Low and High level."));

      addFormCheckBox(F("Log signal level"), F("log"), P170_ENABLE_LOG);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P170_STEP_ACTIVE_LEVEL  = getFormItemInt(F("sens"));
      P170_TRIGGER_LOW_LEVEL  = (getFormItemInt(F("low")) / P170_MM_PER_STEP) * P170_MM_PER_STEP;
      P170_TRIGGER_HIGH_LEVEL = (getFormItemInt(F("high")) / P170_MM_PER_STEP) * P170_MM_PER_STEP;
      P170_TRIGGER_ONCE       = isFormItemChecked(F("once"));
      P170_ENABLE_LOG         = isFormItemChecked(F("log"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P170_data_struct(P170_STEP_ACTIVE_LEVEL, P170_ENABLE_LOG));
      P170_data_struct *P170_data = static_cast<P170_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P170_data) && P170_data->init(event);

      break;
    }

    case PLUGIN_READ:
    {
      P170_data_struct *P170_data = static_cast<P170_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P170_data) {
        success = P170_data->plugin_read(event);
      }

      break;
    }
  }
  return success;
}

#endif // USES_P170
