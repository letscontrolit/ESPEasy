#include "_Plugin_Helper.h"
#ifdef USES_P069

// #######################################################################################################
// ########################### Plugin 69: LM75A Temperature Sensor (I2C) #################################
// #######################################################################################################
// ###################### Library source code for Arduino by QuentinCG, 2016 #############################
// #######################################################################################################
// ##################### Plugin for ESP Easy by B.E.I.C. ELECTRONICS, 2017 ###############################
// ############################## http://www.beicelectronics.com #########################################
// #######################################################################################################
// ########################## Adapted to ESPEasy 2.0 by Jochen Krapf #####################################
// #######################################################################################################


#define PLUGIN_069
#define PLUGIN_ID_069         69
#define PLUGIN_NAME_069       "Environment - LM75A"
#define PLUGIN_VALUENAME1_069 "Temperature"


#include "src/PluginStructs/P069_data_struct.h"


boolean Plugin_069(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_069;
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
      string = F(PLUGIN_NAME_069);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_069));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      int optionValues[8] = { 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F };
      addFormSelectorI2C(F("i2c_addr"), 8, optionValues, PCONFIG(0));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("i2c_addr"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      uint8_t address = PCONFIG(0);

      initPluginTaskData(event->TaskIndex, new (std::nothrow) P069_data_struct(address));
      P069_data_struct *P069_data =
        static_cast<P069_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P069_data) {
        success = true;
      }
      break;
    }

    case PLUGIN_READ:
    {
      P069_data_struct *P069_data =
        static_cast<P069_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P069_data) {
        return success;
      }

      P069_data->setAddress((uint8_t)PCONFIG(0));

      const float tempC = P069_data->getTemperatureInDegrees();
      UserVar[event->BaseVarIndex] = tempC;
      success                      = !isnan(tempC);

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        if (!success) {
          String log = F("LM75A: No reading!");
          addLog(LOG_LEVEL_INFO, log);
        }
        else
        {
          String log = F("LM75A: Temperature: ");
          log += tempC;
          addLog(LOG_LEVEL_INFO, log);
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P069
