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

boolean Plugin_188(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number             = PLUGIN_ID_188;                    // Plugin ID number.   (PLUGIN_ID_xxx)
      dev.Type               = DEVICE_TYPE_I2C;                  // How the device is connected. e.g. DEVICE_TYPE_SINGLE => connected through 1 datapin
      dev.VType              = Sensor_VType::SENSOR_TYPE_QUAD;   // Type of value the plugin will return. e.g. SENSOR_TYPE_STRING
      dev.Ports              = 0;                                // Port to use when device has multiple I/O pins  (N.B. not used much)
      dev.ValueCount         = 4;                                // The number of output values of a plugin. The value should match the number of keys PLUGIN_VALUENAME1_xxx
      dev.OutputDataType     = Output_Data_type_t::Simple;       // Subset of selectable output data types  (Default = no selection)
      dev.PullUpOption       = false;                            // Allow to set internal pull-up resistors.
      dev.InverseLogicOption = false;                            // Allow to invert the boolean state (e.g. a switch)
      dev.FormulaOption      = true;                             // Allow to enter a formula to convert values during read. (not possible with Custom enabled)
      dev.Custom             = false;
      dev.SendDataOption     = true;                             // Allow to send data to a controller.
      dev.GlobalSyncOption   = false;                            // No longer used. Was used for ESPeasy values sync between nodes
      dev.TimerOption        = true;                             // Allow to set the "Interval" timer for the plugin.
      dev.TimerOptional      = false;                            // When taskdevice timer is not set and not optional, use default "Interval" delay (Settings.Delay)
      dev.DecimalsOnly       = false;                            // Allow to set the number of decimals (otherwise treated a 0 decimals)
      dev.CustomVTypeVar     = true;                             // Enable to allow the user to configure the Sensor_VType per Value that's available for the plugin
      dev.PluginStats        = true;                             // Support for PluginStats to record last N task values, show charts etc.
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

      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < valueCount) {
          ExtraTaskSettings.setTaskDeviceValueName(i, Plugin_188_output_mapping_name(PCONFIG(i + P188_OUTPUT_MAPPING_OFFSET), false));
        } else {
          ExtraTaskSettings.clearTaskDeviceValueName(i);
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
      success = getDiscoveryVType(event, Plugin_QueryVType_Analog, 255, event->Par5);;
      break;
    }
    # endif // if FEATURE_MQTT_DISCOVER

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
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
#ifdef P188_FEATURE_RESISTOR_MEASUREMENT
      tmp_config.R_Clip = 0.0f;
#endif
      tmp_config.i2cAddress = P188_I2C_ADDR;
      for (int i = 0; i < VARS_PER_TASK; i++)
      {
        PCONFIG(i + P188_OUTPUT_MAPPING_OFFSET) = i;
#ifdef P188_FEATURE_RESISTOR_MEASUREMENT
        tmp_config.Rref[i] = 4700;
        tmp_config.Rpar[i] = 100000;
#endif // P188_FEATURE_RESISTOR_MEASUREMENT
      }

      SaveCustomTaskSettings(event->TaskIndex, (uint8_t *)&(tmp_config), sizeof(tmp_config), 0);  // save configuration to flash

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      const P188_CONFIG_BITS_t P188_configBits(P188_CONFIG_BITS);

      P188_config_struct tmp_config;

      LoadCustomTaskSettings(event->TaskIndex, (uint8_t *)&(tmp_config), sizeof(tmp_config));  // load configuration from flash

      const int valueCount = getValueCountFromSensorType(static_cast<Sensor_VType>(P188_OUTPUT_TYPE));

      addFormFloatNumberBox(F("ADC reference voltage"),F("ADC_Vref"),tmp_config.ADC_Vref,2.35f,5.5f,2,0.01f);
#ifdef P188_FEATURE_RESISTOR_MEASUREMENT
      addFormFloatNumberBox(F("Resistor measurment clipping"),F("R_Clip"),tmp_config.R_Clip,0.0f,1000000000,2,0.01f);
