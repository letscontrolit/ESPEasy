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

/** Changelog:
 * 2024-08-12 tonhuisman: Improved handling of 'Ignore multiple Delta = 0' by peeking the Delta value.
 * 2024-08-10 tonhuisman: Changed option to 'Ignore multiple Delta = 0', and allow Interval = 0, combined with Delta = 0, to send a pulse
 *                        immediately to the Controllers and generate events.
 *                        Moved most PLUGIN_READ logic to P003_data_struct for easy re-use.
 * 2024-08-08 tonhuisman: Add support for 'Ignore Delta = 0' setting, to not send out events and data to controllers if the Delta (Count)
 *                        value is 0.
 * 2024-08-07 tonhuisman: Add support for Time, Total/Time, Time/Delta Counter types. Not included in LIMIT_BUILD_SIZE builds!
 * 2024-08-06 tonhuisman: Add support for PLUGIN_GET_DEVICEVALUECOUNT and PLUGIN_GET_DEVICEVTYPE to use the correct number of values when
 *                        sending data to controllers. Also move Total to first value to have it sent out properly when it's the only value.
 * 2024-08-06 tonhuisman: Start changelog. Uncrustify the source.
 */

# include "src/PluginStructs/P003_data_struct.h"

# include "src/Helpers/ESPEasy_time_calc.h"

# ifndef BUILD_NO_DEBUG
#  define P003_PULSE_STATS_DEFAULT_LOG_LEVEL  LOG_LEVEL_DEBUG
# endif // ifndef BUILD_NO_DEBUG
# define P003_PULSE_STATS_ADHOC_LOG_LEVEL    LOG_LEVEL_INFO

# define PLUGIN_003
# define PLUGIN_ID_003                  3
# define PLUGIN_NAME_003                "Generic - Pulse counter"

// number and name of counted values
# define PLUGIN_NR_VALUENAMES_003       3
# define PLUGIN_VALUENAME1_003          "Count"
# define PLUGIN_VALUENAME2_003          "Total"
# define PLUGIN_VALUENAME3_003          "Time"


