#include "_Plugin_Helper.h"
#ifdef USES_P177

// #######################################################################################################
// ################################ Plugin 177 XDB401 I2C Pressure Sensor ################################
// #######################################################################################################

/** Changelog:
 * 2025-03-03 tonhuisman: Change Sensor_VType to SENSOR_TYPE_TEMP_BARO for correct handling by Domoticz HTTP Controller.
 * 2025-03-01 tonhuisman: Ensure all requested data is read from the sensor, to avoid read-stalling
 *                        Change: We're now ignoring the status bit to see if data is available, and just read it after a ~100 msec delay
 * 2025-02-27 tonhuisman: Swapped Temperature and Pressure values so they can be handled as a single device by Domoticz.
 *                        Re-ordered the settings options for a better grouping of related settings.
 * 2025-02-26 tonhuisman: Fix some typos in calculation.
 * 2025-02-25 tonhuisman: Repair broken temperature calculation.
 * 2025-02-24 tonhuisman: Adjust calculations to avoid zero-division, and use double for accuracy when available.
 *                        Add setting to ignore negative pressure values
 * 2025-02-23 tonhuisman: Add Generate events on pressure change and Raw data options
 *                        Fix pressure and temperature calculation
 * 2025-02-22 tonhuisman: Initial plugin development, direct I2C access,
 *                        based on this documentation: https://www.letscontrolit.com/forum/viewtopic.php?t=10568#p71993 (attachment)
 */

# define PLUGIN_177
# define PLUGIN_ID_177         177
# define PLUGIN_NAME_177       "Environment - XDB401 I2C Pressure"
# define PLUGIN_VALUENAME1_177 "Temperature"
# define PLUGIN_VALUENAME2_177 "Pressure"

# include "./src/PluginStructs/P177_data_struct.h"

boolean Plugin_177(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_177;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_TEMP_BARO;
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
      string = F(PLUGIN_NAME_177);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_177));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_177));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = P177_I2C_ADDR == event->Par1;
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P177_I2C_ADDR;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P177_PRESSURE_SCALE_FACTOR = 1000; // Default to 1000
      P177_GENERATE_EVENTS       = 1;    // Enabled
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Temperature offset"), F("tempoffset"), P177_TEMPERATURE_OFFSET);
      addUnit(F("x 0.1C"));
      addFormNote(F("Offset in units of 0.1 degree Celsius"));

      addFormNumericBox(F("Sensor Full-scale Pressure"), F("scale"), P177_PRESSURE_SCALE_FACTOR, 1);
      addUnit(F("psi, mBar, hPa, etc."));

      addFormCheckBox(F("Generate events on Pressure change"), F("pevents"), P177_GENERATE_EVENTS == 1);

      addFormCheckBox(F("Ignore negative pressure values"),    F("pneg"),    P177_IGNORE_NEGATIVE == 1);

      addFormCheckBox(F("Use raw data from sensor"),           F("praw"),    P177_RAW_DATA == 1);
      addFormNote(F("When checked: Temperature offset and Sensor Full-scale Pressure will be ignored!"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P177_TEMPERATURE_OFFSET    = getFormItemInt(F("tempoffset"), 0);
      P177_PRESSURE_SCALE_FACTOR = getFormItemInt(F("scale"), 1000);
      P177_GENERATE_EVENTS       = isFormItemChecked(F("pevents")) ? 1 : 0;
      P177_IGNORE_NEGATIVE       = isFormItemChecked(F("pneg")) ? 1 : 0;
      P177_RAW_DATA              = isFormItemChecked(F("praw")) ? 1 : 0;

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      success = initPluginTaskData(event->TaskIndex, new (std::nothrow) P177_data_struct(event));
      break;
    }

    case PLUGIN_READ:
    {
      P177_data_struct *P177_data = static_cast<P177_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P177_data) {
        success = P177_data->plugin_read(event);
      }

      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P177_data_struct *P177_data = static_cast<P177_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P177_data) {
        success = P177_data->plugin_ten_per_second(event);
      }

      break;
    }
  }
  return success;
}

#endif // ifdef USES_P177
