#include "_Plugin_Helper.h"
#ifdef USES_P172

// #######################################################################################################
// ################################## Plugin-172: Environment - BMP3xx SPI   #############################
// #######################################################################################################

/**
 * 2024-06-30 tonhuisman: Start SPI plugin, based on P154, re-using most code (dependency checked in define_plugin_sets.h)
 */

# include "src/PluginStructs/P154_data_struct.h"

# define PLUGIN_172
# define PLUGIN_ID_172         172
# define PLUGIN_NAME_172       "Environment - BMP3xx (SPI)"
# define PLUGIN_VALUENAME1_172 "Temperature"
# define PLUGIN_VALUENAME2_172 "Pressure"

boolean Plugin_172(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_172;
      dev.Type           = DEVICE_TYPE_SPI;
      dev.VType          = Sensor_VType::SENSOR_TYPE_TEMP_BARO;
      dev.FormulaOption  = true;
      dev.ValueCount     = 2;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_172);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_172));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_172));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("CS"));
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P154_data_struct(event));
      P154_data_struct *P154_P172_data =
        static_cast<P154_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P154_P172_data && P154_P172_data->begin(false));
      break;
    }

    case PLUGIN_READ:
    {
      P154_data_struct *P154_P172_data =
        static_cast<P154_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P154_P172_data) {
        break;
      }

      float temp, pressure{};

      success = P154_P172_data->read(temp, pressure);
      UserVar.setFloat(event->TaskIndex, 0, temp);
      UserVar.setFloat(event->TaskIndex, 1, pressure);
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      success = P154_data_struct::webformLoad(event, false);
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      success = P154_data_struct::webformSave(event);
      break;
    }
  }
  return success;
}

#endif // ifdef USES_P172
