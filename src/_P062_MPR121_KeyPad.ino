#include "_Plugin_Helper.h"
#ifdef USES_P062

// #######################################################################################################
// #################################### Plugin 062: MPR121 KeyPad ########################################
// #######################################################################################################

// ESPEasy Plugin to scan a 12 key touch pad chip MPR121
// written by Jochen Krapf (jk@nerd2nerd.org)
// 2020-10-14 tonhuisman: Added settings for global and per-sensor sensitivity
//                        and getting 'calibration' touch pressure data (current, min, max)

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



#include "src/PluginStructs/P062_data_struct.h"

#define P062_FLAGS_USE_CALIBRATION    0   // Set in P062_CONFIG_FLAGS

#define P062_CONFIG_FLAGS             PCONFIG_LONG(0)    // 0-31 flags

#define P062_DEFAULT_TOUCH_TRESHOLD   12 // Defaults got from MPR_121 source
#define P062_DEFAULT_RELEASE_TRESHOLD 6

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

      addFormSubHeader(F("Sensitivity"));
      {
        uint8_t touch_treshold   = PCONFIG(2);
        if(touch_treshold == 0) {
          touch_treshold = P062_DEFAULT_TOUCH_TRESHOLD; //default value
        }
        addFormNumericBox(F("Touch treshold (1..255)"), F("p062_touch_treshold"), touch_treshold, 0, 255);
        String unit_ = F("Default: ");
        unit_ += String(P062_DEFAULT_TOUCH_TRESHOLD);
        addUnit(unit_);
      }

      {
        uint8_t release_treshold = PCONFIG(3);
        if(release_treshold == 0) {
          release_treshold = P062_DEFAULT_RELEASE_TRESHOLD; //default value
        }
        addFormNumericBox(F("Release treshold (1..255)"), F("p062_release_treshold"), release_treshold, 0, 255);
        String unit_ = F("Default: ");
        unit_ += String(P062_DEFAULT_RELEASE_TRESHOLD);
        addUnit(unit_);
      }
      {
        bool canCalibrate = true;
        bool tbUseCalibration = bitRead(P062_CONFIG_FLAGS, P062_FLAGS_USE_CALIBRATION);

        P062_data_struct *P062_data = static_cast<P062_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr == P062_data) {
          P062_data = new (std::nothrow) P062_data_struct();
          canCalibrate = false;
          if (P062_data == nullptr) {
            return success;
          }
        }
        P062_data->loadTouchObjects(event->TaskIndex);

        addRowLabel(F("Object"));
        html_table(EMPTY_STRING, false);  // Sub-table
        html_table_header(F("&nbsp;#&nbsp;"));
        html_table_header(F("Touch (0..255)"));
        html_table_header(F("Release (0..255)"));
        if (tbUseCalibration && canCalibrate) {
          html_table_header(F("Current"));
          html_table_header(F("Min"));
          html_table_header(F("Max"));
        }

        for (int objectNr = 0; objectNr < P062_MaxTouchObjects; objectNr++) {
          html_TR_TD();
          addHtml(F("&nbsp;"));
          addHtmlInt(objectNr + 1);
          html_TD();
          addNumericBox(getPluginCustomArgName(objectNr + 100), P062_data->StoredSettings.TouchObjects[objectNr].touch,     0, 255);
          html_TD();
          addNumericBox(getPluginCustomArgName(objectNr + 200), P062_data->StoredSettings.TouchObjects[objectNr].release,     0, 255);
          if (tbUseCalibration && canCalibrate) {
            uint16_t current = 0;
            uint16_t min     = 0;
            uint16_t max     = 0;
            P062_data->getCalibrationData(objectNr, &current, &min, &max);
            html_TD();
            addHtmlInt(current);
            html_TD();
            addHtmlInt(min);
            html_TD();
            addHtmlInt(max);
          }
        }
        html_end_table();
        if (canCalibrate) {
          const __FlashStringHelper * options1[2] = { F("No"), F("Yes") };
          int optionValues1[2] = { 0, 1 };
          int choice1 = tbUseCalibration ? 1 : 0;
          addFormSelector(F("Enable Calibration"), F("p062_use_calibration"), 2, options1, optionValues1, choice1, true);
          if (tbUseCalibration) {
            addFormCheckBox(F("Clear calibrationdata"), F("p062_clear_calibrate"), false);
          }
        } else {
          delete P062_data;
        }
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("i2c_addr"));

      PCONFIG(1) = isFormItemChecked(F("scancode"));

      PCONFIG(2) = getFormItemInt(F("p062_touch_treshold"));
      PCONFIG(3) = getFormItemInt(F("p062_release_treshold"));

      uint32_t lSettings = 0;
      bool tbUseCalibration = getFormItemInt(F("p062_use_calibration")) == 1;
      bitWrite(lSettings, P062_FLAGS_USE_CALIBRATION, tbUseCalibration);
      P062_CONFIG_FLAGS  = lSettings;

      {
        bool canCalibrate = true;
        P062_data_struct *P062_data = static_cast<P062_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr == P062_data) {
          P062_data = new (std::nothrow) P062_data_struct();
          canCalibrate = false;
          if (P062_data == nullptr) {
            return success; // Save other settings even though this didn't initialize properly
          }
        }
        P062_data->loadTouchObjects(event->TaskIndex);

        for (int objectNr = 0; objectNr < P062_MaxTouchObjects; objectNr++) {
          P062_data->StoredSettings.TouchObjects[objectNr].touch   = getFormItemInt(getPluginCustomArgName(objectNr + 100));
          P062_data->StoredSettings.TouchObjects[objectNr].release = getFormItemInt(getPluginCustomArgName(objectNr + 200));
        }
