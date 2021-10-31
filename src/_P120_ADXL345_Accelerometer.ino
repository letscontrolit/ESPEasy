#include "_Plugin_Helper.h"

#ifdef USES_P120

// #######################################################################################################
// ################################ Plugin 120: AAccelerometer - ADXL345 #################################
// #######################################################################################################

/**
 */

/** Changelog:
 *
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

// Make accessing specific parameters more readable in the code
// # define P120_I2C_OR_SPI      PCONFIG(0)
# define P120_I2C_ADDR        PCONFIG(1) // Kept (1) from template
// # define P120_AVERAGE_BUFFER  PCONFIG(2)
// # define P120_FREQUENCY       PCONFIG(3)
// # define P120_FREQUENCY_10    0          // 10x per second
// # define P120_FREQUENCY_50    1          // 50x per second

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
      P120_I2C_ADDR       = 0x53; // Default I2C Address
      // P120_AVERAGE_BUFFER = 10;   // Average averaging ;-)

      // No decimals plausible, as the outputs from the sensor are of type int
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0;
      }

      success = true;
      break;
    }


    case PLUGIN_WEBFORM_LOAD:
    {
      // addFormCheckBox(F("Read 'raw' values from Gyro"), F("p120_rawData"), P120_RAW_DATA == 1);

      // addFormNumericBox(F("Averaging buffer size"), F("p120_average_buf"), P120_AVERAGE_BUFFER, 1, 100);
      // addUnit(F("1..100"));

      // const __FlashStringHelper *frequencyOptions[] = {
      //   F("10x per second"),
      //   F("50x per second") };
      // int frequencyValues[] = { P120_FREQUENCY_10, P120_FREQUENCY_50 };
      // addFormSelector(F("Measuring frequency"), F("p120_frequency"), 2, frequencyOptions, frequencyValues, P120_FREQUENCY);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P120_I2C_ADDR       = getFormItemInt(F("i2c_addr"));
      // P120_RAW_DATA       = isFormItemChecked(F("p120_rawData")) ? 1 : 0;
      // P120_AVERAGE_BUFFER = getFormItemInt(F("p120_average_buf"));
      // P120_FREQUENCY      = getFormItemInt(F("p120_frequency"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      // initPluginTaskData(event->TaskIndex, new (std::nothrow) P120_data_struct(P120_I2C_ADDR, P120_RAW_DATA, P120_AVERAGE_BUFFER));
      // P120_data_struct *P120_data = static_cast<P120_data_struct *>(getPluginTaskData(event->TaskIndex));

      // if (nullptr != P120_data) {
      //   success = true;
      // }

      break;
    }

    case PLUGIN_READ:
    {
      // P120_data_struct *P120_data = static_cast<P120_data_struct *>(getPluginTaskData(event->TaskIndex));

      // if (nullptr != P120_data) {
      //   success = P120_data->initialized();
      // }

      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      // P120_data_struct *P120_data = static_cast<P120_data_struct *>(getPluginTaskData(event->TaskIndex));

      // if (nullptr != P120_data) {
      //   int X, Y, Z;

      //   if (P120_data->read_data(X, Y, Z)) {
      //     UserVar[event->BaseVarIndex]     = X;
      //     UserVar[event->BaseVarIndex + 1] = Y;
      //     UserVar[event->BaseVarIndex + 2] = Z;

      //     success = true;
      //   }
      // }

      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    case PLUGIN_FIFTY_PER_SECOND:
    {
      // if (((function == PLUGIN_TEN_PER_SECOND) && (P120_FREQUENCY == P120_FREQUENCY_10)) ||
      //     ((function == PLUGIN_FIFTY_PER_SECOND) && (P120_FREQUENCY == P120_FREQUENCY_50))) {
      //   P120_data_struct *P120_data = static_cast<P120_data_struct *>(getPluginTaskData(event->TaskIndex));

      //   if (nullptr != P120_data) {
      //     success = P120_data->read_sensor();
      //   }
      // }

      break;
    }
  } // switch
  return success;
}   // function

#endif // ifdef USES_P120
