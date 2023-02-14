#include "_Plugin_Helper.h"
#ifdef USES_P106

// #######################################################################################################
// #################### Plugin 106 BME680 I2C Temp/Hum/Barometric/Pressure/Gas Resistence Sensor  ########
// #######################################################################################################

/*******************************************************************************
* Copyright 2017
* Written by Rossen Tchobanski (rosko@rosko.net)
* BSD license, all text above must be included in any redistribution
*
* Release notes:
   Adafruit_BME680 Library v1.0.5 required (https://github.com/adafruit/Adafruit_BME680/tree/1.0.5)
   /******************************************************************************/


# include "src/PluginStructs/P106_data_struct.h"


# define PLUGIN_106
# define PLUGIN_ID_106         106
# define PLUGIN_NAME_106       "Environment - BME680"
# define PLUGIN_VALUENAME1_106 "Temperature"
# define PLUGIN_VALUENAME2_106 "Humidity"
# define PLUGIN_VALUENAME3_106 "Pressure"
# define PLUGIN_VALUENAME4_106 "Gas"


boolean Plugin_106(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_106;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 4;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_106);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_106));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_106));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_106));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_106));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x77, 0x76 };
      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 2, i2cAddressValues, PCONFIG(0));
        addFormNote(F("SDO Low=0x76, High=0x77"));
      } else {
        success = intArrayContains(2, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2FEATURE_I2C_GET_ADDRESSC_DEVICE_CHECK
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = PCONFIG(0);
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Altitude"), F("elev"), PCONFIG(1));
      addUnit('m');

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
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P106_data_struct());
      P106_data_struct *P106_data =
        static_cast<P106_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P106_data) {
        P106_data->initialized = false; // Force re-init just in case the address changed.
        success = P106_data->begin(PCONFIG(0));
      }
      break;
    }

    case PLUGIN_READ:
    {
      P106_data_struct *P106_data =
        static_cast<P106_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P106_data)
      {
        P106_data->begin(PCONFIG(0));

        if (!P106_data->initialized) {
          break;
        }

        if (!P106_data->bme.performReading()) {
          P106_data->initialized = false;
          addLog(LOG_LEVEL_ERROR, F("BME680 : Failed to perform reading!"));
          break;
        }

        UserVar[event->BaseVarIndex + 0] = P106_data->bme.temperature;
        UserVar[event->BaseVarIndex + 1] = P106_data->bme.humidity;
        UserVar[event->BaseVarIndex + 3] = P106_data->bme.gas_resistance / 1000.0f;

        const int elev = PCONFIG(1);
        if (elev != 0)
        {
          UserVar[event->BaseVarIndex + 2] = pressureElevation(P106_data->bme.pressure / 100.0f, elev);
        } else {
          UserVar[event->BaseVarIndex + 2] = P106_data->bme.pressure / 100.0f;
        }
      }

      success = true;
      break;
    }
  }
  return success;
}

#endif // ifdef USES_P106
