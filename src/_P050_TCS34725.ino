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
// 2021-01-09 tonhuisman: Add R/G/B calibration factors, improved/corrected normalization
// 2021-01-03 tonhuisman: Merged most of the changes in the library, for adding the getRGB() and calculateColorTemperature_dn40(0 functions)
//

#include "Adafruit_TCS34725.h"


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
        optionsRGB[1] = F("Normalized RGB (0.0000..255.0000)");
        int optionValuesRGB[P050_RGB_OPTIONS] = { 0, 1};
        addFormSelector(F("Output RGB Values"), F("p050_outputrgb"), P050_RGB_OPTIONS, optionsRGB, optionValuesRGB, PCONFIG(2));
        addFormNote(F("For 'Normalized RGB', the Red/Green/Blue values Decimals should best be increased."));
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
      }

      addFormSubHeader(F("Calibration settings"));

      addFormFloatNumberBox(F("Calibration factor R"), F("p050_calibration_red"),   PCONFIG_FLOAT(0), 0.0f, 100000.0f); // 
      addFormFloatNumberBox(F("Calibration factor G"), F("p050_calibration_green"), PCONFIG_FLOAT(1), 0.0f, 100000.0f); // 
      addFormFloatNumberBox(F("Calibration factor B"), F("p050_calibration_blue"),  PCONFIG_FLOAT(2), 0.0f, 100000.0f); // 
      addFormNote(F("Check plugin documentation (i) on how to calibrate. Use 0.0 to ignore calibration."));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("p050_integrationTime"));
      PCONFIG(1) = getFormItemInt(F("p050_gain"));
      PCONFIG(2) = getFormItemInt(F("p050_outputrgb"));
      PCONFIG(3) = getFormItemInt(F("p050_output4"));
      PCONFIG_FLOAT(0) = getFormItemFloat(F("p050_calibration_red"));
      PCONFIG_FLOAT(1) = getFormItemFloat(F("p050_calibration_green"));
      PCONFIG_FLOAT(2) = getFormItemFloat(F("p050_calibration_blue"));

      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      tcs34725IntegrationTime_t integrationTime;

      if (PCONFIG(0) == TCS34725_INTEGRATIONTIME_2_4MS) {
        integrationTime = TCS34725_INTEGRATIONTIME_2_4MS;
      }

      if (PCONFIG(0) == TCS34725_INTEGRATIONTIME_24MS) {
        integrationTime = TCS34725_INTEGRATIONTIME_24MS;
      }

      if (PCONFIG(0) == TCS34725_INTEGRATIONTIME_50MS) {
        integrationTime = TCS34725_INTEGRATIONTIME_50MS;
      }

      if (PCONFIG(0) == TCS34725_INTEGRATIONTIME_101MS) {
        integrationTime = TCS34725_INTEGRATIONTIME_101MS;
      }

      if (PCONFIG(0) == TCS34725_INTEGRATIONTIME_154MS) {
        integrationTime = TCS34725_INTEGRATIONTIME_154MS;
      }

      if (PCONFIG(0) == TCS34725_INTEGRATIONTIME_700MS) {
        integrationTime = TCS34725_INTEGRATIONTIME_700MS;
      }

      tcs34725Gain_t gain;

      if (PCONFIG(1) == TCS34725_GAIN_1X) {
        gain = TCS34725_GAIN_1X;
      }

      if (PCONFIG(1) == TCS34725_GAIN_4X) {
        gain = TCS34725_GAIN_4X;
      }

      if (PCONFIG(1) == TCS34725_GAIN_16X) {
        gain = TCS34725_GAIN_16X;
      }

      if (PCONFIG(1) == TCS34725_GAIN_60X) {
        gain = TCS34725_GAIN_60X;
      }

      /* Initialise with specific int time and gain values */
      Adafruit_TCS34725 tcs = Adafruit_TCS34725(integrationTime, gain);

      if (tcs.begin()) {
        addLog(LOG_LEVEL_DEBUG, F("Found TCS34725 sensor"));

        uint16_t r, g, b, c;
        float value4 = 0.0f;

        tcs.getRawData(&r, &g, &b, &c, true);
        switch (PCONFIG(3)) {
          case 0:
            value4 = tcs.calculateColorTemperature(r, g, b); // Deprecated because of deemed inaccurate calculation, kept for backward compatibility
            break;
          case 1:
            value4 = tcs.calculateColorTemperature_dn40(r, g, b, c);
            break;
          case 2:
            value4 = tcs.calculateLux(r, g, b);
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

        // Calibration factors
        float calRed   = 1.0f;
        float calGreen = 1.0f;
        float calBlue  = 1.0f;
        if (PCONFIG_FLOAT(0) > 0.0f) calRed   = PCONFIG_FLOAT(0); // TODO: Needs different compare?
        if (PCONFIG_FLOAT(1) > 0.0f) calGreen = PCONFIG_FLOAT(1);
        if (PCONFIG_FLOAT(2) > 0.0f) calBlue  = PCONFIG_FLOAT(2);

        switch (PCONFIG(2)) {
          case 0:
            UserVar[event->BaseVarIndex]     = r;
            UserVar[event->BaseVarIndex + 1] = g;
            UserVar[event->BaseVarIndex + 2] = b;
            break;
          case 1:
            if (t != 0) { // R/G/B normalized to 0..255 (but avoid divide by 0)
              UserVar[event->BaseVarIndex]     = (float)r / t * 255.0f * calRed;
              UserVar[event->BaseVarIndex + 1] = (float)g / t * 255.0f * calGreen;
              UserVar[event->BaseVarIndex + 2] = (float)b / t * 255.0f * calBlue;
            }
            break;
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
        log += String(UserVar[event->BaseVarIndex + 3], DEC);
        log += F(" R: ");
        log += String(UserVar[event->BaseVarIndex], DEC);
        log += F(" G: ");
        log += String(UserVar[event->BaseVarIndex + 1], DEC);
        log += F(" B: ");
        log += String(UserVar[event->BaseVarIndex + 2], DEC);
        addLog(LOG_LEVEL_INFO, log);
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
