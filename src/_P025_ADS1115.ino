#include "_Plugin_Helper.h"
#ifdef USES_P025

// #######################################################################################################
// #################################### Plugin 025: ADS1115 I2C 0x48)  ###############################################
// #######################################################################################################


#include "src/PluginStructs/P025_data_struct.h"

#define PLUGIN_025
#define PLUGIN_ID_025 25
#define PLUGIN_NAME_025 "Analog input - ADS1115"
#define PLUGIN_VALUENAME1_025 "Analog"


boolean Plugin_025(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  // static byte portValue = 0;
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

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
        #define ADS1115_I2C_OPTION 4
      byte addr                            = PCONFIG(0);
      int optionValues[ADS1115_I2C_OPTION] = { 0x48, 0x49, 0x4A, 0x4B };
      addFormSelectorI2C(F("i2c_addr"), ADS1115_I2C_OPTION, optionValues, addr);
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      byte port = CONFIG_PORT;

      if (port > 0) // map old port logic to new gain and mode settings
      {
        PCONFIG(1)  = PCONFIG(0) / 2;
        PCONFIG(0)  = 0x48 + ((port - 1) / 4);
        PCONFIG(2)  = ((port - 1) & 3) | 4;
        CONFIG_PORT = 0;
      }

      addFormSubHeader(F("Input"));

      {
          #define ADS1115_PGA_OPTION 6
        byte pga                              = PCONFIG(1);
        const __FlashStringHelper * pgaOptions[ADS1115_PGA_OPTION] = {
          F("2/3x gain (FS=6.144V)"),
          F("1x gain (FS=4.096V)"),
          F("2x gain (FS=2.048V)"),
          F("4x gain (FS=1.024V)"),
          F("8x gain (FS=0.512V)"),
          F("16x gain (FS=0.256V)")
        };
        addFormSelector(F("Gain"), F("p025_gain"), ADS1115_PGA_OPTION, pgaOptions, NULL, pga);
      }

      {
          #define ADS1115_MUX_OPTION 8
        byte mux                              = PCONFIG(2);
        const __FlashStringHelper * muxOptions[ADS1115_MUX_OPTION] = {
          F("AIN0 - AIN1 (Differential)"),
          F("AIN0 - AIN3 (Differential)"),
          F("AIN1 - AIN3 (Differential)"),
          F("AIN2 - AIN3 (Differential)"),
          F("AIN0 - GND (Single-Ended)"),
          F("AIN1 - GND (Single-Ended)"),
          F("AIN2 - GND (Single-Ended)"),
          F("AIN3 - GND (Single-Ended)"),
        };
        addFormSelector(F("Input Multiplexer"), F("p025_mode"), ADS1115_MUX_OPTION, muxOptions, NULL, mux);
      }

      addFormSubHeader(F("Two Point Calibration"));

      addFormCheckBox(F("Calibration Enabled"), F("p025_cal"), PCONFIG(3));

      addFormNumericBox(F("Point 1"), F("p025_adc1"), PCONFIG_LONG(0), -32768, 32767);
      html_add_estimate_symbol();
      addTextBox(F("p025_out1"), String(PCONFIG_FLOAT(0), 3), 10);

      addFormNumericBox(F("Point 2"), F("p025_adc2"), PCONFIG_LONG(1), -32768, 32767);
      html_add_estimate_symbol();
      addTextBox(F("p025_out2"), String(PCONFIG_FLOAT(1), 3), 10);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("i2c_addr"));

      PCONFIG(1) = getFormItemInt(F("p025_gain"));

      PCONFIG(2) = getFormItemInt(F("p025_mode"));

      PCONFIG(3) = isFormItemChecked(F("p025_cal"));

      PCONFIG_LONG(0)  = getFormItemInt(F("p025_adc1"));
      PCONFIG_FLOAT(0) = getFormItemFloat(F("p025_out1"));

      PCONFIG_LONG(1)  = getFormItemInt(F("p025_adc2"));
      PCONFIG_FLOAT(1) = getFormItemFloat(F("p025_out2"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      // int value = 0;
      // byte unit = (CONFIG_PORT - 1) / 4;
      // byte port = CONFIG_PORT - (unit * 4);
      // uint8_t address = 0x48 + unit;
      const uint8_t address = PCONFIG(0);
      const uint8_t pga     = PCONFIG(1);
      const uint8_t mux     = PCONFIG(2);

      initPluginTaskData(event->TaskIndex, new (std::nothrow) P025_data_struct(address, pga, mux));
      P025_data_struct *P025_data =
        static_cast<P025_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P025_data) {
        success = true;
      }
      break;
    }

    case PLUGIN_READ:
    {
      P025_data_struct *P025_data =
        static_cast<P025_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P025_data) {
        const int16_t value = P025_data->read();
        UserVar[event->BaseVarIndex] = (float)value;

        String log;
        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          log  = F("ADS1115 : Analog value: ");
          log += value;
        }

        if (PCONFIG(3)) // Calibration?
        {
          int adc1   = PCONFIG_LONG(0);
          int adc2   = PCONFIG_LONG(1);
          float out1 = PCONFIG_FLOAT(0);
          float out2 = PCONFIG_FLOAT(1);

          if (adc1 != adc2)
          {
            float normalized = (float)(value - adc1) / (float)(adc2 - adc1);
            UserVar[event->BaseVarIndex] = normalized * (out2 - out1) + out1;
            if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
              log += ' ';
              log += formatUserVarNoCheck(event->TaskIndex, 0);
            }
          }
        }

        // TEST log += F(" @0x");
        // TEST log += String(config, 16);
        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          addLog(LOG_LEVEL_DEBUG, log);
        }
        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P025
