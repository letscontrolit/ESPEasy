#include "_Plugin_Helper.h"
#ifdef USES_P051

// #######################################################################################################
// #################### Plugin 051 Temperature and Humidity Sensor AM2320 ##############
// #######################################################################################################
//
// Temperature and Humidity Sensor AM2320
// written by https://github.com/krikk
// based on this library: https://github.com/thakshak/AM2320
// this code is based on git-version https://github.com/thakshak/AM2320/commit/ddaabaf37952d4c74f3ea70af20e5a95cfdfcadb
// of the above library
//


#include <AM2320.h>

#define PLUGIN_051
#define PLUGIN_ID_051        51
#define PLUGIN_NAME_051       "Environment - AM2320"
#define PLUGIN_VALUENAME1_051 "Temperature"
#define PLUGIN_VALUENAME2_051 "Humidity"


boolean Plugin_051(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_051;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TEMP_HUM;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_051);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_051));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_051));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = (event->Par1 == 0x5c);
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = 0x5c;
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
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      AM2320 th;

      switch (th.Read()) {
        case 2:
          addLog(LOG_LEVEL_ERROR, F("AM2320: CRC failed"));
          break;
        case 1:
          addLog(LOG_LEVEL_ERROR, F("AM2320: Sensor offline"));
          break;
        case 0:
        {
          UserVar[event->BaseVarIndex]     = th.t;
          UserVar[event->BaseVarIndex + 1] = th.h;

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("AM2320: Temperature: ");
            log += formatUserVarNoCheck(event->TaskIndex, 0);
            addLogMove(LOG_LEVEL_INFO, log);
            log  = F("AM2320: Humidity: ");
            log += formatUserVarNoCheck(event->TaskIndex, 1);
            addLogMove(LOG_LEVEL_INFO, log);
          }
          success = true;
          break;
        }
      }
    }
  }
  return success;
}

#endif // USES_P051
