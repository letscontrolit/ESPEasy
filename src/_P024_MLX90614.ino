#include "_Plugin_Helper.h"
#ifdef USES_P024

// #######################################################################################################
// #################################### Plugin 024: MLX90614 IR temperature I2C 0x5A)  ###############################################
// #######################################################################################################

/** Changelog:
 * 2024-08-17 tonhuisman: Show correct I2C address when non-default address is used (by setting a Port nr. 0..15)
 * 2023-11-23 tonhuisman: Add Device flag for I2CMax100kHz as this sensor won't work at 400 kHz
 * 2023-11-23 tonhuisman: Add Changelog
 */

# include "src/PluginStructs/P024_data_struct.h"

// MyMessage *msgTemp024; // Mysensors

# define PLUGIN_024
# define PLUGIN_ID_024 24
# define PLUGIN_NAME_024 "Environment - MLX90614"
# define PLUGIN_VALUENAME1_024 "Temperature"

boolean Plugin_024(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  // static uint8_t portValue = 0;
  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_024;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_SINGLE;
      dev.Ports          = 16;
      dev.FormulaOption  = true;
      dev.SendDataOption = true;
      dev.ValueCount     = 1;
      dev.TimerOption    = true;
      dev.PluginStats    = true;
      dev.I2CMax100kHz   = true; // Max 100 kHz allowed/supported
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_024);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_024));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = event->Par1 == (0x5a + CONFIG_PORT);
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = 0x5a + CONFIG_PORT;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      const __FlashStringHelper *options[] = {
        F("IR object temperature"),
        F("Ambient temperature")
      };
      const int optionValues[] = {
        (0x07),
        (0x06)
      };
      constexpr size_t optionCount = NR_ELEMENTS(optionValues);
      addFormSelector(F("Option"), F("option"), optionCount, options, optionValues, PCONFIG(0));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("option"));
      success    = true;
      break;
    }

    case PLUGIN_INIT:
    {
      const uint8_t address = 0x5A + CONFIG_PORT;

      success = initPluginTaskData(event->TaskIndex, new (std::nothrow) P024_data_struct(address));
      break;
    }

    case PLUGIN_READ:
    {
      P024_data_struct *P024_data =
        static_cast<P024_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P024_data) {
        UserVar.setFloat(event->TaskIndex, 0, P024_data->readTemperature(PCONFIG(0)));

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, concat(F("MLX90614 : Temperature: "), formatUserVarNoCheck(event, 0)));
        }

        //        send(msgObjTemp024->set(UserVar[event->BaseVarIndex], 1)); // Mysensors
        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P024
