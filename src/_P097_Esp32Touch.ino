#include "_Plugin_Helper.h"
#ifdef USES_P097

// #######################################################################################################
// #################################### Plugin 097: ESP32 Touch ##########################################
// #######################################################################################################


#if defined(SOC_TOUCH_SENSOR_SUPPORTED) && SOC_TOUCH_SENSOR_SUPPORTED

# define LAST_TOUCH_INPUT_INDEX    SOC_TOUCH_SENSOR_NUM

#ifdef ESP32_CLASSIC
  # define HAS_T0_INPUT  1
  # define HAS_T10_TO_T14 0
//  # define LAST_TOUCH_INPUT_INDEX 10
#elif defined(ESP32S2) || defined(ESP32S3)
  # define HAS_T0_INPUT  0
  # define HAS_T10_TO_T14 1
//  # define LAST_TOUCH_INPUT_INDEX 15
#else
  static_assert(false, "Implement processor architecture");
#endif



# define PLUGIN_097
# define PLUGIN_ID_097         97
# define PLUGIN_NAME_097       "Touch (ESP32) - internal"
# define PLUGIN_VALUENAME1_097 "Touch"


# ifdef ESP32
  #  define P097_MAX_ADC_VALUE    4095
# endif // ifdef ESP32
# ifdef ESP8266
  #  define P097_MAX_ADC_VALUE    1023
# endif // ifdef ESP8266


# define P097_SEND_TOUCH_EVENT    PCONFIG(0)
# define P097_SEND_RELEASE_EVENT  PCONFIG(1)
# define P097_SEND_DURATION_EVENT PCONFIG(2)
# define P097_TOUCH_THRESHOLD     PCONFIG(3)

// Share this bitmap among all instances of this plugin
DRAM_ATTR uint32_t p097_pinTouched     = 0;
DRAM_ATTR uint32_t p097_pinTouchedPrev = 0;
DRAM_ATTR uint32_t p097_timestamp[LAST_TOUCH_INPUT_INDEX]  = { 0 };

boolean Plugin_097(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_097;
      dev.Type           = DEVICE_TYPE_ANALOG;
      dev.VType          = Sensor_VType::SENSOR_TYPE_SINGLE;
      dev.FormulaOption  = true;
      dev.ValueCount     = 1;
      dev.SendDataOption = true;
      dev.TimerOptional  = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_097);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_097));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P097_SEND_TOUCH_EVENT = 1;
      P097_SEND_DURATION_EVENT = 1;
      P097_TOUCH_THRESHOLD = 20;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addRowLabel(F("Analog Pin"));
      addADC_PinSelect(AdcPinSelectPurpose::TouchOnly, F("taskdevicepin1"), CONFIG_PIN1);

      addFormSubHeader(F("Touch Settings"));

      addFormCheckBox(F("Send Touch Event"),    F("sendtouch"),    P097_SEND_TOUCH_EVENT);
      addFormCheckBox(F("Send Release Event"),  F("sendrelease"),  P097_SEND_RELEASE_EVENT);
      addFormCheckBox(F("Send Duration Event"), F("sendduration"), P097_SEND_DURATION_EVENT);
      addFormNumericBox(F("Touch Threshold"), F("threshold"), P097_TOUCH_THRESHOLD, 0, P097_MAX_ADC_VALUE);

      // Show current value
      addRowLabel(F("Current Pressure"));
      addHtmlInt(touchRead(CONFIG_PIN1));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P097_SEND_TOUCH_EVENT    = isFormItemChecked(F("sendtouch"));
      P097_SEND_RELEASE_EVENT  = isFormItemChecked(F("sendrelease"));
      P097_SEND_DURATION_EVENT = isFormItemChecked(F("sendduration"));
      P097_TOUCH_THRESHOLD     = getFormItemInt(F("threshold"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      P097_setEventParams(CONFIG_PIN1, P097_TOUCH_THRESHOLD);
      success = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      if ((p097_pinTouched != 0) || (p097_pinTouchedPrev != 0)) {
        // Some pin has been touched or released.
        // Check if it is 'our' pin
        int adc, ch, t;

        if (getADC_gpio_info(CONFIG_PIN1, adc, ch, t)) {
          const bool touched      = bitRead(p097_pinTouched, t);
          const bool touched_prev = bitRead(p097_pinTouchedPrev, t);

          if (touched) {
            bitClear(p097_pinTouched, t);
          }

          if (touched != touched_prev) {
            // state changed
            UserVar.setFloat(event->TaskIndex, 0, touchRead(CONFIG_PIN1));

            if (touched) {
              if (P097_SEND_TOUCH_EVENT) {
                // schedule a read to update output values and send to controllers
                Scheduler.schedule_task_device_timer(event->TaskIndex, millis());
              }
              bitSet(p097_pinTouchedPrev, t);
            } else {
              if (P097_SEND_RELEASE_EVENT) {
                // schedule a read to update output values and send to controllers
                Scheduler.schedule_task_device_timer(event->TaskIndex, millis());
              }

              if (P097_SEND_DURATION_EVENT) {
                if (Settings.UseRules) {
                  eventQueue.add(event->TaskIndex, F("Duration"), timePassedSince(p097_timestamp[t]));
                }
              }
              bitClear(p097_pinTouchedPrev, t);
              p097_timestamp[t] = 0;
            }
          }
        }
      }
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      int raw_value = touchRead(CONFIG_PIN1);
      UserVar.setFloat(event->TaskIndex, 0, raw_value);

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLogMove(LOG_LEVEL_INFO, strformat(F("Touch : %s: %d"), formatGpioName_ADC(CONFIG_PIN1).c_str(), raw_value));
      }
      success = true;
      break;
    }
  }
  return success;
}

