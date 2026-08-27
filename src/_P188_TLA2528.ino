#include "_Plugin_Helper.h"
#ifdef USES_P188

// #######################################################################################################
// ############################### Plugin 188: TLA2528 I2C 0x10  #########################################
// #######################################################################################################

/** Changelog:
 * 2026-08-22 SuksAe: Initial release
 */

# include "src/WebServer/DevicesPage.h" // Needed for format_I2C_port_description

# include "src/PluginStructs/P188_data_struct.h"

# define PLUGIN_188
# define PLUGIN_ID_188 188
# define PLUGIN_NAME_188 "Analog input - TLA2528"

# define P188_DEBUG 0  // change to 1 to include debug output for this plugin

boolean Plugin_188(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number = PLUGIN_ID_188;                      // Plugin ID number.   (PLUGIN_ID_xxx)
      dev.Type   = DEVICE_TYPE_I2C;                    // How the device is connected. e.g. DEVICE_TYPE_SINGLE => connected through 1
                                                       // datapin
      dev.VType  = Sensor_VType::SENSOR_TYPE_QUAD;     // Type of value the plugin will return. e.g. SENSOR_TYPE_STRING
//      dev.Ports              = 0;                      // Port to use when device has multiple I/O pins  (N.B. not used much)
      dev.ValueCount         = 4;                      // The number of output values of a plugin. The value should match the number of keys
                                                       // PLUGIN_VALUENAME1_xxx
      dev.OutputDataType = Output_Data_type_t::Simple; // Subset of selectable output data types  (Default = no selection)
//      dev.PullUpOption       = false;                  // Allow to set internal pull-up resistors.
//      dev.InverseLogicOption = false;                  // Allow to invert the boolean state (e.g. a switch)
      dev.FormulaOption      = true;                   // Allow to enter a formula to convert values during read. (not possible with Custom
                                                       // enabled)
//      dev.Custom             = false;
      dev.SendDataOption     = true;                   // Allow to send data to a controller.
//      dev.GlobalSyncOption   = false;                  // No longer used. Was used for ESPeasy values sync between nodes
      dev.TimerOption        = true;                   // Allow to set the "Interval" timer for the plugin.
//      dev.TimerOptional      = false;                  // When taskdevice timer is not set and not optional, use default
                                                       // "Interval" delay (Settings.Delay)
//      dev.DecimalsOnly       = false;                  // Allow to set the number of decimals (otherwise treated a 0
                                                       // decimals)
      dev.CustomVTypeVar     = true;                   // Enable to allow the user to configure the Sensor_VType per Value that's available
                                                       // for the plugin
      dev.PluginStats        = true;                   // Support for PluginStats to record last N task values, show charts etc.
      dev.MqttStateClass     = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_188);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      const int valueCount = getValueCountFromSensorType(static_cast<Sensor_VType>(P188_OUTPUT_TYPE));

      for (uint8_t chNum = 0; chNum < VARS_PER_TASK; ++chNum) {
        if (chNum < valueCount) {
          ExtraTaskSettings.setTaskDeviceValueName(chNum, Plugin_188_output_mapping_name(PCONFIG(chNum + P188_OUTPUT_MAPPING_OFFSET), false));
        } else {
          ExtraTaskSettings.clearTaskDeviceValueName(chNum);
        }
      }

      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P188_NR_OUTPUT_VALUES;
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(P188_OUTPUT_TYPE);
      event->idx        = P188_OUTPUT_TYPE_INDEX;
      success           = true;
      break;
    }

# if FEATURE_MQTT_DISCOVER
    case PLUGIN_GET_DISCOVERY_VTYPES:
    {
      success = getDiscoveryVType(event, Plugin_QueryVType_Analog, 255, event->Par5);
      break;
    }
# endif // if FEATURE_MQTT_DISCOVER

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17 };
      constexpr int nrAddressOptions   = NR_ELEMENTS(i2cAddressValues);

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), nrAddressOptions, i2cAddressValues, P188_I2C_ADDR);
      } else {
        success = intArrayContains(nrAddressOptions, i2cAddressValues, event->Par1);
      }
      break;
    }

