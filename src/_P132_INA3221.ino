#include "_Plugin_Helper.h"
#ifdef USES_P132

// #######################################################################################################
// ############### Plugin 132: INA3221/INA226/INA228/INA260 DC Voltage/Current sensor ####################
// #######################################################################################################

/**
 * Changelog:
 * 2025-12-25 tonhuisman: Add P132_EXTENDED to also implement support for INA219, INA226, INA228, INA230, INA231 and INA260
 *                        INA230/INA231 are sometimes recognized as INA226
 *                        Without P132_EXTENDED set the original code is used (default for ESP8266).
 * 2025-01-18 tonhuisman: Implement support for MQTT AutoDiscovery
 * 2025-01-12 tonhuisman: Add support for MQTT AutoDiscovery (not supported yet for INA3221)
 * 2022-04-23 tonhuisman: Add separate settings for Conversion rate Voltage and Current
 * 2022-04-21 tonhuisman: Move source into PluginStructs
 * 2022-04-20 tonhuisman: Add averaging of samples and conversion rate settings
 * 2022-04-19 tonhuisman: Adapt to general ESPEasy coding standards
 **/

// Initial development: ## 25 jan 2021 Fred van Duin ####

# include "./src/PluginStructs/P132_data_struct.h"

# define PLUGIN_132
# define PLUGIN_ID_132         132
# define PLUGIN_VALUENAME1_132 "Value1"
# define PLUGIN_VALUENAME2_132 "Value2"
# define PLUGIN_VALUENAME3_132 "Value3"
# define PLUGIN_VALUENAME4_132 "Value4"

