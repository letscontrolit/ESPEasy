#include "_Plugin_Helper.h"
#ifdef USES_P050

// #######################################################################################################
// #################### Plugin 050 I2C TCS34725 RGB Color Sensor with IR filter and White LED ############
// #######################################################################################################
//
// RGB Color Sensor with IR filter and White LED
// like this one: https://www.adafruit.com/products/1334
// based on this library: https://github.com/adafruit/Adafruit_TCS34725
// this code is based on 20170331 date version of the above library
// this code is UNTESTED, because my TCS34725 sensor is still not shipped :(
//
// 2021-01-16 tonhuisman: Move stuff to PluginStructs, add 3x3 matrix calibration
// 2021-01-09 tonhuisman: Add R/G/B calibration factors, improved/corrected normalization
// 2021-01-03 tonhuisman: Merged most of the changes in the library, for adding the getRGB() and calculateColorTemperature_dn40(0 functions)
//

#include "src/PluginStructs/P050_data_struct.h"


#define PLUGIN_050
#define PLUGIN_ID_050         50
#define PLUGIN_NAME_050       "Color - TCS34725  [TESTING]"
#define PLUGIN_VALUENAME1_050 "Red"
#define PLUGIN_VALUENAME2_050 "Green"
#define PLUGIN_VALUENAME3_050 "Blue"
#define PLUGIN_VALUENAME4_050 "ColorTemperature"