# if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P188_I2C_ADDR;
      success     = true;
      break;
    }
# endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      P188_config_struct tmp_config;

      P188_OUTPUT_TYPE = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_QUAD);

      tmp_config.ADC_Vref = 5.0f;

# if P188_FEATURE_RESISTOR_MEASUREMENT
      tmp_config.R_Clip = 10000000.0f;
# endif // P188_FEATURE_RESISTOR_MEASUREMENT

      tmp_config.i2cAddress = P188_I2C_ADDR;

      for (int chNum = 0; chNum < VARS_PER_TASK; chNum++)
      {
        PCONFIG(chNum + P188_OUTPUT_MAPPING_OFFSET) = 2 * chNum;

# if P188_FEATURE_RESISTOR_MEASUREMENT
        tmp_config.Rref[chNum] = 4700;
        tmp_config.Rpar[chNum] = 100000;
# endif // P188_FEATURE_RESISTOR_MEASUREMENT

      }
      SaveCustomTaskSettings(event->TaskIndex, (uint8_t *)&(tmp_config), sizeof(tmp_config), 0); // save configuration to flash
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      const P188_CONFIG_BITS_t P188_configBits(P188_CONFIG_BITS);

      P188_config_struct tmp_config;

      LoadCustomTaskSettings(event->TaskIndex, (uint8_t *)&(tmp_config), sizeof(tmp_config)); // load configuration from flash

      const int valueCount = getValueCountFromSensorType(static_cast<Sensor_VType>(P188_OUTPUT_TYPE));

      addFormFloatNumberBox(F("ADC reference voltage"), F("ADC_Vref"), tmp_config.ADC_Vref, 2.35f, 5.5f, 2, 0.01f);

# if P188_FEATURE_RESISTOR_MEASUREMENT
      if ((P188_OUTPUT_MAPPING_0 > P188_OUTPUT_MAPPING_RESISTOR_MODES_OFFSET) ||
          (P188_OUTPUT_MAPPING_1 > P188_OUTPUT_MAPPING_RESISTOR_MODES_OFFSET) ||
          (P188_OUTPUT_MAPPING_2 > P188_OUTPUT_MAPPING_RESISTOR_MODES_OFFSET) ||
          (P188_OUTPUT_MAPPING_3 > P188_OUTPUT_MAPPING_RESISTOR_MODES_OFFSET))
      {
        addFormFloatNumberBox(F("Resistor measurment clipping"), F("R_Clip"), tmp_config.R_Clip, 0.0f, 10000000.0f, 2, 0.01f);
      }
