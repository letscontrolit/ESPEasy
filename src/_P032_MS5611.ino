#include "_Plugin_Helper.h"
#ifdef USES_P032

// #######################################################################################################
// ################ Plugin 032 MS5611 (GY-63) I2C Temp/Barometric Pressure Sensor  #######################
// #######################################################################################################
// This sketch is based on https://github.com/Schm1tz1/arduino-ms5xxx


# include "src/PluginStructs/P032_data_struct.h"

# define PLUGIN_032
# define PLUGIN_ID_032        32
# define PLUGIN_NAME_032       "Environment - MS5611 (GY-63)"
# define PLUGIN_VALUENAME1_032 "Temperature"
# define PLUGIN_VALUENAME2_032 "Pressure"

boolean Plugin_032(uint8_t function, struct EventStruct *event, String& string)
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
      Device[deviceCount].PluginStats        = true;
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

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x77, 0x76 };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 2, i2cAddressValues, PCONFIG(0));
      } else {
        success = intArrayContains(2, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = PCONFIG(0);
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Altitude [m]"), F("elev"), PCONFIG(1));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("i2c_addr"));
      PCONFIG(1) = getFormItemInt(F("elev"));
      success    = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P032_data_struct(PCONFIG(0)));
      P032_data_struct *P032_data =
        static_cast<P032_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P032_data);
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

          const int elev = PCONFIG(1);

          if (elev != 0)
          {
            UserVar[event->BaseVarIndex + 1] = pressureElevation(P032_data->ms5611_pressure, elev);
          } else {
            UserVar[event->BaseVarIndex + 1] = P032_data->ms5611_pressure;
          }

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("MS5611  : Temperature: ");
            log += formatUserVarNoCheck(event->TaskIndex, 0);
            addLogMove(LOG_LEVEL_INFO, log);
            log  = F("MS5611  : Barometric Pressure: ");
            log += formatUserVarNoCheck(event->TaskIndex, 1);
            addLogMove(LOG_LEVEL_INFO, log);
          }
          success = true;
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P032
