#ifdef USES_P058

// #######################################################################################################
// #################################### Plugin 058: HT16K33 KeyPad #######################################
// #######################################################################################################

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

#include "_Plugin_Helper.h"

#include "src/PluginStructs/P058_data_struct.h"


boolean Plugin_058(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_058;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SWITCH;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].TimerOptional      = true;
      Device[deviceCount].GlobalSyncOption   = true;
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

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      byte addr = PCONFIG(0);

      int optionValues[8] = { 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77 };
      addFormSelectorI2C(F("i2c_addr"), 8, optionValues, addr);
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
      byte address = PCONFIG(0);

      initPluginTaskData(event->TaskIndex, new (std::nothrow) P058_data_struct(address));
      P058_data_struct *P058_data =
        static_cast<P058_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P058_data) {
        success = true;
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P058_data_struct *P058_data =
        static_cast<P058_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P058_data) {
        uint8_t key;

        if (P058_data->readKey(key))
        {
          UserVar[event->BaseVarIndex] = (float)key;
          event->sensorType            = Sensor_VType::SENSOR_TYPE_SWITCH;

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("Mkey : key=0x");
            log += String(key, 16);
            addLog(LOG_LEVEL_INFO, log);
          }

          sendData(event);
        }
      }
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      success = true;
      break;
    }
  }
  return success;
}

#endif // USES_P058