# endif // P188_FEATURE_RESISTOR_MEASUREMENT

      for (int chNum = 0; chNum < valueCount; chNum++)
      {
        addFormSubHeader(strformat(F("Output %d - %s"), chNum + 1,
                                   FsP(Plugin_188_output_mapping_name(PCONFIG(chNum + P188_OUTPUT_MAPPING_OFFSET), true))));

# if P188_FEATURE_RESISTOR_MEASUREMENT
        if (PCONFIG(chNum + P188_OUTPUT_MAPPING_OFFSET) > P188_OUTPUT_MAPPING_RESISTOR_MODES_OFFSET)
        {
          addFormNumericBox(F("Reference Resistor Value"), getPluginCustomArgName(chNum * 8 + 0), tmp_config.Rref[chNum], 100, 470000);
          addFormNumericBox(F("Parallel Resistor Value"),  getPluginCustomArgName(chNum * 8 + 1), tmp_config.Rpar[chNum], 0,   470000);
        }
# endif // P188_FEATURE_RESISTOR_MEASUREMENT

        addFormCheckBox(F("Output Raw ADC Value"), getPluginCustomArgName(chNum * 8 + 2), bitRead(P188_configBits.raw_val, chNum));

        /* Callibration */

        addTableSeparator(F("Two Point Calibration"), 2, 4);

        addFormCheckBox(F("Enable Calibration"), getPluginCustomArgName(chNum * 8 + 3), bitRead(P188_configBits.en_cal, chNum));

        addFormFloatNumberBox(F("Point 1"),
                              getPluginCustomArgName(chNum * 8 + 4),
                              tmp_config.CalIn[chNum][0],
                              -1000000.0f,
                              1000000.0f,
                              2,
                              1.0f);
        html_add_estimate_symbol();
        addFloatNumberBox(getPluginCustomArgName(chNum * 8 + 5), tmp_config.CalOut[chNum][0], -1000000.0f, 1000000.0f, 2, 1.0f);

        addFormFloatNumberBox(F("Point 2"),
                              getPluginCustomArgName(chNum * 8 + 6),
                              tmp_config.CalIn[chNum][1],
                              -1000000.0f,
                              1000000.0f,
                              2,
                              1.0f);
        html_add_estimate_symbol();
        addFloatNumberBox(getPluginCustomArgName(chNum * 8 + 7), tmp_config.CalOut[chNum][1], -1000000.0f, 1000000.0f, 2, 1.0f);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR:
    {
      String options[P188_OUTPUT_OPTION_CNT];

      for (uint8_t option = 0; option < P188_OUTPUT_OPTION_CNT; ++option) {
        options[option] = Plugin_188_output_mapping_name(option, true);
      }

      const int valueCount = getValueCountFromSensorType(static_cast<Sensor_VType>(P188_OUTPUT_TYPE));

      for (uint8_t chNum = 0; chNum < valueCount; ++chNum) {
        const uint8_t pconfigIndex = chNum + P188_OUTPUT_MAPPING_OFFSET;
        sensorTypeHelper_loadOutputSelector(event, pconfigIndex, chNum, P188_OUTPUT_OPTION_CNT, options);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P188_config_struct tmp_config;
      LoadCustomTaskSettings(event->TaskIndex, (uint8_t *)&(tmp_config), sizeof(tmp_config)); // load configuration from flash

      P188_CONFIG_BITS_t P188_configBits(P188_CONFIG_BITS);

      P188_configBits.raw_val = 0;
      P188_configBits.en_cal  = 0;

      P188_I2C_ADDR         = getFormItemInt(F("i2c_addr"));
      tmp_config.i2cAddress = P188_I2C_ADDR;
      tmp_config.ADC_Vref   = getFormItemFloat(F("ADC_Vref"));

# if P188_FEATURE_RESISTOR_MEASUREMENT
      if ((P188_OUTPUT_MAPPING_0 > P188_OUTPUT_MAPPING_RESISTOR_MODES_OFFSET) ||
          (P188_OUTPUT_MAPPING_1 > P188_OUTPUT_MAPPING_RESISTOR_MODES_OFFSET) ||
          (P188_OUTPUT_MAPPING_2 > P188_OUTPUT_MAPPING_RESISTOR_MODES_OFFSET) ||
          (P188_OUTPUT_MAPPING_3 > P188_OUTPUT_MAPPING_RESISTOR_MODES_OFFSET))
      {
        tmp_config.R_Clip = getFormItemFloat(F("R_Clip"));
      }
# endif // P188_FEATURE_RESISTOR_MEASUREMENT

      const int valueCount = getValueCountFromSensorType(static_cast<Sensor_VType>(P188_OUTPUT_TYPE));

      for (int chNum = 0; chNum < valueCount; chNum++)
      {

# if P188_FEATURE_RESISTOR_MEASUREMENT
        if (PCONFIG(chNum + P188_OUTPUT_MAPPING_OFFSET) > P188_OUTPUT_MAPPING_RESISTOR_MODES_OFFSET)
        {
          tmp_config.Rref[chNum] = getFormItemInt(getPluginCustomArgName(chNum * 8 + 0));
          tmp_config.Rpar[chNum] = getFormItemInt(getPluginCustomArgName(chNum * 8 + 1));
        }
# endif // P188_FEATURE_RESISTOR_MEASUREMENT

        bitWrite(P188_configBits.raw_val, chNum, isFormItemChecked(getPluginCustomArgName(chNum * 8 + 2)));

        /* Callibration */

        // Enable
        bitWrite(P188_configBits.en_cal, chNum, isFormItemChecked(getPluginCustomArgName(chNum * 8 + 3)));

        // Save configBits
        P188_CONFIG_BITS = P188_configBits.pconfigvalue();

        // Point 1
        tmp_config.CalIn[chNum][0]  = getFormItemFloat(getPluginCustomArgName(chNum * 8 + 4));
        tmp_config.CalOut[chNum][0] = getFormItemFloat(getPluginCustomArgName(chNum * 8 + 5));

        // Point 2
        tmp_config.CalIn[chNum][1]  = getFormItemFloat(getPluginCustomArgName(chNum * 8 + 6));
        tmp_config.CalOut[chNum][1] = getFormItemFloat(getPluginCustomArgName(chNum * 8 + 7));
      }

      for (uint8_t chNum = 0; chNum < P188_NR_OUTPUT_VALUES; ++chNum) {
        const uint8_t pconfigIndex = P188_OUTPUT_MAPPING_OFFSET + chNum;
        const uint8_t choice       = PCONFIG(pconfigIndex);
        sensorTypeHelper_saveOutputSelector(event, pconfigIndex, chNum, Plugin_188_output_mapping_name(choice, false));
      }

      success = SaveCustomTaskSettings(event->TaskIndex, (uint8_t *)&(tmp_config), sizeof(tmp_config), 0); // save configuration to flash
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      format_I2C_port_description(event->TaskIndex);

      for (uint8_t chNum = 0; chNum < P188_NR_OUTPUT_VALUES; ++chNum) {
        const uint8_t choice = PCONFIG(P188_OUTPUT_MAPPING_OFFSET + chNum);

        if ((choice >= 0) && (choice < 16)) {
          addHtml(F("<br>"));
          addHtml(Plugin_188_output_mapping_name(choice, false));
        }
      }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (initPluginTaskData(event->TaskIndex, new (std::nothrow) P188_data_struct(event))) {
        P188_data_struct *P188_data = static_cast<P188_data_struct *>(getPluginTaskData(event->TaskIndex));

        success = (nullptr != P188_data) && P188_data->init(event);
      }
      break;
    }

    case PLUGIN_READ:
    {
      const P188_data_struct  *P188_data = static_cast<P188_data_struct *>(getPluginTaskData(event->TaskIndex));
      const P188_CONFIG_BITS_t P188_configBits(P188_CONFIG_BITS);

      if (nullptr != P188_data) {
        const float VoltLSB =  P188_data->P188_config.ADC_Vref / 4096.0f;

# if P188_DEBUG
        String log;
# endif // P188_DEBUG

        for (taskVarIndex_t chNum = 0; chNum < P188_NR_OUTPUT_VALUES; ++chNum) {

          if (PCONFIG(P188_OUTPUT_MAPPING_OFFSET + chNum) <= P188_OUTPUT_MAPPING_DIFFERENCE_OFFSET) // single-ended measurement
          {
            float value{};
            success = P188_data->read_raw(event, value, PCONFIG(P188_OUTPUT_MAPPING_OFFSET + chNum));

            if (success)
            {

# if P188_DEBUG
              log = strformat(F("P188 : Output: %d / Mapping %d / Raw Value: %.2f / VoltsLSB: %.5f "),
                              chNum,
                              PCONFIG(P188_OUTPUT_MAPPING_OFFSET + chNum),
                              value,
                              VoltLSB);
# endif // P188_DEBUG

              if (bitRead(P188_configBits.raw_val, chNum))
              { // return raw value
                UserVar.setFloat(event->TaskIndex, chNum, value);
              }
              else
              {
                if (bitRead(P188_configBits.en_cal, chNum)) // do calibration
                {
                  const float adc1 = P188_data->P188_config.CalIn[chNum][0];
                  const float adc2 = P188_data->P188_config.CalIn[chNum][1];
                  const float out1 = P188_data->P188_config.CalOut[chNum][0];
                  const float out2 = P188_data->P188_config.CalOut[chNum][1];

                  if (!essentiallyEqual(adc1, adc2))
                  {
                    const float normalized = (value - adc1) / (adc2 - adc1);
                    UserVar.setFloat(event->TaskIndex, chNum, normalized * (out2 - out1) + out1);

# if P188_DEBUG
                    log += F(" Callibrated Value: ");
                    log += formatUserVarNoCheck(event, chNum);
# endif // P188_DEBUG

                  }
                }
                else // return voltage value
                {
                  UserVar.setFloat(event->TaskIndex, chNum, value * VoltLSB);

# if P188_DEBUG
                  log += F(" Output Value: ");
                  log += formatUserVarNoCheck(event, chNum);
# endif // P188_DEBUG

                }
              }
            }
          }
          else // difference measurement
          {
            float value{};
            float ref_value{};

            success =            P188_data->read_raw(event, value,     ((PCONFIG(P188_OUTPUT_MAPPING_OFFSET + chNum) - 8) % 4) * 2);       // ch 0, 2, 4, 6
            success = success && P188_data->read_raw(event, ref_value, (((PCONFIG(P188_OUTPUT_MAPPING_OFFSET + chNum) - 8) % 4) * 2) + 1); // ch 1, 3, 5, 7

            if (success)
            {

# if P188_DEBUG
              log = strformat(F("P188 : Output: %d / Mapping %d / Difference - Raw Value: %.2f / Reference Raw Value %.2f "), chNum,
                              PCONFIG(P188_OUTPUT_MAPPING_OFFSET + chNum), value, ref_value);
# endif // P188_DEBUG

              if (PCONFIG(P188_OUTPUT_MAPPING_OFFSET + chNum) <= P188_OUTPUT_MAPPING_RESISTOR_MODES_OFFSET) // difference measurement
              {
                value = abs(ref_value - value) * VoltLSB;
              }

# if P188_FEATURE_RESISTOR_MEASUREMENT
              else // resistor measurement
              {
                if (abs(ref_value - value) > 0)
                {
                  const float R_total = value * P188_data->P188_config.Rref[chNum] / abs(ref_value - value);

                  if (P188_data->P188_config.Rpar[chNum])
                  {
                    float Rpar_chNum = (float)P188_data->P188_config.Rpar[chNum];

                    if ((Rpar_chNum - R_total) > 1) {
                      value = Rpar_chNum * R_total / (Rpar_chNum - R_total);
                    }
                    else {
                      value = P188_data->P188_config.R_Clip; // clip
                    }
                  }
                  else {
                    value = R_total;
                  }

                  if (value > P188_data->P188_config.R_Clip) {
                    value = P188_data->P188_config.R_Clip; // clip
                  }
                }
                else {
                  value = P188_data->P188_config.R_Clip; // clip
                }
              }
# endif // P188_FEATURE_RESISTOR_MEASUREMENT

              if (bitRead(P188_configBits.en_cal, chNum)) // do calibration
              {
                const float adc1 = P188_data->P188_config.CalIn[chNum][0];
                const float adc2 = P188_data->P188_config.CalIn[chNum][1];
                const float out1 = P188_data->P188_config.CalOut[chNum][0];
                const float out2 = P188_data->P188_config.CalOut[chNum][1];

                if (!essentiallyEqual(adc1, adc2))
                {
                  const float normalized = (value - adc1) / (adc2 - adc1);
                  UserVar.setFloat(event->TaskIndex, chNum, normalized * (out2 - out1) + out1);

# if P188_DEBUG
                  log += F(" Difference Callibrated Value: ");
                  log += formatUserVarNoCheck(event, chNum);
# endif // P188_DEBUG

                }
              }
              else // return voltage value
              {
                UserVar.setFloat(event->TaskIndex, chNum, value);

# if P188_DEBUG
                log += F(" Difference Output Value: ");
                log += formatUserVarNoCheck(event, chNum);
# endif // P188_DEBUG

              }
            }
          }

# if P188_DEBUG
          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            addLogMove(LOG_LEVEL_DEBUG, log);
          }
# endif // P188_DEBUG

        }
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P188_data_struct *P188_data = static_cast<P188_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P188_data) {
        success = P188_data->sample();
      }
      break;
    }
  }
  return success;
}

#endif // USES_P188
