#include "_Plugin_Helper.h"
#ifdef USES_P024

// #######################################################################################################
// #################################### Plugin 024: MLX90614 IR temperature I2C 0x5A)  ###############################################
// #######################################################################################################


#include "src/PluginStructs/P024_data_struct.h"

// MyMessage *msgTemp024; // Mysensors

#define PLUGIN_024
#define PLUGIN_ID_024 24
#define PLUGIN_NAME_024 "Environment - MLX90614"
#define PLUGIN_VALUENAME1_024 "Temperature"

boolean Plugin_024(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  // static byte portValue = 0;
  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_024;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 16;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_024);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_024));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
        #define MLX90614_OPTION 2

      byte choice = PCONFIG(0);
      String options[MLX90614_OPTION];
      int optionValues[MLX90614_OPTION];
      optionValues[0] = (0x07);
      options[0]      = F("IR object temperature");
      optionValues[1] = (0x06);
      options[1]      = F("Ambient temperature");
      addFormSelector(F("Option"), F("p024_option"), MLX90614_OPTION, options, optionValues, choice);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0)      = getFormItemInt(F("p024_option"));
      success         = true;
      break;
    }

    case PLUGIN_INIT:
    {
      byte unit       = CONFIG_PORT;
      uint8_t address = 0x5A + unit;

      initPluginTaskData(event->TaskIndex, new (std::nothrow) P024_data_struct(address));
      P024_data_struct *P024_data =
        static_cast<P024_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P024_data) {
        //        if (!msgTemp024) // Mysensors
        //          msgTemp024 = new MyMessage(event->BaseVarIndex, V_TEMP); //Mysensors
        //        present(event->BaseVarIndex, S_TEMP); //Mysensors
        //        serialPrint("Present MLX90614: "); //Mysensors
        //        serialPrintln(event->BaseVarIndex); //Mysensors
        success = true;
      }
      break;
    }

    case PLUGIN_READ:
    {
      P024_data_struct *P024_data =
        static_cast<P024_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P024_data) {
        UserVar[event->BaseVarIndex] = (float)P024_data->readTemperature(PCONFIG(0));
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("MLX90614  : Temperature: ");
          log += formatUserVarNoCheck(event->TaskIndex, 0);
          addLog(LOG_LEVEL_INFO, log);
        }
        //        send(msgObjTemp024->set(UserVar[event->BaseVarIndex], 1)); // Mysensors
        success = true;
      }
      break;
    }
  }
  return success;
}

#endif // USES_P024
