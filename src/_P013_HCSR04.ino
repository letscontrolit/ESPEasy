#include "_Plugin_Helper.h"

#ifdef USES_P013

// #######################################################################################################
// ############################### Plugin 013: HC-SR04, RCW-0001, etc. ###################################
// #######################################################################################################

/** Changelog:
 * 2022-12-31 tonhuisman: Code improvements, change start-trigger range to 10-50 usec.
 *                        Optionally not send regular Interval events when using State mode
 * 2022-12-29 tonhuisman: Add start-trigger setting, range 10-30 usec. See https://github.com/letscontrolit/ESPEasy/issues/3857
 * 2022-12-29 tonhuisman: Add changelog
 */

# define PLUGIN_013
# define PLUGIN_ID_013        13
# define PLUGIN_NAME_013       "Position - HC-SR04, RCW-0001, etc."
# define PLUGIN_VALUENAME1_013 "Distance"

# include <map>
# include <NewPing.h>

// PlugIn specific defines
// operatingMode
# define OPMODE_VALUE        (0)
# define OPMODE_STATE        (1)

// measuringUnit
# define UNIT_CM             (0)
# define UNIT_INCH           (1)

// filterType
# define FILTER_NONE         (0)
# define FILTER_MEDIAN       (1)

# define P013_TRIGGER_PIN       CONFIG_PIN1
# define P013_ECHO_PIN          CONFIG_PIN2

# define P013_OPERATINGMODE     PCONFIG(0)
# define P013_THRESHOLD         PCONFIG(1)
# define P013_MAX_DISTANCE      PCONFIG(2)
# define P013_MEASURINGUNIT     PCONFIG(3)
# define P013_FILTERTYPE        PCONFIG(4)
# define P013_FILTER_SIZE       PCONFIG(5)
# define P013_TRIGGER_WIDTH     PCONFIG(6)
# define P013_SEND_STATE_VALUE  PCONFIG(7)

# define P013_DEFAULT_FILTER_SIZE     (5)
# define P013_DEFAULT_TRIGGER_WIDTH   (10)

