#include "_Plugin_Helper.h"
#ifdef USES_P025

// #######################################################################################################
// #################################### Plugin 025: ADS1115 I2C 0x48)  ###############################################
// #######################################################################################################


# include "src/PluginStructs/P025_data_struct.h"

# define PLUGIN_025
# define PLUGIN_ID_025 25
# define PLUGIN_NAME_025 "Analog input - ADS1115"
# define PLUGIN_VALUENAME1_025 "Analog"



boolean Plugin_025(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  // static uint8_t portValue = 0;
  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_025;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      Device[deviceCount].PluginStats        = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_025);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_025));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      # define ADS1115_I2C_OPTION 4
      const uint8_t i2cAddressValues[] = { 0x48, 0x49, 0x4A, 0x4B };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), ADS1115_I2C_OPTION, i2cAddressValues, P025_I2C_ADDR);
      } else {
        success = intArrayContains(ADS1115_I2C_OPTION, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P025_I2C_ADDR;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {

      success = P025_data_struct::webformLoad(event);
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      success = P025_data_struct::webformSave(event);
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      success = P025_data_struct::webform_showConfig(event);
      break;
    }

    case PLUGIN_INIT:
    {
      // int value = 0;
      // uint8_t unit = (CONFIG_PORT - 1) / 4;
      // uint8_t port = CONFIG_PORT - (unit * 4);
      // uint8_t address = 0x48 + unit;

      initPluginTaskData(event->TaskIndex, new (std::nothrow) P025_data_struct(event));
      P025_data_struct *P025_data = static_cast<P025_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P025_data);
      break;
    }

    case PLUGIN_READ:
    {
      const P025_data_struct *P025_data = static_cast<P025_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P025_data) {
        float value{};

        if (P025_data->read(value)) {
          success = true;

        # ifndef BUILD_NO_DEBUG
          String log;

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            log  = F("ADS1115 : Analog value: ");
            log += value;
            log += F(" / Channel: ");
            log += P025_MUX;
          }
        # endif // ifndef BUILD_NO_DEBUG

          if (!P025_CAL_GET) { // Calibration?
            UserVar[event->BaseVarIndex] = value;
          }
          else {
            const int   adc1 = P025_CAL_ADC1;
            const int   adc2 = P025_CAL_ADC2;
            const float out1 = P025_CAL_OUT1;
            const float out2 = P025_CAL_OUT2;

            if (adc1 != adc2)
            {
              const float normalized = static_cast<float>(value - adc1) / static_cast<float>(adc2 - adc1);
              UserVar[event->BaseVarIndex] = normalized * (out2 - out1) + out1;
            # ifndef BUILD_NO_DEBUG

              if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
                log += ' ';
                log += formatUserVarNoCheck(event->TaskIndex, 0);
              }
            # endif // ifndef BUILD_NO_DEBUG
            }
          }

        # ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            addLogMove(LOG_LEVEL_DEBUG, log);
          }
        # endif // ifndef BUILD_NO_DEBUG
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P025
