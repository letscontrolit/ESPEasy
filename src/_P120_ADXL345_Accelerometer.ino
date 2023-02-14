#include "_Plugin_Helper.h"

#ifdef USES_P120

// #######################################################################################################
// ############################## Plugin 120: Accelerometer - ADXL345 I2C ################################
// #######################################################################################################

/**
 * Plugin to support the ADXL345 Accelerometer, using the Sparkfun ADXL345 Arduino library
 */

/** Changelog:
 * 2023-01-09, tonhuisman: Fixed a bug that the Inactivity threshold wasn't saved, and thus not applied
 * 2021-12-10, tonhuisman: Split functional parts into P120_data_struc to re-use for P125 ADXL345 SPI plugin
 * 2021-11-22, tonhuisman: Move from DEVELOPMENT to TESTING
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
# define PLUGIN_NAME_120        "Accelerometer - ADXL345 (I2C)"


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
      Device[deviceCount].PluginStats    = true;
      Device[deviceCount].OutputDataType = Output_Data_type_t::Simple;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_120);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      P120_data_struct::plugin_get_device_value_names(event);
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P120_NR_OUTPUT_VALUES;
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(P120_SENSOR_TYPE_INDEX));
      event->idx        = P120_SENSOR_TYPE_INDEX;
      success           = true;
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES: {
      serialHelper_getGpioNames(event);
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

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P120_I2C_ADDR;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P120_I2C_ADDR       = 0x53; // Default I2C Address
      P120_AVERAGE_BUFFER = 10;   // Average averaging ;-)

      P120_data_struct *P120_data = new (std::nothrow) P120_data_struct(P120_AVERAGE_BUFFER);

      if (nullptr != P120_data) {
        P120_data->setI2Caddress(static_cast<uint8_t>(P120_I2C_ADDR));
        success = P120_data->plugin_set_defaults(event); // This shouldn't fail
        delete P120_data;
      }

      // No decimals plausible, as the outputs from the sensor are of type int
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0;
      }

      break;
    }

    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
    {
      success = P120_data_struct::plugin_webform_loadOutputSelector(event);
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      P120_data_struct *P120_data = new (std::nothrow) P120_data_struct(P120_AVERAGE_BUFFER);

      if (nullptr != P120_data) {
        P120_data->setI2Caddress(static_cast<uint8_t>(P120_I2C_ADDR));
        success = P120_data->plugin_webform_load(event); // This shouldn't fail
        delete P120_data;
      }
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P120_I2C_ADDR       = getFormItemInt(F("i2c_addr"));
      P120_AVERAGE_BUFFER = getFormItemInt(F("average_buf"));

      P120_data_struct *P120_data = new (std::nothrow) P120_data_struct(P120_AVERAGE_BUFFER);

      if (nullptr != P120_data) {
        P120_data->setI2Caddress(static_cast<uint8_t>(P120_I2C_ADDR));
        success = P120_data->plugin_webform_save(event); // This shouldn't fail
        delete P120_data;
      }
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P120_data_struct(P120_AVERAGE_BUFFER));
      P120_data_struct *P120_data = static_cast<P120_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P120_data) {
        P120_data->setI2Caddress(static_cast<uint8_t>(P120_I2C_ADDR));
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

    case PLUGIN_GET_CONFIG_VALUE:
    {
      P120_data_struct *P120_data =
        static_cast<P120_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P120_data) {
        success = P120_data->plugin_get_config_value(event, string);
      }
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P120_data_struct *P120_data = static_cast<P120_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P120_data) {
        success = P120_data->read_data(event);
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
