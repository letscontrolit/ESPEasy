//#######################################################################################################
//#################################### Plugin 059: Rotary Encoder #######################################
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


#ifdef PLUGIN_BUILD_TESTING

#define PLUGIN_059
#define PLUGIN_ID_059         59
#define PLUGIN_NAME_059       "Switch - Rotary Encoder [TESTING]"
#define PLUGIN_VALUENAME1_059 "Counter"

#include <QEIx4.h>

QEIx4* Plugin_059_QE = NULL;

#ifndef CONFIG
#define CONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][n])
#endif


boolean Plugin_059(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_059;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
        Device[deviceCount].PullUpOption = true;
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
        string = F(PLUGIN_NAME_059);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_059));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte addr = CONFIG(0);

        String options[3] = { F("1 pulse per cycle"), F("2 pulses per cycle"), F("4 pulses per cycle") };
        int optionValues[3] = { 1, 2, 4 };
        addFormSelector(string, F("Mode"), F("qei_mode"), 3, options, optionValues, CONFIG(0));

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        CONFIG(0) = getFormItemInt(F("qei_mode"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        byte addr = CONFIG(0);

        if (!Plugin_059_QE)
          Plugin_059_QE = new QEIx4;

        Plugin_059_QE->begin(1,2,-1,4);

        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (Plugin_059_QE)
        {
          if (Plugin_059_QE->hasChanged())
          {
            UserVar[event->BaseVarIndex] = (float)Plugin_059_QE->read();
            event->sensorType = SENSOR_TYPE_SWITCH;

            //String log = F("Mkey : key=0x");
            //log += String(key, 16);
            //addLog(LOG_LEVEL_INFO, log);

            sendData(event);
          }

        }
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        if (Plugin_059_QE)
        {
          UserVar[event->BaseVarIndex] = (float)Plugin_059_QE->read();
        }
        success = true;
        break;
      }

  }
  return success;
}

#endif
