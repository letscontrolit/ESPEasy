#include "_Plugin_Helper.h"

#ifdef USES_P125

// #######################################################################################################
// ############################## Plugin 125: Accelerometer - ADXL345 SPI ################################
// #######################################################################################################

/**
 * Plugin to support the ADXL345 Accelerometer, using the Sparkfun ADXL345 Arduino library using SPI interface
 */

/** Changelog:
 * 2025-01-12 tonhuisman: Add support for MQTT AutoDiscovery (not supported yet for Accelerometer)
 *                        Use all relevant code from P120 (I2C variant of the same sensor)
 * 2021-12-10 tonhuisman, Start SPI interface version of ADXL345 plugin, based on P120 ADXL345 I2C plugin
 *                        Using Sparkfun ADXL345 library
 *                        https://github.com/sparkfun/SparkFun_ADXL345_Arduino_Library
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

boolean Plugin_125(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_125;
      dev.Type           = DEVICE_TYPE_SPI;
      dev.VType          = Sensor_VType::SENSOR_TYPE_TRIPLE;
      dev.ValueCount     = 3;
      dev.FormulaOption  = true;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.TimerOptional  = true;
      dev.PluginStats    = true;
      dev.OutputDataType = Output_Data_type_t::Simple;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_125);
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

      P120_data_struct *P120_data = new (std::nothrow) P120_data_struct(P120_AVERAGE_BUFFER);

      if (nullptr != P120_data) {
        P120_data->setSPI_CSpin(static_cast<int>(P120_CS_PIN));
        success = P120_data->plugin_set_defaults(event); // This shouldn't fail
        delete P120_data;
      }

      // No decimals plausible, as the outputs from the sensor are of type int
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0;
      }

      break;
    }


    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P120_data_struct(P120_AVERAGE_BUFFER));
      P120_data_struct *P120_data = static_cast<P120_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P120_data) {
        P120_data->setSPI_CSpin(static_cast<int>(P120_CS_PIN));
        success = true;
      }

      break;
    }


    default: // Hand over further handling to P120
      success = Plugin_120(function, event, string);
  } // switch
  return success;
} // function

#endif // ifdef USES_P125
