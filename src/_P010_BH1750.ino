#include "_Plugin_Helper.h"

#ifdef USES_P010

// #######################################################################################################
// #################################### Plugin-010: LuxRead   ############################################
// #######################################################################################################


# include "AS_BH1750.h"

# define PLUGIN_010
# define PLUGIN_ID_010         10
# define PLUGIN_NAME_010       "Light/Lux - BH1750"
# define PLUGIN_VALUENAME1_010 "Lux"


boolean Plugin_010(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_010;
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
      string = F(PLUGIN_NAME_010);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_010));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      byte choice = PCONFIG(0);

      /*
         String options[2];
         options[0] = F("0x23 - default settings (ADDR Low)");
         options[1] = F("0x5c - alternate settings (ADDR High)");
       */
      int optionValues[2];
      optionValues[0] = BH1750_DEFAULT_I2CADDR;
      optionValues[1] = BH1750_SECOND_I2CADDR;
      addFormSelectorI2C(F("i2c_addr"), 2, optionValues, choice);
      addFormNote(F("ADDR Low=0x23, High=0x5c"));
      break;
    }


    case PLUGIN_WEBFORM_LOAD:
    {
      byte   choiceMode = PCONFIG(1);
      String optionsMode[4];
      optionsMode[0] = F("RESOLUTION_LOW");
      optionsMode[1] = F("RESOLUTION_NORMAL");
      optionsMode[2] = F("RESOLUTION_HIGH");
      optionsMode[3] = F("RESOLUTION_AUTO_HIGH");
      int optionValuesMode[4];
      optionValuesMode[0] = RESOLUTION_LOW;
      optionValuesMode[1] = RESOLUTION_NORMAL;
      optionValuesMode[2] = RESOLUTION_HIGH;
      optionValuesMode[3] = RESOLUTION_AUTO_HIGH;
      addFormSelector(F("Measurement mode"), F("p010_mode"), 4, optionsMode, optionValuesMode, choiceMode);

      addFormCheckBox(F("Send sensor to sleep"), F("p010_sleep"), PCONFIG(2));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("i2c_addr"));
      PCONFIG(1) = getFormItemInt(F("p010_mode"));
      PCONFIG(2) = isFormItemChecked(F("p010_sleep"));
      success    = true;
      break;
    }

    case PLUGIN_READ:
    {
      uint8_t address = PCONFIG(0);


      AS_BH1750 sensor = AS_BH1750(address);

      // replaced the 8 lines below to optimize code
      sensors_resolution_t mode = static_cast<sensors_resolution_t>(PCONFIG(1));

      // if (PCONFIG(1)==RESOLUTION_LOW)
      //        mode = RESOLUTION_LOW;
      // if (PCONFIG(1)==RESOLUTION_NORMAL)
      //        mode = RESOLUTION_NORMAL;
      // if (PCONFIG(1)==RESOLUTION_HIGH)
      //        mode = RESOLUTION_HIGH;
      // if (PCONFIG(1)==RESOLUTION_AUTO_HIGH)
      //        mode = RESOLUTION_AUTO_HIGH;

      bool autoPowerDown = PCONFIG(2);
      sensor.begin(mode, autoPowerDown);

      float lux = sensor.readLightLevel();

      if (lux != -1) {
        UserVar[event->BaseVarIndex] = lux;
        String log = F("BH1750 Address: 0x");
        log += String(address, HEX);
        log += F(" Mode: 0x");
        log += String(mode);
        log += F(" : Light intensity: ");
        log += formatUserVarNoCheck(event->TaskIndex, 0);
        addLog(LOG_LEVEL_INFO, log);
        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P010