boolean Plugin_003(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number           = PLUGIN_ID_003;
      dev.Type             = DEVICE_TYPE_SINGLE;
      dev.VType            = Sensor_VType::SENSOR_TYPE_SINGLE;
      dev.PullUpOption     = true;
      dev.FormulaOption    = true;
      dev.ValueCount       = PLUGIN_NR_VALUENAMES_003;
      dev.SendDataOption   = true;
      dev.TimerOption      = true;
      dev.TimerOptional    = true;
      dev.PluginStats      = true;
      dev.TaskLogsOwnPeaks = true;
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

    case PLUGIN_SET_DEFAULTS:
    {
      Settings.TaskDeviceTimer[event->TaskIndex] = Settings.Delay; // Previous default for non-TimerOptional
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      switch (PCONFIG(P003_IDX_COUNTERTYPE)) {
        case P003_CT_INDEX_COUNTER:
        case P003_CT_INDEX_TOTAL:
        # if P003_USE_EXTRA_COUNTERTYPES
        case P003_CT_INDEX_TIME:
        # endif // if P003_USE_EXTRA_COUNTERTYPES
          event->Par1 = 1;
          break;
        case P003_CT_INDEX_COUNTER_TOTAL_TIME:
          event->Par1 = 3;
          break;
        case P003_CT_INDEX_COUNTER_TOTAL:
        # if P003_USE_EXTRA_COUNTERTYPES
        case P003_CT_INDEX_TOTAL_TIME:
        case P003_CT_INDEX_TIME_COUNTER:
        # endif // if P003_USE_EXTRA_COUNTERTYPES
          event->Par1 = 2;
          break;
      }
      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      switch (PCONFIG(P003_IDX_COUNTERTYPE)) {
        case P003_CT_INDEX_COUNTER:
        case P003_CT_INDEX_TOTAL:
        # if P003_USE_EXTRA_COUNTERTYPES
        case P003_CT_INDEX_TIME:
        # endif // if P003_USE_EXTRA_COUNTERTYPES
          event->sensorType = Sensor_VType::SENSOR_TYPE_SINGLE;
          break;
        case P003_CT_INDEX_COUNTER_TOTAL_TIME:
          event->sensorType = Sensor_VType::SENSOR_TYPE_TRIPLE;
          break;
        case P003_CT_INDEX_COUNTER_TOTAL:
        # if P003_USE_EXTRA_COUNTERTYPES
        case P003_CT_INDEX_TOTAL_TIME:
        case P003_CT_INDEX_TIME_COUNTER:
        # endif // if P003_USE_EXTRA_COUNTERTYPES
          event->sensorType = Sensor_VType::SENSOR_TYPE_DUAL;
          break;
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Debounce Time"), F("debounce")
                        , PCONFIG(P003_IDX_DEBOUNCETIME));
      addUnit(F("mSec"));

      {
        const uint8_t choice                 = PCONFIG(P003_IDX_COUNTERTYPE);
        const __FlashStringHelper *options[] = {
          F("Delta"),
          F("Delta/Total/Time"),
          F("Total"),
          F("Delta/Total"),
          # if P003_USE_EXTRA_COUNTERTYPES
          F("Time"),
          F("Total/Time"),
          F("Time/Delta"),
          # endif // if P003_USE_EXTRA_COUNTERTYPES
        };
        constexpr size_t optionCount = NR_ELEMENTS(options);
        addFormSelector(F("Counter Type"), F("countertype"), optionCount, options, nullptr, choice);

        if (choice != 0) {
          addHtml(F("<span style=\"color:red\">Total count is not persistent!</span>"));
        }

        if ((choice == P003_CT_INDEX_TOTAL)
            # if P003_USE_EXTRA_COUNTERTYPES
            || (choice >= P003_CT_INDEX_TIME)
            # endif // if P003_USE_EXTRA_COUNTERTYPES
            ) {
          addFormNote(F("Value names are not auto-updated!"));
        }
      }

      Internal_GPIO_pulseHelper::addGPIOtriggerMode(
        F("Mode Type"),
        F("raisetype"),
        static_cast<Internal_GPIO_pulseHelper::GPIOtriggerMode>(PCONFIG(P003_IDX_MODETYPE)));

      addFormCheckBox(F("Ignore multiple Delta = 0"), F("nozero"), PCONFIG(P003_IDX_IGNORE_ZERO));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(P003_IDX_DEBOUNCETIME) = getFormItemInt(F("debounce"));
      PCONFIG(P003_IDX_COUNTERTYPE)  = getFormItemInt(F("countertype"));
      PCONFIG(P003_IDX_MODETYPE)     = getFormItemInt(F("raisetype"));
      PCONFIG(P003_IDX_IGNORE_ZERO)  = isFormItemChecked(F("nozero"));
      success                        = true;
      break;
    }

    case PLUGIN_INIT:
    {
      Internal_GPIO_pulseHelper::pulseCounterConfig config;
      config.setDebounceTime(PCONFIG(P003_IDX_DEBOUNCETIME));
      config.gpio             = CONFIG_PIN1;
      config.taskIndex        = event->TaskIndex;
      config.interruptPinMode = static_cast<Internal_GPIO_pulseHelper::GPIOtriggerMode>(PCONFIG(P003_IDX_MODETYPE));
      config.pullupPinMode    = Settings.TaskDevicePin1PullUp[event->TaskIndex] ? INPUT_PULLUP : INPUT;

      // FIXME TD-er: Must set the state using globalMapPortStatus


      initPluginTaskData(event->TaskIndex, new (std::nothrow) P003_data_struct(config));
      P003_data_struct *P003_data =
        static_cast<P003_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P003_data) {
        # ifdef PULSE_STATISTIC
        #  ifndef BUILD_NO_DEBUG
        P003_data->pulseHelper.setStatsLogLevel(P003_PULSE_STATS_DEFAULT_LOG_LEVEL);
        #  endif // ifndef BUILD_NO_DEBUG
        # endif // ifdef PULSE_STATISTIC

        // Restore the total counter from the unused 4th UserVar value.
        // It may be using a formula to generate the output, which makes it impossible to restore
        // the true internal state.
        P003_data->pulseHelper.setPulseCountTotal(UserVar[event->BaseVarIndex + P003_IDX_persistedTotalCounter]);

        // Restore any values from the RTC-memory (persistent as long as power is on. Survives warm reset or deep sleep)
        switch (PCONFIG(P003_IDX_COUNTERTYPE)) {
          case P003_CT_INDEX_COUNTER:
          case P003_CT_INDEX_COUNTER_TOTAL:
            P003_data->pulseHelper.setPulseCounter(UserVar.getFloat(event->TaskIndex, P003_IDX_pulseCounter));
            break;
          case P003_CT_INDEX_TOTAL:
            break;
          case P003_CT_INDEX_COUNTER_TOTAL_TIME:
            P003_data->pulseHelper.setPulseCounter(UserVar.getFloat(event->TaskIndex, P003_IDX_pulseCounter),
                                                   UserVar.getFloat(event->TaskIndex, P003_IDX_pulseTime));
            break;
          # if P003_USE_EXTRA_COUNTERTYPES
          case P003_CT_INDEX_TIME:
            break;
          case P003_CT_INDEX_TOTAL_TIME:
            break;
          case P003_CT_INDEX_TIME_COUNTER:
            break;
          # endif // if P003_USE_EXTRA_COUNTERTYPES
        }

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLogMove(LOG_LEVEL_INFO, strformat(F("INIT : PulsePin: %d"), CONFIG_PIN1));
        }

        // set up device pin and estabish interupt handlers
        success = P003_data->pulseHelper.init();
      }
      break;
    }

    case PLUGIN_READ:
    {
      P003_data_struct *P003_data =
        static_cast<P003_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P003_data) {
        success = P003_data->plugin_read(event);
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P003_data_struct *P003_data =
        static_cast<P003_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P003_data) {
        const String command    = parseString(string, 1);
        bool mustCallPluginRead = false;

        const bool cmd_resetpulsecounter    = equals(command, F("resetpulsecounter"));
        const bool cmd_setpulsecountertotal = equals(command, F("setpulsecountertotal"));

        if (cmd_resetpulsecounter || cmd_setpulsecountertotal)
        {
          // Legacy commands       ({...} indicate optional parameters):
          // - {[<TaskName/Number>].}resetpulsecounter
          // - {[<TaskName/Number>].}setpulsecountertotal,value
          // - resetpulsecounter{,taskindex}            <== legacy method
          // - setpulsecountertotal,value{,taskindex}   <== legacy method
          //     when "[<TaskName/Number>]." or ",taskindex" is ommitted, the command applies
          //     to the first active P003 task instance

          // Legacy: Allow for an optional taskIndex parameter.
          uint8_t tidx = 1;

          if (cmd_setpulsecountertotal) { tidx = 2; }

          if (!pluginOptionalTaskIndexArgumentMatch(event->TaskIndex, string, tidx)) {
            break;
          }

          int32_t par1 = 0;

          if (cmd_setpulsecountertotal) {
            if (!validIntFromString(parseString(string, 2), par1)) { break; }
          }
          P003_data->pulseHelper.setPulseCountTotal(par1);

          if (cmd_resetpulsecounter) {
            P003_data->pulseHelper.resetPulseCounter();
          }
          mustCallPluginRead = true;

          # ifdef PULSE_STATISTIC

          // adjust the statistical step counters relative to TotalCounter, in order to keep statistic correct
          P003_data->pulseHelper.updateStatisticalCounters(par1);
          # endif // PULSE_STATISTIC

          success = true;
        }
        else if (equals(command, F("logpulsestatistic")))
        {
          # ifdef PULSE_STATISTIC

          // Valid command:
          // - [<TaskName/Number>].logpulsestatistic{,<subcommand>}
          //     when "[<TaskName/Number>]." is ommitted, the command applies
          //      to the first active P003 task instance
          //     optional subcommand:
          //       r = reset error and overdue counters after logging
          //       i = increase the log level for regular statstic logs to "info"

          const String subcommand = parseString(string, 2);
          const bool   sub_i      = equals(subcommand, F("i"));
          const bool   sub_r      = equals(subcommand, F("r"));

          if ((sub_i) || (sub_r) || (subcommand.isEmpty())) {
            P003_data->pulseHelper.doStatisticLogging(P003_PULSE_STATS_ADHOC_LOG_LEVEL);
            P003_data->pulseHelper.doTimingLogging(P003_PULSE_STATS_ADHOC_LOG_LEVEL);

            if (sub_i) { P003_data->pulseHelper.setStatsLogLevel(LOG_LEVEL_INFO); }

            if (sub_r) { P003_data->pulseHelper.resetStatsErrorVars(); }
            success = true;
          }
          # else // ifdef PULSE_STATISTIC
          success = false; // Command not available
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

        if ((Settings.TaskDeviceTimer[event->TaskIndex] == 0) &&
            PCONFIG(P003_IDX_IGNORE_ZERO) &&
            P003_data->plugin_peek(event)) {
          Scheduler.schedule_task_device_timer(event->TaskIndex, millis() - 10);
        }
      }
      break;
    }

    case PLUGIN_TASKTIMER_IN:
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