boolean Plugin_050(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_050;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 4;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_050);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_050));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_050));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_050));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_050));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      PCONFIG(2) = 1; // RGB values: Calibrated RGB
      PCONFIG(3) = 1; // Value #4: Color Temperature (DN40)

      P050_data_struct *P050_data = new (std::nothrow) P050_data_struct(PCONFIG(0), PCONFIG(1));

      if (nullptr != P050_data) {
        P050_data->resetCalibration(); // Explicit reset
        P050_data->saveSettings(event->TaskIndex);
        delete P050_data;
      }
      break;
    }
    case PLUGIN_WEBFORM_LOAD:
    {
      byte   choiceMode = PCONFIG(0);
      {
        String optionsMode[6];
        optionsMode[0] = F("2.4 ms");
        optionsMode[1] = F("24 ms");
        optionsMode[2] = F("50 ms");
        optionsMode[3] = F("101 ms");
        optionsMode[4] = F("154 ms");
        optionsMode[5] = F("700 ms");
        int optionValuesMode[6];
        optionValuesMode[0] = TCS34725_INTEGRATIONTIME_2_4MS;
        optionValuesMode[1] = TCS34725_INTEGRATIONTIME_24MS;
        optionValuesMode[2] = TCS34725_INTEGRATIONTIME_50MS;
        optionValuesMode[3] = TCS34725_INTEGRATIONTIME_101MS;
        optionValuesMode[4] = TCS34725_INTEGRATIONTIME_154MS;
        optionValuesMode[5] = TCS34725_INTEGRATIONTIME_700MS;
        addFormSelector(F("Integration Time"), F("p050_integrationTime"), 6, optionsMode, optionValuesMode, choiceMode);
      }

      byte   choiceMode2 = PCONFIG(1);
      {
        String optionsMode2[4];
        optionsMode2[0] = F("1x");
        optionsMode2[1] = F("4x");
        optionsMode2[2] = F("16x");
        optionsMode2[3] = F("60x");
        int optionValuesMode2[4];
        optionValuesMode2[0] = TCS34725_GAIN_1X;
        optionValuesMode2[1] = TCS34725_GAIN_4X;
        optionValuesMode2[2] = TCS34725_GAIN_16X;
        optionValuesMode2[3] = TCS34725_GAIN_60X;
        addFormSelector(F("Gain"), F("p050_gain"), 4, optionsMode2, optionValuesMode2, choiceMode2);
      }

      addFormSubHeader(F("Output settings"));

      {
        #define P050_RGB_OPTIONS 2
        String optionsRGB[P050_RGB_OPTIONS];
        optionsRGB[0] = F("Raw RGB (uncalibrated)");
        optionsRGB[1] = F("Calibrated RGB (3x3 matrix, below)");
        int optionValuesRGB[P050_RGB_OPTIONS] = { 0, 1};
        addFormSelector(F("Output RGB Values"), F("p050_outputrgb"), P050_RGB_OPTIONS, optionsRGB, optionValuesRGB, PCONFIG(2));
        addFormNote(F("For 'Calibrated RGB', the Red/Green/Blue values Decimals should best be increased."));
      }

      {
        #define P050_VALUE4_OPTIONS 4
        String optionsOutput[P050_VALUE4_OPTIONS];
        optionsOutput[0] = F("Color Temperature (deprecated)");
        optionsOutput[1] = F("Color Temperature (DN40)");
        optionsOutput[2] = F("Lux");
        optionsOutput[3] = F("Clear");
        int optionValuesOutput[P050_VALUE4_OPTIONS] = { 0, 1, 2, 3};
        addFormSelector(F("Output at Values #4"), F("p050_output4"), P050_VALUE4_OPTIONS, optionsOutput, optionValuesOutput, PCONFIG(3));
        addFormNote(F("Optionally adjust Values #4 name accordingly."));

        addFormCheckBox(F("Generate all as events"), F("p050_generate_all_events"), PCONFIG(4) == 1);
        addFormNote(F("Eventnames: &lt;taskname&gt;+ #CCT, #CCT_DN40, #Lux, #Clear"));
      }

      {
        P050_data_struct *P050_data = new (std::nothrow) P050_data_struct(PCONFIG(0), PCONFIG(1));

        if (nullptr != P050_data) {
          addFormSubHeader(F("Calibration matrix"));

          P050_data->loadSettings(event->TaskIndex);

          // Display current settings
          const String RGB = F("RGB");
          for (int i = 0; i < 3; i++) {
            String label = F("Calibration - ");
            label += RGB.charAt(i);
            addRowLabel(label);
            String id = F("p050_cal_");
            for (int j = 0; j < 3; j++) {
              addHtml(String(static_cast<char>('a' + i)) + String(F("<sub>")) + String(j + 1) + String(F("</sub>")) + ':');
              addFloatNumberBox(id + static_cast<char>('a' + i) + '_' + String(j), P050_data->CalibrationSettings.matrix[i][j], -255.999f, 255.999f);
            }
          }
          addFormNote(F("Check plugin documentation (i) on how to calibrate."));

          addFormCheckBox(F("Reset calibration matrix"), F("p050_reset_calibration"), false);
          addFormNote(F("Select then Submit to confirm. Reset calibration matrix can't be un-done!"));

          // Need to delete the allocated object here
          delete P050_data;
        }
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("p050_integrationTime"));
      PCONFIG(1) = getFormItemInt(F("p050_gain"));
      PCONFIG(2) = getFormItemInt(F("p050_outputrgb"));
      PCONFIG(3) = getFormItemInt(F("p050_output4"));
      PCONFIG(4) = isFormItemChecked(F("p050_generate_all_events")) ? 1 : 0;
      bool resetCalibration = isFormItemChecked(F("p050_reset_calibration"));
      {
        P050_data_struct *P050_data = new (std::nothrow) P050_data_struct(PCONFIG(0), PCONFIG(1));

        if (nullptr != P050_data) {

          P050_data->loadSettings(event->TaskIndex);

          if (resetCalibration) {
            // Clear calibration settings
            P050_data->resetCalibration();
          } else {
            // Save new settings
            for (int i = 0; i < 3; i++) {
              String id = F("p050_cal_");
              for (int j = 0; j < 3; j++) {
                P050_data->CalibrationSettings.matrix[i][j] = getFormItemFloat(id + static_cast<char>('a' + i) + '_' + String(j));
              }
            }
          }
          P050_data->saveSettings(event->TaskIndex);

          // Need to delete the allocated object here
          delete P050_data;
        }
      }
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      /* Initialise with specific int time and gain values */
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P050_data_struct(PCONFIG(0), PCONFIG(1)));
      P050_data_struct *P050_data = static_cast<P050_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P050_data) {
        success = true;
      }
      break;
    }

    case PLUGIN_EXIT:
    {
      P050_data_struct *P050_data = static_cast<P050_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P050_data) {
        delete P050_data; // call destructor
        success = true;
      }
      break;
    }

    case PLUGIN_READ:
    {

      P050_data_struct *P050_data = static_cast<P050_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P050_data) {
        return success;
      }

      if (P050_data->tcs.begin()) {
        addLog(LOG_LEVEL_DEBUG, F("Found TCS34725 sensor"));

        uint16_t r, g, b, c;
        float value4 = 0.0f;

        P050_data->loadSettings(event->TaskIndex);

        P050_data->tcs.getRawData(&r, &g, &b, &c, true);
        switch (PCONFIG(3)) {
          case 0:
            value4 = P050_data->tcs.calculateColorTemperature(r, g, b); // Deprecated because of deemed inaccurate calculation, kept for backward compatibility
            break;
          case 1:
            value4 = P050_data->tcs.calculateColorTemperature_dn40(r, g, b, c);
            break;
          case 2:
            value4 = P050_data->tcs.calculateLux(r, g, b);
            break;
          case 3:
            value4 = c;
            break;
        }

        uint32_t t = r + g + b; // Normalization factor
        if (t == 0) {
          UserVar[event->BaseVarIndex]     = 0.0f;
          UserVar[event->BaseVarIndex + 1] = 0.0f;
          UserVar[event->BaseVarIndex + 2] = 0.0f;
        }

        switch (PCONFIG(2)) {
          case 0:
            UserVar[event->BaseVarIndex + 0] = r;
            UserVar[event->BaseVarIndex + 1] = g;
            UserVar[event->BaseVarIndex + 2] = b;
            break;
          case 1:
            if (t != 0) { // R/G/B normalized
              P050_data->applyCalibration(r, g, b,
                                          &UserVar[event->BaseVarIndex + 0],
                                          &UserVar[event->BaseVarIndex + 1],
                                          &UserVar[event->BaseVarIndex + 2]);
            }            break;
        }
        UserVar[event->BaseVarIndex + 3] = value4;

        String log = F("TCS34725: ");
        switch (PCONFIG(3)) {
          case 0:
          case 1:
            log += F("Color Temp (K): ");
            break;
          case 2:
            log += F("Lux : ");
            break;
          case 3:
            log += F("Clear : ");
            break;
        }
        log += formatUserVarNoCheck(event->TaskIndex, 3);
        log += F(" R: ");
        log += formatUserVarNoCheck(event->TaskIndex, 0);
        log += F(" G: ");
        log += formatUserVarNoCheck(event->TaskIndex, 1);
        log += F(" B: ");
        log += formatUserVarNoCheck(event->TaskIndex, 2);
        addLog(LOG_LEVEL_INFO, log);

        if (PCONFIG(4) == 1) {
          String RuleEvent;
          RuleEvent.reserve(48);
          for (int i = 0; i < 4; i++) {
            RuleEvent  = getTaskDeviceName(event->TaskIndex);
            RuleEvent += '#';
            switch (i) {
            case 0:
              RuleEvent += F("CCT=");
              RuleEvent += P050_data->tcs.calculateColorTemperature(r, g, b);
              break;
            case 1:
              RuleEvent += F("CCT_DN40=");
              RuleEvent += P050_data->tcs.calculateColorTemperature_dn40(r, g, b, c);
              break;
            case 2:
              RuleEvent += F("Lux=");
              RuleEvent += P050_data->tcs.calculateLux(r, g, b);
              break;
            case 3:
              RuleEvent += F("Clear=");
              RuleEvent += c;
              break;
            default:
              RuleEvent = F("");
              break;
            }
            if (RuleEvent.length() != 0) {
              eventQueue.add(RuleEvent);
            }
          }
        }

        success = true;
      } else {
        addLog(LOG_LEVEL_DEBUG, F("No TCS34725 found"));
        success = false;
      }

      break;
    }
  }
  return success;
}

#endif // USES_P050
