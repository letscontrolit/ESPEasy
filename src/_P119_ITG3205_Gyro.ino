#include "_Plugin_Helper.h"

#ifdef USES_P119

// #######################################################################################################
// #################################### Plugin 119: Gyro - ITG3205 #######################################
// #######################################################################################################

/**
 * Gyro plugin for ITG3205 chip. This chip is often combined with ADXL345 and HMC5883L on a single board, and then will
 * support only the default I2C address. For a 'standalone' chip/board, often an AD0 pin is available to select the
 * secondary I2C address.
 */

/** Changelog:
 *
 * 2021-11-22, tonhuisman: Moved from DEVELOPMENT to TESTING 'status'
 * 2021-10-28, tonhuisman: Tested reading (chip) temperature measurement, but it isn't useful, so removed again.
 * 2021-10-26, tonhuisman: Add averaging and frequency features
 * 2021-10-24, tonhuisman: Initial plugin created from template, using ITG3205 library https://github.com/ikiselev/ITG3205
 *
 *************************************************************************************************************************/

// #include section
# include "src/PluginStructs/P119_data_struct.h"

# define PLUGIN_119
# define PLUGIN_ID_119          119 // plugin id
# define PLUGIN_NAME_119        "Gyro - ITG3205"
# define PLUGIN_VALUENAME1_119  "X"
# define PLUGIN_VALUENAME2_119  "Y"
# define PLUGIN_VALUENAME3_119  "Z"

// Make accessing specific parameters more readable in the code
# define P119_RAW_DATA        PCONFIG(0)
# define P119_I2C_ADDR        PCONFIG(1) // Kept (1) from template
# define P119_AVERAGE_BUFFER  PCONFIG(2)
# define P119_FREQUENCY       PCONFIG(3)
# define P119_FREQUENCY_10    0          // 10x per second
# define P119_FREQUENCY_50    1          // 50x per second

boolean Plugin_119(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number       = PLUGIN_ID_119;
      Device[deviceCount].Type           = DEVICE_TYPE_I2C;
      Device[deviceCount].VType          = Sensor_VType::SENSOR_TYPE_TRIPLE;
      Device[deviceCount].Ports          = 0;
      Device[deviceCount].ValueCount     = 3;
      Device[deviceCount].FormulaOption  = true;
      Device[deviceCount].SendDataOption = true;
      Device[deviceCount].TimerOption    = true;
      Device[deviceCount].TimerOptional  = true;
      Device[deviceCount].PluginStats    = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_119);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_119));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_119));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_119));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x68, 0x69 };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 2, i2cAddressValues, P119_I2C_ADDR);
        addFormNote(F("AD0 Low=0x68, High=0x69"));
      } else {
        success = intArrayContains(2, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P119_I2C_ADDR;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P119_I2C_ADDR       = 0x68; // Default I2C Address
      P119_AVERAGE_BUFFER = 10;   // Average averaging ;-)

      // No decimals plausible, as the outputs from the sensor are of type int
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0;
      }

      success = true;
      break;
    }


    case PLUGIN_WEBFORM_LOAD:
    {
      addFormCheckBox(F("Read 'raw' values from Gyro"), F("rawData"), P119_RAW_DATA == 1);

      addFormNumericBox(F("Averaging buffer size"), F("average_buf"), P119_AVERAGE_BUFFER, 1, 100);
      addUnit(F("1..100"));

      const __FlashStringHelper *frequencyOptions[] = {
        F("10"),
        F("50") };
      const int frequencyValues[] = { P119_FREQUENCY_10, P119_FREQUENCY_50 };
      addFormSelector(F("Measuring frequency"), F("frequency"), 2, frequencyOptions, frequencyValues, P119_FREQUENCY);
      addUnit(F("Hz"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P119_I2C_ADDR       = getFormItemInt(F("i2c_addr"));
      P119_RAW_DATA       = isFormItemChecked(F("rawData")) ? 1 : 0;
      P119_AVERAGE_BUFFER = getFormItemInt(F("average_buf"));
      P119_FREQUENCY      = getFormItemInt(F("frequency"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P119_data_struct(P119_I2C_ADDR, P119_RAW_DATA, P119_AVERAGE_BUFFER));
      P119_data_struct *P119_data = static_cast<P119_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P119_data);

      break;
    }

    case PLUGIN_READ:
    {
      P119_data_struct *P119_data = static_cast<P119_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P119_data) {
        success = P119_data->initialized();
      }

      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P119_data_struct *P119_data = static_cast<P119_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P119_data) {
        int X, Y, Z;

        if (P119_data->read_data(X, Y, Z)) {
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
      if (((function == PLUGIN_TEN_PER_SECOND) && (P119_FREQUENCY == P119_FREQUENCY_10)) ||
          ((function == PLUGIN_FIFTY_PER_SECOND) && (P119_FREQUENCY == P119_FREQUENCY_50))) {
        P119_data_struct *P119_data = static_cast<P119_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P119_data) {
          success = P119_data->read_sensor();
        }
      }

      break;
    }
  } // switch
  return success;
}   // function

#endif // ifdef USES_P119
