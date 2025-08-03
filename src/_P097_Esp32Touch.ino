#include "_Plugin_Helper.h"
#ifdef USES_P097

// #######################################################################################################
// #################################### Plugin 097: ESP32 Touch ##########################################
// #######################################################################################################

/** Changelog:
 * 2025-06-14 tonhuisman: Add support for Custom Value Type per task value
 * 2025-01-16 tonhuisman: Move technical #defines to P097_data_struct.h to avoid Arduino compiler warning
 * 2025-01-12 tonhuisman: Add support for MQTT AutoDiscovery (not supported yet for Touch)
 * 2024-12-11 chromoxdor: Added extra routine for ESP32S2 and ESP32S3.
 *                        Added "Wake Up from Sleep", Switch like behaviour + toggle and long press option.
 */

# if defined(SOC_TOUCH_SENSOR_SUPPORTED) && SOC_TOUCH_SENSOR_SUPPORTED

#  include "src/PluginStructs/P097_data_struct.h"

#  define PLUGIN_097
#  define PLUGIN_ID_097              97
#  define PLUGIN_NAME_097            "Touch (ESP32) - internal"
#  define PLUGIN_VALUENAME1_097      "Touch"
#  define PLUGIN_VALUENAME2_097      "State"

// Share this bitmap among all instances of this plugin
DRAM_ATTR uint32_t p097_pinTouched                          = 0;
DRAM_ATTR uint32_t p097_pinTouchedLong                      = 0;
DRAM_ATTR uint32_t p097_pinTouchedPrev                      = 0;
DRAM_ATTR uint32_t p097_timestamp[LAST_TOUCH_INPUT_INDEX]   = { 0 };
DRAM_ATTR uint32_t p097_touchstart[LAST_TOUCH_INPUT_INDEX]  = { 0 };
DRAM_ATTR uint32_t p097_togglevalue[LAST_TOUCH_INPUT_INDEX] = { 0 };

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
      dev.ValueCount     = 2;
      dev.SendDataOption = true;
      dev.TimerOptional  = true;
      dev.CustomVTypeVar = true;
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
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_097));
      ExtraTaskSettings.TaskDeviceValueDecimals[0] = 0;
      ExtraTaskSettings.TaskDeviceValueDecimals[1] = 0;
      break;
    }

    #  if FEATURE_MQTT_DISCOVER || FEATURE_CUSTOM_TASKVAR_VTYPE
    case PLUGIN_GET_DISCOVERY_VTYPES:
    {
      #   if FEATURE_CUSTOM_TASKVAR_VTYPE

      for (uint8_t i = 0; i < event->Par5; ++i) {
        event->ParN[i] = ExtraTaskSettings.getTaskVarCustomVType(i);  // Custom/User selection
      }
      #   else // if FEATURE_CUSTOM_TASKVAR_VTYPE
      event->Par1 = static_cast<int>(Sensor_VType::SENSOR_TYPE_NONE); // Not yet supported
      #   endif // if FEATURE_CUSTOM_TASKVAR_VTYPE
      success = true;
      break;
    }
    #  endif // if FEATURE_MQTT_DISCOVER || FEATURE_CUSTOM_TASKVAR_VTYPE

    case PLUGIN_SET_DEFAULTS:
    {
      P097_SEND_TOUCH_EVENT    = 1;
      P097_SEND_RELEASE_EVENT  = 1;
      P097_SEND_DURATION_EVENT = 0;
      P097_LONG_PRESS_TIME     = 1000;
      P097_TOUCH_THRESHOLD     = P097_DEFAULT_TOUCH_THRESHOLD;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addRowLabel(F("Analog Pin"));
      addADC_PinSelect(AdcPinSelectPurpose::TouchOnly, F("taskdevicepin1"), CONFIG_PIN1);
      #  if (defined(ESP32S2) || defined(ESP32S3)) && !HAS_T10_TO_T14
      addFormNote(F("For now touch pins T10 to T14 are not supported!"));
      #  endif // if (defined(ESP32S2) || defined(ESP32S3)) && !HASS_T10_TO_T14

      addFormCheckBox(F("Toggle State"),          F("typetoggle"),    P097_TYPE_TOGGLE);
      addFormCheckBox(F("Send Long Press Event"), F("sendlongpress"), P097_SEND_LONG_PRESS_EVENT);
      addFormNumericBox(F("Long Press Time"), F("longpress"), P097_LONG_PRESS_TIME, 500, P097_MAX_LONGPRESS_VALUE);
      addUnit(F("500..10000 msec."));
      addFormCheckBox(F("Wake Up from Sleep"), F("sleepwakeup"), P097_SLEEP_WAKEUP);
      #  if defined(ESP32S2) || defined(ESP32S3)
      addFormNote(F("Wake up from sleep is only supported on one touch pin!"));
      #  endif // if defined(ESP32S2) || defined(ESP32S3)

      addFormSubHeader(F("Touch Settings"));

      addFormCheckBox(F("Send Touch Event"),    F("sendtouch"),    P097_SEND_TOUCH_EVENT);
      addFormCheckBox(F("Send Release Event"),  F("sendrelease"),  P097_SEND_RELEASE_EVENT);
      addFormCheckBox(F("Send Duration Event"), F("sendduration"), P097_SEND_DURATION_EVENT);
      addFormNumericBox(F("Touch Threshold"), F("threshold"), P097_TOUCH_THRESHOLD, 0, P097_MAX_THRESHOLD_VALUE);

      // Show current value
      addRowLabel(F("Current Pressure"));
      addHtmlInt(touchRead(CONFIG_PIN1));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P097_SEND_TOUCH_EVENT   = isFormItemChecked(F("sendtouch"));
      P097_SEND_RELEASE_EVENT = isFormItemChecked(F("sendrelease"));
      P097_TYPE_TOGGLE        = isFormItemChecked(F("typetoggle"));

      P097_SEND_LONG_PRESS_EVENT = isFormItemChecked(F("sendlongpress"));
      P097_LONG_PRESS_TIME       = getFormItemInt(F("longpress"));
      P097_SEND_DURATION_EVENT   = isFormItemChecked(F("sendduration"));
      P097_TOUCH_THRESHOLD       = getFormItemInt(F("threshold"));
      P097_SLEEP_WAKEUP          = isFormItemChecked(F("sleepwakeup"));
      success                    = true;
      break;
    }

    case PLUGIN_INIT:
    {
      P097_setEventParams(CONFIG_PIN1, P097_TOUCH_THRESHOLD);

      if (P097_SLEEP_WAKEUP) {
        touchSleepWakeUpEnable(CONFIG_PIN1, P097_TOUCH_THRESHOLD);
      }
      success = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      int adc, ch, t;

      if (getADC_gpio_info(CONFIG_PIN1, adc, ch, t)) {
        if (t >= 0) { // check if there is a touch pad "t" since "getADC_gpio_info" returns true even if there is no touch pad
          if (P097_SEND_LONG_PRESS_EVENT &&
              (p097_touchstart[t] >= 1) &&
              (timePassedSince(p097_touchstart[t]) >= P097_LONG_PRESS_TIME)) {
            UserVar.setFloat(event->TaskIndex, 1, 10);
            eventQueue.add(event->TaskIndex, (getTaskValueName(event->TaskIndex, 1)), 10);
            p097_touchstart[t] = 0;
          }


          if ((p097_pinTouched != 0) || (p097_pinTouchedPrev != 0)) {
            // Some pin has been touched or released.

            const bool touched = bitRead(p097_pinTouched, t);
            #  ifdef ESP32_CLASSIC
            const bool touched_prev = bitRead(p097_pinTouchedPrev, t);
            #  endif // ifdef ESP32_CLASSIC

            #  if defined(ESP32S2) || defined(ESP32S3)

            if (touched) {
              bitClear(p097_pinTouched, t);
              UserVar.setFloat(event->TaskIndex, 0, touchRead(CONFIG_PIN1));

              if (touchInterruptGetLastStatus(CONFIG_PIN1)) {
                if (p097_touchstart[t] == 0) { p097_touchstart[t] = millis(); }

                if (P097_TYPE_TOGGLE) {
                  p097_togglevalue[t] = !UserVar.getInt32(event->TaskIndex, 1);
                } else {
                  p097_togglevalue[t] = 1;
                }
                UserVar.setFloat(event->TaskIndex, 1, p097_togglevalue[t]);
                eventQueue.add(event->TaskIndex, (getTaskValueName(event->TaskIndex, 1)), p097_togglevalue[t]);

                if (P097_SEND_TOUCH_EVENT) {
                  eventQueue.add(event->TaskIndex, (getTaskValueName(event->TaskIndex, 0)), UserVar.getFloat(event->TaskIndex, 0));
                }
              } else { // Touch released
                p097_touchstart[t] = 0;

                if (!P097_TYPE_TOGGLE) {
                  UserVar.setFloat(event->TaskIndex, 1, 0);
                  eventQueue.add(event->TaskIndex, (getTaskValueName(event->TaskIndex, 1)), 0);
                } else {
                  // set only the taskvalue back to previous state after long press release
                  if (P097_SEND_LONG_PRESS_EVENT) { UserVar.setFloat(event->TaskIndex, 1, p097_togglevalue[t]); }
                }

                if (P097_SEND_RELEASE_EVENT) {
                  eventQueue.add(event->TaskIndex, (getTaskValueName(event->TaskIndex, 0)), UserVar.getFloat(event->TaskIndex, 0));
                }

                if (P097_SEND_DURATION_EVENT) {
                  if (Settings.UseRules) {
                    eventQueue.add(event->TaskIndex, F("Duration"), timePassedSince(p097_timestamp[t]));
                  }
                }

                p097_timestamp[t] = 0;
              }
            }

            #  else // if defined(ESP32S2) || defined(ESP32S3)

            if (touched) {
              bitClear(p097_pinTouched, t);
            }

            if (touched != touched_prev) {
              // state changed
              UserVar.setFloat(event->TaskIndex, 0, touchRead(CONFIG_PIN1));

              if (touched) {
                if (p097_touchstart[t] == 0) { p097_touchstart[t] = millis(); }

                if (P097_TYPE_TOGGLE) {
                  p097_togglevalue[t] = !UserVar.getInt32(event->TaskIndex, 1);
                } else {
                  p097_togglevalue[t] = 1;
                }
                UserVar.setFloat(event->TaskIndex, 1, p097_togglevalue[t]);
                eventQueue.add(event->TaskIndex, (getTaskValueName(event->TaskIndex, 1)), p097_togglevalue[t]);

                if (P097_SEND_TOUCH_EVENT) {
                  eventQueue.add(event->TaskIndex, (getTaskValueName(event->TaskIndex, 0)), UserVar.getFloat(event->TaskIndex, 0));
                }

                bitSet(p097_pinTouchedPrev, t);
              } else { // Touch released
                p097_touchstart[t] = 0;

                if (!P097_TYPE_TOGGLE) {
                  UserVar.setFloat(event->TaskIndex, 1, 0);
                  eventQueue.add(event->TaskIndex, (getTaskValueName(event->TaskIndex, 1)), 0);
                } else {
                  // set only the taskvalue back to previous state after long press release
                  if (P097_SEND_LONG_PRESS_EVENT) { UserVar.setFloat(event->TaskIndex, 1, p097_togglevalue[t]); }
                }

                if (P097_SEND_RELEASE_EVENT) {
                  eventQueue.add(event->TaskIndex, (getTaskValueName(event->TaskIndex, 0)), UserVar.getFloat(event->TaskIndex, 0));
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
            #  endif // if defined(ESP32S2) || defined(ESP32S3)
          }
          success = true;
          break;
        }
      }
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
void P097_setEventParams(int pin, uint32_t threshold) {
  int adc, ch, t;

  if (getADC_gpio_info(pin, adc, ch, t)) {
    switch (t) {
#  if HAS_T0_INPUT
      case 0: touchAttachInterrupt(T0, P097_got_T0, threshold); break;
#  endif // if HAS_T0_INPUT
      case 1: touchAttachInterrupt(T1, P097_got_T1, threshold); break;
      case 2: touchAttachInterrupt(T2, P097_got_T2, threshold); break;
      case 3: touchAttachInterrupt(T3, P097_got_T3, threshold); break;
      case 4: touchAttachInterrupt(T4, P097_got_T4, threshold); break;
      case 5: touchAttachInterrupt(T5, P097_got_T5, threshold); break;
      case 6: touchAttachInterrupt(T6, P097_got_T6, threshold); break;
      case 7: touchAttachInterrupt(T7, P097_got_T7, threshold); break;
      case 8: touchAttachInterrupt(T8, P097_got_T8, threshold); break;
      case 9: touchAttachInterrupt(T9, P097_got_T9, threshold); break;
#  if HAS_T10_TO_T14
      case 10: touchAttachInterrupt(T10, P097_got_T10, threshold); break;
      case 11: touchAttachInterrupt(T11, P097_got_T11, threshold); break;
      case 12: touchAttachInterrupt(T12, P097_got_T12, threshold); break;
      case 13: touchAttachInterrupt(T13, P097_got_T13, threshold); break;
      case 14: touchAttachInterrupt(T14, P097_got_T14, threshold); break;

#  endif // if HAS_T10_TO_T14
    }
  }
}

#  if HAS_T0_INPUT
void P097_got_T0() IRAM_ATTR;
#  endif // if HAS_T0_INPUT
void P097_got_T1() IRAM_ATTR;
void P097_got_T2() IRAM_ATTR;
void P097_got_T3() IRAM_ATTR;
void P097_got_T4() IRAM_ATTR;
void P097_got_T5() IRAM_ATTR;
void P097_got_T6() IRAM_ATTR;
void P097_got_T7() IRAM_ATTR;
void P097_got_T8() IRAM_ATTR;
void P097_got_T9() IRAM_ATTR;
#  if HAS_T10_TO_T14
void P097_got_T10() IRAM_ATTR;
void P097_got_T11() IRAM_ATTR;
void P097_got_T12() IRAM_ATTR;
void P097_got_T13() IRAM_ATTR;
void P097_got_T14() IRAM_ATTR;
#  endif // if HAS_T10_TO_T14
void P097_got_Touched(int pin) IRAM_ATTR;

#  if HAS_T0_INPUT
void P097_got_T0() {
  P097_got_Touched(0);
}

#  endif // if HAS_T0_INPUT

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

#  if HAS_T10_TO_T14
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

#  endif // if HAS_T10_TO_T14

void P097_got_Touched(int pin) {
  bitSet(p097_pinTouched, pin);

  if (p097_timestamp[pin] == 0) { p097_timestamp[pin] = millis(); }
}

# endif // if defined(SOC_TOUCH_SENSOR_SUPPORTED) && SOC_TOUCH_SENSOR_SUPPORTED


#endif // USES_P097
