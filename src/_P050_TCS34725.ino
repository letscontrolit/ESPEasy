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
// 2021-01-20 tonhuisman: Renamed Calibration to Transformation, fix some textual issues
// 2021-01-20 tonhuisman: Added optional events for not selected RGB outputs, compile-time optional
// 2021-01-19 tonhuisman: (Re)Added additional transformation & calculation options
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

// #ifndef LIMIT_BUILD_SIZE
#define P050_OPTION_RGB_EVENTS
// #endif

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
        P050_data->resetTransformation(); // Explicit reset
        P050_data->saveSettings(event->TaskIndex);
        delete P050_data;
      }
      break;
    }
    case PLUGIN_WEBFORM_LOAD:
    {
      byte   choiceMode = PCONFIG(0);
      {
        const __FlashStringHelper * optionsMode[6];
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
        const __FlashStringHelper * optionsMode2[4];
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
        #define P050_RGB_OPTIONS 6
        const __FlashStringHelper * optionsRGB[P050_RGB_OPTIONS];
        optionsRGB[0] = F("Raw RGB");
        optionsRGB[1] = F("Raw RGB transformed (3x3 matrix, below)");
        optionsRGB[2] = F("Normalized RGB (0..255)");
        optionsRGB[3] = F("Normalized RGB transformed (3x3 matrix, below)");
        optionsRGB[4] = F("Normalized RGB (0.0000..1.0000)");
        optionsRGB[5] = F("Normalized RGB (0.0000..1.0000) transformed (3x3 matrix, below)");
        int optionValuesRGB[P050_RGB_OPTIONS] = { 0, 1, 2, 3, 4, 5};
        addFormSelector(F("Output RGB Values"), F("p050_outputrgb"), P050_RGB_OPTIONS, optionsRGB, optionValuesRGB, PCONFIG(2));
        addFormNote(F("For 'normalized' or 'transformed' options, the Red/Green/Blue Decimals should best be increased."));

#ifdef P050_OPTION_RGB_EVENTS
        addFormCheckBox(F("Generate RGB events"), F("p050_generate_rgb_events"), PCONFIG(5) == 1);
        addFormNote(F("Eventnames: taskname + #RawRGB, #RawRGBtransformed, #NormRGB, #NormRGBtransformed, #NormSRGB, #NormSRGBtransformed"));
        addFormNote(F("Only generated for not selected outputs, 3 values per event, =&lt;r&gt;,&lt;g&gt;,&lt;b&gt;"));
#endif
      }

      {
        #define P050_VALUE4_OPTIONS 4
        const __FlashStringHelper * optionsOutput[P050_VALUE4_OPTIONS];
        optionsOutput[0] = F("Color Temperature (deprecated) [K]");
        optionsOutput[1] = F("Color Temperature (DN40) [K]");
        optionsOutput[2] = F("Ambient Light [Lux]");
        optionsOutput[3] = F("Clear Channel");
        int optionValuesOutput[P050_VALUE4_OPTIONS] = { 0, 1, 2, 3};
        addFormSelector(F("Output at Values #4"), F("p050_output4"), P050_VALUE4_OPTIONS, optionsOutput, optionValuesOutput, PCONFIG(3));
        addFormNote(F("Optionally adjust Values #4 name accordingly."));

        addFormCheckBox(F("Generate all as events"), F("p050_generate_all_events"), PCONFIG(4) == 1);
        addFormNote(F("Eventnames: taskname + #CCT, #CCT_DN40, #Lux, #Clear"));
      }

      {
        P050_data_struct *P050_data = new (std::nothrow) P050_data_struct(PCONFIG(0), PCONFIG(1));

        if (nullptr != P050_data) {
          addFormSubHeader(F("Transformation matrix"));

          P050_data->loadSettings(event->TaskIndex);

          // Display current settings
          const String RGB = F("RGB");
          for (int i = 0; i < 3; i++) {
            addRowLabel(RGB.substring(i, i + 1));
            String id = F("p050_cal_");
            for (int j = 0; j < 3; j++) {
              addHtml(String(static_cast<char>('a' + i)));
              addHtml(F("<sub>"));
              addHtmlInt(j + 1);
              addHtml(F("</sub>"));
              addHtml(':');
              addFloatNumberBox(id + static_cast<char>('a' + i) + '_' + String(j), P050_data->TransformationSettings.matrix[i][j], -255.999f, 255.999f);
            }
          }
          addFormNote(F("Check plugin documentation (i) on how to calibrate and how to calculate transformation matrix."));

          addFormCheckBox(F("Reset transformation matrix"), F("p050_reset_transformation"), false);
          addFormNote(F("Select then Submit to confirm. Reset transformation matrix can't be un-done!"));

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
#ifdef P050_OPTION_RGB_EVENTS
      PCONFIG(5) = isFormItemChecked(F("p050_generate_rgb_events")) ? 1 : 0;
#endif
      bool resetTransformation = isFormItemChecked(F("p050_reset_transformation"));
      {
        P050_data_struct *P050_data = new (std::nothrow) P050_data_struct(PCONFIG(0), PCONFIG(1));

        if (nullptr != P050_data) {

          P050_data->loadSettings(event->TaskIndex);

          if (resetTransformation) {
            // Clear Transformation settings
            P050_data->resetTransformation();
          } else {
            // Save new settings
            for (int i = 0; i < 3; i++) {
              String id = F("p050_cal_");
              for (int j = 0; j < 3; j++) {
                P050_data->TransformationSettings.matrix[i][j] = getFormItemFloat(id + static_cast<char>('a' + i) + '_' + String(j));
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

        float sRGBFactor = 1.0f; // (s)RGB factor 1.0 or 255.0
        uint32_t t = r + g + b;  // Normalization factor
        if (t == 0) {
          UserVar[event->BaseVarIndex + 0] = 0.0f;
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
            if (t != 0) { // R/G/B transformed
              P050_data->applyTransformation(r, g, b,
                                             &UserVar[event->BaseVarIndex + 0],
                                             &UserVar[event->BaseVarIndex + 1],
                                             &UserVar[event->BaseVarIndex + 2]);
            }
            break;
          case 2:
            sRGBFactor = 255.0f;
            // Fall through
          case 4:
            if (t != 0) { // r/g/b (normalized to 0.00..255.00 (but avoid divide by 0)
              UserVar[event->BaseVarIndex + 0] = (float)r / t * sRGBFactor;
              UserVar[event->BaseVarIndex + 1] = (float)g / t * sRGBFactor;
              UserVar[event->BaseVarIndex + 2] = (float)b / t * sRGBFactor;
            }
            break;
          case 3:
            sRGBFactor = 255.0f;
            // Fall through
          case 5:
            if (t != 0) { // R/G/B normalized & transformed
              float nr = (float)r / t * sRGBFactor;
              float ng = (float)g / t * sRGBFactor;
              float nb = (float)b / t * sRGBFactor;
              P050_data->applyTransformation(nr, ng, nb,
                                             &UserVar[event->BaseVarIndex + 0],
                                             &UserVar[event->BaseVarIndex + 1],
                                             &UserVar[event->BaseVarIndex + 2]);
            }
            break;
        }
        UserVar[event->BaseVarIndex + 3] = value4;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
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
        }

#ifdef P050_OPTION_RGB_EVENTS
        // First RGB events
        if (PCONFIG(5) == 1 && t != 0) { // Not if invalid read/data
          String RuleEvent;
          float tr, tg, tb, nr, ng, nb;
          RuleEvent.reserve(48);
          for (int i = 0; i < 6; i++) {
            if (i != PCONFIG(2)) { // Skip currently selected RGB output to keep nr. of events a bit limited
              sRGBFactor = 1.0f;
              RuleEvent  = getTaskDeviceName(event->TaskIndex);
              RuleEvent += '#';
              switch (i) {
              case 0:
                RuleEvent += F("RawRGB=");
                RuleEvent += r;
                RuleEvent += ',';
                RuleEvent += g;
                RuleEvent += ',';
                RuleEvent += b;
                break;
              case 3:
                sRGBFactor = 255.0f;
                // Fall through
              case 1:
              case 5:
                if (i == 1) {
                  RuleEvent += F("RawRGBtransformed=");
                  P050_data->applyTransformation(r, g, b, &tr, &tg, &tb);
                } else {
                  if (i == 3) {
                    RuleEvent += F("NormRGBtransformed=");
                  } else {
                    RuleEvent += F("NormSRGBtransformed=");
                  }
                  nr = (float)r / t * sRGBFactor;
                  ng = (float)g / t * sRGBFactor;
                  nb = (float)b / t * sRGBFactor;
                  P050_data->applyTransformation(nr, ng, nb, &tr, &tg, &tb);
                }
                RuleEvent += String(tr, 4);
                RuleEvent += ',';
                RuleEvent += String(tg, 4);
                RuleEvent += ',';
                RuleEvent += String(tb, 4);
                break;
              case 2:
                sRGBFactor = 255.0f;
                // Fall through
              case 4:
                if (i == 2) {
                  RuleEvent += F("NormRGB=");
                } else {
                  RuleEvent += F("NormSRGB=");
                }
                RuleEvent += String((float)r / t * sRGBFactor, 4);
                RuleEvent += ',';
                RuleEvent += String((float)g / t * sRGBFactor, 4);
                RuleEvent += ',';
                RuleEvent += String((float)b / t * sRGBFactor, 4);
                break;
              default:
                RuleEvent = EMPTY_STRING;
                break;
              }
              if (!RuleEvent.isEmpty()) {
                eventQueue.add(RuleEvent);
              }
            }
          }
        }
#endif

        // Then Values #4 events
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
              RuleEvent = EMPTY_STRING;
              break;
            }
            if (!RuleEvent.isEmpty()) {
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