#endif

      for (int i = 0; i < valueCount; i++)
      {
        addFormSubHeader(strformat(F("Output %d - %s"), i + 1, FsP(Plugin_188_output_mapping_name(PCONFIG(i + P188_OUTPUT_MAPPING_OFFSET), true))));
#ifdef P188_FEATURE_RESISTOR_MEASUREMENT
        addFormNumericBox(F("Reference Resistor Value"), getPluginCustomArgName(i * 8 + 0), tmp_config.Rref[i], 100, 470000);
        addFormNumericBox(F("Parallel Resistor Value"),  getPluginCustomArgName(i * 8 + 1), tmp_config.Rpar[i],   0, 470000);
#endif // P188_FEATURE_RESISTOR_MEASUREMENT
        addFormCheckBox(F("Output Raw ADC Value"), getPluginCustomArgName(i * 8 + 2), ((P188_configBits.raw_val >> i) & 0x01));

        /* Callibration */

        addTableSeparator(F("Two Point Calibration"), 2, 4);

        addFormCheckBox(F("Enable Calibration"), getPluginCustomArgName(i * 8 + 3), ((P188_configBits.en_cal >> i) & 0x01));

        addFormNumericBox(F("Point 1"), getPluginCustomArgName(i * 8 + 4), PCONFIG_LONG(P188_CAL_INDEX + 2*i), -32768, 32767);
        html_add_estimate_symbol();
        addTextBox(getPluginCustomArgName(i * 8 + 5), toString(PCONFIG_FLOAT(P188_CAL_INDEX + 2*i), 3), 10);

        addFormNumericBox(F("Point 2"), getPluginCustomArgName(i * 8 + 6), PCONFIG_LONG(P188_CAL_INDEX + 2*i+1), -32768, 32767);
        html_add_estimate_symbol();
        addTextBox(getPluginCustomArgName(i * 8 + 7), toString(PCONFIG_FLOAT(P188_CAL_INDEX + 2*i+1), 3), 10);
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

      for (uint8_t i = 0; i < valueCount; ++i) {
        const uint8_t pconfigIndex = i + P188_OUTPUT_MAPPING_OFFSET;
        sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, P188_OUTPUT_OPTION_CNT, options);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P188_config_struct tmp_config;
      P188_CONFIG_BITS_t P188_configBits(P188_CONFIG_BITS);

      P188_configBits.raw_val = 0;
      P188_configBits.en_cal = 0;

      P188_I2C_ADDR = getFormItemInt(F("i2c_addr"));
      tmp_config.i2cAddress = P188_I2C_ADDR;
      tmp_config.ADC_Vref = getFormItemFloat(F("ADC_Vref"));

#ifdef P188_FEATURE_RESISTOR_MEASUREMENT
      tmp_config.R_Clip = getFormItemFloat(F("R_Clip"));
#endif

      for (uint8_t i = 0; i < P188_NR_OUTPUT_VALUES; ++i) {
        const uint8_t pconfigIndex = P188_OUTPUT_MAPPING_OFFSET + i;
        const uint8_t choice       = PCONFIG(pconfigIndex);
        sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, Plugin_188_output_mapping_name(choice, false));
      }

      const int valueCount = getValueCountFromSensorType(static_cast<Sensor_VType>(P188_OUTPUT_TYPE));

      for (int i = 0; i < valueCount; i++)
      {
#ifdef P188_FEATURE_RESISTOR_MEASUREMENT
        tmp_config.Rref[i] = getFormItemInt(getPluginCustomArgName(i * 8 + 0));
        tmp_config.Rpar[i] = getFormItemInt(getPluginCustomArgName(i * 8 + 1));
#endif // P188_FEATURE_RESISTOR_MEASUREMENT

        P188_configBits.raw_val |= ((uint8_t)isFormItemChecked(getPluginCustomArgName(i * 8 + 2))) << i;

        /* Callibration */

        // Enable
        P188_configBits.en_cal  |= ((uint8_t)isFormItemChecked(getPluginCustomArgName(i * 8 + 3))) << i;

        // Save configBits
        P188_CONFIG_BITS           = P188_configBits.pconfigvalue(); 

        // Point 1
        PCONFIG_LONG(P188_CAL_INDEX + 2*i) = getFormItemInt(getPluginCustomArgName(i * 8 + 4));
        PCONFIG_FLOAT(P188_CAL_INDEX + 2*i) = getFormItemFloat(getPluginCustomArgName(i * 8 + 5));

        // Point 2
        PCONFIG_LONG(P188_CAL_INDEX + 2*i+1) = getFormItemInt(getPluginCustomArgName(i * 8 + 6));
        PCONFIG_FLOAT(P188_CAL_INDEX + 2*i+1) = getFormItemFloat(getPluginCustomArgName(i * 8 + 7));
      }

      success = SaveCustomTaskSettings(event->TaskIndex, (uint8_t *)&(tmp_config), sizeof(tmp_config), 0);  // save configuration to flash
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      format_I2C_port_description(event->TaskIndex);

      for (uint8_t i = 0; i < P188_NR_OUTPUT_VALUES; ++i) {
        const uint8_t choice = PCONFIG(P188_OUTPUT_MAPPING_OFFSET + i);

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
      const P188_data_struct *P188_data = static_cast<P188_data_struct *>(getPluginTaskData(event->TaskIndex));
      const P188_CONFIG_BITS_t P188_configBits(P188_CONFIG_BITS);

      if (nullptr != P188_data) {
        const float VoltLSB =  P188_data->P188_config.ADC_Vref / 4096.0f;
#ifndef BUILD_NO_DEBUG
        String log;
#endif // BUILD_NO_DEBUG

        for (taskVarIndex_t i = 0; i < P188_NR_OUTPUT_VALUES; ++i) {

          if (PCONFIG(P188_OUTPUT_MAPPING_OFFSET + i) < 8) { // single-ended measurement
            float value{};
            success = P188_data->read_raw(event, value, PCONFIG(P188_OUTPUT_MAPPING_OFFSET + i));

            if (success) {
#ifndef BUILD_NO_DEBUG
              log = strformat(F("P188 : Output: %d / Mapping %d / Raw Value: %.2f "), i, PCONFIG(P188_OUTPUT_MAPPING_OFFSET + i), value);
#endif // BUILD_NO_DEBUG
              if ((P188_configBits.raw_val >> i) & 0x01) 
              { // return raw value
                UserVar.setFloat(event->TaskIndex, i, value);
              }
              else 
              {
                if ((P188_configBits.en_cal >> i) & 0x01) 
                { // do calibration
                  const int   adc1 = PCONFIG_LONG(P188_CAL_INDEX + 2*i);
                  const int   adc2 = PCONFIG_LONG(P188_CAL_INDEX + 2*i+1);
                  const float out1 = PCONFIG_FLOAT(P188_CAL_INDEX + 2*i);
                  const float out2 = PCONFIG_FLOAT(P188_CAL_INDEX + 2*i+1);

                  if (adc1 != adc2) 
                  {
                    const float normalized = static_cast<float>(value - adc1) / static_cast<float>(adc2 - adc1);
                    UserVar.setFloat(event->TaskIndex, i, normalized * (out2 - out1) + out1);
#ifndef BUILD_NO_DEBUG
                    log += " Callibrated Value: ";
                    log += formatUserVarNoCheck(event, i);
#endif // BUILD_NO_DEBUG
                  }
                }
                else 
                { // return voltage value
                  UserVar.setFloat(event->TaskIndex, i, value * VoltLSB);
#ifndef BUILD_NO_DEBUG
                  log += " Output Value: ";
                  log += formatUserVarNoCheck(event, i);
#endif // BUILD_NO_DEBUG
                }
              }
            }
          }
          else // differential measurement
          {
            float value{};
            float ref_value{};

            success =            P188_data->read_raw(event, value,     (( PCONFIG(P188_OUTPUT_MAPPING_OFFSET + i)-8) % 4) * 2);      // ch 0, 2, 4, 6
            success = success && P188_data->read_raw(event, ref_value, (((PCONFIG(P188_OUTPUT_MAPPING_OFFSET + i)-8) % 4) * 2) + 1); // ch 1, 3, 5, 7

            if (success){
#ifndef BUILD_NO_DEBUG
              log = strformat(F("P188 : Output: %d / Mapping %d / Differential - Raw Value: %.2f / Reference Raw Value %.2f "), i, PCONFIG(P188_OUTPUT_MAPPING_OFFSET + i), value, ref_value);
#endif // BUILD_NO_DEBUG
              if (PCONFIG(P188_OUTPUT_MAPPING_OFFSET + i) < 12) { // plain differential
                value = abs(ref_value - value) * VoltLSB;
              }
#ifdef P188_FEATURE_RESISTOR_MEASUREMENT
              else
              { // resistor measurement
                if (abs(ref_value - value) > 0) 
                {
                  const float R_total = value * P188_data->P188_config.Rref[i] / abs(ref_value - value);

                  if (P188_data->P188_config.Rpar[i])
                  {
                    if (((float)P188_data->P188_config.Rpar[i] - R_total) > 1)
                      value = (float)P188_data->P188_config.Rpar[i] * R_total / ((float)P188_data->P188_config.Rpar[i] - R_total);
                    else
                      value = P188_data->P188_config.R_Clip; // clip
                  }
                  else 
                    value = R_total;

                  if (value > P188_data->P188_config.R_Clip)
                    value = P188_data->P188_config.R_Clip; // clip
                }
                else
                    value = P188_data->P188_config.R_Clip; // clip

              }
#endif // P188_FEATURE_RESISTOR_MEASUREMENT

              if ((P188_configBits.en_cal >> i) & 0x01) 
              { // do calibration
                const int   adc1 = PCONFIG_LONG(P188_CAL_INDEX + 2*i);
                const int   adc2 = PCONFIG_LONG(P188_CAL_INDEX + 2*i+1);
                const float out1 = PCONFIG_FLOAT(P188_CAL_INDEX + 2*i);
                const float out2 = PCONFIG_FLOAT(P188_CAL_INDEX + 2*i+1);

                if (adc1 != adc2) 
                {
                  const float normalized = static_cast<float>(value - adc1) / static_cast<float>(adc2 - adc1);
                  UserVar.setFloat(event->TaskIndex, i, normalized * (out2 - out1) + out1);
#ifndef BUILD_NO_DEBUG
                  log += " Difference Callibrated Value: ";
                  log += formatUserVarNoCheck(event, i);
#endif // BUILD_NO_DEBUG
                }
              }
              else
              { // return voltage value
                UserVar.setFloat(event->TaskIndex, i, value);
#ifndef BUILD_NO_DEBUG
                log += " Difference Output Value: ";
                log += formatUserVarNoCheck(event, i); 
#endif // BUILD_NO_DEBUG
              }
            }
          }
#ifndef BUILD_NO_DEBUG
          if (loglevelActiveFor(LOG_LEVEL_DEBUG))
            addLogMove(LOG_LEVEL_DEBUG, log);
#endif
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
