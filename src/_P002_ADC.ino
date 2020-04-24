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

#define P002_NO_TOUCH            0
#define P002_TOUCH               1
#define P002_TOUCH_EVENT         2


#define P002_OVERSAMPLING        PCONFIG(0)
#define P002_READ_AS_TOUCH       PCONFIG(1)
#define P002_TOUCH_THRESHOLD     PCONFIG(2)
#define P002_CALIBRATION_ENABLED PCONFIG(3)
#define P002_CALIBRATION_POINT1  PCONFIG_LONG(0)
#define P002_CALIBRATION_POINT2  PCONFIG_LONG(1)
#define P002_CALIBRATION_VALUE1  PCONFIG_FLOAT(0)
#define P002_CALIBRATION_VALUE2  PCONFIG_FLOAT(1)

#ifdef ESP32

// Share this bitmap among all instances of this plugin
DRAM_ATTR uint32_t p002_pinTouched = 0;

#endif // ifdef ESP32

struct P002_data_struct : public PluginTaskData_base {
  P002_data_struct() {}

  ~P002_data_struct() {
    reset();
  }

  void reset() {
    OversamplingValue  = 0;
    OversamplingCount  = 0;
    OversamplingMinVal = P002_MAX_ADC_VALUE;
    OversamplingMaxVal = -P002_MAX_ADC_VALUE;
  }

  void addOversamplingValue(int currentValue) {
    // Extra check to only add min or max readings once.
    // They will be taken out of the averaging only one time.
    if ((currentValue == 0) && (currentValue == OversamplingMinVal)) {
      return;
    }

    if ((currentValue == P002_MAX_ADC_VALUE) && (currentValue == OversamplingMaxVal)) {
      return;
    }

    OversamplingValue += currentValue;
    ++OversamplingCount;

    if (currentValue > OversamplingMaxVal) {
      OversamplingMaxVal = currentValue;
    }

    if (currentValue < OversamplingMinVal) {
      OversamplingMinVal = currentValue;
    }
  }

  bool getOversamplingValue(float& float_value, int& raw_value) {
    if (OversamplingCount > 0) {
      float sum   = static_cast<float>(OversamplingValue);
      float count = static_cast<float>(OversamplingCount);

      if (OversamplingCount >= 3) {
        sum   -= OversamplingMaxVal;
        sum   -= OversamplingMinVal;
        count -= 2;
      }
      float_value = sum / count;
      raw_value   = static_cast<int16_t>(float_value);
      return true;
    }
    return false;
  }

  uint16_t OversamplingCount = 0;

private:

