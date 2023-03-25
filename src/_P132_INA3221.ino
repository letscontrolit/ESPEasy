#ifdef USES_P132

// #######################################################################################################
// ######################### Plugin 132: INA3221 DC Voltage/Current sensor ###############################
// #######################################################################################################

/**
 * Changelog:
 * 2022-04-23, tonhuisman: Add separate settings for Conversion rate Voltage and Current
 * 2022-04-21, tonhuisman: Move source into PluginStructs
 * 2022-04-20, tonhuisman: Add averaging of samples and conversion rate settings
 * 2022-04-19, tonhuisman: Adapt to general ESPEasy coding standards
 **/

// Initial development: ## 25 jan 2021 Fred van Duin ####

#include "_Plugin_Helper.h"

#define PLUGIN_132
#define PLUGIN_ID_132         132
#define PLUGIN_NAME_132       "Energy (DC) - INA3221"
#define PLUGIN_VALUENAME1_132 "Value1"
#define PLUGIN_VALUENAME2_132 "Value2"
#define PLUGIN_VALUENAME3_132 "Value3"
#define PLUGIN_VALUENAME4_132 "Value4"

#include "./src/PluginStructs/P132_data_struct.h"

boolean Plugin_132(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_132;
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
      string = F(PLUGIN_NAME_132);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_132));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_132));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_132));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_132));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x40, 0x41, 0x42, 0x43 };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 4, i2cAddressValues, P132_I2C_ADDR);
        addFormNote(F("A0 connected to: GND= 0x40, VCC= 0x41, SDA= 0x42, SCL= 0x43"));
      } else {
        success = intArrayContains(4, i2cAddressValues, event->Par1);
      }
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P132_I2C_ADDR;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P132_VALUE_1 = 0; // Configure randomly
      P132_VALUE_2 = 1;
      P132_VALUE_3 = 2;
      P132_VALUE_4 = 3;
      uint32_t lSettings = 0;
      set3BitToUL(lSettings, P132_FLAG_AVERAGE,      0x00);
      set3BitToUL(lSettings, P132_FLAG_CONVERSION_B, 0x04); // Voltage
      set3BitToUL(lSettings, P132_FLAG_CONVERSION_S, 0x04); // Current
      P132_CONFIG_FLAGS = lSettings;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      #define INA3221_var_OPTION 6
      {
        const __FlashStringHelper *varOptions[] = {
          F("Current channel 1"),
          F("Voltage channel 1"),
          F("Current channel 2"),
          F("Voltage channel 2"),
          F("Current channel 3"),
          F("Voltage channel 3")
        };

        for (uint8_t r = 0; r < VARS_PER_TASK; r++) {
          addFormSelector(concat(F("Power value "), r + 1),
                          getPluginCustomArgName(r), INA3221_var_OPTION, varOptions, NULL, PCONFIG(P132_CONFIG_BASE + r));
        }
      }


      addFormSubHeader(F("Hardware"));

      #define INA3221_shunt_OPTION 3
      {
        const __FlashStringHelper *varshuntptions[] = {
          F("0.1 ohm"),
          F("0.01 ohm"),
          F("0.005 ohm"),
        };
        const int shuntvalue[] = { 1, 10, 20 };
        addFormSelector(F("Shunt resistor"), F("shunt"), INA3221_shunt_OPTION, varshuntptions, shuntvalue, P132_SHUNT);
        addFormNote(F("Select as is installed on the board."));
      }

      addFormSubHeader(F("Measurement"));

      #define INA3221_average_OPTION 8
      {
        const __FlashStringHelper *averagingSamples[] = {
          F("1 (default)"),
          F("4"),
          F("16"),
          F("64"),
          F("128"),
          F("256"),
          F("512"),
          F("1024"),
        };
        const int averageValue[] = { 0b000, 0b001, 0b010, 0b011, 0b100, 0b101, 0b110, 0b111 };
        addFormSelector(F("Averaging samples"),
                        F("average"),
                        INA3221_average_OPTION,
                        averagingSamples,
                        averageValue,
                        P132_GET_AVERAGE);
        addFormNote(F("Samples &gt; 16 then min. Interval: 64= 4, 128= 7, 256= 14, 512= 26, 1024= 52 seconds!"));
      }

      #define INA3221_conversion_OPTION 8
      {
        const __FlashStringHelper *conversionRates[] = {
          F("140 &micro;sec"),
          F("204 &micro;sec"),
          F("332 &micro;sec"),
          F("588 &micro;sec"),
          F("1.1 msec (default)"),
          F("2.116 msec"),
          F("4.156 msec"),
          F("8.244 msec"),
        };

        //                               140us  204us  332us  588us  1.1ms  2.1ms  4.1ms  8.2ms
        const int conversionValues[] = { 0b000, 0b001, 0b010, 0b011, 0b100, 0b101, 0b110, 0b111 };
        addFormSelector(F("Conversion rate Voltage"),
                        F("conv_v"),
                        INA3221_conversion_OPTION,
                        conversionRates,
                        conversionValues,
                        P132_GET_CONVERSION_B);

        addFormSelector(F("Conversion rate Current"),
                        F("conv_c"),
                        INA3221_conversion_OPTION,
                        conversionRates,
                        conversionValues,
                        P132_GET_CONVERSION_S);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P132_I2C_ADDR = getFormItemInt(F("i2c_addr"));

      for (uint8_t r = 0; r < VARS_PER_TASK; r++) {
        PCONFIG(P132_CONFIG_BASE + r) = getFormItemInt(getPluginCustomArgName(r));
      }
      P132_SHUNT = getFormItemInt(F("shunt"));

      uint32_t lSettings = 0;
      set3BitToUL(lSettings, P132_FLAG_AVERAGE,      getFormItemInt(F("average")));
      set3BitToUL(lSettings, P132_FLAG_CONVERSION_B, getFormItemInt(F("conv_v")));
      set3BitToUL(lSettings, P132_FLAG_CONVERSION_S, getFormItemInt(F("conv_c")));
      P132_CONFIG_FLAGS = lSettings;

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P132_data_struct(event));
      P132_data_struct *P132_data = static_cast<P132_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P132_data) {
        P132_data->setCalibration_INA3221(event);
        success = true;
      }

      break;
    }

    case PLUGIN_READ:
    {
      P132_data_struct *P132_data = static_cast<P132_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P132_data) {
        return success;
      }

      uint8_t reg;

      for (uint8_t r = 0; r < VARS_PER_TASK; r++) {
        // VALUES 1..4
        reg = static_cast<uint8_t>(PCONFIG(P132_CONFIG_BASE + r) + 1);

        if ((reg == 2) || (reg == 4) || (reg == 6)) {
          UserVar[event->BaseVarIndex + r] = P132_data->getBusVoltage_V(reg)
                                             + (P132_data->getShuntVoltage_mV(reg - 1) / 1000.0f);
        } else {
          UserVar[event->BaseVarIndex + r] = (P132_data->getShuntVoltage_mV(reg) / 100.0f) * P132_SHUNT;
        }
      }

      #ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("INA3221: Values: ");
        log += UserVar[event->BaseVarIndex];
        log += F(", ");
        log += UserVar[event->BaseVarIndex + 1];
        log += F(", ");
        log += UserVar[event->BaseVarIndex + 2];
        log += F(", ");
        log += UserVar[event->BaseVarIndex + 3];
        addLog(LOG_LEVEL_INFO, log);
      }
      #endif // ifndef BUILD_NO_DEBUG

      success = true;
      break;
    }
  }

  return success;
}

#endif // USES_P132
