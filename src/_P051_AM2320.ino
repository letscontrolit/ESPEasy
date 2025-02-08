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

/** Changelog:
 * 2023-09-05 tonhuisman: Disable I2C device-check during read, as the sensor seems a bit 'itchy' about that
 * 2023-09-05 tonhuisman: Add changelog
 */


# include <AM2320.h>

# define PLUGIN_051
# define PLUGIN_ID_051        51
# define PLUGIN_NAME_051       "Environment - AM2320"
# define PLUGIN_VALUENAME1_051 "Temperature"
# define PLUGIN_VALUENAME2_051 "Humidity"


boolean Plugin_051(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number           = PLUGIN_ID_051;
      dev.Type             = DEVICE_TYPE_I2C;
      dev.VType            = Sensor_VType::SENSOR_TYPE_TEMP_HUM;
      dev.FormulaOption    = true;
      dev.ValueCount       = 2;
      dev.SendDataOption   = true;
      dev.TimerOption      = true;
      dev.PluginStats      = true;
      dev.I2CNoDeviceCheck = true; // Avoid device check
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
    case PLUGIN_WEBFORM_SAVE:
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
          UserVar.setFloat(event->TaskIndex, 0, th.t);
          UserVar.setFloat(event->TaskIndex, 1, th.h);

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLogMove(LOG_LEVEL_INFO, concat(F("AM2320: Temperature: "), formatUserVarNoCheck(event, 0)));
            addLogMove(LOG_LEVEL_INFO, concat(F("AM2320: Humidity: "), formatUserVarNoCheck(event, 1)));
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
