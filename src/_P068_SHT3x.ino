#include "_Plugin_Helper.h"
#ifdef USES_P068

# include "src/PluginStructs/P068_data_struct.h"

# include "src/Helpers/Convert.h"
# include "src/Helpers/ESPEasy_math.h"

// #######################################################################################################
// ################ Plugin 68: SHT30/SHT31/SHT35 Temperature and Humidity Sensor (I2C) ###################
// #######################################################################################################
// ######################## Library source code for Arduino by WeMos, 2016 ###############################
// #######################################################################################################
// ###################### Plugin for ESP Easy by B.E.I.C. ELECTRONICS, 2017 ##############################
// ############################### http://www.beicelectronics.com ########################################
// #######################################################################################################
// ########################## Adapted to ESPEasy 2.0 by Jochen Krapf #####################################
// #######################################################################################################

// Changelog:
// 2021-06-12 @tonhuisman: Add temperature offset setting, with humidity compensation method 'borrowed' from BME280 sensor
// 2020-??    @TD-er: Maitenance updates
// 2017-07-18 @JK-de: Plugin adaption for ESPEasy 2.0

# define PLUGIN_068
# define PLUGIN_ID_068         68
# define PLUGIN_NAME_068       "Environment - SHT30/31/35"
# define PLUGIN_VALUENAME1_068 "Temperature"
# define PLUGIN_VALUENAME2_068 "Humidity"


// ==============================================
// PLUGIN
// =============================================

boolean Plugin_068(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_068;
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
      string = F(PLUGIN_NAME_068);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_068));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_068));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x44, 0x45 };

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
      addFormNumericBox(F("Temperature offset"), F("tempoffset"), PCONFIG(1));
      addUnit(F("x 0.1C"));
      addFormNote(F("Offset in units of 0.1 degree Celsius"));
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("i2c_addr"));
      PCONFIG(1) = getFormItemInt(F("tempoffset"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P068_SHT3X(PCONFIG(0)));
      P068_SHT3X *P068_data = static_cast<P068_SHT3X *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P068_data);
      break;
    }

    case PLUGIN_READ:
    {
      P068_SHT3X *sht3x = static_cast<P068_SHT3X *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == sht3x) {
        addLog(LOG_LEVEL_ERROR, F("SHT3x: not initialised!"));
        return success;
      }

      sht3x->tmpOff = PCONFIG(1) / 10.0f;
      sht3x->readFromSensor();
      UserVar[event->BaseVarIndex + 0] = sht3x->tmp;
      UserVar[event->BaseVarIndex + 1] = sht3x->hum;

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log;

        if (log.reserve(25)) {
          log  = F("SHT3x: Temperature: ");
          log += formatUserVarNoCheck(event->TaskIndex, 0);
          addLogMove(LOG_LEVEL_INFO, log);
          log  = F("SHT3x: Humidity: ");
          log += formatUserVarNoCheck(event->TaskIndex, 1);
          addLogMove(LOG_LEVEL_INFO, log);
        }
      }
      success = true;
      break;
    }
  }
  return success;
}

#endif // USES_P068
