#include "_Plugin_Helper.h"
#ifdef USES_P168

// #######################################################################################################
// ####################### Plugin 168: Light/Lux - VEML6030/VEML7700 I2C Light sensor ####################
// #######################################################################################################

/**
 * 2024-06-21 tonhuisman: Fix support for VEML6030, using by default the alternate I2C address, by modifying the VEML7700 library
 * 2024-05-18 tonhuisman: Implement AutoLux feature, and Get Config Value for automatically determined gain and integration
 * 2024-05-16 tonhuisman: Start plugin for VEML6030/VEML7700 I2C Light sensor, using a slightly adjusted Adafruit library:
 *                        https://github.com/adafruit/Adafruit_VEML7700
 **/

# define PLUGIN_168
# define PLUGIN_ID_168          168
# define PLUGIN_NAME_168        "Light/Lux - VEML6030/VEML7700"
# define PLUGIN_VALUENAME1_168  "Lux"
# define PLUGIN_VALUENAME2_168  "White"
# define PLUGIN_VALUENAME3_168  "Raw"

# include "./src/PluginStructs/P168_data_struct.h"

boolean Plugin_168(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_168;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_SINGLE;
      dev.FormulaOption  = true;
      dev.ValueCount     = 3;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.PluginStats    = true;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_168);

      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_168));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_168));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_168));

      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x10, 0x48 };

      if (PLUGIN_WEBFORM_SHOW_I2C_PARAMS == function) {
        addFormSelectorI2C(F("i2c_addr"), 2, i2cAddressValues, P168_I2C_ADDRESS);
        # ifndef BUILD_NO_DEBUG
        addFormNote(F("Address 0x48 only supported by VEML6030, ADDR -> VCC"));
        # endif // ifndef BUILD_NO_DEBUG
      } else {
        success = intArrayContains(2, i2cAddressValues, event->Par1);
      }

      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P168_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P168_READLUX_MODE = VEML_LUX_AUTO;
      P168_PSM_MODE     = static_cast<int>(P168_power_save_mode_e::Disabled);

      ExtraTaskSettings.TaskDeviceValueDecimals[1] = 0; // White
      ExtraTaskSettings.TaskDeviceValueDecimals[2] = 0; // Raw

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper *readMethod[] = {
          F("Normal"),
          F("Corrected"),
          F("Auto"),
          F("Normal (no wait)"),
          F("Corrected (no wait)"),
        };
        const int readMethodOptions[] = {
          VEML_LUX_NORMAL,
          VEML_LUX_CORRECTED,
          VEML_LUX_AUTO,
          VEML_LUX_NORMAL_NOWAIT,
          VEML_LUX_CORRECTED_NOWAIT,
        };
        constexpr size_t optionCount = NR_ELEMENTS(readMethodOptions);
        addFormSelector(F("Lux Read-method"),
                        F("rmth"),
                        optionCount,
                        readMethod,
                        readMethodOptions,
                        P168_READLUX_MODE);
        addFormNote(F("For 'Auto' Read-method, the Gain factor and Integration time settings are ignored."));
      }
      {
        const __FlashStringHelper *alsGain[] = {
          F("x1"),
          F("x2"),
          F("x(1/8)"),
          F("x(1/4)"),
        };
        const int alsGainOptions[] = {
          0b00,
          0b01,
          0b10,
          0b11,
        };
        constexpr size_t optionCount = NR_ELEMENTS(alsGainOptions);
        addFormSelector(F("Gain factor"),
                        F("gain"),
                        optionCount,
                        alsGain,
                        alsGainOptions,
                        P168_ALS_GAIN);
      }
      {
        const __FlashStringHelper *alsIntegration[] = {
          F("25 ms"),
          F("50 ms"),
          F("100 ms"),
          F("200 ms"),
          F("400 ms"),
          F("800 ms"),
        };
        const int alsIntegrationOptions[] = {
          0b1100,
          0b1000,
          0b0000,
          0b0001,
          0b0010,
          0b0011,
        };
        constexpr size_t optionCount = NR_ELEMENTS(alsIntegrationOptions);
        addFormSelector(F("Integration time"),
                        F("int"),
                        optionCount,
                        alsIntegration,
                        alsIntegrationOptions,
                        P168_ALS_INTEGRATION);
      }
      addFormSeparator(2);
      {
        const __FlashStringHelper *psmMode[] = {
          F("Disabled"),
          F("Mode 1"),
          F("Mode 2"),
          F("Mode 3"),
          F("Mode 4"),
        };
        const int psmModeOptions[] = {
          static_cast<int>(P168_power_save_mode_e::Disabled),
          static_cast<int>(P168_power_save_mode_e::Mode1),
          static_cast<int>(P168_power_save_mode_e::Mode2),
          static_cast<int>(P168_power_save_mode_e::Mode3),
          static_cast<int>(P168_power_save_mode_e::Mode4),
        };
        constexpr size_t optionCount = NR_ELEMENTS(psmModeOptions);
        addFormSelector(F("Power Save Mode"),
                        F("psm"),
                        optionCount,
                        psmMode,
                        psmModeOptions,
                        P168_PSM_MODE);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P168_I2C_ADDRESS     = getFormItemInt(F("i2c_addr"));
      P168_READLUX_MODE    = getFormItemInt(F("rmth"));
      P168_ALS_GAIN        = getFormItemInt(F("gain"));
      P168_ALS_INTEGRATION = getFormItemInt(F("int"));
      P168_PSM_MODE        = getFormItemInt(F("psm"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P168_data_struct(P168_ALS_GAIN,
                                                                               P168_ALS_INTEGRATION,
                                                                               P168_PSM_MODE,
                                                                               P168_READLUX_MODE));
      P168_data_struct *P168_data = static_cast<P168_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P168_data) && P168_data->init(event);

      break;
    }

    case PLUGIN_READ:
    {
      P168_data_struct *P168_data = static_cast<P168_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P168_data) {
        success = P168_data->plugin_read(event);
      }

      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      P168_data_struct *P168_data = static_cast<P168_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P168_data) {
        success = P168_data->plugin_get_config_value(event, string);
      }

      break;
    }
  }
  return success;
}

#endif // USES_P168
