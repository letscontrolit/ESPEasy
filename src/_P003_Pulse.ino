#include "_Plugin_Helper.h"
#ifdef USES_P003

// #######################################################################################################
// #################################### Plugin 003: Pulse  ###############################################
// #######################################################################################################
// KP: enhanced version 2.2 for pulse counting
//
// Make sure physical connections are electrically well sepparated so no crossover of the signals happen.
// Especially at rates above ~5'000 RPM with longer lines. Best use a cable with ground and signal twisted.
// The Mode Types "PULSE low/high/change" are suited for low frequence pulses but for and precise counting
// with pulse rates of less than 750 RPM with DebounceTime > 20ms and pulse length > 40ms. This type may
// tolerate less good signals. After a pulse and debounce time it verifies the signal 3 times.


# include "src/PluginStructs/P003_data_struct.h"

# include "src/Helpers/ESPEasy_time_calc.h"

# define P003_PULSE_STATS_DEFAULT_LOG_LEVEL  LOG_LEVEL_DEBUG
# define P003_PULSE_STATS_ADHOC_LOG_LEVEL    LOG_LEVEL_INFO

# define PLUGIN_003
# define PLUGIN_ID_003                  3
# define PLUGIN_NAME_003                "Generic - Pulse counter"

// number and name of counted values
# define PLUGIN_NR_VALUENAMES_003       3
# define PLUGIN_VALUENAME1_003          "Count"
# define PLUGIN_VALUENAME2_003          "Total"
# define PLUGIN_VALUENAME3_003          "Time"

// ... their index in UserVar and TaskDeviceValueNames
# define P003_IDX_pulseCounter           0
# define P003_IDX_pulseTotalCounter      1
# define P003_IDX_pulseTime              2

// ... and the following index into UserVar for storing the persisted TotalCounter
# define P003_IDX_persistedTotalCounter  3

// indexes for config parameters
# define P003_IDX_DEBOUNCETIME   0
# define P003_IDX_COUNTERTYPE    1
# define P003_IDX_MODETYPE       2

// values for WEBFORM Counter Types
# define P003_NR_COUNTERTYPES               4
# define P003_COUNTERTYPE_LIST { F("Delta"), F("Delta/Total/Time"), F("Total"), F("Delta/Total"), }
# define P003_CT_INDEX_COUNTER              0
# define P003_CT_INDEX_COUNTER_TOTAL_TIME   1
# define P003_CT_INDEX_TOTAL                2
# define P003_CT_INDEX_COUNTER_TOTAL        3

bool validIntFromString(const String& tBuf,
                        int         & result);


