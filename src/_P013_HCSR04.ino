#include "_Plugin_Helper.h"

#ifdef USES_P013

// #######################################################################################################
// ############################### Plugin 013: HC-SR04, RCW-0001, etc. ###################################
// #######################################################################################################

/** Changelog:
 * 2023-02-25 tonhuisman: Make Interval optional, and also disable the added feature P013_FEATURE_INTERVALEVENT, as setting Interval
 *                        to 0 is effectively the same. (Small code reduction)
 *                        Changed second value label for Combined mode to State
 * 2023-02-19 tonhuisman: Suggested modification with variable trigger width was only partly accepted by NewPing library, and as
 *                        we already have a modified library using the DIRECT_Gpio functions, I'm not merging back that change
 *                        (compile-time setting, default now: 12 usec) but keep the proposed changes for a runtime configurable setting.
 * 2023-01-28 tonhuisman: Add Combined mode, as started in https://github.com/letscontrolit/ESPEasy/pull/3157
 * 2023-01-20 tonhuisman: Limit trigger-range to 10-20 usec. (20 already seems to be on the high side)
 *                        Reduce build-size by disabling new features on 1M builds and leaving out some non-essential messages
 *                        Move #define stuff and #includes to src/PluginStructs/P013_data_struct.h
 * 2022-12-31 tonhuisman: Code improvements, change start-trigger range to 10-50 usec.
 *                        Optionally not send regular Interval events when using State mode
 * 2022-12-29 tonhuisman: Add start-trigger setting, range 10-30 usec. See https://github.com/letscontrolit/ESPEasy/issues/3857
 * 2022-12-29 tonhuisman: Add changelog
 */

# define PLUGIN_013
# define PLUGIN_ID_013         13
# define PLUGIN_NAME_013       "Position - HC-SR04, RCW-0001, etc."
# define PLUGIN_VALUENAME1_013 "Distance"
# define PLUGIN_VALUENAME2_013 "State"

# include "src/PluginStructs/P013_data_struct.h"

// map of sensors
std::map<unsigned int, std::shared_ptr<NewPing> > P_013_sensordefs;

// Forward declarations
float                      Plugin_013_read(struct EventStruct *event);
const __FlashStringHelper* Plugin_013_getErrorStatusString(struct EventStruct *event);

