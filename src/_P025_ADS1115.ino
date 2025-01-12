#include "_Plugin_Helper.h"
#ifdef USES_P025

// #######################################################################################################
// #################################### Plugin 025: ADS1x15 I2C 0x48)  ###############################################
// #######################################################################################################


# include "src/PluginStructs/P025_data_struct.h"

# define PLUGIN_025
# define PLUGIN_ID_025 25
# define PLUGIN_NAME_025 "Analog input - ADS1x15"
# define PLUGIN_VALUENAME1_025 "Analog"


boolean Plugin_025(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  // static uint8_t portValue = 0;
  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_025;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_SINGLE;
      dev.FormulaOption  = true;
      dev.ValueCount     = 1;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.OutputDataType = Output_Data_type_t::Simple;
      dev.PluginStats    = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_025);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      const int valueCount = P025_NR_OUTPUT_VALUES;

      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < valueCount) {
          const uint8_t pconfigIndex = P025_PCONFIG_INDEX(i);
          ExtraTaskSettings.setTaskDeviceValueName(i, Plugin_025_valuename(PCONFIG(pconfigIndex), false));
        } else {
          ExtraTaskSettings.clearTaskDeviceValueName(i);
        }
      }
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P025_NR_OUTPUT_VALUES;
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(P025_SENSOR_TYPE_INDEX));
      event->idx        = P025_SENSOR_TYPE_INDEX;
      success           = true;
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x48, 0x49, 0x4A, 0x4B };
      constexpr int nrAddressOptions   = NR_ELEMENTS(i2cAddressValues);

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), nrAddressOptions, i2cAddressValues, P025_I2C_ADDR);
      } else {
        success = intArrayContains(nrAddressOptions, i2cAddressValues, event->Par1);
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

    case PLUGIN_SET_DEFAULTS:
    {
      PCONFIG(P025_SENSOR_TYPE_INDEX) = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_SINGLE);
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      success = P025_data_struct::webformLoad(event);
      break;
    }

    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
    {
      const __FlashStringHelper *valOptions[] = {
        Plugin_025_valuename(0, true),
        Plugin_025_valuename(1, true),
        Plugin_025_valuename(2, true),
        Plugin_025_valuename(3, true),
        Plugin_025_valuename(4, true),
        Plugin_025_valuename(5, true),
        Plugin_025_valuename(6, true),
        Plugin_025_valuename(7, true)
      };
      constexpr int nrOptions = NR_ELEMENTS(valOptions);

      for (uint8_t i = 0; i < P025_NR_OUTPUT_VALUES; i++) {
        sensorTypeHelper_loadOutputSelector(event,
                                            P025_PCONFIG_INDEX(i),
                                            i,
                                            nrOptions,
                                            valOptions);
      }

      success = true;
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

      success = initPluginTaskData(event->TaskIndex, new (std::nothrow) P025_data_struct(event));
      break;
    }

    case PLUGIN_READ:
    {
      const P025_data_struct *P025_data = static_cast<P025_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P025_data) {
        for (taskVarIndex_t i = 0; i < P025_NR_OUTPUT_VALUES; ++i) {
          float value{};

          if (P025_data->read(value, i)) {
            success = true;

            # ifndef BUILD_NO_DEBUG
            String log;

            if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
              log = strformat(F("ADS1x15 : Analog value: %.2f / Channel: %d"), value, P025_MUX(i));
            }
            # endif // ifndef BUILD_NO_DEBUG

            UserVar.setFloat(event->TaskIndex, i, value);

            const P025_VARIOUS_BITS_t p025_variousBits(P025_VARIOUS_BITS);

            if (p025_variousBits.cal) { // Calibration?
              const int   adc1 = P025_CAL_ADC1;
              const int   adc2 = P025_CAL_ADC2;
              const float out1 = P025_CAL_OUT1;
              const float out2 = P025_CAL_OUT2;

              if (adc1 != adc2)
              {
                const float normalized = static_cast<float>(value - adc1) / static_cast<float>(adc2 - adc1);
                UserVar.setFloat(event->TaskIndex, i, normalized * (out2 - out1) + out1);
                # ifndef BUILD_NO_DEBUG

                if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
                  log += ' ';
                  log += formatUserVarNoCheck(event, i);
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
      }
      break;
    }
  }
  return success;
}

#endif // USES_P025