// See below for original code with just INA3221 support
# if P132_EXTENDED
boolean Plugin_132(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_132;
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

    #  if FEATURE_MQTT_DISCOVER
    case PLUGIN_GET_DISCOVERY_VTYPES:
    {
      success = getDiscoveryVType(event, Plugin_132_QueryVType, P132_CONFIG_BASE, event->Par5);
      break;
    }
    #  endif // if FEATURE_MQTT_DISCOVER

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
                                           0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F };
      constexpr size_t optionCount = NR_ELEMENTS(i2cAddressValues);

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        const P132_DeviceType deviceType = static_cast<P132_DeviceType>(P132_INA_TYPE);
        const size_t options             = P132_DeviceType::Ina3221 == deviceType ? 4 : optionCount;
        addFormSelectorI2C(F("i2c_addr"), options, i2cAddressValues, P132_I2C_ADDR);
        addFormNote(F("Address selection: see docs"));
      } else {
        success = intArrayContains(optionCount, i2cAddressValues, event->Par1);
      }
      break;
    }

    #  if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P132_I2C_ADDR;
      success     = true;
      break;
    }
    #  endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P132_VALUE_1 = 1; // Configure 'randomly'
      P132_VALUE_2 = 0;
      P132_VALUE_3 = 3;
      P132_VALUE_4 = 2;
      uint32_t lSettings = 0;
      set3BitToUL(lSettings, P132_FLAG_AVERAGE, 0x00);
      set4BitToUL(lSettings, P132_FLAG_V2_CONVERSION_B, 0x04);         // Voltage
      set4BitToUL(lSettings, P132_FLAG_V2_CONVERSION_S, 0x04);         // Current
      set2BitToUL(lSettings, P132_FLAG_CFG_VERSION, P132_CFG_VERSION); // V2
      P132_CONFIG_FLAGS = lSettings;
      P132_MAX_CURRENT  = 10;                                          // Guestimated
      P132_SHUNT_V2     = 100000;                                      // Default != 0
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper*typOptions[] = {
          toString(P132_DeviceType::Ina3221),
          toString(P132_DeviceType::Ina219),
          toString(P132_DeviceType::Ina226),
          toString(P132_DeviceType::Ina228),
          toString(P132_DeviceType::Ina230),
          toString(P132_DeviceType::Ina231),
          toString(P132_DeviceType::Ina260),
        };
        constexpr size_t typOptionsCount = NR_ELEMENTS(typOptions);

        const int typeOptionIndexes[] = {
          static_cast<int>(P132_DeviceType::Ina3221),
          static_cast<int>(P132_DeviceType::Ina219),
          static_cast<int>(P132_DeviceType::Ina226),
          static_cast<int>(P132_DeviceType::Ina228),
          static_cast<int>(P132_DeviceType::Ina230),
          static_cast<int>(P132_DeviceType::Ina231),
          static_cast<int>(P132_DeviceType::Ina260),
        };
        FormSelectorOptions typSelector(typOptionsCount, typOptions, typeOptionIndexes);
        typSelector.reloadonchange = true;
        typSelector.addFormSelector(F("INA type"), F("ityp"), P132_INA_TYPE);
      }

      addFormNumericBox(F("Max. Current"), F("mcur"), P132_MAX_CURRENT, 1, 1022);
      addUnit(F("1..1022 A"));

      const P132_DeviceType deviceType = static_cast<P132_DeviceType>(P132_INA_TYPE);

      {
        const __FlashStringHelper *varOptions[9];
        int varValues[9];

        if (P132_DeviceType::Ina3221 == deviceType)
        {
          varOptions[0] = F("Voltage channel 1");
          varOptions[1] = F("Current channel 1");
          varOptions[2] = F("Power channel 1");
          varOptions[3] = F("Voltage channel 2");
          varOptions[4] = F("Current channel 2");
          varOptions[5] = F("Power channel 2");
          varOptions[6] = F("Voltage channel 3");
          varOptions[7] = F("Current channel 3");
          varOptions[8] = F("Power channel 3");
          varValues[0]  = 1;
          varValues[1]  = 0;
          varValues[2]  = 6;
          varValues[3]  = 3;
          varValues[4]  = 2;
          varValues[5]  = 7;
          varValues[6]  = 5;
          varValues[7]  = 4;
          varValues[8]  = 8;
        }
        else
        {
          varOptions[0] = F("Voltage");
          varOptions[1] = F("Current");
          varOptions[2] = F("Power");
          varValues[0]  = 1;
          varValues[1]  = 0;
          varValues[2]  = 6;
        }
        const size_t optionCount = P132_DeviceTypeToMaxValues(deviceType);

        if (P132_INA_PREVIOUS != P132_INA_TYPE) {
          P132_VALUE_1 = 1; // Configure 'randomly' Voltage/Current/
          P132_VALUE_2 = 0;

          if (P132_DeviceType::Ina3221 == deviceType) {
            P132_VALUE_3 = 3; // Voltage/Current
            P132_VALUE_4 = 2;
          } else {
            P132_VALUE_3 = 6; // Power
          }

          if (P132_DeviceType::Ina219 == deviceType) {
            P132_SET_V2_CONVERSION_B(0b1000);
            P132_SET_V2_CONVERSION_S(0b1000);
          } else if (P132_DeviceType::Ina228 == deviceType) {
            P132_SET_V2_CONVERSION_B(0b0101);
            P132_SET_V2_CONVERSION_S(0b0101);
          } else {
            P132_SET_V2_CONVERSION_B(0b0100);
            P132_SET_V2_CONVERSION_S(0b0100);
          }

          if (P132_DeviceType::Ina260 == deviceType) {
            P132_SHUNT_V2 = 2000; // INA260 has 2 mOhm shunt built-in
          }
        }

        const FormSelectorOptions selector(optionCount, varOptions, varValues);

        for (uint8_t r = 0; r < min(optionCount, (size_t)VARS_PER_TASK); ++r) {
          selector.addFormSelector(
            concat(F("Power value "), r + 1),
            getPluginCustomArgName(r),
            PCONFIG(P132_CONFIG_BASE + r));
        }
      }

      addFormSubHeader(F("Hardware"));

      {
        const __FlashStringHelper *varShuntOptions[] = {
          F("0.1 &#8486;"),
          F("0.01 &#8486;"),
          F("0.015 &#8486;"), // INA228 often used
          F("0.005 &#8486;"),
          F("0.002 &#8486;"), // INA260 built-in
          F("20 A @ 75mV"),
          F("30 A @ 75mV"),
          F("50 A @ 75mV"),
          F("75 A @ 75mV"),
          F("100 A @ 75mV"),
          F("150 A @ 75mV"),
          F("200 A @ 75mV"),
          F("300 A @ 75mV"),
          F("400 A @ 75mV"),
          F("500 A @ 75mV"),
          F("600 A @ 75mV"),
          F("750 A @ 75mV"),
          F("1000 A @ 75mV"),
        };

        /* *INDENT-OFF* */
        const int shuntValues[] = { 100000, 10000, 15000, 5000, 2000,                                         // 'Ohm'
                                    3750,   2500,  1500,  1000, 750, 500, 375, 250, 188, 150, 125, 100, 75 }; // Amp@75mV
        // Values in microOhm
 /* *INDENT-ON* */

        constexpr size_t optionCount = NR_ELEMENTS(shuntValues);
        FormSelectorOptions selector(optionCount, varShuntOptions, shuntValues);
        selector.enabled = P132_DeviceType::Ina260 != deviceType; // Built-in shunt
        const uint32_t shunt = P132_CFG_VERSION != P132_GET_CFG_VERSION ? (100 / P132_SHUNT) * 1000 : P132_SHUNT_V2;
        selector.addFormSelector(F("Shunt resistor"), F("shunt"), shunt);
        addUnit(F("Ohm / Ampere @ 75mV"));
        addFormNote(F("Select as is installed on the board."));
      }

      addFormSubHeader(F("Measurement"));

      if (P132_DeviceType::Ina219 != deviceType) {
        const __FlashStringHelper *averagingSamples[] = {
          F("1"),
          F("4"),
          F("16"),
          F("64"),
          F("128"),
          F("256"),
          F("512"),
          F("1024"),
        };
        const int averageValue[]     = { 0b000, 0b001, 0b010, 0b011, 0b100, 0b101, 0b110, 0b111 };
        constexpr size_t optionCount = NR_ELEMENTS(averageValue);
        const FormSelectorOptions selector(optionCount, averagingSamples, averageValue);

        selector.addFormSelector(F("Averaging samples"), F("average"), P132_GET_AVERAGE);
      }

      {
        const __FlashStringHelper *conversionRates[11];
        int conversionValues[11];

        if (P132_DeviceType::Ina219 == deviceType) {
          conversionRates[0]   = F("9 bits / 84 &micro;sec");
          conversionRates[1]   = F("10 bits / 148 &micro;sec");
          conversionRates[2]   = F("11 bits / 276 &micro;sec");
          conversionRates[3]   = F("12 bits / 532 &micro;sec");
          conversionRates[4]   = F("2 samples / 1.106 msec");
          conversionRates[5]   = F("4 samples / 2.13 msec");
          conversionRates[6]   = F("8 samples / 4.26 msec");
          conversionRates[7]   = F("16 samples / 8.51 msec");
          conversionRates[8]   = F("32 samples / 17.02 msec");
          conversionRates[9]   = F("54 samples / 34.05 msec");
          conversionRates[10]  = F("128 samples / 68.10 msec");
          conversionValues[0]  = 0b0000;
          conversionValues[1]  = 0b0001;
          conversionValues[2]  = 0b0010;
          conversionValues[3]  = 0b1000;
          conversionValues[4]  = 0b1001;
          conversionValues[5]  = 0b1010;
          conversionValues[6]  = 0b1011;
          conversionValues[7]  = 0b1100;
          conversionValues[8]  = 0b1101;
          conversionValues[9]  = 0b1110;
          conversionValues[10] = 0b1111;
        } else {
          if (P132_DeviceType::Ina228 == deviceType) {
            conversionRates[0] = F("50 &micro;sec");
            conversionRates[1] = F("84 &micro;sec");
            conversionRates[2] = F("150 &micro;sec");
            conversionRates[3] = F("280 &micro;sec");
            conversionRates[4] = F("588 &micro;sec");
            conversionRates[5] = F("1.052 msec");
            conversionRates[6] = F("2.074 msec");
            conversionRates[7] = F("4.120 msec");
          } else {
            conversionRates[0] = F("140 &micro;sec");
            conversionRates[1] = F("204 &micro;sec");
            conversionRates[2] = F("332 &micro;sec");
            conversionRates[3] = F("588 &micro;sec");
            conversionRates[4] = F("1.1 msec");
            conversionRates[5] = F("2.116 msec");
            conversionRates[6] = F("4.156 msec");
            conversionRates[7] = F("8.244 msec");
          }
          conversionValues[0] = 0b0000;
          conversionValues[1] = 0b0001;
          conversionValues[2] = 0b0010;
          conversionValues[3] = 0b0011;
          conversionValues[4] = 0b0100;
          conversionValues[5] = 0b0101;
          conversionValues[6] = 0b0110;
          conversionValues[7] = 0b0111;
        }

        const size_t optionCount = P132_DeviceType::Ina219 == deviceType ? 11 : 8;
        FormSelectorOptions selector(optionCount, conversionRates, conversionValues);

        // INA219: 12 bit / 523 usec INA228: 1.052 msec, other INA: 1.1 msec
        selector.default_index = P132_DeviceType::Ina219 == deviceType ? 0b1000 :
                                 (P132_DeviceType::Ina228 == deviceType ? 0b0101 : 0b0100);
        const uint8_t convB = 0 == P132_GET_CFG_VERSION ? P132_GET_CONVERSION_B : P132_GET_V2_CONVERSION_B;
        const uint8_t convS = 0 == P132_GET_CFG_VERSION ? P132_GET_CONVERSION_S : P132_GET_V2_CONVERSION_S;
        selector.addFormSelector(F("Conversion rate Voltage"), F("conv_v"), convB);
        selector.addFormSelector(F("Conversion rate Current"), F("conv_c"), convS);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P132_I2C_ADDR     = getFormItemInt(F("i2c_addr"));
      P132_INA_PREVIOUS = P132_INA_TYPE;
      P132_INA_TYPE     = getFormItemInt(F("ityp"));
      P132_MAX_CURRENT  = getFormItemInt(F("mcur"));

      const P132_DeviceType deviceType = static_cast<P132_DeviceType>(P132_INA_TYPE);
      const size_t optionCount         = P132_DeviceTypeToMaxValues(deviceType);

      for (uint8_t r = 0; r < min(optionCount, (size_t)VARS_PER_TASK); ++r) {
        PCONFIG(P132_CONFIG_BASE + r) = getFormItemIntCustomArgName(r);
      }
      P132_SHUNT_V2 = getFormItemInt(F("shunt"));

      uint32_t lSettings = 0;

      if (P132_DeviceType::Ina219 != deviceType) {
        set3BitToUL(lSettings, P132_FLAG_AVERAGE, getFormItemInt(F("average")));
      }

      set4BitToUL(lSettings, P132_FLAG_V2_CONVERSION_B, getFormItemInt(F("conv_v")));
      set4BitToUL(lSettings, P132_FLAG_V2_CONVERSION_S, getFormItemInt(F("conv_c")));

      set2BitToUL(lSettings, P132_FLAG_CFG_VERSION, P132_CFG_VERSION); // Write version update
      P132_CONFIG_FLAGS = lSettings;

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P132_data_struct(event));
      P132_data_struct *P132_data = static_cast<P132_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = nullptr != P132_data && P132_data->isInitialized();

      break;
    }

    case PLUGIN_READ:
    {
      P132_data_struct *P132_data = static_cast<P132_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P132_data) {
        return success;
      }

      const P132_DeviceType deviceType = static_cast<P132_DeviceType>(P132_INA_TYPE);
      const size_t optionCount         = P132_DeviceTypeToMaxValues(deviceType);

      for (uint8_t r = 0; r < min(optionCount, (size_t)VARS_PER_TASK); ++r) {
        // VALUES 1..4
        const uint8_t reg = static_cast<uint8_t>(PCONFIG(P132_CONFIG_BASE + r));
        uint8_t channel   = 0; //

        if ((2 == reg) || (3 == reg) || (7 == reg)) {
          channel = 1;
        } else if ((4 == reg) || (5 == reg) || (8 == reg)) {
          channel = 2;
        }

        if (P132_data->conversionFinished(channel)) {
          switch (reg) {
            case 0: // Current
            case 2: // Current
            case 4: // Current
              UserVar.setFloat(event->TaskIndex, r,
                               P132_data->getBusCurrent_mA(channel));
              break;
            case 1: // Voltage
            case 3: // Voltage
            case 5: // Voltage
              UserVar.setFloat(event->TaskIndex, r,
                               P132_data->getBusVoltage_V(channel)
                               + (P132_data->getShuntVoltage_mV(channel) / 1000.0f));
              break;
            case 6: // Power
            case 7: // Power
            case 8: // Power
              UserVar.setFloat(event->TaskIndex, r,
                               P132_data->getBusPower_mW(channel));
              break;
          }
          success = true; // Only if we have read data
        }
      }

      #  ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLog(LOG_LEVEL_INFO, strformat(F("%s: Values: %.2f/%.2f/%.2f/%.2f"),
                                         FsP(toString(static_cast<P132_DeviceType>(P132_INA_TYPE))),
                                         UserVar.getFloat(event->TaskIndex, 0),
                                         UserVar.getFloat(event->TaskIndex, 1),
                                         UserVar.getFloat(event->TaskIndex, 2),
                                         UserVar.getFloat(event->TaskIndex, 3)));
      }
      #  endif // ifndef BUILD_NO_DEBUG

      break;
    }
  }

  return success;
}

