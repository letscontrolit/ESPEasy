#include "_Plugin_Helper.h"

#ifdef USES_P120

// #######################################################################################################
// ################################ Plugin 120: Accelerometer - ADXL345 ##################################
// #######################################################################################################

/**
 * Plugin to support the ADXL345 Accelerometer, using the Sparkfun ADXL345 Arduino library
 */

/** Changelog:
 *
 * 2021-11-02, tonhuisman: Add Axis offsets for calibration
 * 2021-11-01, tonhuisman: Add event processing (pseudo-interrupts), improve settings
 * 2021-10-31, tonhuisman: Add Single/Double-tap and Freefall detection
 * 2021-10-30, tonhuisman: Add required settings for sensor, read X/Y/Z values
 *                         Add get2BitFromUL/set2BitToUL/get3BitFromUL/set3BitToUL support functions
 * 2021-10-29, tonhuisman: Initial plugin created from template, using Sparkfun ADXL345 library
 *                         https://github.com/sparkfun/SparkFun_ADXL345_Arduino_Library
 *
 *************************************************************************************************************************/

// #include section
# include "src/PluginStructs/P120_data_struct.h"

# define PLUGIN_120
# define PLUGIN_ID_120          120 // plugin id
# define PLUGIN_NAME_120        "Accelerometer - ADXL345 (I2C) [DEVELOPMENT]"
# define PLUGIN_VALUENAME1_120  "X"
# define PLUGIN_VALUENAME2_120  "Y"
# define PLUGIN_VALUENAME3_120  "Z"

