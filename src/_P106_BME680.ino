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


# include <Wire.h>
# include <SPI.h>
# include <Adafruit_Sensor.h>
# include <Adafruit_BME680.h>


# define PLUGIN_106
# define PLUGIN_ID_106         106
# define PLUGIN_NAME_106       "Environment - BME680"
# define PLUGIN_VALUENAME1_106 "Temperature"
# define PLUGIN_VALUENAME2_106 "Humidity"
# define PLUGIN_VALUENAME3_106 "Pressure"
# define PLUGIN_VALUENAME4_106 "Gas"

# define BME_SCK 13
# define BME_MISO 12
# define BME_MOSI 11
# define BME_CS 10

# define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme; // I2C
// Adafruit_BME680 bme(BME_CS); // hardware SPI
// Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);


boolean Plugin_106_init = false;

boolean Plugin_106(byte function, struct EventStruct *event, String& string)
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

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {

      byte choice = PCONFIG(0);

      /*
         String options[2];
         options[0] = F("0x76 - default settings (SDO Low)");
         options[1] = F("0x77 - alternate settings (SDO HIGH)");
       */
      int optionValues[2] = { 0x77, 0x76 };
      addFormSelectorI2C(F("plugin_106_BME680_i2c"), 2, optionValues, choice);
      addFormNote(F("SDO Low=0x76, High=0x77"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Altitude"), F("plugin_106_BME680_elev"), PCONFIG(1));
      addUnit(F("m"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("plugin_106_BME680_i2c"));
      PCONFIG(1) = getFormItemInt(F("plugin_106_BME680_elev"));
      success                                              = true;
      break;
    }

    case PLUGIN_READ:
    {
      if (!Plugin_106_init)
      {
        addLog(LOG_LEVEL_INFO, F("BME680  : init"));

        Plugin_106_init = bme.begin();

        // Set up oversampling and filter initialization
        bme.setTemperatureOversampling(BME680_OS_8X);
        bme.setHumidityOversampling(BME680_OS_2X);
        bme.setPressureOversampling(BME680_OS_4X);
        bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
        bme.setGasHeater(320, 150); // 320*C for 150 ms

        success = true;
        break;
      }

      if (Plugin_106_init)
      {
        if (!bme.performReading()) {
          addLog(LOG_LEVEL_ERROR, F("BME680 : Failed to perform reading!"));
          success = false;
          break;
        }

        UserVar[event->BaseVarIndex + 0] = bme.temperature;
        UserVar[event->BaseVarIndex + 1] = bme.humidity;
        UserVar[event->BaseVarIndex + 2] = bme.pressure / 100.0;
        UserVar[event->BaseVarIndex + 3] = bme.gas_resistance / 1000.0;
      }

      success = true;
      break;
    }
  }
  return success;
}

#endif // ifdef USES_P106