/**********************************************************************************
* Touch pin callback functions
**********************************************************************************/
void P097_setEventParams(int pin, uint16_t threshold) {
  int adc, ch, t;

  if (getADC_gpio_info(pin, adc, ch, t)) {
    switch (t) {
#if HAS_T0_INPUT
      case 0: touchAttachInterrupt(T0, P097_got_T0, threshold); break;
#endif
      case 1: touchAttachInterrupt(T1, P097_got_T1, threshold); break;
      case 2: touchAttachInterrupt(T2, P097_got_T2, threshold); break;
      case 3: touchAttachInterrupt(T3, P097_got_T3, threshold); break;
      case 4: touchAttachInterrupt(T4, P097_got_T4, threshold); break;
      case 5: touchAttachInterrupt(T5, P097_got_T5, threshold); break;
      case 6: touchAttachInterrupt(T6, P097_got_T6, threshold); break;
      case 7: touchAttachInterrupt(T7, P097_got_T7, threshold); break;
      case 8: touchAttachInterrupt(T8, P097_got_T8, threshold); break;
      case 9: touchAttachInterrupt(T9, P097_got_T9, threshold); break;
#if HAS_T10_TO_T14
/*
      case 10: touchAttachInterrupt(T10, P097_got_T10, threshold); break;
      case 11: touchAttachInterrupt(T11, P097_got_T11, threshold); break;
      case 12: touchAttachInterrupt(T12, P097_got_T12, threshold); break;
      case 13: touchAttachInterrupt(T13, P097_got_T13, threshold); break;
      case 14: touchAttachInterrupt(T14, P097_got_T14, threshold); break;
*/
#endif
    }
  }
}

#if HAS_T0_INPUT
void P097_got_T0() IRAM_ATTR;
#endif
void P097_got_T1() IRAM_ATTR;
void P097_got_T2() IRAM_ATTR;
void P097_got_T3() IRAM_ATTR;
void P097_got_T4() IRAM_ATTR;
void P097_got_T5() IRAM_ATTR;
void P097_got_T6() IRAM_ATTR;
void P097_got_T7() IRAM_ATTR;
void P097_got_T8() IRAM_ATTR;
void P097_got_T9() IRAM_ATTR;
#if HAS_T10_TO_T14
void P097_got_T10() IRAM_ATTR;
void P097_got_T11() IRAM_ATTR;
void P097_got_T12() IRAM_ATTR;
void P097_got_T13() IRAM_ATTR;
void P097_got_T14() IRAM_ATTR;
#endif
void P097_got_Touched(int pin) IRAM_ATTR;

#if HAS_T0_INPUT
void P097_got_T0() {
  P097_got_Touched(0);
}
#endif

void P097_got_T1() {
  P097_got_Touched(1);
}

void P097_got_T2() {
  P097_got_Touched(2);
}

void P097_got_T3() {
  P097_got_Touched(3);
}

void P097_got_T4() {
  P097_got_Touched(4);
}

void P097_got_T5() {
  P097_got_Touched(6);
}

void P097_got_T6() {
  P097_got_Touched(6);
}

void P097_got_T7() {
  P097_got_Touched(7);
}

void P097_got_T8() {
  P097_got_Touched(8);
}

void P097_got_T9() {
  P097_got_Touched(9);
}

#if HAS_T10_TO_T14
void P097_got_T10() {
  P097_got_Touched(10);
}

void P097_got_T11() {
  P097_got_Touched(11);
}

void P097_got_T12() {
  P097_got_Touched(12);
}

void P097_got_T13() {
  P097_got_Touched(13);
}

void P097_got_T14() {
  P097_got_Touched(14);
}

#endif

void P097_got_Touched(int pin) {
  bitSet(p097_pinTouched, pin);

  if (p097_timestamp[pin] == 0) { p097_timestamp[pin] = millis(); }
}
#endif 


#endif // USES_P097
