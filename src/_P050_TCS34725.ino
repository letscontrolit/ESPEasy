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

# include "src/PluginStructs/P050_data_struct.h"


# define PLUGIN_050
# define PLUGIN_ID_050         50
# define PLUGIN_NAME_050       "Color - TCS34725"
# define PLUGIN_VALUENAME1_050 "Red"
# define PLUGIN_VALUENAME2_050 "Green"
# define PLUGIN_VALUENAME3_050 "Blue"
# define PLUGIN_VALUENAME4_050 "ColorTemperature"

// #ifndef LIMIT_BUILD_SIZE
# define P050_OPTION_RGB_EVENTS

// #endif

boolean Plugin_050(uint8_t function, struct EventStruct *event, String& string)
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
      Device[deviceCount].PluginStats        = true;
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

      # if FEATURE_I2C_DEVICE_CHECK

      if (!I2C_deviceCheck(0x29)) {
        break;        // Will return the default false for success
      }
      # endif // if FEATURE_I2C_DEVICE_CHECK
      P050_data_struct *P050_data = new (std::nothrow) P050_data_struct(PCONFIG(0), PCONFIG(1));

      if (nullptr != P050_data) {
        P050_data->resetTransformation(); // Explicit reset
        P050_data->saveSettings(event->TaskIndex);
        delete P050_data;
      }
      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = (event->Par1 == 0x29);
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = 0x29;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      uint8_t choiceMode = PCONFIG(0);
      {
        const __FlashStringHelper *optionsMode[] = {
          F("2.4 ms"),
          F("24 ms"),
          F("50 ms"),
          F("101 ms"),
          F("154 ms"),
          F("700 ms"),
        };
        const int optionValuesMode[] = {
          TCS34725_INTEGRATIONTIME_2_4MS,
          TCS34725_INTEGRATIONTIME_24MS,
          TCS34725_INTEGRATIONTIME_50MS,
          TCS34725_INTEGRATIONTIME_101MS,
          TCS34725_INTEGRATIONTIME_154MS,
          TCS34725_INTEGRATIONTIME_700MS,
        };
        addFormSelector(F("Integration Time"), F("inttime"), 6, optionsMode, optionValuesMode, choiceMode);
      }

      uint8_t choiceMode2 = PCONFIG(1);
      {
        const __FlashStringHelper *optionsMode2[] = {
          F("1x"),
          F("4x"),
          F("16x"),
          F("60x"),
        };
        const int optionValuesMode2[] = {
          TCS34725_GAIN_1X,
          TCS34725_GAIN_4X,
          TCS34725_GAIN_16X,
          TCS34725_GAIN_60X,
        };
        addFormSelector(F("Gain"), F("gain"), 4, optionsMode2, optionValuesMode2, choiceMode2);
      }

      addFormSubHeader(F("Output settings"));

      {
        # define P050_RGB_OPTIONS 6
        const __FlashStringHelper *optionsRGB[P050_RGB_OPTIONS] = {
          F("Raw RGB"),
          F("Raw RGB transformed (3x3 matrix, below)"),
          F("Normalized RGB (0..255)"),
          F("Normalized RGB transformed (3x3 matrix, below)"),
          F("Normalized RGB (0.0000..1.0000)"),
          F("Normalized RGB (0.0000..1.0000) transformed (3x3 matrix, below)"),
        };
        const int optionValuesRGB[P050_RGB_OPTIONS] = { 0, 1, 2, 3, 4, 5 };
        addFormSelector(F("Output RGB Values"), F("outputrgb"), P050_RGB_OPTIONS, optionsRGB, optionValuesRGB, PCONFIG(2));
        addFormNote(F("For 'normalized' or 'transformed' options, the Red/Green/Blue Decimals should best be increased."));

# ifdef P050_OPTION_RGB_EVENTS
        addFormCheckBox(F("Generate RGB events"), F("rgbevents"), PCONFIG(5) == 1);
        addFormNote(F("Eventnames: taskname + #RawRGB, #RawRGBtransformed, #NormRGB, #NormRGBtransformed, #NormSRGB, #NormSRGBtransformed"));
        addFormNote(F("Only generated for not selected outputs, 3 values per event, =&lt;r&gt;,&lt;g&gt;,&lt;b&gt;"));
# endif // ifdef P050_OPTION_RGB_EVENTS
      }

      {
        # define P050_VALUE4_OPTIONS 4
        const __FlashStringHelper *optionsOutput[P050_VALUE4_OPTIONS] = {
          F("Color Temperature (deprecated) [K]"),
          F("Color Temperature (DN40) [K]"),
          F("Ambient Light [Lux]"),
          F("Clear Channel"),
        };
        const int optionValuesOutput[P050_VALUE4_OPTIONS] = { 0, 1, 2, 3 };
        addFormSelector(F("Output at Values #4"), F("output4"), P050_VALUE4_OPTIONS, optionsOutput, optionValuesOutput, PCONFIG(3));
        addFormNote(F("Optionally adjust Values #4 name accordingly."));

        addFormCheckBox(F("Generate all as events"), F("allevents"), PCONFIG(4) == 1);
        addFormNote(F("Eventnames: taskname + #CCT, #CCT_DN40, #Lux, #Clear"));
      }

      {
        P050_data_struct *P050_data = new (std::nothrow) P050_data_struct(PCONFIG(0), PCONFIG(1));

        if (nullptr != P050_data) {
          addFormSubHeader(F("Transformation matrix"));

          P050_data->resetTransformation();
          P050_data->loadSettings(event->TaskIndex);

          // Display current settings
          const String RGB = F("RGB");

          for (int i = 0; i < 3; i++) {
            addRowLabel(RGB.substring(i, i + 1));
            String id = F("cal_");

            for (int j = 0; j < 3; j++) {
              addHtml(static_cast<char>('a' + i));
              addHtml(F("<sub>"));
              addHtmlInt(j + 1);
              addHtml(F("</sub>"));
              addHtml(':');
              addFloatNumberBox(id + static_cast<char>('a' + i) + '_' + String(j),
                                P050_data->TransformationSettings.matrix[i][j],
                                -255.999f,
                                255.999f);
            }
          }
          addFormNote(F("Check plugin documentation (i) on how to calibrate and how to calculate transformation matrix."));

          addFormCheckBox(F("Reset transformation matrix"), F("resettrans"), false);
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
      PCONFIG(0) = getFormItemInt(F("inttime"));
      PCONFIG(1) = getFormItemInt(F("gain"));
      PCONFIG(2) = getFormItemInt(F("outputrgb"));
      PCONFIG(3) = getFormItemInt(F("output4"));
      PCONFIG(4) = isFormItemChecked(F("allevents")) ? 1 : 0;
# ifdef P050_OPTION_RGB_EVENTS
      PCONFIG(5) = isFormItemChecked(F("rgbevents")) ? 1 : 0;
# endif // ifdef P050_OPTION_RGB_EVENTS
      bool resetTransformation = isFormItemChecked(F("resettrans"));
      {
        P050_data_struct *P050_data = new (std::nothrow) P050_data_struct(PCONFIG(0), PCONFIG(1));

        if (nullptr != P050_data) {
          P050_data->resetTransformation();
          P050_data->loadSettings(event->TaskIndex);

          if (resetTransformation) {
            // Clear Transformation settings
            P050_data->resetTransformation();
          } else {
            // Save new settings
            for (int i = 0; i < 3; i++) {
              String id = F("cal_");

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
        P050_data->resetTransformation();
        success = true;
      }
      break;
    }

    case PLUGIN_EXIT:
    {
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      P050_data_struct *P050_data = static_cast<P050_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P050_data) {
        return success;
      }

      if (P050_data->tcs.begin()) {
# ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("Found TCS34725 sensor"));
# endif // ifndef BUILD_NO_DEBUG

        uint16_t r, g, b, c;
        float value4 = 0.0f;

        P050_data->loadSettings(event->TaskIndex);

        P050_data->tcs.getRawData(&r, &g, &b, &c, true);

        switch (PCONFIG(3)) {
          case 0:
            value4 = P050_data->tcs.calculateColorTemperature(r, g, b); // Deprecated because of deemed inaccurate calculation, kept for
                                                                        // backward compatibility
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

        float sRGBFactor = 1.0f;      // (s)RGB factor 1.0 or 255.0
        uint32_t t       = r + g + b; // Normalization factor

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
              UserVar[event->BaseVarIndex + 0] = static_cast<float>(r) / t * sRGBFactor;
              UserVar[event->BaseVarIndex + 1] = static_cast<float>(g) / t * sRGBFactor;
              UserVar[event->BaseVarIndex + 2] = static_cast<float>(b) / t * sRGBFactor;
            }
            break;
          case 3:
            sRGBFactor = 255.0f;

          // Fall through
          case 5:

            if (t != 0) { // R/G/B normalized & transformed
              const float nr = static_cast<float>(r) / t * sRGBFactor;
              const float ng = static_cast<float>(g) / t * sRGBFactor;
              const float nb = static_cast<float>(b) / t * sRGBFactor;
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
          addLogMove(LOG_LEVEL_INFO, log);
        }

# ifdef P050_OPTION_RGB_EVENTS

        // First RGB events
        if ((PCONFIG(5) == 1) && (t != 0)) { // Not if invalid read/data
          float tr, tg, tb, nr, ng, nb;

          for (int i = 0; i < 6; i++) {
            if (i != PCONFIG(2)) { // Skip currently selected RGB output to keep nr. of events a bit limited
              const __FlashStringHelper* varName = F("");
              String eventValues;
              sRGBFactor = 1.0f;

              switch (i) {
                case 0:
                  varName = F("RawRGB");
                  eventValues += r;
                  eventValues += ',';
                  eventValues += g;
                  eventValues += ',';
                  eventValues += b;
                  break;
                case 3:
                  sRGBFactor = 255.0f;

                // Fall through
                case 1:
                case 5:

                  if (i == 1) {
                    varName = F("RawRGBtransformed");
                    P050_data->applyTransformation(r, g, b, &tr, &tg, &tb);
                  } else {
                    if (i == 3) {
                      varName = F("NormRGBtransformed");
                    } else {
                      varName = F("NormSRGBtransformed");
                    }
                    nr = static_cast<float>(r) / t * sRGBFactor;
                    ng = static_cast<float>(g) / t * sRGBFactor;
                    nb = static_cast<float>(b) / t * sRGBFactor;
                    P050_data->applyTransformation(nr, ng, nb, &tr, &tg, &tb);
                  }
                  eventValues += toString(tr, 4);
                  eventValues += ',';
                  eventValues += toString(tg, 4);
                  eventValues += ',';
                  eventValues += toString(tb, 4);
                  break;
                case 2:
                  sRGBFactor = 255.0f;

                // Fall through
                case 4:

                  if (i == 2) {
                    varName = F("NormRGB");
                  } else {
                    varName = F("NormSRGB");
                  }
                  eventValues += toString(static_cast<float>(r) / t * sRGBFactor, 4);
                  eventValues += ',';
                  eventValues += toString(static_cast<float>(g) / t * sRGBFactor, 4);
                  eventValues += ',';
                  eventValues += toString(static_cast<float>(b) / t * sRGBFactor, 4);
                  break;
                default:
                  eventValues.clear();
                  break;
              }

              if (!eventValues.isEmpty()) {
                eventQueue.add(event->TaskIndex, varName, eventValues);
              }
            }
          }
        }
# endif // ifdef P050_OPTION_RGB_EVENTS

        // Then Values #4 events
        if (PCONFIG(4) == 1) {
          for (int i = 0; i < 4; i++) {
            switch (i) {
              case 0:
                eventQueue.add(event->TaskIndex, F("CCT"), P050_data->tcs.calculateColorTemperature(r, g, b));
                break;
              case 1:
                eventQueue.add(event->TaskIndex, F("CCT_DN40"), P050_data->tcs.calculateColorTemperature_dn40(r, g, b, c));
                break;
              case 2:
                eventQueue.add(event->TaskIndex, F("Lux"), P050_data->tcs.calculateLux(r, g, b));
                break;
              case 3:
                eventQueue.add(event->TaskIndex, F("Clear"), String(c));
                break;
              default:
                break;
            }
          }
        }

        success = true;
      } else {
# ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("No TCS34725 found"));
# endif // ifndef BUILD_NO_DEBUG
        success = false;
      }

      break;
    }
  }
  return success;
}

#endif // USES_P050
