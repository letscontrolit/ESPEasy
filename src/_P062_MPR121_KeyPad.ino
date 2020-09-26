#ifdef USES_P062

// #######################################################################################################
// #################################### Plugin 062: MPR121 KeyPad ########################################
// #######################################################################################################

// ESPEasy Plugin to scan a 12 key touch pad chip MPR121
// written by Jochen Krapf (jk@nerd2nerd.org)

// ScanCode;
// Value 1...12 for the key number
// No key - the code 0
// If more than one key is pressed, the scan code is the code with the lowest value

// If ScanCode is unchecked the value is the KeyMap 1.Key=1, 2.Key=2, 3.Key=4, 4.Key=8 ... 1.Key=2048
// If more than one key is pressed, the value is sum of all KeyMap-values


#define PLUGIN_062
#define PLUGIN_ID_062         62
#define PLUGIN_NAME_062       "Keypad - MPR121 Touch [TESTING]"
#define PLUGIN_VALUENAME1_062 "ScanCode"


#include "_Plugin_Helper.h"

#include "src/PluginStructs/P062_data_struct.h"

boolean Plugin_062(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_062;
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
      string = F(PLUGIN_NAME_062);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_062));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      byte addr = PCONFIG(0);

      int optionValues[4] = { 0x5A, 0x5B, 0x5C, 0x5D };
      addFormSelectorI2C(F("i2c_addr"), 4, optionValues, addr);
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormCheckBox(F("ScanCode"), F("scancode"), PCONFIG(1));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("i2c_addr"));

      PCONFIG(1) = isFormItemChecked(F("scancode"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      byte address = PCONFIG(0);

      initPluginTaskData(event->TaskIndex, new (std::nothrow) P062_data_struct(address, PCONFIG(1)));
      P062_data_struct *P062_data =
        static_cast<P062_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P062_data) {
        success = true;
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P062_data_struct *P062_data =
        static_cast<P062_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P062_data) {
        uint16_t key;

        if (P062_data->readKey(key))
        {
          UserVar[event->BaseVarIndex] = (float)key;
          event->sensorType            = Sensor_VType::SENSOR_TYPE_SWITCH;

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("Tkey : ");

            if (PCONFIG(1)) {
              log = F("ScanCode=0x");
            }
            else {
              log = F("KeyMap=0x");
            }
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

#endif // USES_P062
