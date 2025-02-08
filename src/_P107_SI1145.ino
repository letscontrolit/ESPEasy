#include "_Plugin_Helper.h"
#ifdef USES_P107

// #######################################################################################################
// #################################### Plugin-107: SI1145 - UV index / IR / visible  ####################
// #######################################################################################################

# include "src/PluginStructs/P107_data_struct.h"

# define PLUGIN_107
# define PLUGIN_ID_107         107
# define PLUGIN_NAME_107       "UV - SI1145"
# define PLUGIN_VALUENAME1_107 "Visible"
# define PLUGIN_VALUENAME2_107 "Infra"
# define PLUGIN_VALUENAME3_107 "UV"

boolean Plugin_107(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_107;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_SINGLE;
      dev.FormulaOption  = true;
      dev.ValueCount     = 3;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_107);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_107));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_107));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_107));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = (event->Par1 == 0x60);
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = 0x60;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P107_data_struct());
      P107_data_struct *P107_data =
        static_cast<P107_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P107_data && P107_data->begin());
      break;
    }

    case PLUGIN_READ:
    {
      P107_data_struct *P107_data =
        static_cast<P107_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P107_data) {
        break;
      }

      if (!P107_data->begin()) {
        break;
      }
      delay(8); // Measurement Rate: 255 * 31.25uS = 8ms

      UserVar.setFloat(event->TaskIndex, 0, P107_data->uv.readVisible());
      UserVar.setFloat(event->TaskIndex, 1, P107_data->uv.readIR());
      UserVar.setFloat(event->TaskIndex, 2, P107_data->uv.readUV() / 100.0f);

      P107_data->uv.reset(); // Stop the sensor reading

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLogMove(LOG_LEVEL_INFO, concat(F("SI1145: Visible: "), formatUserVarNoCheck(event, 0)));
        addLogMove(LOG_LEVEL_INFO, concat(F("SI1145: Infrared: "), formatUserVarNoCheck(event, 1)));
        addLogMove(LOG_LEVEL_INFO, concat(F("SI1145: UV index: "), formatUserVarNoCheck(event, 2)));
      }
      success = true;
      break;
    }
  }
  return success;
}

#endif // ifdef USES_P107