boolean                    Plugin_013(uint8_t function, struct EventStruct *event, String& string)
{
  static uint8_t switchstate[TASKS_MAX];
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number         = PLUGIN_ID_013;
      Device[deviceCount].Type             = DEVICE_TYPE_DUAL;
      Device[deviceCount].VType            = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports            = 0;
      Device[deviceCount].FormulaOption    = true;
      Device[deviceCount].ValueCount       = 1;
      Device[deviceCount].SendDataOption   = true;
      Device[deviceCount].TimerOption      = true;
      Device[deviceCount].TimerOptional    = true;
      Device[deviceCount].GlobalSyncOption = true;
      Device[deviceCount].PluginStats      = true;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_013);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_013));
      # if P013_FEATURE_COMBINED_MODE
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_013));
      # endif // if P013_FEATURE_COMBINED_MODE
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("Trigger"));
      event->String2 = formatGpioName_input(F("Echo, 5V"));
      break;
    }

    # if P013_FEATURE_COMBINED_MODE
    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P013_OPERATINGMODE == OPMODE_COMBINED ? 2 : 1;
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(P013_OPERATINGMODE == OPMODE_COMBINED ? 2 : 1);
      event->idx        = P013_OPERATINGMODE == OPMODE_COMBINED ? 2 : 1;
      success           = true;
      break;
    }
    # endif // if P013_FEATURE_COMBINED_MODE

    case PLUGIN_SET_DEFAULTS:
    {
      P013_FILTER_SIZE = P013_DEFAULT_FILTER_SIZE;
      # if P013_FEATURE_TRIGGERWIDTH
      P013_TRIGGER_WIDTH = P013_DEFAULT_TRIGGER_WIDTH;
      # endif // if P013_FEATURE_TRIGGERWIDTH

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      const __FlashStringHelper *strUnit = (P013_MEASURINGUNIT == UNIT_CM) ? F("cm") : F("inch");

      {
        const int optionValuesOpMode[] = {
          OPMODE_VALUE,
          OPMODE_STATE,
          # if P013_FEATURE_COMBINED_MODE
          OPMODE_COMBINED,
          # endif // if P013_FEATURE_COMBINED_MODE
        };
        const __FlashStringHelper *optionsOpMode[] {
          F("Value"),
          F("State"),
          # if P013_FEATURE_COMBINED_MODE
          F("Combined"),
          # endif // if P013_FEATURE_COMBINED_MODE
        };
        addFormSelector(F("Mode"), F("pmode"),
                        # if P013_FEATURE_COMBINED_MODE
                        3
                        # else // if P013_FEATURE_COMBINED_MODE
                        2
                        # endif // if P013_FEATURE_COMBINED_MODE
                        , optionsOpMode, optionValuesOpMode, P013_OPERATINGMODE);
      }

      if ((P013_OPERATINGMODE == OPMODE_STATE)
          # if P013_FEATURE_COMBINED_MODE
          || (P013_OPERATINGMODE == OPMODE_COMBINED)
          # endif // if P013_FEATURE_COMBINED_MODE
          ) {
        # if P013_FEATURE_INTERVALEVENT
        addFormCheckBox(F("State event (also) on Interval"), F("pevent"), P013_SEND_STATE_VALUE == 0);
        # endif // if P013_FEATURE_INTERVALEVENT
        addFormNumericBox(F("Threshold"), F("thres"), P013_THRESHOLD);
        addUnit(strUnit);
      }
      addFormNumericBox(F("Max Distance"), F("max_d"), P013_MAX_DISTANCE, 0, 500);
      addUnit(strUnit);

      {
        const int optionValuesUnit[2] = { UNIT_CM, UNIT_INCH };
        const __FlashStringHelper *optionsUnit[] {
          F("Metric"),
          F("Imperial"),
        };
        addFormSelector(F("Unit"), F("pUnit"), 2, optionsUnit, optionValuesUnit, P013_MEASURINGUNIT);
      }

      {
        const int optionValuesFilter[2] = { FILTER_NONE, FILTER_MEDIAN };
        const __FlashStringHelper *optionsFilter[] {
          F("None"),
          F("Median"),
        };
        addFormSelector(F("Filter"), F("fltr"), 2, optionsFilter, optionValuesFilter, P013_FILTERTYPE);
      }

      // enable filtersize option if filter is used,
      if (P013_FILTERTYPE != FILTER_NONE) {
        addFormNumericBox(F("Number of Pings"), F("size"), P013_FILTER_SIZE, 2, 20);
        # if P013_EXTENDED_LOG
        addUnit(F("2..20"));
        # endif // if P013_EXTENDED_LOG
      }

      # if P013_FEATURE_TRIGGERWIDTH
      addFormNumericBox(F("Trigger width"), F("wdth"), P013_TRIGGER_WIDTH, 10, 20);
      addUnit(F("10..20 &micro;sec"));
      # endif // if P013_FEATURE_TRIGGERWIDTH

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      int16_t prevOperatingMode = P013_OPERATINGMODE;
      int16_t prevFilterType    = P013_FILTERTYPE;

      P013_OPERATINGMODE = getFormItemInt(F("pmode"));

      if ((prevOperatingMode == OPMODE_STATE)
          # if P013_FEATURE_COMBINED_MODE
          || (prevOperatingMode == OPMODE_COMBINED)
          # endif // if P013_FEATURE_COMBINED_MODE
          ) {
        # if P013_FEATURE_INTERVALEVENT
        P013_SEND_STATE_VALUE = isFormItemChecked(F("pevent")) ? 0 : 1; // Inverted state
        # endif // if P013_FEATURE_INTERVALEVENT
        P013_THRESHOLD = getFormItemInt(F("thres"));
      }
      # if P013_FEATURE_COMBINED_MODE

      if ((P013_OPERATINGMODE == OPMODE_COMBINED) && (ExtraTaskSettings.TaskDeviceValueNames[1][0] == '\0')) {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_013));
      }
      # endif // if P013_FEATURE_COMBINED_MODE
      P013_MAX_DISTANCE = getFormItemInt(F("max_d"));

      P013_MEASURINGUNIT = getFormItemInt(F("pUnit"));
      P013_FILTERTYPE    = getFormItemInt(F("fltr"));

      if (prevFilterType != FILTER_NONE) {
        P013_FILTER_SIZE = getFormItemInt(F("size"));
      }
      # if P013_FEATURE_TRIGGERWIDTH
      P013_TRIGGER_WIDTH = getFormItemInt(F("wdth"));
      # endif // if P013_FEATURE_TRIGGERWIDTH

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (P013_FILTER_SIZE == 0) { P013_FILTER_SIZE = P013_DEFAULT_FILTER_SIZE; }

      # if P013_FEATURE_TRIGGERWIDTH

      if (P013_TRIGGER_WIDTH == 0) { P013_TRIGGER_WIDTH = P013_DEFAULT_TRIGGER_WIDTH; }
      # endif // if P013_FEATURE_TRIGGERWIDTH

      int16_t max_distance_cm = (P013_MEASURINGUNIT == UNIT_CM) ? P013_MAX_DISTANCE : static_cast<float>(P013_MAX_DISTANCE) * 2.54f;

      // create sensor instance and add to std::map
      P_013_sensordefs.erase(event->TaskIndex);
      P_013_sensordefs[event->TaskIndex] = std::shared_ptr<NewPing>(new NewPing(P013_TRIGGER_PIN,
                                                                                P013_ECHO_PIN,
                                                                                max_distance_cm,
                                                                                P013_TRIGGER_WIDTH));
      success = true;

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("ULTRASONIC : TaskNr: ");
        log += event->TaskIndex + 1;
        log += F(" TrigPin: ");
        log += P013_TRIGGER_PIN;
        log += F(" IRQ_Pin: ");
        log += P013_ECHO_PIN;

        if (nullptr != P_013_sensordefs[event->TaskIndex]) { // Initialization successful
          # if P013_EXTENDED_LOG
          log += F(" width [usec]: ");
          log += P013_TRIGGER_WIDTH;
          log += F(" max dist ");
          log += (P013_MEASURINGUNIT == UNIT_CM) ? F("[cm]: ") : F("[inch]: ");
          log += P013_MAX_DISTANCE;

          log += F(" max echo: ");
          log += P_013_sensordefs[event->TaskIndex]->getMaxEchoTime();
          log += F(" Filter: ");

          if (P013_FILTERTYPE == FILTER_NONE) {
            log += F("none");
          }
          else if (P013_FILTERTYPE == FILTER_MEDIAN) {
            log += F("Median size: ");
            log += P013_FILTER_SIZE;
          } else {
            log += F("invalid!");
          }

          log += F(" nr_tasks: ");
          log += P_013_sensordefs.size();
          # endif // if P013_EXTENDED_LOG
        } else {
          log    += F(" CONSTRUCTOR FAILED!");
          success = false; // Initialization failed
        }
        addLogMove(LOG_LEVEL_INFO, log);
      }

      break;
    }

    case PLUGIN_EXIT:
    {
      P_013_sensordefs.erase(event->TaskIndex);
      break;
    }

    case PLUGIN_READ: // If we select value mode, read and send the value based on global timer
    {
      if ((P013_OPERATINGMODE == OPMODE_VALUE)
          # if P013_FEATURE_COMBINED_MODE
          || (P013_OPERATINGMODE == OPMODE_COMBINED)
          # endif // if P013_FEATURE_COMBINED_MODE
          ) {
        const float value = Plugin_013_read(event);
        UserVar[event->BaseVarIndex] = value;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("ULTRASONIC : TaskNr: ");
          log += event->TaskIndex + 1;
          # if P013_EXTENDED_LOG
          log += F(" Distance: ");
          log += formatUserVarNoCheck(event->TaskIndex, 0);
          log += ' ';
          log += (P013_MEASURINGUNIT == UNIT_CM) ? F("cm") : F("inch");
          # endif // if P013_EXTENDED_LOG

          if (essentiallyEqual(value, NO_ECHO)) {
            log += F(" Error: ");
            log += Plugin_013_getErrorStatusString(event);
          }

          addLogMove(LOG_LEVEL_INFO, log);
        }
        success = true; // Only send out when actually using Value mode
      } else {
        # if P013_FEATURE_INTERVALEVENT

        if (P013_SEND_STATE_VALUE == 0) { // Also send on Interval when using State mode
          success = true;
        }
        # else // if P013_FEATURE_INTERVALEVENT
        success = true;
        # endif // if P013_FEATURE_INTERVALEVENT
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND: // If we select state mode, do more frequent checks and send only state changes
    {
      if ((P013_OPERATINGMODE == OPMODE_STATE)
          # if P013_FEATURE_COMBINED_MODE
          || (P013_OPERATINGMODE == OPMODE_COMBINED)
          # endif // if P013_FEATURE_COMBINED_MODE
          ) {
        uint8_t state     = 0;
        const float value = Plugin_013_read(event);

        if (!essentiallyEqual(value, NO_ECHO) && definitelyLessThan(value, P013_THRESHOLD)) {
          state = 1;
        }

        if (state != switchstate[event->TaskIndex]) {
          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("ULTRASONIC : TaskNr: ");
            log += event->TaskIndex + 1;

            if (value != NO_ECHO) {
              log += F(" state: ");
              log += state;
            } else {
              log += F(" Error: ");
              log += Plugin_013_getErrorStatusString(event);
            }
            addLogMove(LOG_LEVEL_INFO, log);
          }
          switchstate[event->TaskIndex] = state;
          UserVar[event->BaseVarIndex]  = state;
          event->sensorType             = Sensor_VType::SENSOR_TYPE_SWITCH;
          sendData(event);
        }
      }
      success = true;

      break;
    }
  }
  return success;
}