# endif // if P132_EXTENDED

# if !P132_EXTENDED
boolean Plugin_132(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_132;
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

    #  if FEATURE_MQTT_DISCOVER
    case PLUGIN_GET_DISCOVERY_VTYPES:
    {
      success = getDiscoveryVType(event, Plugin_132_QueryVType, P132_CONFIG_BASE, event->Par5);
      break;
    }
    #  endif // if FEATURE_MQTT_DISCOVER

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x40, 0x41, 0x42, 0x43 };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), NR_ELEMENTS(i2cAddressValues), i2cAddressValues, P132_I2C_ADDR);
        addFormNote(F("A0 connected to: GND= 0x40, VCC= 0x41, SDA= 0x42, SCL= 0x43"));
      } else {
        success = intArrayContains(NR_ELEMENTS(i2cAddressValues), i2cAddressValues, event->Par1);
      }
      break;
    }

    #  if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P132_I2C_ADDR;
      success     = true;
      break;
    }
    #  endif // if FEATURE_I2C_GET_ADDRESS

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
      {
        const __FlashStringHelper *varOptions[] = {
          F("Current channel 1"),
          F("Voltage channel 1"),
          F("Current channel 2"),
          F("Voltage channel 2"),
          F("Current channel 3"),
          F("Voltage channel 3")
        };
        constexpr size_t optionCount = NR_ELEMENTS(varOptions);

        const FormSelectorOptions selector(optionCount, varOptions);

        for (uint8_t r = 0; r < VARS_PER_TASK; ++r) {
          selector.addFormSelector(
            concat(F("Power value "), r + 1),
            getPluginCustomArgName(r),
            PCONFIG(P132_CONFIG_BASE + r));
        }
      }


      addFormSubHeader(F("Hardware"));

      {
        const __FlashStringHelper *varshuntptions[] = {
          F("0.1"),
          F("0.01"),
          F("0.005"),
        };
        const int shuntvalue[]       = { 1, 10, 20 };
        constexpr size_t optionCount = NR_ELEMENTS(shuntvalue);
        const FormSelectorOptions selector(optionCount, varshuntptions, shuntvalue);
        selector.addFormSelector(F("Shunt resistor"), F("shunt"), P132_SHUNT);
        addUnit(F("Ohm"));
        addFormNote(F("Select as is installed on the board."));
      }

      addFormSubHeader(F("Measurement"));

      {
        const __FlashStringHelper *averagingSamples[] = {
          F("1"),
          F("4"),
          F("16"),
          F("64"),
          F("128"),
          F("256"),
          F("512"),
          F("1024"),
        };
        const int averageValue[]     = { 0b000, 0b001, 0b010, 0b011, 0b100, 0b101, 0b110, 0b111 };
        constexpr size_t optionCount = NR_ELEMENTS(averageValue);
        FormSelectorOptions selector(optionCount, averagingSamples, averageValue);
        selector.default_index = 0b000;
        selector.addFormSelector(F("Averaging samples"), F("average"), P132_GET_AVERAGE);
        addFormNote(F("Samples &gt; 16 then min. Interval: 64= 4, 128= 7, 256= 14, 512= 26, 1024= 52 seconds!"));
      }

      {
        const __FlashStringHelper *conversionRates[] = {
          F("140 &micro;sec"),
          F("204 &micro;sec"),
          F("332 &micro;sec"),
          F("588 &micro;sec"),
          F("1.1 msec"),
          F("2.116 msec"),
          F("4.156 msec"),
          F("8.244 msec"),
        };

        //                               140us  204us  332us  588us  1.1ms  2.1ms  4.1ms  8.2ms
        const int conversionValues[] = { 0b000, 0b001, 0b010, 0b011, 0b100, 0b101, 0b110, 0b111 };
        constexpr size_t optionCount = NR_ELEMENTS(conversionValues);
        FormSelectorOptions selector(optionCount, conversionRates, conversionValues);
        selector.default_index = 0b100; // 1.1ms
        selector.addFormSelector(F("Conversion rate Voltage"), F("conv_v"), P132_GET_CONVERSION_B);
        selector.addFormSelector(F("Conversion rate Current"), F("conv_c"), P132_GET_CONVERSION_S);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P132_I2C_ADDR = getFormItemInt(F("i2c_addr"));

      for (uint8_t r = 0; r < VARS_PER_TASK; ++r) {
        PCONFIG(P132_CONFIG_BASE + r) = getFormItemIntCustomArgName(r);
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

      for (uint8_t r = 0; r < VARS_PER_TASK; ++r) {
        // VALUES 1..4
        const uint8_t reg = static_cast<uint8_t>(PCONFIG(P132_CONFIG_BASE + r) + 1);

        if ((reg == 2) || (reg == 4) || (reg == 6)) {
          UserVar.setFloat(event->TaskIndex, r,
                           P132_data->getBusVoltage_V(reg)
                           + (P132_data->getShuntVoltage_mV(reg - 1) / 1000.0f));
        } else {
          UserVar.setFloat(event->TaskIndex, r, (P132_data->getShuntVoltage_mV(reg) / 100.0f) * P132_SHUNT);
        }
      }

      #  ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLog(LOG_LEVEL_INFO, strformat(F("INA3221: Values: %.2f/%.2f/%.2f/%.2f"),
                                         UserVar[event->BaseVarIndex],
                                         UserVar[event->BaseVarIndex + 1],
                                         UserVar[event->BaseVarIndex + 2],
                                         UserVar[event->BaseVarIndex + 3]));
      }
      #  endif // ifndef BUILD_NO_DEBUG

      success = true;
      break;
    }
  }

  return success;
}

# endif // if !P132_EXTENDED

#endif // USES_P132
