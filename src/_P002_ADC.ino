#include "_Plugin_Helper.h"
#ifdef USES_P002


# include "src/Helpers/Hardware.h"
# include "src/PluginStructs/P002_data_struct.h"

// #######################################################################################################
// #################################### Plugin 002: Analog ###############################################
// #######################################################################################################

# define PLUGIN_002
# define PLUGIN_ID_002         2
# define PLUGIN_NAME_002       "Analog input - internal"
# define PLUGIN_VALUENAME1_002 "Analog"


boolean Plugin_002(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_002;
      Device[deviceCount].Type               = DEVICE_TYPE_ANALOG;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;
      Device[deviceCount].TaskLogsOwnPeaks   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_002);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_002));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      P002_data_struct *P002_data =
        static_cast<P002_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P002_data) {
        P002_data->webformLoad(event);
        success = true;
      } else {
        P002_data = new (std::nothrow) P002_data_struct();

        if (nullptr != P002_data) {
          P002_data->init(event);
          P002_data->webformLoad(event);
          success = true;
          delete P002_data;
        }
      }
      break;
    }

# if FEATURE_PLUGIN_STATS
    case PLUGIN_WEBFORM_LOAD_SHOW_STATS:
    {
      P002_data_struct *P002_data =
        static_cast<P002_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P002_data) {
        success = P002_data->webformLoad_show_stats(event);
      }
      break;
    }
# endif // if FEATURE_PLUGIN_STATS

    case PLUGIN_WEBFORM_SAVE:
    {
      addHtmlError(P002_data_struct::webformSave(event));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P002_data_struct());
      P002_data_struct *P002_data =
        static_cast<P002_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P002_data) {
        success = true;
        P002_data->init(event);
      }
      break;
    }
    case PLUGIN_TEN_PER_SECOND:
    {
      if (P002_OVERSAMPLING != P002_USE_CURENT_SAMPLE) // Use multiple samples
      {
        P002_data_struct *P002_data =
          static_cast<P002_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P002_data) {
          P002_data->takeSample();
        }
      }
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      int   raw_value = 0;
      float res_value = 0.0f;

      P002_data_struct *P002_data =
        static_cast<P002_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((P002_data != nullptr) && P002_data->getValue(res_value, raw_value)) {
        UserVar[event->BaseVarIndex] = res_value;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("ADC  : Analog value: ");
          log += raw_value;
          log += F(" = ");
          log += formatUserVarNoCheck(event->TaskIndex, 0);

          if (P002_OVERSAMPLING == P002_USE_OVERSAMPLING) {
            log += F(" (");
            log += P002_data->OversamplingCount;
            log += F(" samples)");
          }
          addLogMove(LOG_LEVEL_INFO, log);
        }
        P002_data->reset();
        success = true;
      } else {
        addLog(LOG_LEVEL_ERROR, F("ADC  : No value received "));
        success = false;
      }

      break;
    }

    case PLUGIN_SET_CONFIG:
    {
      P002_data_struct *P002_data =
        static_cast<P002_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (P002_data != nullptr) {
        success = P002_data->plugin_set_config(event, string);
        if (success) {
          P002_data->init(event);
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P002