boolean Plugin_003(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_003;
      Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = true;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = PLUGIN_NR_VALUENAMES_003;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_003);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_003));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_003));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_003));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_input(F("Pulse"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Debounce Time (mSec)"), F("p003_debounce")
                        , PCONFIG(P003_IDX_DEBOUNCETIME));

      {
        byte choice  = PCONFIG(P003_IDX_COUNTERTYPE);
        const __FlashStringHelper *options[P003_NR_COUNTERTYPES] = P003_COUNTERTYPE_LIST;
        addFormSelector(F("Counter Type"), F("p003_countertype"), P003_NR_COUNTERTYPES, options, NULL, choice);
        if (choice != 0) {
          addHtml(F("<span style=\"color:red\">Total count is not persistent!</span>"));
        }
      }

      Internal_GPIO_pulseHelper::addGPIOtriggerMode(
        F("Mode Type"), 
        F("p003_raisetype"), 
        static_cast<Internal_GPIO_pulseHelper::GPIOtriggerMode>(PCONFIG(P003_IDX_MODETYPE)));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(P003_IDX_DEBOUNCETIME) = getFormItemInt(F("p003_debounce"));
      PCONFIG(P003_IDX_COUNTERTYPE)  = getFormItemInt(F("p003_countertype"));
      PCONFIG(P003_IDX_MODETYPE)     = getFormItemInt(F("p003_raisetype"));
      success                        = true;
      break;
    }

    case PLUGIN_INIT:
    {
      Internal_GPIO_pulseHelper::pulseCounterConfig config;
      config.setDebounceTime(PCONFIG(P003_IDX_DEBOUNCETIME));
      config.gpio             = Settings.TaskDevicePin1[event->TaskIndex];
      config.taskIndex        = event->TaskIndex;
      config.interruptPinMode = static_cast<Internal_GPIO_pulseHelper::GPIOtriggerMode>(PCONFIG(P003_IDX_MODETYPE));
      config.pullupPinMode    = Settings.TaskDevicePin1PullUp[event->TaskIndex] ? INPUT_PULLUP : INPUT;

      // FIXME TD-er: Must set the state using globalMapPortStatus


      initPluginTaskData(event->TaskIndex, new (std::nothrow) P003_data_struct(config));
      P003_data_struct *P003_data =
        static_cast<P003_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P003_data) {
        #ifdef PULSE_STATISTIC
        P003_data->pulseHelper.setStatsLogLevel(P003_PULSE_STATS_DEFAULT_LOG_LEVEL);
        #endif

        // Restore the total counter from the unused 4th UserVar value.
        // It may be using a formula to generate the output, which makes it impossible to restore
        // the true internal state.
        P003_data->pulseHelper.setPulseCountTotal(UserVar[event->BaseVarIndex + P003_IDX_persistedTotalCounter]);

        // Restore any values from the RTC-memory (persistent as long as power is on. Survives warm reset or deep sleep)
        switch (PCONFIG(P003_IDX_COUNTERTYPE))
        {
          case P003_CT_INDEX_COUNTER:
          {
            P003_data->pulseHelper.setPulseCounter(UserVar[event->BaseVarIndex + P003_IDX_pulseCounter]);
            break;
          }
          case P003_CT_INDEX_COUNTER_TOTAL_TIME:
          {
            P003_data->pulseHelper.setPulseCounter(UserVar[event->BaseVarIndex + P003_IDX_pulseCounter],
                                                   UserVar[event->BaseVarIndex + P003_IDX_pulseTime]);
            break;
          }
          case P003_CT_INDEX_TOTAL:
          {
            break;
          }
          case P003_CT_INDEX_COUNTER_TOTAL:
          {
            P003_data->pulseHelper.setPulseCounter(UserVar[event->BaseVarIndex + P003_IDX_pulseCounter]);
            break;
          }
        }

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log; log.reserve(20);
          log = F("INIT : PulsePin: "); log += Settings.TaskDevicePin1[event->TaskIndex];
          addLog(LOG_LEVEL_INFO, log);
        }

        // set up device pin and estabish interupt handlers
        P003_data->pulseHelper.init();
      }
      break;
    }

    case PLUGIN_READ:
    {
      P003_data_struct *P003_data =
        static_cast<P003_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P003_data) {
        unsigned long pulseCounter, pulseCounterTotal;
        float pulseTime_msec;
        P003_data->pulseHelper.getPulseCounters(pulseCounter, pulseCounterTotal, pulseTime_msec);
        P003_data->pulseHelper.resetPulseCounter();


        // store the current counter values into UserVar (RTC-memory)
        // FIXME TD-er: Is it correct to write the first 3  UserVar values, regardless the set counter type?
        UserVar[event->BaseVarIndex + P003_IDX_pulseCounter]      = pulseCounter;
        UserVar[event->BaseVarIndex + P003_IDX_pulseTotalCounter] = pulseCounterTotal;
        UserVar[event->BaseVarIndex + P003_IDX_pulseTime]         = pulseTime_msec;

        // Store the raw value in the unused 4th position.
        // This is needed to restore the value from RTC as it may be converted into another output value using a formula.
        UserVar[event->BaseVarIndex + P003_IDX_persistedTotalCounter] = pulseCounterTotal;

        switch (PCONFIG(P003_IDX_COUNTERTYPE))
        {
          case P003_CT_INDEX_COUNTER:
          {
            event->sensorType = Sensor_VType::SENSOR_TYPE_SINGLE;
            break;
          }
          case P003_CT_INDEX_COUNTER_TOTAL_TIME:
          {
            event->sensorType = Sensor_VType::SENSOR_TYPE_TRIPLE;
            break;
          }
          case P003_CT_INDEX_TOTAL:
          {
            event->sensorType = Sensor_VType::SENSOR_TYPE_SINGLE;
            break;
          }
          case P003_CT_INDEX_COUNTER_TOTAL:
          {
            event->sensorType = Sensor_VType::SENSOR_TYPE_DUAL;
            break;
          }
        }
        success = true;
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P003_data_struct *P003_data =
        static_cast<P003_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P003_data) {
        String command            = parseString(string, 1);
        bool   mustCallPluginRead = false;

        if ((command == F("resetpulsecounter")) || (command == F("setpulsecountertotal")))
        {
          // Legacy commands       ({...} indicate optional parameters):
          // - {[<TaskName/Number>].}resetpulsecounter
          // - {[<TaskName/Number>].}setpulsecountertotal,value
          // - resetpulsecounter{,taskindex}            <== legacy method
          // - setpulsecountertotal,value{,taskindex}   <== legacy method
          //     when "[<TaskName/Number>]." or ",taskindex" is ommitted, the command applies
          //     to the first active P003 task instance

          // Legacy: Allow for an optional taskIndex parameter.
          byte tidx = 1;

          if (command == F("setpulsecountertotal")) { tidx = 2; }

          if (!pluginOptionalTaskIndexArgumentMatch(event->TaskIndex, string, tidx)) {
            break;
          }

          int par1 = 0;

          if (command == F("setpulsecountertotal")) {
            if (!validIntFromString(parseString(string, 2), par1)) { break; }
          }
          P003_data->pulseHelper.setPulseCountTotal(par1);

          if (command == F("resetpulsecounter")) {
            P003_data->pulseHelper.resetPulseCounter();
          }
          mustCallPluginRead = true;

          # ifdef PULSE_STATISTIC

          // adjust the statistical step counters relative to TotalCounter, in order to keep statistic correct
          P003_data->pulseHelper.updateStatisticalCounters(par1);
          # endif // PULSE_STATISTIC

          success = true;
        }
        else if (command == F("logpulsestatistic"))
        {
          # ifdef PULSE_STATISTIC

          // Valid command:
          // - [<TaskName/Number>].logpulsestatistic{,<subcommand>}
          //     when "[<TaskName/Number>]." is ommitted, the command applies
          //      to the first active P003 task instance
          //     optional subcommand:
          //       r = reset error and overdue counters after logging
          //       i = increase the log level for regular statstic logs to "info"

          String subcommand = parseString(string, 2);

          if ((subcommand == F("i")) || (subcommand == F("r")) || (subcommand == "")) {
            P003_data->pulseHelper.doStatisticLogging(P003_PULSE_STATS_ADHOC_LOG_LEVEL);
            P003_data->pulseHelper.doTimingLogging(P003_PULSE_STATS_ADHOC_LOG_LEVEL);

            if (subcommand == F("i")) { P003_data->pulseHelper.setStatsLogLevel(LOG_LEVEL_INFO); }

            if (subcommand == F("r")) { P003_data->pulseHelper.resetStatsErrorVars(); }
            success = true;
          }
          # else // ifdef PULSE_STATISTIC
          success = false;   // Command not available
          # endif // PULSE_STATISTIC
        }

        if (mustCallPluginRead) {
          // Note that the set time is before the current moment, so we call the read as soon as possible.
          // The read does also use any set formula and stored the value in RTC.
          Scheduler.schedule_task_device_timer(event->TaskIndex, millis() - 10);
        }
      }
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      P003_data_struct *P003_data =
        static_cast<P003_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P003_data) {
        // step 0 will check if a new signal edge is to be processed and then schedule step 1
        P003_data->pulseHelper.doPulseStepProcessing(GPIO_PULSE_HELPER_PROCESSING_STEP_0);
      }
      break;
    }

    case PLUGIN_TIMER_IN:
    {
      P003_data_struct *P003_data =
        static_cast<P003_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P003_data) {
        // this function is called when the next (1,2,3) processing step (Par1) is scheduled
        P003_data->pulseHelper.doPulseStepProcessing(event->Par1);
      }

      break;
    }
  }
  return success;
}

#endif // USES_P003
