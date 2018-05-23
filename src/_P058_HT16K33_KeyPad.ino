#ifdef USES_P058
//#######################################################################################################
//#################################### Plugin 058: HT16K33 KeyPad #######################################
//#######################################################################################################

// ESPEasy Plugin to scan a 13x3 key pad matrix chip HT16K33
// written by Jochen Krapf (jk@nerd2nerd.org)

// Connecting KeyPad to HT16K33-board:
// Column 1 = C1 (over diode)
// Column 2 = C2 (over diode)
// Column 3 = C3 (over diode)
// Row 1 = A3
// Row 2 = A4
// Row 3 = A5
// ...
// Row 13 = A15

// ScanCode;
// 16*Column + Row
// Pressing the top left key (typically "1") the code is 17 (0x11)
// Pressing the key in column 2 and row 3 (typically "8") the code is 35 (0x23)

// Use diodes (e.g. 1N4148) for column lines:
//   HT16K33]-----|>|-----[key-matrix

// Note: The HT16K33-LED-plugin and the HT16K33-key-plugin can be used at the same time with the same I2C address



#define PLUGIN_058
#define PLUGIN_ID_058         58
#define PLUGIN_NAME_058       "Keypad - HT16K33 [TESTING]"
#define PLUGIN_VALUENAME1_058 "ScanCode"

#include <HT16K33.h>

CHT16K33* Plugin_058_K = NULL;

#ifndef CONFIG
#define CONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][n])
#endif


boolean Plugin_058(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_058;
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
        string = F(PLUGIN_NAME_058);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_058));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte addr = CONFIG(0);

        int optionValues[8] = { 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77 };
        addFormSelectorI2C(F("i2c_addr"), 8, optionValues, addr);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        CONFIG(0) = getFormItemInt(F("i2c_addr"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        byte addr = CONFIG(0);

        if (!Plugin_058_K)
          Plugin_058_K = new CHT16K33;

        Plugin_058_K->Init(addr);

        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (Plugin_058_K)
        {
          static uint8_t keyLast = 0;

          uint8_t key = Plugin_058_K->ReadKeys();

          if (keyLast != key)
          {
            keyLast = key;
            UserVar[event->BaseVarIndex] = (float)key;
            event->sensorType = SENSOR_TYPE_SWITCH;

            String log = F("Mkey : key=0x");
            log += String(key, 16);
            addLog(LOG_LEVEL_INFO, log);

            sendData(event);
          }

        }
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        if (Plugin_058_K)
        {
        }
        success = true;
        break;
      }

  }
  return success;
}

#endif // USES_P058