/*********************************************************************/
float Plugin_013_read(struct EventStruct *event)

/*********************************************************************/
{
  if (P_013_sensordefs.count(event->TaskIndex) == 0u) {
    return 0.0f;
  }

  int16_t max_distance_cm = (P013_MEASURINGUNIT == UNIT_CM) ? P013_MAX_DISTANCE : static_cast<float>(P013_MAX_DISTANCE) * 2.54f;

  unsigned int echoTime = 0;

  switch  (P013_FILTERTYPE) {
    case FILTER_NONE:
      echoTime = (P_013_sensordefs[event->TaskIndex])->ping();
      break;
    case FILTER_MEDIAN:
      echoTime = (P_013_sensordefs[event->TaskIndex])->ping_median(P013_FILTER_SIZE, max_distance_cm);
      break;
    # if P013_EXTENDED_LOG
    default:
      addLog(LOG_LEVEL_ERROR, F("invalid Filter Type setting!")); // Should not be possible...
    # endif // if P013_EXTENDED_LOG
  }

  if (P013_MEASURINGUNIT == UNIT_CM) {
    return NewPing::convert_cm_F(echoTime);
  }
  else {
    return NewPing::convert_in_F(echoTime);
  }
}

/*********************************************************************/
const __FlashStringHelper* Plugin_013_getErrorStatusString(struct EventStruct *event)

