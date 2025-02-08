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

/** Changelog:
 * 2023-04-16 tonhuisman: Add option to present Gas(resistance) as Ohm instead of kOhm
 *                        Rename sensor to BME68x from BME680, as BME688 is backward compatible.
 *                        NB: AI-features of BME688 are not supported!
 *                        Use updated #defines since BME680 library update
 * 2023-04-15 tonhuisman: Fix copy/paste error for FEATURE_I2C_GET_ADDRESS
 *                        Update Adafruit_BME680 library to v2.0.2
 * 2023-04-15 tonhuisman: Started Changelog
 */

# include "src/PluginStructs/P106_data_struct.h"


# define PLUGIN_106
# define PLUGIN_ID_106         106
# define PLUGIN_NAME_106       "Environment - BME68x"
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
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_106;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_QUAD;
      dev.FormulaOption  = true;
      dev.ValueCount     = 4;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;
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
      const uint8_t i2cAddressValues[] = { 0x76, 0x77 };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 2, i2cAddressValues, P106_I2C_ADDRESS, 0x77);
        addFormNote(F("SDO Low=0x76, High=0x77"));
      } else {
        success = intArrayContains(2, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P106_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P106_I2C_ADDRESS = 0x77; // Default address

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Altitude"), F("elev"), P106_ALTITUDE);
      addUnit('m');

      addFormCheckBox(F("Present `Gas` in Ohm (not kOhm)"), F("gas"), P106_GET_OPT_GAS_OHM);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P106_I2C_ADDRESS = getFormItemInt(F("i2c_addr"));
      P106_ALTITUDE    = getFormItemInt(F("elev"));
      P106_SET_OPT_GAS_OHM(isFormItemChecked(F("gas")));
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P106_data_struct());
      P106_data_struct *P106_data =
        static_cast<P106_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P106_data) {
        P106_data->initialized = false; // Force re-init just in case the address changed.
        success                = P106_data->begin(P106_I2C_ADDRESS);
      }
      break;
    }

    case PLUGIN_READ:
    {
      P106_data_struct *P106_data =
        static_cast<P106_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P106_data)
      {
        P106_data->begin(P106_I2C_ADDRESS);

        if (!P106_data->initialized) {
          break;
        }

        if (!P106_data->performReading()) {
          P106_data->initialized = false;
          addLog(LOG_LEVEL_ERROR, F("BME68x : Failed to perform reading!"));
          break;
        }

        UserVar.setFloat(event->TaskIndex, 0, P106_data->getTemperature());
        UserVar.setFloat(event->TaskIndex, 1, P106_data->getHumidity());
        UserVar.setFloat(event->TaskIndex, 3, P106_GET_OPT_GAS_OHM ? P106_data->getGasResistance() : P106_data->getGasResistance() / 1000.0f);

        const int elev = P106_ALTITUDE;

        if (elev != 0)
        {
          UserVar.setFloat(event->TaskIndex, 2, pressureElevation(P106_data->getPressure(), elev));
        } else {
          UserVar.setFloat(event->TaskIndex, 2, P106_data->getPressure());
        }
      }

      success = true;
      break;
    }
  }
  return success;
}

#endif // ifdef USES_P106
