#include "_Plugin_Helper.h"
#ifdef USES_P069

// #######################################################################################################
// ########################### Plugin 69: LM75A Temperature Sensor (I2C) #################################
// #######################################################################################################
// ###################### Library source code for Arduino by QuentinCG, 2016 #############################
// #######################################################################################################
// ##################### Plugin for ESP Easy by B.E.I.C. ELECTRONICS, 2017 ###############################
// ############################## http://www.beicelectronics.com #########################################
// #######################################################################################################
// ########################## Adapted to ESPEasy 2.0 by Jochen Krapf #####################################
// #######################################################################################################


# define PLUGIN_069
# define PLUGIN_ID_069         69
# define PLUGIN_NAME_069       "Environment - LM75A"
# define PLUGIN_VALUENAME1_069 "Temperature"


# include "src/PluginStructs/P069_data_struct.h"


boolean Plugin_069(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_069;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_SINGLE;
      dev.FormulaOption  = true;
      dev.ValueCount     = 1;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_069);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_069));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 8, i2cAddressValues, PCONFIG(0));
      } else {
        success = intArrayContains(8, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = PCONFIG(0);
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("i2c_addr"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      success = initPluginTaskData(event->TaskIndex, new (std::nothrow) P069_data_struct(static_cast<uint8_t>(PCONFIG(0))));
      break;
    }

    case PLUGIN_READ:
    {
      P069_data_struct *P069_data =
        static_cast<P069_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P069_data) {
        return success;
      }

      const float tempC = P069_data->getTemperatureInDegrees();
      UserVar.setFloat(event->TaskIndex, 0, tempC);
      success = !isnan(tempC);

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        if (!success) {
          addLog(LOG_LEVEL_INFO, F("LM75A: No reading!"));
        }
        else
        {
          addLogMove(LOG_LEVEL_INFO, concat(F("LM75A: Temperature: "), formatUserVarNoCheck(event, 0)));
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P069