  int32_t OversamplingValue  = 0;
  int16_t OversamplingMinVal = P002_MAX_ADC_VALUE;
  int16_t OversamplingMaxVal = 0;
};

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
      addADC_PinSelect(F("taskdevicepin1"), CONFIG_PIN1);

      {
        String options[3] = { F("No Touch"), F("Touch"), F("Touch + Event") };
        int    values[3]  = { P002_NO_TOUCH, P002_TOUCH, P002_TOUCH_EVENT };
        addFormSelector(F("Read as Touch Sensor"), F("p002_readtouch"), 3, options, values, P002_READ_AS_TOUCH);
      }

      addFormNumericBox(F("Touch Threshold"), F("p002_threshold"), P002_TOUCH_THRESHOLD, 0, P002_MAX_ADC_VALUE);
      addFormNote(F("Used to trigger an event"));

      #endif // if defined(ESP32)

      addFormCheckBox(F("Oversampling"), F("p002_oversampling"), P002_OVERSAMPLING);

      addFormSubHeader(F("Two Point Calibration"));

      addFormCheckBox(F("Calibration Enabled"), F("p002_cal"), P002_CALIBRATION_ENABLED);

      addFormNumericBox(F("Point 1"), F("p002_adc1"), P002_CALIBRATION_POINT1, 0, P002_MAX_ADC_VALUE);
      html_add_estimate_symbol();
      addTextBox(F("p002_out1"), String(P002_CALIBRATION_VALUE1, 3), 10);

      addFormNumericBox(F("Point 2"), F("p002_adc2"), P002_CALIBRATION_POINT2, 0, P002_MAX_ADC_VALUE);
      html_add_estimate_symbol();
      addTextBox(F("p002_out2"), String(P002_CALIBRATION_VALUE2, 3), 10);

      {
        // Output the statistics for the current settings.
        int   raw_value = 0;
        float value;

        if (P002_getOutputValue(event, raw_value, value)) {
          P002_formatStatistics(F("Current"), raw_value, value);
        }

        if (P002_CALIBRATION_ENABLED) {
          P002_formatStatistics(F("Minimum"),   0,                  P002_applyCalibration(event, 0));
          P002_formatStatistics(F("Maximum"),   P002_MAX_ADC_VALUE, P002_applyCalibration(event, P002_MAX_ADC_VALUE));

          float stepsize = P002_applyCalibration(event, 1.0) - P002_applyCalibration(event, 0.0);
          P002_formatStatistics(F("Step size"), 1,                               stepsize);
        }
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      #ifdef ESP32
      P002_READ_AS_TOUCH   = getFormItemInt(F("p002_readtouch"));
      P002_TOUCH_THRESHOLD = getFormItemInt(F("p002_threshold"));
      #endif // ifdef ESP32
      P002_OVERSAMPLING = isFormItemChecked(F("p002_oversampling"));

      P002_CALIBRATION_ENABLED = isFormItemChecked(F("p002_cal"));

      P002_CALIBRATION_POINT1 = getFormItemInt(F("p002_adc1"));
      P002_CALIBRATION_VALUE1 = getFormItemFloat(F("p002_out1"));

      P002_CALIBRATION_POINT2 = getFormItemInt(F("p002_adc2"));
      P002_CALIBRATION_VALUE2 = getFormItemFloat(F("p002_out2"));

      success = true;
      break;
    }

    case PLUGIN_EXIT: {
      clearPluginTaskData(event->TaskIndex);
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new P002_data_struct());
      P002_data_struct *P002_data =
        static_cast<P002_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P002_data);
      #ifdef ESP32

      if (P002_READ_AS_TOUCH == P002_TOUCH_EVENT) {
        P002_setEventParams(CONFIG_PIN1, P002_TOUCH_THRESHOLD);
      }
      #endif // ifdef ESP32
      break;
    }
    case PLUGIN_TEN_PER_SECOND:
    {
      if (P002_OVERSAMPLING) // Oversampling
      {
        P002_data_struct *P002_data =
          static_cast<P002_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P002_data) {
          int currentValue;

          P002_performRead(event, currentValue);
          P002_data->addOversamplingValue(currentValue);
        }
      }
      #ifdef ESP32

      if (p002_pinTouched != 0) {
        // Some pin has been touched. Try to find which pin it was and send event.
        for (unsigned int i = 0; i < 10; ++i) {
          if (p002_pinTouched & (1 << i)) {
            p002_pinTouched &= ~(1 << i); // reset bit

            if (Settings.UseRules) {
              String eventString = F("ADCtouch=");
              eventString += i;
              eventQueue.add(eventString);
            }
          }
        }
      }
      #endif // ifdef ESP32
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      int   raw_value = 0;
      float res_value = 0.0;

      if (P002_getOutputValue(event, raw_value, res_value)) {
        UserVar[event->BaseVarIndex] = res_value;

        P002_data_struct *P002_data =
          static_cast<P002_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P002_data) {
          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("ADC  : Analog value: ");
            log += String(raw_value);
            log += F(" = ");
            log += String(UserVar[event->BaseVarIndex], 3);

            if (P002_OVERSAMPLING) {
              log += F(" (");
              log += P002_data->OversamplingCount;
              log += F(" samples)");
            }
            addLog(LOG_LEVEL_INFO, log);
          }
          P002_data->reset();
          success = true;
        } else {
          addLog(LOG_LEVEL_ERROR, F("ADC  : No value received "));
          success = false;
        }
      }

      break;
    }
  }
  return success;
}

