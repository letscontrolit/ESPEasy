#include "_Plugin_Helper.h"
#ifdef USES_P032

// #######################################################################################################
// ################ Plugin 032 MS5611 (GY-63) I2C Temp/Barometric Pressure Sensor  #######################
// #######################################################################################################
// This sketch is based on https://github.com/Schm1tz1/arduino-ms5xxx


#include "src/PluginStructs/P032_data_struct.h"

#define PLUGIN_032
#define PLUGIN_ID_032        32
#define PLUGIN_NAME_032       "Environment - MS5611 (GY-63)"
#define PLUGIN_VALUENAME1_032 "Temperature"
#define PLUGIN_VALUENAME2_032 "Pressure"

boolean Plugin_032(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_032;
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
      string = F(PLUGIN_NAME_032);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_032));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_032));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      byte choice = PCONFIG(0);

      /*String options[2] = { F("0x77 - default I2C address"), F("0x76 - alternate I2C address") };*/
      int optionValues[2] = { 0x77, 0x76 };
      addFormSelectorI2C(F("p032_ms5611_i2c"), 2, optionValues, choice);
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Altitude [m]"), F("p032_ms5611_elev"), PCONFIG(1));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("p032_ms5611_i2c"));
      PCONFIG(1) = getFormItemInt(F("p032_ms5611_elev"));
      success    = true;
      break;
    }

    case PLUGIN_INIT:
    {
      uint8_t address = PCONFIG(0);

      initPluginTaskData(event->TaskIndex, new (std::nothrow) P032_data_struct(address));
      P032_data_struct *P032_data =
        static_cast<P032_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P032_data) {
        success = true;
      }
      break;
    }

    case PLUGIN_READ:
    {
      P032_data_struct *P032_data =
        static_cast<P032_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P032_data) {
        if (P032_data->begin()) {
          P032_data->read_prom();
          P032_data->readout();

          UserVar[event->BaseVarIndex] = P032_data->ms5611_temperature / 100;
          int elev = PCONFIG(1);

          if (elev)
          {
            UserVar[event->BaseVarIndex + 1] = P032_data->pressureElevation(P032_data->ms5611_pressure, elev);
          } else {
            UserVar[event->BaseVarIndex + 1] = P032_data->ms5611_pressure;
          }

          String log = F("MS5611  : Temperature: ");
          log += UserVar[event->BaseVarIndex];
          addLog(LOG_LEVEL_INFO, log);
          log  = F("MS5611  : Barometric Pressure: ");
          log += UserVar[event->BaseVarIndex + 1];
          addLog(LOG_LEVEL_INFO, log);
          success = true;
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P032
