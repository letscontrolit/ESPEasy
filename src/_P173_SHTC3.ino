#include "_Plugin_Helper.h"
#ifdef USES_P173

// #######################################################################################################
// ######################### Plugin 173: Environment - SHTC3 Temperature, Humidity #######################
// #######################################################################################################

/**
 * 2024-08-27 tonhuisman: Start plugin for SHTC3 I2C Temperature and Humidity sensor
 *                        Using direct I2C communication, based of this library: https://github.com/cdjq/DFRobot_SHTC3
 **/

# define PLUGIN_173
# define PLUGIN_ID_173          173
# define PLUGIN_NAME_173        "Environment - SHTC3"
# define PLUGIN_VALUENAME1_173  "Temperature"
# define PLUGIN_VALUENAME2_173  "Humidity"

# include "./src/PluginStructs/P173_data_struct.h"

boolean Plugin_173(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_173;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_TEMP_HUM;
      dev.FormulaOption  = true;
      dev.ValueCount     = 2;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_173);

      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_173));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_173));

      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = P173_I2C_ADDRESS == event->Par1;

      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P173_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormTextBox(F("Temperature offset"), F("toffs"), toString(P173_TEMPERATURE_OFFSET, 2), 5);
      addUnit(F("&deg;C"));

      addFormCheckBox(F("Read in Low-Power mode"), F("lpmode"), P173_CONFIG_LOW_POWER == 1);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P173_TEMPERATURE_OFFSET = getFormItemFloat(F("toffs"));
      P173_CONFIG_LOW_POWER   = isFormItemChecked(F("lpmode")) ? 1 : 0;

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P173_data_struct(P173_TEMPERATURE_OFFSET, P173_CONFIG_LOW_POWER));
      P173_data_struct *P173_data = static_cast<P173_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P173_data) && P173_data->init();

      break;
    }

    case PLUGIN_READ:
    {
      P173_data_struct *P173_data = static_cast<P173_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P173_data) && P173_data->plugin_read(event);

      break;
    }
  }
  return success;
}

#endif // USES_P173