#ifdef PLUGIN_062_DEBUG
        String log = F("p062_data save size: ");
        log += sizeof(P062_data->StoredSettings);
        addLog(LOG_LEVEL_INFO, log);
#endif // PLUGIN_062_DEBUG
        SaveCustomTaskSettings(event->TaskIndex, (uint8_t *)&(P062_data->StoredSettings), sizeof(P062_data->StoredSettings));
        if (!canCalibrate) {
          delete P062_data;
        } else {
          bool clearCalibration = isFormItemChecked(F("p062_clear_calibrate"));
          if (clearCalibration) {
            P062_data->clearCalibrationData();
#ifdef PLUGIN_062_DEBUG
            addLog(LOG_LEVEL_INFO, F("p062 clear calibration"));
#endif // PLUGIN_062_DEBUG
          }
        }
      }
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      bool tbUseCalibration = bitRead(P062_CONFIG_FLAGS, P062_FLAGS_USE_CALIBRATION);

      initPluginTaskData(event->TaskIndex, new (std::nothrow) P062_data_struct());
      P062_data_struct *P062_data = static_cast<P062_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P062_data) {
        success = true;
        if (!P062_data->init(event->TaskIndex, PCONFIG(0), PCONFIG(1), tbUseCalibration)) {
          clearPluginTaskData(event->TaskIndex);
          P062_data = nullptr;
        } else {
          uint8_t touch_treshold   = PCONFIG(2);
          if(touch_treshold == 0) {
            touch_treshold = P062_DEFAULT_TOUCH_TRESHOLD; //default value
          }
          uint8_t release_treshold = PCONFIG(3);
          if(release_treshold == 0) {
            release_treshold = P062_DEFAULT_RELEASE_TRESHOLD; //default value
          }
          if (touch_treshold != P062_DEFAULT_TOUCH_TRESHOLD && release_treshold != P062_DEFAULT_RELEASE_TRESHOLD) {
            P062_data->setThresholds(touch_treshold, release_treshold); // Set custom tresholds, ignore default values
          }
          for (uint8_t objectNr = 0; objectNr < P062_MaxTouchObjects; objectNr++) {
            if (P062_data->StoredSettings.TouchObjects[objectNr].touch != 0 && P062_data->StoredSettings.TouchObjects[objectNr].release != 0) {
              P062_data->setThreshold(objectNr, P062_data->StoredSettings.TouchObjects[objectNr].touch, P062_data->StoredSettings.TouchObjects[objectNr].release);
            }
          }
        }
      }

      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P062_data_struct *P062_data =
        static_cast<P062_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P062_data) {
        return success;
      } else {
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

            bool tbUseCalibration = bitRead(P062_CONFIG_FLAGS, P062_FLAGS_USE_CALIBRATION);

            if (tbUseCalibration) {
              uint16_t colMask = 0x01;
              log.reserve(55);

              for (byte col = 0; col < P062_MaxTouchObjects; col++)
              {
                if (key & colMask) // this key pressed?
                {
                  uint16_t current = 0;
                  uint16_t min     = 0;
                  uint16_t max     = 0;
                  P062_data->getCalibrationData(col, &current, &min, &max);
                  log  = F("P062 touch #");
                  log += col;
                  log += F(" current: ");
                  log += current;
                  log += F(" min: ");
                  log += min;
                  log += F(" max: ");
                  log += max;
                  addLog(LOG_LEVEL_INFO, log);
                  if (!PCONFIG(1)) {
                    break;
                  }
                }
                colMask <<= 1;
              }
            }
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
