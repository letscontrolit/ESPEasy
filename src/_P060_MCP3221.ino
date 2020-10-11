#ifdef USES_P060

// #######################################################################################################
// #################################### Plugin 060: MCP3221 ##############################################
// #######################################################################################################

// Plugin to read 12-bit-values from ADC chip MCP3221. It is used e.g. in MinipH pH interface to sample a pH probe in an aquarium
// written by Jochen Krapf (jk@nerd2nerd.org)

#include "_Plugin_Helper.h"

#include "src/PluginStructs/P060_data_struct.h"

#define PLUGIN_060
#define PLUGIN_ID_060         60
#define PLUGIN_NAME_060       "Analog input - MCP3221 [TESTING]"
#define PLUGIN_VALUENAME1_060 "Analog"


boolean Plugin_060(byte function, struct EventStruct *event, String& string)
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

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      byte addr = PCONFIG(0);

      int optionValues[8] = { 0x4D, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4E, 0x4F };
      addFormSelectorI2C(F("i2c_addr"), 8, optionValues, addr);
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormCheckBox(F("Oversampling"), F("p060_oversampling"), PCONFIG(1));

      addFormSubHeader(F("Two Point Calibration"));

      addFormCheckBox(F("Calibration Enabled"), F("p060_cal"), PCONFIG(3));

      addFormNumericBox(F("Point 1"), F("p060_adc1"), PCONFIG_LONG(0), 0, 4095);
      html_add_estimate_symbol();
      addTextBox(F("p060_out1"), String(PCONFIG_FLOAT(0), 3), 10);

      addFormNumericBox(F("Point 2"), F("p060_adc2"), PCONFIG_LONG(1), 0, 4095);
      html_add_estimate_symbol();
      addTextBox(F("p060_out2"), String(PCONFIG_FLOAT(1), 3), 10);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("i2c_addr"));

      PCONFIG(1) = isFormItemChecked(F("p060_oversampling"));

      PCONFIG(3) = isFormItemChecked(F("p060_cal"));

      PCONFIG_LONG(0)  = getFormItemInt(F("p060_adc1"));
      PCONFIG_FLOAT(0) = getFormItemFloat(F("p060_out1"));

      PCONFIG_LONG(1)  = getFormItemInt(F("p060_adc2"));
      PCONFIG_FLOAT(1) = getFormItemFloat(F("p060_out2"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      byte address = PCONFIG(0);

      initPluginTaskData(event->TaskIndex, new (std::nothrow) P060_data_struct(address));
      P060_data_struct *P060_data =
        static_cast<P060_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P060_data) {
        success = true;
      }
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
        log += String(UserVar[event->BaseVarIndex], 3);

        if (PCONFIG(3)) // Calibration?
        {
          int   adc1 = PCONFIG_LONG(0);
          int   adc2 = PCONFIG_LONG(1);
          float out1 = PCONFIG_FLOAT(0);
          float out2 = PCONFIG_FLOAT(1);

          if (adc1 != adc2)
          {
            float normalized = (float)(UserVar[event->BaseVarIndex] - adc1) / (float)(adc2 - adc1);
            UserVar[event->BaseVarIndex] = normalized * (out2 - out1) + out1;

            log += F(" = ");
            log += String(UserVar[event->BaseVarIndex], 3);
          }
        }

        addLog(LOG_LEVEL_INFO, log);
        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P060