// map of sensors
std::map<unsigned int, std::shared_ptr<NewPing> >P_013_sensordefs;

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
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("Trigger"));
      event->String2 = formatGpioName_input(F("Echo, 5V"));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P013_FILTER_SIZE   = P013_DEFAULT_FILTER_SIZE;
      P013_TRIGGER_WIDTH = P013_DEFAULT_TRIGGER_WIDTH;

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      // default filtersize
      if (P013_FILTER_SIZE == 0) { P013_FILTER_SIZE = P013_DEFAULT_FILTER_SIZE; }

      // default trigger width
      if (P013_TRIGGER_WIDTH == 0) { P013_TRIGGER_WIDTH = P013_DEFAULT_TRIGGER_WIDTH; }

      const __FlashStringHelper *strUnit = (P013_MEASURINGUNIT == UNIT_CM) ? F("cm") : F("inch");

      {
        const int optionValuesOpMode[] = { OPMODE_VALUE, OPMODE_STATE };
        const __FlashStringHelper *optionsOpMode[] {
          F("Value"),
          F("State"),
        };
        addFormSelector(F("Mode"), F("pmode"), 2, optionsOpMode, optionValuesOpMode, P013_OPERATINGMODE);
      }

      if (P013_OPERATINGMODE == OPMODE_STATE) {
        addFormCheckBox(F("State event (also) on Interval"), F("pevent"), P013_SEND_STATE_VALUE == 0);
        addFormNumericBox(F("Threshold"), F("pthreshold"), P013_THRESHOLD);
        addUnit(strUnit);
      }
      addFormNumericBox(F("Max Distance"), F("pmax_distance"), P013_MAX_DISTANCE, 0, 500);
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
        addFormSelector(F("Filter"), F("fltrType"), 2, optionsFilter, optionValuesFilter, P013_FILTERTYPE);
      }

      // enable filtersize option if filter is used,
      if (P013_FILTERTYPE != FILTER_NONE) {
        addFormNumericBox(F("Number of Pings"), F("fltrSize"), P013_FILTER_SIZE, 2, 20);
        addUnit(F("2..20"));
      }

      addFormNumericBox(F("Trigger width"), F("trigWidth"), P013_TRIGGER_WIDTH, 10, 50);
      addUnit(F("10..50 &micro;sec"));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      int16_t prevOperatingMode = P013_OPERATINGMODE;
      int16_t prevFilterType    = P013_FILTERTYPE;

      P013_OPERATINGMODE = getFormItemInt(F("pmode"));

      if (prevOperatingMode == OPMODE_STATE) {
        P013_SEND_STATE_VALUE = isFormItemChecked(F("pevent")) ? 0 : 1; // Inverted state
        P013_THRESHOLD        = getFormItemInt(F("pthreshold"));
      }
      P013_MAX_DISTANCE = getFormItemInt(F("pmax_distance"));

      P013_MEASURINGUNIT = getFormItemInt(F("pUnit"));
      P013_FILTERTYPE    = getFormItemInt(F("fltrType"));

      if (prevFilterType != FILTER_NONE) {
        P013_FILTER_SIZE = getFormItemInt(F("fltrSize"));
      }
      P013_TRIGGER_WIDTH = getFormItemInt(F("trigWidth"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      if (P013_FILTER_SIZE == 0) { P013_FILTER_SIZE = P013_DEFAULT_FILTER_SIZE; }

      if (P013_TRIGGER_WIDTH == 0) { P013_TRIGGER_WIDTH = P013_DEFAULT_TRIGGER_WIDTH; }

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
          log += F(" TrigWidth [usec]: ");
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
      if (P013_OPERATINGMODE == OPMODE_VALUE) {
        const float value = Plugin_013_read(event);
        UserVar[event->BaseVarIndex] = value;

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("ULTRASONIC : TaskNr: ");
          log += event->TaskIndex + 1;
          log += F(" Distance: ");
          log += formatUserVarNoCheck(event->TaskIndex, 0);
          log += (P013_MEASURINGUNIT == UNIT_CM) ? F(" cm ") : F(" inch ");

          if (essentiallyEqual(value, NO_ECHO)) {
            log += F(" Error: ");
            log += Plugin_013_getErrorStatusString(event);
          }

          addLogMove(LOG_LEVEL_INFO, log);
        }
        success = true;                 // Only send out when actually using Value mode
      }

      if (P013_SEND_STATE_VALUE == 0) { // Also send on Interval when using State mode
        success = true;
      }
      break;
    }

    case PLUGIN_TEN_PER_SECOND: // If we select state mode, do more frequent checks and send only state changes
    {
      if (P013_OPERATINGMODE == OPMODE_STATE) {
        uint8_t state = 0;
        float   value = Plugin_013_read(event);

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
  if (P_013_sensordefs.count(event->TaskIndex) == 0) {
    return 0;
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
    default:
      addLog(LOG_LEVEL_ERROR, F("invalid Filter Type setting!"));
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
    case NewPing::STATUS_SENSOR_READY: {
      return F("Sensor ready");
    }

    case NewPing::STATUS_MEASUREMENT_VALID: {
      return F("no error, measurement valid");
    }

    case NewPing::STATUS_ECHO_TRIGGERED: {
      return F("Echo triggered, waiting for Echo end");
    }

    case NewPing::STATUS_ECHO_STATE_ERROR: {
      return F("Echo pulse error, Echopin not low on trigger");
    }

    case NewPing::STATUS_ECHO_START_TIMEOUT_50ms: {
      return F("Echo timeout error, no echo start whithin 50 ms");
    }

    case NewPing::STATUS_ECHO_START_TIMEOUT_DISTANCE: {
      return F("Echo timeout error, no echo start whithin time for max. distance");
    }

    case NewPing::STATUS_MAX_DISTANCE_EXCEEDED: {
      return F("Echo too late, maximum distance exceeded");
    }

    default: {
      return F("unknown error");
    }
  }
}

#endif // USES_P013