boolean Plugin_120(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number       = PLUGIN_ID_120;
      Device[deviceCount].Type           = DEVICE_TYPE_I2C;
      Device[deviceCount].VType          = Sensor_VType::SENSOR_TYPE_TRIPLE;
      Device[deviceCount].Ports          = 0;
      Device[deviceCount].ValueCount     = 3;
      Device[deviceCount].FormulaOption  = true;
      Device[deviceCount].SendDataOption = true;
      Device[deviceCount].TimerOption    = true;
      Device[deviceCount].TimerOptional  = true;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_120);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_120));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_120));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_120));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x53, 0x1D };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 2, i2cAddressValues, P120_I2C_ADDR);
        addFormNote(F("AD0 Low=0x53, High=0x1D"));
      } else {
        success = intArrayContains(2, i2cAddressValues, event->Par1);
      }
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P120_I2C_ADDR       = 0x53;                            // Default I2C Address
      P120_AVERAGE_BUFFER = 10;                              // Average averaging ;-)
      uint32_t flags = 0ul;
      set2BitToUL(flags, P120_FLAGS1_RANGE, P120_RANGE_16G); // Default to 16g range for highest resolution
      bitSet(flags, P120_FLAGS1_ACTIVITY_X);                 // Detect activity on all axes
      bitSet(flags, P120_FLAGS1_ACTIVITY_Y);
      bitSet(flags, P120_FLAGS1_ACTIVITY_Z);
      set8BitToUL(flags, P120_FLAGS1_ACTIVITY_TRESHOLD,   P120_DEFAULT_ACTIVITY_TRESHOLD);
      set8BitToUL(flags, P120_FLAGS1_INACTIVITY_TRESHOLD, P120_DEFAULT_INACTIVITY_TRESHOLD);
      P120_CONFIG_FLAGS1 = flags;

      flags = 0ul;
      set8BitToUL(flags, P120_FLAGS2_TAP_TRESHOLD,    P120_DEFAULT_TAP_TRESHOLD);
      set8BitToUL(flags, P120_FLAGS2_TAP_DURATION,    P120_DEFAULT_TAP_DURATION);
      set8BitToUL(flags, P120_FLAGS2_DBL_TAP_LATENCY, P120_DEFAULT_DBL_TAP_LATENCY);
      set8BitToUL(flags, P120_FLAGS2_DBL_TAP_WINDOW,  P120_DEFAULT_DBL_TAP_WINDOW);
      P120_CONFIG_FLAGS2 = flags;

      flags = 0ul;
      set8BitToUL(flags, P120_FLAGS3_FREEFALL_TRESHOLD, P120_DEFAULT_FREEFALL_TRESHOLD);
      set8BitToUL(flags, P120_FLAGS3_FREEFALL_DURATION, P120_DEFAULT_FREEFALL_DURATION);
      P120_CONFIG_FLAGS3 = flags;

      // No decimals plausible, as the outputs from the sensor are of type int
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0;
      }

      success = true;
      break;
    }


    case PLUGIN_WEBFORM_LOAD:
    {
      // Range
      {
        const __FlashStringHelper *rangeOptions[] = {
          F("2g"),
          F("4g"),
          F("8g"),
          F("16g (default)") };
        int rangeValues[] = { P120_RANGE_2G, P120_RANGE_4G, P120_RANGE_8G, P120_RANGE_16G };
        addFormSelector(F("Range"), F("p120_range"), 4, rangeOptions, rangeValues,
                        get2BitFromUL(P120_CONFIG_FLAGS1, P120_FLAGS1_RANGE));
      }

      // Axis selection
      {
        addFormCheckBox(F("X-axis activity sensing"), F("p120_activity_x"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_X) == 1);
        addFormCheckBox(F("Y-axis activity sensing"), F("p120_activity_y"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_Y) == 1);
        addFormCheckBox(F("Z-axis activity sensing"), F("p120_activity_z"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_Z) == 1);
        addFormNumericBox(F("Activity treshold"), F("p120_activity_treshold"),
                          get8BitFromUL(P120_CONFIG_FLAGS1, P120_FLAGS1_ACTIVITY_TRESHOLD), 1, 255);
        addUnit(F("1..255 * 62.5 mg"));
        addFormNumericBox(F("In-activity treshold"), F("p120_inactivity_treshold"),
                          get8BitFromUL(P120_CONFIG_FLAGS1, P120_FLAGS1_INACTIVITY_TRESHOLD), 1, 255);
        addUnit(F("1..255 * 62.5 mg"));
      }

      // Activity logging and send events for (in)activity
      {
        addFormCheckBox(F("Enable (in)activity events"), F("p120_send_activity"),
                        bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_SEND_ACTIVITY) == 1);
        addFormCheckBox(F("Log sensor activity (INFO)"), F("p120_log_activity"),
                        bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_LOG_ACTIVITY) == 1);
        addFormCheckBox(F("Events with raw measurements"), F("p120_raw_measurement"),
                        bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_EVENT_RAW_VALUES) == 1);
      }

      // Tap detection
      {
        addFormSubHeader(F("Tap detection"));

        addFormCheckBox(F("X-axis"), F("p120_tap_x"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_TAP_X) == 1);
        addFormCheckBox(F("Y-axis"), F("p120_tap_y"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_TAP_Y) == 1);
        addFormCheckBox(F("Z-axis"), F("p120_tap_z"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_TAP_Z) == 1);
        addFormNote(F("Also enables taskname#Tapped event."));
        addFormNumericBox(F("Tap treshold"), F("p120_tap_treshold"),
                          get8BitFromUL(P120_CONFIG_FLAGS2, P120_FLAGS2_TAP_TRESHOLD), 1, 255);
        addUnit(F("1..255 * 62.5 mg"));
        addFormNumericBox(F("Tap duration"), F("p120_tap_duration"),
                          get8BitFromUL(P120_CONFIG_FLAGS2, P120_FLAGS2_TAP_DURATION), 1, 255);
        addUnit(F("1..255 * 625 &micro;s"));
      }

      // Double-tap detection
      {
        addFormCheckBox(F("Enable double-tap detection"), F("p120_dbl_tap"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_DBL_TAP) == 1);
        addFormNote(F("Also enables taskname#DoubleTapped event."));
        addFormNumericBox(F("Double-tap latency"), F("p120_dbl_tap_latency"),
                          get8BitFromUL(P120_CONFIG_FLAGS2, P120_FLAGS2_DBL_TAP_LATENCY), 1, 255);
        addUnit(F("1..255 * 1.25 ms"));
        addFormNumericBox(F("Double-tap window"), F("p120_dbl_tap_window"),
                          get8BitFromUL(P120_CONFIG_FLAGS2, P120_FLAGS2_DBL_TAP_WINDOW), 1, 255);
        addUnit(F("1..255 * 1.25 ms"));
      }

      // Free-fall detection
      {
        addFormSubHeader(F("Free-fall detection"));

        addFormCheckBox(F("Enable free-fall detection"), F("p120_free_fall"), bitRead(P120_CONFIG_FLAGS1, P120_FLAGS1_FREE_FALL) == 1);
        addFormNote(F("Also enables taskname#FreeFall event."));
        addFormNumericBox(F("Free-fall treshold"), F("p120_free_fall_treshold"),
                          get8BitFromUL(P120_CONFIG_FLAGS3, P120_FLAGS3_FREEFALL_TRESHOLD), 1, 255);
        addUnit(F("1..255 * 62.5 mg"));
        addFormNumericBox(F("Free-fall duration"), F("p120_free_fall_duration"),
                          get8BitFromUL(P120_CONFIG_FLAGS3, P120_FLAGS3_FREEFALL_DURATION), 1, 255);
        addUnit(F("1..255 * 625 &micro;s"));
      }

      // Axis Offsets (calibration)
      {
        addFormSubHeader(F("Axis calibration"));
        addFormNumericBox(F("X-Axis offset"), F("p120_offset_x"),
                          get8BitFromUL(P120_CONFIG_FLAGS4, P120_FLAGS4_OFFSET_X) - 0x80, -127, 127);
        addUnit(F("-127..127"));
        addFormNumericBox(F("Y-Axis offset"), F("p120_offset_y"),
                          get8BitFromUL(P120_CONFIG_FLAGS4, P120_FLAGS4_OFFSET_Y) - 0x80, -127, 127);
        addUnit(F("-127..127"));
        addFormNumericBox(F("Z-Axis offset"), F("p120_offset_z"),
                          get8BitFromUL(P120_CONFIG_FLAGS4, P120_FLAGS4_OFFSET_Z) - 0x80, -127, 127);
        addUnit(F("-127..127"));
      }

      // Data retrieval options
      {
        addFormSubHeader(F("Data retrieval"));

        addFormNumericBox(F("Averaging buffer size"), F("p120_average_buf"), P120_AVERAGE_BUFFER, 1, 100);
        addUnit(F("1..100"));

        const __FlashStringHelper *frequencyOptions[] = {
          F("10x per second"),
          F("50x per second") };
        int frequencyValues[] = { P120_FREQUENCY_10, P120_FREQUENCY_50 };
        addFormSelector(F("Measuring frequency"), F("p120_frequency"), 2, frequencyOptions, frequencyValues, P120_FREQUENCY);
        addFormNote(F("Values X/Y/Z are updated 1x per second, Controller updates &amp; Value-events are based on 'Interval' setting."));
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P120_I2C_ADDR = getFormItemInt(F("i2c_addr"));

      // P120_RAW_DATA       = isFormItemChecked(F("p120_rawData")) ? 1 : 0;
      P120_AVERAGE_BUFFER = getFormItemInt(F("p120_average_buf"));
      P120_FREQUENCY      = getFormItemInt(F("p120_frequency"));
      uint32_t flags = 0ul;
      set2BitToUL(flags, P120_FLAGS1_RANGE, getFormItemInt(F("p120_range")));
      bitWrite(flags, P120_FLAGS1_ACTIVITY_X,       isFormItemChecked(F("p120_activity_x")));
      bitWrite(flags, P120_FLAGS1_ACTIVITY_Y,       isFormItemChecked(F("p120_activity_y")));
      bitWrite(flags, P120_FLAGS1_ACTIVITY_Z,       isFormItemChecked(F("p120_activity_z")));
      bitWrite(flags, P120_FLAGS1_TAP_X,            isFormItemChecked(F("p120_tap_x")));
      bitWrite(flags, P120_FLAGS1_TAP_Y,            isFormItemChecked(F("p120_tap_y")));
      bitWrite(flags, P120_FLAGS1_TAP_Z,            isFormItemChecked(F("p120_tap_z")));
      bitWrite(flags, P120_FLAGS1_DBL_TAP,          isFormItemChecked(F("p120_dbl_tap")));
      bitWrite(flags, P120_FLAGS1_FREE_FALL,        isFormItemChecked(F("p120_free_fall")));
      bitWrite(flags, P120_FLAGS1_SEND_ACTIVITY,    isFormItemChecked(F("p120_send_activity")));
      bitWrite(flags, P120_FLAGS1_LOG_ACTIVITY,     isFormItemChecked(F("p120_log_activity")));
      bitWrite(flags, P120_FLAGS1_EVENT_RAW_VALUES, isFormItemChecked(F("p120_raw_measurement")));
      set8BitToUL(flags, P120_FLAGS1_ACTIVITY_TRESHOLD,   getFormItemInt(F("p120_activity_treshold")));
      set8BitToUL(flags, P120_FLAGS1_INACTIVITY_TRESHOLD, getFormItemInt(F("p120_inactivity_treshold")));
      P120_CONFIG_FLAGS1 = flags;

      flags = 0ul;
      set8BitToUL(flags, P120_FLAGS2_TAP_TRESHOLD,    getFormItemInt(F("p120_tap_treshold")));
      set8BitToUL(flags, P120_FLAGS2_TAP_DURATION,    getFormItemInt(F("p120_tap_duration")));
      set8BitToUL(flags, P120_FLAGS2_DBL_TAP_LATENCY, getFormItemInt(F("p120_dbl_tap_latency")));
      set8BitToUL(flags, P120_FLAGS2_DBL_TAP_WINDOW,  getFormItemInt(F("p120_dbl_tap_window")));
      P120_CONFIG_FLAGS2 = flags;

      flags = 0ul;
      set8BitToUL(flags, P120_FLAGS3_FREEFALL_TRESHOLD, getFormItemInt(F("p120_free_fall_treshold")));
      set8BitToUL(flags, P120_FLAGS3_FREEFALL_DURATION, getFormItemInt(F("p120_free_fall_duration")));
      P120_CONFIG_FLAGS3 = flags;

      flags = 0ul;
      set8BitToUL(flags, P120_FLAGS4_OFFSET_X, getFormItemInt(F("p120_offset_x")) + 0x80);
      set8BitToUL(flags, P120_FLAGS4_OFFSET_Y, getFormItemInt(F("p120_offset_y")) + 0x80);
      set8BitToUL(flags, P120_FLAGS4_OFFSET_Z, getFormItemInt(F("p120_offset_z")) + 0x80);
      P120_CONFIG_FLAGS4 = flags;

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P120_data_struct(P120_I2C_ADDR, P120_AVERAGE_BUFFER));
      P120_data_struct *P120_data = static_cast<P120_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P120_data) {
        success = true;
      }

      break;
    }

    case PLUGIN_READ:
    {
      P120_data_struct *P120_data = static_cast<P120_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P120_data) {
        success = P120_data->initialized();
      }

      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P120_data_struct *P120_data = static_cast<P120_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P120_data) {
        int X, Y, Z;

        if (P120_data->read_data(event, X, Y, Z)) {
          UserVar[event->BaseVarIndex]     = X;
          UserVar[event->BaseVarIndex + 1] = Y;
          UserVar[event->BaseVarIndex + 2] = Z;

          success = true;
        }
      }

      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    case PLUGIN_FIFTY_PER_SECOND:
    {
      if (((function == PLUGIN_TEN_PER_SECOND) && (P120_FREQUENCY == P120_FREQUENCY_10)) ||
          ((function == PLUGIN_FIFTY_PER_SECOND) && (P120_FREQUENCY == P120_FREQUENCY_50))) {
        P120_data_struct *P120_data = static_cast<P120_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P120_data) {
          success = P120_data->read_sensor(event);
        }
      }

      break;
    }
  } // switch
  return success;
}   // function

#endif // ifdef USES_P120
