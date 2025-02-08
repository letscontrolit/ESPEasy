#include "_Plugin_Helper.h"
#ifdef USES_P175

// #######################################################################################################
// ################################# Plugin 175: Plantower PMSx003i (I2C) ################################
// #######################################################################################################

/** Changelog:
 * 2024-10-13 tonhuisman: Better employ existing code for handling incoming data.
 * 2024-10-12 tonhuisman: Start plugin for Plantower PMSx003i I2C dust/particle sensor, similar to PMSx003_S & _ST serial sensors.
 *                        Shares most code with P053, so that's required to be enabled if this plugin is enabled.
 *                        P053 has been adjusted to handle the I2C communication with the sensor, instead of the serial comms
 */

# include "src/PluginStructs/P053_data_struct.h"

# define PLUGIN_175
# define PLUGIN_ID_175    175
# define PLUGIN_NAME_175  "Dust - PMSx003i (I2C)"

boolean Plugin_175(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  P053_for_P175 = true; // Ignore this error, compiler can see it as all .ino files are smart-copied into ESPEasy.ino

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_175;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_QUAD;
      dev.FormulaOption  = true;
      dev.ValueCount     = 4;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;
      dev.I2CMax100kHz   = true; // Max I2C Clock speed 100 kHz

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string  = F(PLUGIN_NAME_175);
      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      PLUGIN_053_RST_PIN = -1;
      PLUGIN_053_PWR_PIN = -1;

      PLUGIN_053_SENSOR_MODEL_SELECTOR = static_cast<int>(PMSx003_type::PMSA003i);
      PLUGIN_053_SEC_IGNORE_AFTER_WAKE = 30; // Startup time of the sensor is ~30 seconds

      bitSet(PLUGIN_053_DATA_PROCESSING_FLAGS, PLUGIN_053_OVERSAMPLING_BIT);

      success = true;
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = (event->Par1 == P175_I2C_ADDR);
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P175_I2C_ADDR;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_INIT:
    {
      const  ESPEasySerialPort port = static_cast<ESPEasySerialPort>(0);

      const PMSx003_type Plugin_053_sensortype = GET_PLUGIN_053_SENSOR_MODEL_SELECTOR;

      initPluginTaskData(
        event->TaskIndex,
        new (std::nothrow) P053_data_struct(
          event->TaskIndex,
          -1, -1, port,
          PLUGIN_053_RST_PIN,
          PLUGIN_053_PWR_PIN,
          Plugin_053_sensortype,
          PLUGIN_053_SEC_IGNORE_AFTER_WAKE * 1000u,
          bitRead(PLUGIN_053_DATA_PROCESSING_FLAGS, PLUGIN_053_OVERSAMPLING_BIT),
          bitRead(PLUGIN_053_DATA_PROCESSING_FLAGS, PLUGIN_053_SPLIT_CNT_BINS_BIT),
          P053_for_P175
          ));
      P053_data_struct *P053_data =
        static_cast<P053_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P053_data) {
        success = P053_data->init();
      }

      break;
    }

    default: // Hand over further handling to P053
      success = Plugin_053(function, event, string);
  }
  P053_for_P175 = false;
  return success;
}

#endif // USES_P175
