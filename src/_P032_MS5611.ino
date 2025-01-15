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
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_032;
      dev.Type           = DEVICE_TYPE_I2C;
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
      const uint8_t i2cAddressValues[] = { 0x76, 0x77 };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 2, i2cAddressValues, PCONFIG(0), 0x77);
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

    case PLUGIN_SET_DEFAULTS:
    {
      PCONFIG(0) = 0x77; // Default address

      success = true;
      break;
    }

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
      success = initPluginTaskData(
        event->TaskIndex,
        new (std::nothrow) P032_data_struct(PCONFIG(0)));
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

          UserVar.setFloat(event->TaskIndex, 0, P032_data->ms5611_temperature / 100);

          const int elev = PCONFIG(1);

          if (elev != 0)
          {
            UserVar.setFloat(event->TaskIndex, 1, pressureElevation(P032_data->ms5611_pressure, elev));
          } else {
            UserVar.setFloat(event->TaskIndex, 1, P032_data->ms5611_pressure);
          }

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLog(LOG_LEVEL_INFO,
                   concat(F("MS5611  : Temperature: "), formatUserVarNoCheck(event, 0)));
            addLog(LOG_LEVEL_INFO,
                   concat(F("MS5611  : Barometric Pressure: "), formatUserVarNoCheck(event, 1)));
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
