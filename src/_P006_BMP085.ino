#ifdef USES_P006

// #######################################################################################################
// ######################## Plugin 006 BMP085/180 I2C Barometric Pressure Sensor  ########################
// #######################################################################################################

#include "_Plugin_Helper.h"

#include "src/PluginStructs/P006_data_struct.h"

#define PLUGIN_006
#define PLUGIN_ID_006        6
#define PLUGIN_NAME_006       "Environment - BMP085/180"
#define PLUGIN_VALUENAME1_006 "Temperature"
#define PLUGIN_VALUENAME2_006 "Pressure"


boolean Plugin_006(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_006;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TEMP_BARO;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_006);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_006));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_006));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Altitude [m]"), F("_p006_bmp085_elev"), PCONFIG(1));
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(1) = getFormItemInt(F("_p006_bmp085_elev"));
      success    = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P006_data_struct());
      P006_data_struct *P006_data =
        static_cast<P006_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P006_data) {
        success = true;
      }
      break;
    }

    case PLUGIN_READ:
    {
      P006_data_struct *P006_data =
        static_cast<P006_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P006_data) {
        if (P006_data->begin())
        {
          UserVar[event->BaseVarIndex] = P006_data->readTemperature();
          int   elev     = PCONFIG(1);
          float pressure = (float)P006_data->readPressure() / 100.0f;

          if (elev != 0)
          {
            pressure = P006_data->pressureElevation(
              pressure,
              elev);
          }
          UserVar[event->BaseVarIndex + 1] = pressure;

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("BMP  : Temperature: ");
            log += UserVar[event->BaseVarIndex];
            addLog(LOG_LEVEL_INFO, log);
            log  = F("BMP  : Barometric Pressure: ");
            log += UserVar[event->BaseVarIndex + 1];
            addLog(LOG_LEVEL_INFO, log);
          }
          success = true;
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P006
