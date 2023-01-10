#include "_Plugin_Helper.h"
#ifdef USES_P134

// #######################################################################################################
// #################################### Plugin 134: A02YYUW Distance #####################################
// #######################################################################################################

/**
 * 2022-09-10 tonhuisman: Remove TESTING tag, code improvements
 * 2022-08-01 tonhuisman: Implement P134_data_struct to enable multi-instance use, set TESTING tag
 *                        Transform error numbers to meaningful text
 * 2022-07-18 tonhuisman: Migrate plugin to ESPEasy mega branch as P134
 * 2021-08-18 seb82     : Published initial plugin (P251) in the ESPEasy forum:
 *                        https://www.letscontrolit.com/forum/viewtopic.php?p=54458#p54458
 **/
# define PLUGIN_134
# define PLUGIN_ID_134          134
# define PLUGIN_NAME_134        "Distance - A02YYUW"
# define PLUGIN_VALUENAME1_134  "Distance"

# include "./src/PluginStructs/P134_data_struct.h"

boolean Plugin_134(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_134;
      Device[deviceCount].Type               = DEVICE_TYPE_SERIAL;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_134);

      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_134));

      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      serialHelper_getGpioNames(event, false, true); // TX optional

      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += serialHelper_getSerialTypeLabel(event);

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P134_data_struct(CONFIG_PORT, CONFIG_PIN1, CONFIG_PIN2));
      P134_data_struct *P134_data = static_cast<P134_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P134_data) && P134_data->isInitialized();

      break;
    }

    case PLUGIN_READ:
    {
      P134_data_struct *P134_data = static_cast<P134_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P134_data) {
        success = P134_data->plugin_read(event);
      }

      break;
    }
  }
  return success;
}

#endif // USES_P134
