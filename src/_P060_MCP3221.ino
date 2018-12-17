#ifdef USES_P060
//#######################################################################################################
//#################################### Plugin 060: MCP3221 ##############################################
//#######################################################################################################

// Plugin to read 12-bit-values from ADC chip MCP3221. It is used e.g. in MinipH pH interface to sample a pH probe in an aquarium
// written by Jochen Krapf (jk@nerd2nerd.org)


#define PLUGIN_060
#define PLUGIN_ID_060         60
#define PLUGIN_NAME_060       "Analog input - MCP3221 [TESTING]"
#define PLUGIN_VALUENAME1_060 "Analog"

uint32_t Plugin_060_OversamplingValue = 0;
uint16_t Plugin_060_OversamplingCount = 0;

#ifndef CONFIG
#define CONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][n])
#endif


uint16_t readMCP3221(byte addr)
{
  uint16_t value;
  Wire.requestFrom(addr, (uint8_t)2);
  if (Wire.available() == 2)
  {
    value = (Wire.read() << 8) | Wire.read();
  }
  else
    value = 9999;

  return value;
}

boolean Plugin_060(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_060;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
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

    case PLUGIN_WEBFORM_LOAD:
      {
        byte addr = CONFIG(0);

        int optionValues[8] = { 0x4D, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4E, 0x4F };
        addFormSelectorI2C(F("i2c_addr"), 8, optionValues, addr);

        addFormCheckBox(F("Oversampling"), F("p060_oversampling"), CONFIG(1));

        addFormSubHeader(F("Two Point Calibration"));

        addFormCheckBox(F("Calibration Enabled"), F("p060_cal"), CONFIG(3));

        addFormNumericBox(F("Point 1"), F("p060_adc1"), Settings.TaskDevicePluginConfigLong[event->TaskIndex][0], 0, 4095);
        html_add_estimate_symbol();
        addTextBox(F("p060_out1"), String(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0], 3), 10);

        addFormNumericBox(F("Point 2"), F("p060_adc2"), Settings.TaskDevicePluginConfigLong[event->TaskIndex][1], 0, 4095);
        html_add_estimate_symbol();
        addTextBox(F("p060_out2"), String(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1], 3), 10);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        CONFIG(0) = getFormItemInt(F("i2c_addr"));

        CONFIG(1) = isFormItemChecked(F("p060_oversampling"));

        CONFIG(3) = isFormItemChecked(F("p060_cal"));

        Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] = getFormItemInt(F("p060_adc1"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0] = getFormItemFloat(F("p060_out1"));

        Settings.TaskDevicePluginConfigLong[event->TaskIndex][1] = getFormItemInt(F("p060_adc2"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] = getFormItemFloat(F("p060_out2"));

        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (CONFIG(1))   //Oversampling?
        {
          Plugin_060_OversamplingValue += readMCP3221(CONFIG(0));
          Plugin_060_OversamplingCount ++;
        }
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        String log = F("ADMCP: Analog value: ");

        if (Plugin_060_OversamplingCount > 0)
        {
          UserVar[event->BaseVarIndex] = (float)Plugin_060_OversamplingValue / Plugin_060_OversamplingCount;
          Plugin_060_OversamplingValue = 0;
          Plugin_060_OversamplingCount = 0;

          log += String(UserVar[event->BaseVarIndex], 3);
        }
        else
        {
          int16_t value = readMCP3221(CONFIG(0));
          UserVar[event->BaseVarIndex] = (float)value;

          log += value;
        }

        if (Settings.TaskDevicePluginConfig[event->TaskIndex][3])   //Calibration?
        {
          int adc1 = Settings.TaskDevicePluginConfigLong[event->TaskIndex][0];
          int adc2 = Settings.TaskDevicePluginConfigLong[event->TaskIndex][1];
          float out1 = Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0];
          float out2 = Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1];
          if (adc1 != adc2)
          {
            float normalized = (float)(UserVar[event->BaseVarIndex] - adc1) / (float)(adc2 - adc1);
            UserVar[event->BaseVarIndex] = normalized * (out2 - out1) + out1;

            log += F(" = ");
            log += String(UserVar[event->BaseVarIndex], 3);
          }
        }

        addLog(LOG_LEVEL_INFO,log);
        success = true;
        break;
      }
  }
  return success;
}

#endif // USES_P060