/*********************************************************************/
{
  if (P_013_sensordefs.count(event->TaskIndex) == 0) {
    return F("invalid taskindex");
  }

  switch ((P_013_sensordefs[event->TaskIndex])->getErrorState()) {
    case NewPing::STATUS_SENSOR_READY: { // 0
      # if P013_EXTENDED_LOG
      return F("Sensor ready");
      # endif // if P013_EXTENDED_LOG
    }

    case NewPing::STATUS_MEASUREMENT_VALID: { // 1
      # if P013_EXTENDED_LOG
      return F("no error, measurement valid");
      # endif // if P013_EXTENDED_LOG
    }

    case NewPing::STATUS_ECHO_TRIGGERED: { // 2
      # if P013_EXTENDED_LOG
      return F("Echo triggered, waiting for Echo end");
      # else // if P013_EXTENDED_LOG
      return F("Ok");
      # endif // if P013_EXTENDED_LOG
    }

    case NewPing::STATUS_ECHO_STATE_ERROR: { // 6
      return F("Error, Echopin not low on trigger");
    }

    case NewPing::STATUS_ECHO_START_TIMEOUT_50ms: { // 4
      return F("Error, no echo start whithin 50 ms");
    }

    case NewPing::STATUS_ECHO_START_TIMEOUT_DISTANCE: { // 5
      return F("Error, no echo start whithin time for max. distance");
    }

    case NewPing::STATUS_MAX_DISTANCE_EXCEEDED: { // 3
      return F("Echo too late, maximum distance exceeded");
    }

    default: {
      return F("unknown error");
    }
  }
}

#endif // USES_P013
