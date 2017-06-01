//######################################## HT16K33 ####################################################
//#######################################################################################################

// ESPEasy Plugin to scan a 13x3 key pad matrix chip HT16K33
// written by Jochen Krapf (jk@nerd2nerd.org)

//#include <*.h>   //no lib needed

#ifdef PLUGIN_BUILD_TESTING

#define PLUGIN_147
#define PLUGIN_ID_147         147
#define PLUGIN_NAME_147       "KeyPad - HT16K33 [TESTING]"

extern class CHT16K33;

CHT16K33* Plugin_147_K = NULL;


boolean Plugin_147(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_147;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_147);
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte addr = Settings.TaskDevicePluginConfig[event->TaskIndex][0];

        int optionValues[8] = { 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77 };
        addFormSelectorI2C(string, F("i2c_addr"), 8, optionValues, addr);

        addFormCheckBox(string, F("Scan Keys"), F("usekeys"), Settings.TaskDevicePluginConfig[event->TaskIndex][1]);

        //Settings.TaskDevicePin1[event->TaskIndex] = 2;
        //Settings.TaskDevicePluginConfig[event->TaskIndex][0] = Plugin_147_DMXSize;
        //addFormNote(string, F("Only GPIO-2 (D4) can be used as TX1!"));
        //addFormNumericBox(string, F("Channels"), F("channels"), Plugin_147_DMXSize, 1, 512);
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("i2c_addr"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = isFormItemChecked(F("usekeys"));

        //Plugin_147_DMXSize = getFormItemInt(F("channels"));
        //Limit (Plugin_147_DMXSize, 1, 512);
        //Settings.TaskDevicePluginConfig[event->TaskIndex][0] = Plugin_147_DMXSize;
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        byte addr = Settings.TaskDevicePluginConfig[event->TaskIndex][0];

        if (!Plugin_147_K)
          Plugin_147_K = new CHT16K33;

        Plugin_147_K->Init(addr);

        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (Plugin_147_K && Settings.TaskDevicePluginConfig[event->TaskIndex][1])
        {
          static uint8_t keyLast = 0;

          uint8_t key = Plugin_147_K->ReadKeys();

          if (keyLast != key)
          {
            keyLast = key;
            UserVar[event->BaseVarIndex] = (float)key;
            event->sensorType = SENSOR_TYPE_SWITCH;

            String log = F("M    : key=");
            log += key;
            addLog(LOG_LEVEL_INFO, log);

            sendData(event);
          }

        }
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        if (Plugin_147_K && Settings.TaskDevicePluginConfig[event->TaskIndex][1])
        {
        }
        success = true;
        break;
      }

  }
  return success;
}

#endif
