#include "_Plugin_Helper.h"
#ifdef USES_P060

// #######################################################################################################
// #################################### Plugin 060: MCP3221 ##############################################
// #######################################################################################################

// Plugin to read 12-bit-values from ADC chip MCP3221. It is used e.g. in MinipH pH interface to sample a pH probe in an aquarium
// written by Jochen Krapf (jk@nerd2nerd.org)


# include "src/PluginStructs/P060_data_struct.h"

# define PLUGIN_060
# define PLUGIN_ID_060         60
# define PLUGIN_NAME_060       "Analog input - MCP3221"
# define PLUGIN_VALUENAME1_060 "Analog"


boolean Plugin_060(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_060;
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
      string = F(PLUGIN_NAME_060);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_060));
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      const uint8_t i2cAddressValues[] = { 0x4D, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4E, 0x4F };

      if (function == PLUGIN_WEBFORM_SHOW_I2C_PARAMS) {
        addFormSelectorI2C(F("i2c_addr"), 8, i2cAddressValues, PCONFIG(0));
      } else {
        success = intArrayContains(8, i2cAddressValues, event->Par1);
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
      addFormCheckBox(F("Oversampling"), F("oversampling"), PCONFIG(1));

      addFormSubHeader(F("Two Point Calibration"));

      addFormCheckBox(F("Calibration Enabled"), F("cal"), PCONFIG(3));

      addFormNumericBox(F("Point 1"), F("adc1"), PCONFIG_LONG(0), 0, 4095);
      html_add_estimate_symbol();
      addTextBox(F("out1"), toString(PCONFIG_FLOAT(0), 3), 10);

      addFormNumericBox(F("Point 2"), F("adc2"), PCONFIG_LONG(1), 0, 4095);
      html_add_estimate_symbol();
      addTextBox(F("out2"), toString(PCONFIG_FLOAT(1), 3), 10);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("i2c_addr"));

      PCONFIG(1) = isFormItemChecked(F("oversampling"));

      PCONFIG(3) = isFormItemChecked(F("cal"));

      PCONFIG_LONG(0)  = getFormItemInt(F("adc1"));
      PCONFIG_FLOAT(0) = getFormItemFloat(F("out1"));

      PCONFIG_LONG(1)  = getFormItemInt(F("adc2"));
      PCONFIG_FLOAT(1) = getFormItemFloat(F("out2"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P060_data_struct(PCONFIG(0)));
      P060_data_struct *P060_data =
        static_cast<P060_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P060_data);
      break;
    }


    case PLUGIN_TEN_PER_SECOND:
    {
      if (PCONFIG(1)) // Oversampling?
      {
        P060_data_struct *P060_data =
          static_cast<P060_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P060_data) {
          P060_data->overSampleRead();
          success = true;
        }
      }
      break;
    }

    case PLUGIN_READ:
    {
      P060_data_struct *P060_data =
        static_cast<P060_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P060_data) {
        UserVar[event->BaseVarIndex] = P060_data->getValue();

        String log = F("ADMCP: Analog value: ");
        log += formatUserVarNoCheck(event->TaskIndex, 0);

        if (PCONFIG(3)) // Calibration?
        {
          int   adc1 = PCONFIG_LONG(0);
          int   adc2 = PCONFIG_LONG(1);
          float out1 = PCONFIG_FLOAT(0);
          float out2 = PCONFIG_FLOAT(1);

          if (adc1 != adc2)
          {
            const float normalized = (UserVar[event->BaseVarIndex] - adc1) / static_cast<float>(adc2 - adc1);
            UserVar[event->BaseVarIndex] = normalized * (out2 - out1) + out1;

            log += F(" = ");
            log += formatUserVarNoCheck(event->TaskIndex, 0);
          }
        }

        addLogMove(LOG_LEVEL_INFO, log);
        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P060
