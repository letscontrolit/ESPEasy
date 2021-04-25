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

boolean Plugin_107(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_107;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
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

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P107_data_struct());
      P107_data_struct *P107_data =
        static_cast<P107_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P107_data && P107_data->begin()) {
        success = true;
      }
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

      UserVar[event->BaseVarIndex]     = P107_data->uv.readVisible();
      UserVar[event->BaseVarIndex + 1] = P107_data->uv.readIR();
      UserVar[event->BaseVarIndex + 2] = P107_data->uv.readUV() / 100.0f;

      P107_data->uv.reset(); // Stop the sensor reading

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("SI1145: Visible: ");
        log += formatUserVarNoCheck(event->TaskIndex, 0);
        addLog(LOG_LEVEL_INFO, log);
        log  = F("SI1145: Infrared: ");
        log += formatUserVarNoCheck(event->TaskIndex, 1);
        addLog(LOG_LEVEL_INFO, log);
        log  = F("SI1145: UV index: ");
        log += formatUserVarNoCheck(event->TaskIndex, 2);
        addLog(LOG_LEVEL_INFO, log);
      }
      success = true;
      break;
    }
  }
  return success;
}

#endif // ifdef USES_P107
