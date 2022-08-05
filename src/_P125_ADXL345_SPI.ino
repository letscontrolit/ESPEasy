#include "_Plugin_Helper.h"

#ifdef USES_P125

// #######################################################################################################
// ############################## Plugin 125: Accelerometer - ADXL345 SPI ################################
// #######################################################################################################

/**
 * Plugin to support the ADXL345 Accelerometer, using the Sparkfun ADXL345 Arduino library using SPI interface
 */

/** Changelog:
 * 2021-12-10, tonhuisman, Start SPI interface version of ADXL345 plugin, based on P120 ADXL345 I2C plugin
 *                         Using Sparkfun ADXL345 library
 *                         https://github.com/sparkfun/SparkFun_ADXL345_Arduino_Library
 *
 *************************************************************************************************************************/

// ****************************************************************************************************************************
// ############################################################################################################################
// THIS PLUGIN USES THE DATA STRUCTURES FROM PLUGIN P120, AS IT IS BASICALLY THE SAME PLUGIN, JUST USING A DIFFERENT INTERFACE
// ############################################################################################################################
// ****************************************************************************************************************************

// #include section
# include "src/PluginStructs/P120_data_struct.h"

# define PLUGIN_125
# define PLUGIN_ID_125          125 // plugin id
# define PLUGIN_NAME_125        "Accelerometer - ADXL345 (SPI)"
# define PLUGIN_VALUENAME1_125  "X"
# define PLUGIN_VALUENAME2_125  "Y"
# define PLUGIN_VALUENAME3_125  "Z"

boolean Plugin_125(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number       = PLUGIN_ID_125;
      Device[deviceCount].Type           = DEVICE_TYPE_SPI;
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
      string = F(PLUGIN_NAME_125);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_125));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_125));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_125));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("CS"));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P120_CS_PIN         = -1; // Default not selected
      P120_AVERAGE_BUFFER = 10; // Average averaging ;-)

      P120_data_struct *P120_data = new (std::nothrow) P120_data_struct(static_cast<int>(P120_CS_PIN), P120_AVERAGE_BUFFER);

      if (nullptr != P120_data) {
        success = P120_data->plugin_set_defaults(event); // This shouldn't fail
        delete P120_data;
      }

      // No decimals plausible, as the outputs from the sensor are of type int
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0;
      }

      break;
    }


    case PLUGIN_WEBFORM_LOAD:
    {
      P120_data_struct *P120_data = new (std::nothrow) P120_data_struct(static_cast<int>(P120_CS_PIN), P120_AVERAGE_BUFFER);

      if (nullptr != P120_data) {
        success = P120_data->plugin_webform_load(event); // This shouldn't fail
        delete P120_data;
      }
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P120_AVERAGE_BUFFER = getFormItemInt(F("p120_average_buf"));

      P120_data_struct *P120_data = new (std::nothrow) P120_data_struct(static_cast<int>(P120_CS_PIN), P120_AVERAGE_BUFFER);

      if (nullptr != P120_data) {
        success = P120_data->plugin_webform_save(event); // This shouldn't fail
        delete P120_data;
      }
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P120_data_struct(static_cast<int>(P120_CS_PIN), P120_AVERAGE_BUFFER));
      P120_data_struct *P120_data = static_cast<P120_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = nullptr != P120_data;

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

#endif // ifdef USES_P125
