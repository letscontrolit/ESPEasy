#ifdef USES_P002

// #######################################################################################################
// #################################### Plugin 002: Analog ###############################################
// #######################################################################################################

#define PLUGIN_002
#define PLUGIN_ID_002         2
#define PLUGIN_NAME_002       "Analog input - internal"
#define PLUGIN_VALUENAME1_002 "Analog"

#include "_Plugin_Helper.h"

#ifdef ESP32
  # define P002_MAX_ADC_VALUE    4095
#endif // ifdef ESP32
#ifdef ESP8266
  # define P002_MAX_ADC_VALUE    1023
#endif // ifdef ESP8266

uint32_t Plugin_002_OversamplingValue  = 0;
uint16_t Plugin_002_OversamplingCount  = 0;
uint16_t Plugin_002_OversamplingMinVal = P002_MAX_ADC_VALUE;
uint16_t Plugin_002_OversamplingMaxVal = 0;

boolean Plugin_002(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_002;
      Device[deviceCount].Type               = DEVICE_TYPE_ANALOG;
      Device[deviceCount].VType              = SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_002);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_002));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
        #if defined(ESP32)
      addHtml(F("<TR><TD>Analog Pin:<TD>"));
      addPinSelect(false, F("taskdevicepin1"), CONFIG_PIN1);
        #endif // if defined(ESP32)

      addFormCheckBox(F("Oversampling"), F("p002_oversampling"), PCONFIG(0));

      addFormSubHeader(F("Two Point Calibration"));

      addFormCheckBox(F("Calibration Enabled"), F("p002_cal"), PCONFIG(3));

      addFormNumericBox(F("Point 1"), F("p002_adc1"), PCONFIG_LONG(0), 0, P002_MAX_ADC_VALUE);
      html_add_estimate_symbol();
      addTextBox(F("p002_out1"), String(PCONFIG_FLOAT(0), 3), 10);

      addFormNumericBox(F("Point 2"), F("p002_adc2"), PCONFIG_LONG(1), 0, P002_MAX_ADC_VALUE);
      html_add_estimate_symbol();
      addTextBox(F("p002_out2"), String(PCONFIG_FLOAT(1), 3), 10);

      {
        // Output the statistics for the current settings.
        uint16_t raw_value = 0;
        float value;
        if (P002_getOutputValue(event, raw_value, value)) {
          P002_formatStatistics(F("Current"), raw_value, value);
        }

        if (PCONFIG(3)) {
          P002_formatStatistics(F("Minimum"),   0,                  P002_applyCalibration(event, 0));
          P002_formatStatistics(F("Maximum"),   P002_MAX_ADC_VALUE, P002_applyCalibration(event, P002_MAX_ADC_VALUE));

          float stepsize = P002_applyCalibration(event, 1.0) - P002_applyCalibration(event, 0.0);
          P002_formatStatistics(F("Step size"), 1,                  stepsize);
        }
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = isFormItemChecked(F("p002_oversampling"));

      PCONFIG(3) = isFormItemChecked(F("p002_cal"));

      PCONFIG_LONG(0)  = getFormItemInt(F("p002_adc1"));
      PCONFIG_FLOAT(0) = getFormItemFloat(F("p002_out1"));

      PCONFIG_LONG(1)  = getFormItemInt(F("p002_adc2"));
      PCONFIG_FLOAT(1) = getFormItemFloat(F("p002_out2"));

      success = true;
      break;
    }

    case PLUGIN_INIT:  
      if (!PCONFIG(0)) // Oversampling
      {
        // Read at least one value, see https://github.com/letscontrolit/ESPEasy/issues/2646
        uint16_t currentValue;
        P002_performRead(event, currentValue);
      }
      // Fall through to PLUGIN_TEN_PER_SECOND
    case PLUGIN_TEN_PER_SECOND:
    {
      if (PCONFIG(0)) // Oversampling
      {
        uint16_t currentValue;

        if (P002_performRead(event, currentValue)) {
          Plugin_002_OversamplingValue += currentValue;
          ++Plugin_002_OversamplingCount;

          if (currentValue > Plugin_002_OversamplingMaxVal) {
            Plugin_002_OversamplingMaxVal = currentValue;
          }

          if (currentValue < Plugin_002_OversamplingMinVal) {
            Plugin_002_OversamplingMinVal = currentValue;
          }
        }
      }
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      uint16_t raw_value = 0;
      float res_value = 0.0;
      if (P002_getOutputValue(event, raw_value, res_value)) {;
        UserVar[event->BaseVarIndex] = res_value;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("ADC  : Analog value: ");
          log += String(raw_value);
          log += F(" = ");
          log += String(UserVar[event->BaseVarIndex], 3);

          if (PCONFIG(0)) {
            log += F(" (");
            log += Plugin_002_OversamplingCount;
            log += F(" samples)");
          }
        addLog(LOG_LEVEL_INFO, log);
        }
        // Now reset the oversampling variables.
        Plugin_002_OversamplingValue  = 0;
        Plugin_002_OversamplingCount  = 0;
        Plugin_002_OversamplingMinVal = P002_MAX_ADC_VALUE;
        Plugin_002_OversamplingMaxVal = 0;
        success                       = true;
      } else {
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log = F("ADC  : No value received ");
          addLog(LOG_LEVEL_ERROR, log);
        }
        success = false;
      }

      break;
    }
  }
  return success;
}

bool P002_getOutputValue(struct EventStruct *event, uint16_t& raw_value, float& res_value) {
  float float_value = 0.0;
  bool success;

  if (PCONFIG(0) && (Plugin_002_OversamplingCount > 0)) {
    float sum   = static_cast<float>(Plugin_002_OversamplingValue);
    float count = static_cast<float>(Plugin_002_OversamplingCount);

    if (Plugin_002_OversamplingCount >= 3) {
      sum   -= Plugin_002_OversamplingMaxVal;
      sum   -= Plugin_002_OversamplingMinVal;
      count -= 2;
    }
    float_value = sum / count;
    raw_value   = static_cast<int16_t>(float_value);
    success = true;
  } else {
    if (P002_performRead(event, raw_value)) {
      float_value = static_cast<float>(raw_value);
      success = true;
    } else {
      success = false;
    }
  }
  if (success) res_value = P002_applyCalibration(event, float_value);
  return success;
}

float P002_applyCalibration(struct EventStruct *event, float float_value) {
  if (PCONFIG(3)) // Calibration?
  {
    int   adc1 = PCONFIG_LONG(0);
    int   adc2 = PCONFIG_LONG(1);
    float out1 = PCONFIG_FLOAT(0);
    float out2 = PCONFIG_FLOAT(1);

    if (adc1 != adc2)
    {
      const float normalized = static_cast<float>(float_value - adc1) / static_cast<float>(adc2 - adc1);
      float_value = normalized * (out2 - out1) + out1;
    }
  }
  return float_value;
}

bool P002_performRead(struct EventStruct *event, uint16_t& value) {
  // Define it static so we just return the last value when no read can be performed.
  static uint16_t last_value = 0;
  bool valid                 = true;

  #if defined(ESP8266)

  if (!wifiConnectInProgress) {
    last_value = analogRead(A0);
  } else {
    valid = false;
  }
  #endif // if defined(ESP8266)
  #if defined(ESP32)
  last_value = analogRead(CONFIG_PIN1);
  #endif // if defined(ESP32)
  value = last_value;
  return valid;
}

void P002_formatStatistics(const String& label, int16_t raw, float float_value) {
  addRowLabel(label);
  addHtml(String(raw));
  html_add_estimate_symbol();
  addHtml(String(float_value, 3));
}

#endif // USES_P002