bool P002_getOutputValue(struct EventStruct *event, int& raw_value, float& res_value) {
  P002_data_struct *P002_data =
    static_cast<P002_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P002_data) {
    return false;
  }
  float float_value = 0.0;

  bool valueRead = P002_OVERSAMPLING && P002_data->getOversamplingValue(float_value, raw_value);

  if (!valueRead) {
    P002_performRead(event, raw_value);
    float_value = static_cast<float>(raw_value);
  }

  res_value = P002_applyCalibration(event, float_value);
  return true;
}

float P002_applyCalibration(struct EventStruct *event, float float_value) {
  if (P002_CALIBRATION_ENABLED) // Calibration?
  {
    int   adc1 = P002_CALIBRATION_POINT1;
    int   adc2 = P002_CALIBRATION_POINT2;
    float out1 = P002_CALIBRATION_VALUE1;
    float out2 = P002_CALIBRATION_VALUE2;

    if (adc1 != adc2)
    {
      const float normalized = static_cast<float>(float_value - adc1) / static_cast<float>(adc2 - adc1);
      float_value = normalized * (out2 - out1) + out1;
    }
  }
  return float_value;
}

void P002_performRead(struct EventStruct *event, int& value) {
  #ifdef ESP8266
  value = espeasy_analogRead(A0);
  #endif // if defined(ESP8266)
  #if defined(ESP32)
  value = espeasy_analogRead(CONFIG_PIN1, P002_READ_AS_TOUCH != P002_NO_TOUCH);
  #endif // if defined(ESP32)
}

void P002_formatStatistics(const String& label, int raw, float float_value) {
  addRowLabel(label);
  addHtml(String(raw));
  html_add_estimate_symbol();
  addHtml(String(float_value, 3));
}

#ifdef ESP32

/**********************************************************************************
* Touch pin configuration
**********************************************************************************/
void P002_setEventParams(int pin, uint16_t threshold) {
  int adc, ch, t;

  if (getADC_gpio_info(pin, adc, ch, t)) {
    switch (t) {
      case 0: touchAttachInterrupt(T0, P002_got_T0, threshold); break;
      case 1: touchAttachInterrupt(T1, P002_got_T1, threshold); break;
      case 2: touchAttachInterrupt(T2, P002_got_T2, threshold); break;
      case 3: touchAttachInterrupt(T3, P002_got_T3, threshold); break;
      case 4: touchAttachInterrupt(T4, P002_got_T4, threshold); break;
      case 5: touchAttachInterrupt(T5, P002_got_T5, threshold); break;
      case 6: touchAttachInterrupt(T6, P002_got_T6, threshold); break;
      case 7: touchAttachInterrupt(T7, P002_got_T7, threshold); break;
      case 8: touchAttachInterrupt(T8, P002_got_T8, threshold); break;
      case 9: touchAttachInterrupt(T9, P002_got_T9, threshold); break;
    }
  }
}

void P002_got_T0() IRAM_ATTR;
void P002_got_T1() IRAM_ATTR;
void P002_got_T2() IRAM_ATTR;
void P002_got_T3() IRAM_ATTR;
void P002_got_T4() IRAM_ATTR;
void P002_got_T5() IRAM_ATTR;
void P002_got_T6() IRAM_ATTR;
void P002_got_T7() IRAM_ATTR;
void P002_got_T8() IRAM_ATTR;
void P002_got_T9() IRAM_ATTR;

void P002_got_T0() { p002_pinTouched |= (1 << 0); }
void P002_got_T1() { p002_pinTouched |= (1 << 1); }
void P002_got_T2() { p002_pinTouched |= (1 << 2); }
void P002_got_T3() { p002_pinTouched |= (1 << 3); }
void P002_got_T4() { p002_pinTouched |= (1 << 4); }
void P002_got_T5() { p002_pinTouched |= (1 << 5); }
void P002_got_T6() { p002_pinTouched |= (1 << 6); }
void P002_got_T7() { p002_pinTouched |= (1 << 7); }
void P002_got_T8() { p002_pinTouched |= (1 << 8); }
void P002_got_T9() { p002_pinTouched |= (1 << 9); }


#endif // ifdef ESP32


#endif // USES_P002
