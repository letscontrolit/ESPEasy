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

# include "src/Helpers/ESPEasy_time_calc.h"

// additional debug/tuning messages from PULSE mode into the logfile
# ifndef LIMIT_BUILD_SIZE
  #  define P003_PULSE_STATISTIC
# endif // ifndef LIMIT_BUILD_SIZE
# define P003_PULSE_STATS_DEFAULT_LOG_LEVEL  LOG_LEVEL_DEBUG
# define P003_PULSE_STATS_ADHOC_LOG_LEVEL    LOG_LEVEL_INFO

// Pulse Counter only suports up to 4 in order to save space (if number is increased, add related IRQ functions)
# define P003_TASKS_MAX                 4

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

// special Mode Type. Note: Lower 3 bits are significant for GPIO Interupt type. The upper bits distinguish the Mode Types
# define PULSE_LOW               (0x10 | CHANGE)
# define PULSE_HIGH              (0x20 | CHANGE)
# define PULSE_CHANGE            (0x30 | CHANGE)
# define PULSE_MODE_MASK         0x30
# define MODE_INTERRUPT_MASK     0x03

// values for WEBFORM Counter Types
# define P003_NR_COUNTERTYPES               4
# define P003_COUNTERTYPE_LIST { F("Delta"), F("Delta/Total/Time"), F("Total"), F("Delta/Total"), }
# define P003_CT_INDEX_COUNTER              0
# define P003_CT_INDEX_COUNTER_TOTAL_TIME   1
# define P003_CT_INDEX_TOTAL                2
# define P003_CT_INDEX_COUNTER_TOTAL        3

// processing Steps in PLUGIN_TIMER_IN
# define P003_PROCESSING_STEP_0             0
# define P003_PROCESSING_STEP_1             1
# define P003_PROCESSING_STEP_2             2
# define P003_PROCESSING_STEP_3             3
# define P003_PSTEP_MAX                     P003_PROCESSING_STEP_3

bool validIntFromString(const String& tBuf,
                        int         & result);

void Plugin_003_pulse_interrupt1() ICACHE_RAM_ATTR;
void Plugin_003_pulse_interrupt2() ICACHE_RAM_ATTR;
void Plugin_003_pulse_interrupt3() ICACHE_RAM_ATTR;
void Plugin_003_pulse_interrupt4() ICACHE_RAM_ATTR;

// void Plugin_003_pulse_interrupt5() ICACHE_RAM_ATTR;
// void Plugin_003_pulse_interrupt6() ICACHE_RAM_ATTR;
// void Plugin_003_pulse_interrupt7() ICACHE_RAM_ATTR;
// void Plugin_003_pulse_interrupt8() ICACHE_RAM_ATTR;
void Plugin_003_pulsecheck(taskIndex_t taskID) ICACHE_RAM_ATTR;

// volatile counter variables for use in ISR
volatile unsigned long P003_pulseCounter[P003_TASKS_MAX];      // number of counted pulses within most recent data collection/sent interval
volatile unsigned long P003_pulseTotalCounter[P003_TASKS_MAX]; // total number of pulses counted since last reset
volatile uint64_t P003_pulseTime[P003_TASKS_MAX];              // time between previous and most recently counted edge/pulse
volatile uint64_t P003_currentStableStartTime[P003_TASKS_MAX]; // stores the start time of the current stable pulse.
volatile uint64_t P003_debounceTime[P003_TASKS_MAX];           // 64 bit version of PCONFIG(P003_IDX_DEBOUNCETIME) in micoseconds
volatile int P003_initStepsFlags;                              // indicates that the pulse processing steps shall be initiated. One bit per
                                                               // task.
volatile int P003_processingFlags;                             // indicates pulse processing is running and interrupts must be ignored. One
                                                               // bit per task.
volatile uint64_t P003_triggerTimestamp[P003_TASKS_MAX];       // timestamp, when the signal change was detected in the ISR

// internal variables for PULSE mode
unsigned long P003_pulseLowTime[P003_TASKS_MAX];               // indicates the length of the most recent stable low pulse (in ms)
unsigned long P003_pulseHighTime[P003_TASKS_MAX];              // indicates the length of the most recent stable high pulse (in ms)
int P003_currentStableState[P003_TASKS_MAX];                   // stores current stable pin state. Set in Step 3 when new stable pulse
                                                               // started
int P003_lastCheckState[P003_TASKS_MAX];                       // most recent pin state, that was read. Set in Step1,2,3

# ifdef P003_PULSE_STATISTIC

// debug/tuning variables for PULSE mode statistical logging
volatile unsigned int P003_Step0counter[P003_TASKS_MAX];      // counts how often step 0 was entered (volatile <- in ISR)
unsigned int P003_Step1counter[P003_TASKS_MAX];               // counts how often step 1 was entered
unsigned int P003_Step2OKcounter[P003_TASKS_MAX];             // counts how often step 2 detected the expected pin state (first
                                                              // verification)
unsigned int P003_Step2NOKcounter[P003_TASKS_MAX];            // counts how often step 2 detected the wrong pin state (first verification
                                                              // failed)
unsigned int P003_Step3OKcounter[P003_TASKS_MAX];             // counts how often step 3 detected the expected pin state (2nd verification)
unsigned int P003_Step3NOKcounter[P003_TASKS_MAX];            // counts how often step 3 detected the wrong pin state (2nd verification
                                                              // failed)
unsigned int P003_Step3IGNcounter[P003_TASKS_MAX];            // counts how often step 3 detected the wrong pin state (2nd verification
                                                              // failed)
unsigned int P003_Step0ODcounter[P003_TASKS_MAX];             // counts how often the debounce time timed out before step 0 was reached
long P003_StepOverdueMax[P003_TASKS_MAX][P003_PSTEP_MAX + 1]; // longest recognised overdue time per step in ms
byte P003_StatsLogLevel[P003_TASKS_MAX];                      // log level for regular statistics logging (default =
                                                              // P003_PULSE_STATS_ADHOC_LOG_LEVEL)

# endif // P003_PULSE_STATISTIC


boolean Plugin_003(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD: // ** Set my technical parameters as ESPEasy device **
    {
      Device[++deviceCount].Number           = PLUGIN_ID_003;
      Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = PLUGIN_NR_VALUENAMES_003;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME: // ** deliver my own device name **
    {
      string = F(PLUGIN_NAME_003);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: // ** define the names for the data that the plugin provides **
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_003));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_003));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_003));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES: // ** return the pin name as show in the configuration webform **
    {
      event->String1 = formatGpioName_input(F("Pulse"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD: // ** add configuration entry boxes into webform html-code **
    {
      if (!validP003TaskIndex(event->TaskIndex)) { break; }
      addFormNumericBox(F("Debounce Time (mSec)"), F("p003")
                        , PCONFIG(P003_IDX_DEBOUNCETIME));

      byte choice  = PCONFIG(P003_IDX_COUNTERTYPE);
      byte choice2 = PCONFIG(P003_IDX_MODETYPE);
      {
        const __FlashStringHelper *options[P003_NR_COUNTERTYPES] = P003_COUNTERTYPE_LIST;
        addFormSelector(F("Counter Type"), F("p003_countertype"), P003_NR_COUNTERTYPES, options, NULL, choice);
      }

      if (choice != 0) {
        addHtml(F("<span style=\"color:red\">Total count is not persistent!</span>"));
      }

      # define P003_NR_MODETYPES  7
      {
        const __FlashStringHelper *modeRaise[P003_NR_MODETYPES]; // displayed texts
        // KP: correction for modeRaise[0]: LOW=0 did not generate interupts and does not make sense. Thus it changed to "none"
        //      Note: A correction to ONLOW = 0x04 (cf. Arduino.h) causes problems as it fires consecutive interupts, while GPIO is low
        modeRaise[0] = F("none");
        modeRaise[1] = F("CHANGE");
        modeRaise[2] = F("RISING");
        modeRaise[3] = F("FALLING");
        modeRaise[4] = F("PULSE low");
        modeRaise[5] = F("PULSE high");
        modeRaise[6] = F("PULSE change");

        int modeValues[P003_NR_MODETYPES]; // trigger flags
        modeValues[0] = 0;                 // KP: replaced LOW by 0, as it does nothing
        modeValues[1] = CHANGE;
        modeValues[2] = RISING;
        modeValues[3] = FALLING;
        modeValues[4] = PULSE_LOW;
        modeValues[5] = PULSE_HIGH;
        modeValues[6] = PULSE_CHANGE;

        addFormSelector(F("Mode Type"), F("p003_raisetype"), P003_NR_MODETYPES, modeRaise, modeValues, choice2);
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: // ** save configuration date as provided by webform **
    {
      if (!validP003TaskIndex(event->TaskIndex)) { break; }
      PCONFIG(P003_IDX_DEBOUNCETIME) = getFormItemInt(F("p003"));
      PCONFIG(P003_IDX_COUNTERTYPE)  = getFormItemInt(F("p003_countertype"));
      PCONFIG(P003_IDX_MODETYPE)     = getFormItemInt(F("p003_raisetype"));
      success                        = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_VALUES: // ** provide current plugin data to webform  **
    {
      if (!validP003TaskIndex(event->TaskIndex)) { break; }
      pluginWebformShowValue(ExtraTaskSettings.TaskDeviceValueNames[P003_IDX_pulseCounter], String(P003_pulseCounter[event->TaskIndex]));
      pluginWebformShowValue(ExtraTaskSettings.TaskDeviceValueNames[P003_IDX_pulseTotalCounter],
                             String(P003_pulseTotalCounter[event->TaskIndex]));
      pluginWebformShowValue(ExtraTaskSettings.TaskDeviceValueNames[P003_IDX_pulseTime],
                             String(P003_pulseTime[event->TaskIndex] / 1000.0f), false);
      success = true;
      break;
    }

    case PLUGIN_INIT: // ** initialize plugin **
    {
      if (!validP003TaskIndex(event->TaskIndex)) { break; }

      // Restore any values from the RTC-memory (persistent as long as power is on. Survives warm reset or deep sleep)
      switch (PCONFIG(P003_IDX_COUNTERTYPE))
      {
        case P003_CT_INDEX_COUNTER:
        {
          P003_pulseCounter[event->TaskIndex] = UserVar[event->BaseVarIndex + P003_IDX_pulseCounter];
          break;
        }
        case P003_CT_INDEX_COUNTER_TOTAL_TIME:
        {
          P003_pulseCounter[event->TaskIndex]      = UserVar[event->BaseVarIndex + P003_IDX_pulseCounter];
          P003_pulseTotalCounter[event->TaskIndex] = UserVar[event->BaseVarIndex + P003_IDX_pulseTotalCounter];
          P003_pulseTime[event->TaskIndex]         = UserVar[event->BaseVarIndex + P003_IDX_pulseTime] * 1000L;
          break;
        }
        case P003_CT_INDEX_TOTAL:
        {
          P003_pulseTotalCounter[event->TaskIndex] = UserVar[event->BaseVarIndex + P003_IDX_pulseCounter];
          break;
        }
        case P003_CT_INDEX_COUNTER_TOTAL:
        {
          P003_pulseCounter[event->TaskIndex]      = UserVar[event->BaseVarIndex + P003_IDX_pulseCounter];
          P003_pulseTotalCounter[event->TaskIndex] = UserVar[event->BaseVarIndex + P003_IDX_pulseTotalCounter];
          break;
        }
      }

      // Restore the total counter from the unused 4th UserVar value.
      // It may be using a formula to generate the output, which makes it impossible to restore
      // the true internal state.
      P003_pulseTotalCounter[event->TaskIndex] = UserVar[event->BaseVarIndex + P003_IDX_persistedTotalCounter];
      P003_debounceTime[event->TaskIndex]      = (uint64_t)Settings.TaskDevicePluginConfig[event->TaskIndex][P003_IDX_DEBOUNCETIME] * 1000L;

      // task indexes larger than 32 should never happen
      if (event->TaskIndex > sizeof(P003_initStepsFlags) * 8) {
        // P003_initStepsFlags and P003_processingFlags can only serve as much tasks as their size in bits (32)
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log; log.reserve(40);
          log =  F("P003: Error! TaskIndex "); log += event->TaskIndex; log += F("is too large");
          addLog(LOG_LEVEL_ERROR, log);
        }
        break;
      }

      // initialize internal variables for PULSE mode handling
      bitClear(P003_initStepsFlags,  event->TaskIndex);
      bitClear(P003_processingFlags, event->TaskIndex);
      P003_pulseLowTime[event->TaskIndex]           = 0;
      P003_pulseHighTime[event->TaskIndex]          = 0;
      P003_currentStableState[event->TaskIndex]     = ((PCONFIG(P003_IDX_MODETYPE) == PULSE_LOW) ? HIGH : LOW);
      P003_currentStableStartTime[event->TaskIndex] = 0;
      P003_lastCheckState[event->TaskIndex]         = HIGH;

      # ifdef P003_PULSE_STATISTIC
      P003_StatsLogLevel[event->TaskIndex]  = P003_PULSE_STATS_DEFAULT_LOG_LEVEL;
      P003_Step0counter[event->TaskIndex]   = P003_pulseTotalCounter[event->TaskIndex];
      P003_Step1counter[event->TaskIndex]   = P003_pulseTotalCounter[event->TaskIndex];
      P003_Step2OKcounter[event->TaskIndex] = P003_pulseTotalCounter[event->TaskIndex];
      P003_Step3OKcounter[event->TaskIndex] = P003_pulseTotalCounter[event->TaskIndex];
      resetStatsErrorVars(event->TaskIndex);
      # endif // P003_PULSE_STATISTIC

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log; log.reserve(20);
        log = F("INIT : PulsePin: "); log += Settings.TaskDevicePin1[event->TaskIndex];
        addLog(LOG_LEVEL_INFO, log);
      }

      // set up device pin and estabish interupt handlers
      pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT_PULLUP);
      success =
        Plugin_003_pulseinit(Settings.TaskDevicePin1[event->TaskIndex], event->TaskIndex,
                             (PCONFIG(P003_IDX_MODETYPE) & MODE_INTERRUPT_MASK));

      break;
    }

    case PLUGIN_READ: // ** PLUGIN_READ **
    {
      if (!validP003TaskIndex(event->TaskIndex)) { break; }

      // store the current counter values into UserVar (RTC-memory)
      // FIXME TD-er: Is it correct to write the first 3  UserVar values, regardless the set counter type?
      UserVar[event->BaseVarIndex + P003_IDX_pulseCounter]      = P003_pulseCounter[event->TaskIndex];
      UserVar[event->BaseVarIndex + P003_IDX_pulseTotalCounter] = P003_pulseTotalCounter[event->TaskIndex];
      UserVar[event->BaseVarIndex + P003_IDX_pulseTime]         = P003_pulseTime[event->TaskIndex] / 1000.0f;

      // Store the raw value in the unused 4th position.
      // This is needed to restore the value from RTC as it may be converted into another output value using a formula.
      UserVar[event->BaseVarIndex + P003_IDX_persistedTotalCounter] = P003_pulseTotalCounter[event->TaskIndex];


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
      P003_pulseCounter[event->TaskIndex] = 0;
      success                             = true;
      break;
    }

    case PLUGIN_WRITE: // ** execute functional command **
    {
      if (!validP003TaskIndex(event->TaskIndex)) { break; }

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
        P003_pulseTotalCounter[event->TaskIndex] = par1;

        if (command == F("resetpulsecounter")) {
          P003_pulseCounter[event->TaskIndex] = 0;
          P003_pulseTime[event->TaskIndex]    = 0;
        }
        mustCallPluginRead = true;

        # ifdef P003_PULSE_STATISTIC

        // adjust the statistical step counters relative to TotalCounter, in order to keep statistic correct
        P003_Step0counter[event->TaskIndex]   -= P003_pulseTotalCounter[event->TaskIndex] - par1;
        P003_Step1counter[event->TaskIndex]   -= P003_pulseTotalCounter[event->TaskIndex] - par1;
        P003_Step2OKcounter[event->TaskIndex] -= P003_pulseTotalCounter[event->TaskIndex] - par1;
        P003_Step3OKcounter[event->TaskIndex] -= P003_pulseTotalCounter[event->TaskIndex] - par1;
        # endif // P003_PULSE_STATISTIC

        success = true;
      }
      else if (command == F("logpulsestatistic"))
      {
        # ifdef P003_PULSE_STATISTIC

        // Valid command:
        // - [<TaskName/Number>].logpulsestatistic{,<subcommand>}
        //     when "[<TaskName/Number>]." is ommitted, the command applies
        //      to the first active P003 task instance
        //     optional subcommand:
        //       r = reset error and overdue counters after logging
        //       i = increase the log level for regular statstic logs to "info"

        String subcommand = parseString(string, 2);

        if ((subcommand == F("i")) || (subcommand == F("r")) || (subcommand == "")) {
          doStatisticLogging(event->TaskIndex, F("P003+"), P003_PULSE_STATS_ADHOC_LOG_LEVEL);
          doTimingLogging(event->TaskIndex, F("P003+"), P003_PULSE_STATS_ADHOC_LOG_LEVEL);

          if (subcommand == F("i")) { P003_StatsLogLevel[event->TaskIndex] = LOG_LEVEL_INFO; }

          if (subcommand == F("r")) { resetStatsErrorVars(event->TaskIndex); }
          success = true;
        }
        # else // ifdef P003_PULSE_STATISTIC
        success = false; // Command not available
        # endif // P003_PULSE_STATISTIC
      }

      if (mustCallPluginRead) {
        // Note that the set time is before the current moment, so we call the read as soon as possible.
        // The read does also use any set formula and stored the value in RTC.
        Scheduler.schedule_task_device_timer(event->TaskIndex, millis() - 10);
      }
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND: // ** called 50 times per second **
    {
      if (!validP003TaskIndex(event->TaskIndex)) { break; }

      // step 0 will check if a new signal edge is to be processed and then schedule step 1
      doPulseStepProcessing(P003_PROCESSING_STEP_0, event->TaskIndex, PCONFIG(P003_IDX_DEBOUNCETIME));
      break;
    }

    case PLUGIN_TIMER_IN: // ** process scheduled task timer event **
    {
      if (!validP003TaskIndex(event->TaskIndex)) { break; }

      // this function is called when the next (1,2,3) processing step (Par1) is scheduled
      doPulseStepProcessing(event->Par1, event->TaskIndex, PCONFIG(P003_IDX_DEBOUNCETIME));
      break;
    }
  }
  return success;
}

/*********************************************************************************************\
*  Process pulse mode steps
\*********************************************************************************************/
void doPulseStepProcessing(int pStep, taskIndex_t taskIndex, uint16_t debounceTime)
{
  int pinState;

  # ifdef P003_PULSE_STATISTIC
  long overdueTime;
  # endif // ifdef P003_PULSE_STATISTIC

  switch (pStep)
  {
    case P003_PROCESSING_STEP_0:
      // regularily called to check if the trigger has flagged the next signal edge
    {
      if (bitRead(P003_initStepsFlags, taskIndex))
      {
        // schedule step 1 in remaining milliseconds from debounce time
        long delayTime =
          static_cast<long>((static_cast<int64_t>(P003_debounceTime[taskIndex])) -
                            static_cast<int64_t>(getMicros64() - P003_triggerTimestamp[taskIndex])) / 1000L;

        if (delayTime < 0)
        {
          // if debounce time was too short or we were called too late by Scheduler
          # ifdef P003_PULSE_STATISTIC
          P003_Step0ODcounter[taskIndex]++; // count occurences
          P003_StepOverdueMax[taskIndex][pStep] = max(P003_StepOverdueMax[taskIndex][pStep], -delayTime);
          # endif // P003_PULSE_STATISTIC
          delayTime = 0;
        }
        Scheduler.setPluginTaskTimer(delayTime, taskIndex, P003_PROCESSING_STEP_1);

        # ifdef P003_PULSE_STATISTIC
        P003_triggerTimestamp[taskIndex] = getMicros64() + delayTime * 1000L;
        # endif // P003_PULSE_STATISTIC

        // initialization done
        bitClear(P003_initStepsFlags, taskIndex);
      }
      break;
    }

    case P003_PROCESSING_STEP_1: // read pin status
    {
      # ifdef P003_PULSE_STATISTIC
      P003_Step1counter[taskIndex]++;
      overdueTime                           = (long)(getMicros64() - P003_triggerTimestamp[taskIndex]) / 1000L;
      P003_StepOverdueMax[taskIndex][pStep] = max(P003_StepOverdueMax[taskIndex][pStep], overdueTime);
      # endif // P003_PULSE_STATISTIC

      //  read current state from this tasks's GPIO
      P003_lastCheckState[taskIndex] = digitalRead(Settings.TaskDevicePin1[taskIndex]);

      // after debounceTime/2, do step 2
      Scheduler.setPluginTaskTimer(debounceTime >> 1, taskIndex, P003_PROCESSING_STEP_2);

      # ifdef P003_PULSE_STATISTIC
      P003_triggerTimestamp[taskIndex] = getMicros64() + (debounceTime >> 1) * 1000L;
      # endif // P003_PULSE_STATISTIC
      break;
    }

    case P003_PROCESSING_STEP_2: // 1st validation of pin status
    {
      # ifdef P003_PULSE_STATISTIC
      overdueTime                           = (long)(getMicros64() - P003_triggerTimestamp[taskIndex]) / 1000L;
      P003_StepOverdueMax[taskIndex][pStep] = max(P003_StepOverdueMax[taskIndex][pStep], overdueTime);
      # endif // P003_PULSE_STATISTIC

      //  read current state from this tasks's GPIO
      pinState = digitalRead(Settings.TaskDevicePin1[taskIndex]);

      if (pinState == (int)P003_lastCheckState[taskIndex])

      // we found stable state
      {
        # ifdef P003_PULSE_STATISTIC
        P003_Step2OKcounter[taskIndex]++;
        # endif // P003_PULSE_STATISTIC
        // after debounceTime/2, do step 3
        Scheduler.setPluginTaskTimer(debounceTime >> 1, taskIndex, P003_PROCESSING_STEP_3);
      }
      else

      // we found unexpected different pin state
      {
        # ifdef P003_PULSE_STATISTIC
        P003_Step2NOKcounter[taskIndex]++;
        # endif // P003_PULSE_STATISTIC
        // lets ignore previous pin status. It might have been a spike. Try to detect stable signal
        P003_lastCheckState[taskIndex] = pinState;     // now trust the new state
        // after debounceTime/2, do step 2 again
        Scheduler.setPluginTaskTimer(debounceTime >> 1, taskIndex, P003_PROCESSING_STEP_2);
      }

      # ifdef P003_PULSE_STATISTIC
      P003_triggerTimestamp[taskIndex] = getMicros64() + (debounceTime >> 1) * 1000L;
      # endif // P003_PULSE_STATISTIC
      break;
    }

    case P003_PROCESSING_STEP_3: // 2nd validation of pin status and counting
    {
      # ifdef P003_PULSE_STATISTIC
      overdueTime                           = (long)(getMicros64() - P003_triggerTimestamp[taskIndex]) / 1000L;
      P003_StepOverdueMax[taskIndex][pStep] = max(P003_StepOverdueMax[taskIndex][pStep], overdueTime);
      # endif // P003_PULSE_STATISTIC

      // determine earliest effective start time of current stable pulse (= NOW - 2 * DebounceTime )
      uint64_t pulseChangeTime = getMicros64() - (P003_debounceTime[taskIndex] << 1);

      // determine how long the current stable pulse was lasting
      if (P003_currentStableState[taskIndex] == HIGH) // pulse was HIGH
      { P003_pulseHighTime[taskIndex] = pulseChangeTime - P003_currentStableStartTime[taskIndex]; }
      else                                            // pulse was LOW
      { P003_pulseLowTime[taskIndex] = pulseChangeTime - P003_currentStableStartTime[taskIndex]; }

      //  read current state from this tasks's GPIO
      pinState = digitalRead(Settings.TaskDevicePin1[taskIndex]);

      if (pinState == P003_lastCheckState[taskIndex])

      // we found the same state as in step 2. It is stable and valid.
      {
        processStablePulse(taskIndex, pinState, pulseChangeTime);
      }
      else

      // we found unexpected different pin state
      {
        # ifdef P003_PULSE_STATISTIC
        P003_Step3NOKcounter[taskIndex]++;
        # endif // P003_PULSE_STATISTIC

        // ignore spike from previous step. It is regarded as spike within previous=current signal. Again try to detect stable signal
        P003_lastCheckState[taskIndex] = pinState;     // now trust the previous=new state
        Scheduler.setPluginTaskTimer(debounceTime >> 1, taskIndex, P003_PROCESSING_STEP_2);

        # ifdef P003_PULSE_STATISTIC
        P003_triggerTimestamp[taskIndex] = getMicros64() + (debounceTime >> 1) * 1000L;
        # endif // P003_PULSE_STATISTIC
      }
      break;
    }
    default:
    {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        String log; log.reserve(48);
        log = F("_P003:PLUGIN_TIMER_IN: Invalid processingStep: "); log += pStep;
        addLog(LOG_LEVEL_ERROR, log);
      }
      break;
    }
  }
}

/*********************************************************************************************\
*  Processing for found stable pulse
\*********************************************************************************************/
void processStablePulse(taskIndex_t taskIndex, int pinState, uint64_t pulseChangeTime)
{
  if (pinState != P003_currentStableState[taskIndex])

  // The state changed. Previous sable pulse ends, new starts
  {
    # ifdef P003_PULSE_STATISTIC
    P003_Step3OKcounter[taskIndex]++;
    # endif // P003_PULSE_STATISTIC

    // lets terminate the previous pulse and setup start point for new stable one
    P003_currentStableState[taskIndex]     = !P003_currentStableState[taskIndex];
    P003_currentStableStartTime[taskIndex] = pulseChangeTime;

    // now provide the counter result values for the ended pulse ( depending on mode type)
    switch (Settings.TaskDevicePluginConfig[taskIndex][P003_IDX_MODETYPE])
    {
      case PULSE_CHANGE:
      {
        if (P003_currentStableState[taskIndex] == LOW) { // HIGH had ended
          P003_pulseTime[taskIndex] = P003_pulseHighTime[taskIndex];
        }
        else {                                           // LOW has ended
          P003_pulseTime[taskIndex] = P003_pulseLowTime[taskIndex];
        }

        P003_pulseCounter[taskIndex]++;
        P003_pulseTotalCounter[taskIndex]++;
        break;
      }
      case PULSE_HIGH:
      {
        if (P003_currentStableState[taskIndex] == LOW) // HIGH had ended (else do nothing)
        {
          P003_pulseTime[taskIndex] = P003_pulseLowTime[taskIndex] + P003_pulseHighTime[taskIndex];
          P003_pulseCounter[taskIndex]++;
          P003_pulseTotalCounter[taskIndex]++;
        }
        break;
      }
      case PULSE_LOW:
      {
        if (P003_currentStableState[taskIndex] == HIGH) // LOW had ended (else do nothing)
        {
          P003_pulseTime[taskIndex] = P003_pulseLowTime[taskIndex] + P003_pulseHighTime[taskIndex];
          P003_pulseCounter[taskIndex]++;
          P003_pulseTotalCounter[taskIndex]++;
        }
        break;
      }
      default:
      {
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log; log.reserve(48);
          log  = F("_P003:PLUGIN_TIMER_IN: Invalid modeType: ");
          log += Settings.TaskDevicePluginConfig[taskIndex][P003_IDX_MODETYPE];
          addLog(LOG_LEVEL_ERROR, log);
        }
        break;
      }
    }
  }
  else

  // we found the same stable state as before
  {
    # ifdef P003_PULSE_STATISTIC
    P003_Step3IGNcounter[taskIndex]++;
    # endif // P003_PULSE_STATISTIC
    // do nothing. Ignore interupt. previous stable state was confirmed probably after a spike
  }

  # ifdef P003_PULSE_STATISTIC
  doStatisticLogging(taskIndex, F("P003:"), P003_StatsLogLevel[taskIndex]);
  # endif // P003_PULSE_STATISTIC

  // allow next pulse check call from interrupt
  bitClear(P003_processingFlags, taskIndex);
}

# ifdef P003_PULSE_STATISTIC

/*********************************************************************************************\
*  reset statistical error cunters and overview variables
\*********************************************************************************************/
void resetStatsErrorVars(taskIndex_t taskIndex) {
  // initialize statistical step counters from TotalCounter and error counters with 0
  P003_Step2NOKcounter[taskIndex] = 0;
  P003_Step3NOKcounter[taskIndex] = 0;
  P003_Step3IGNcounter[taskIndex] = 0;
  P003_Step0ODcounter[taskIndex]  = 0;

  for (int pStep = 0; pStep <= P003_PSTEP_MAX; pStep++) { P003_StepOverdueMax[taskIndex][pStep] = 0; }
}

/*********************************************************************************************\
*  write statistic counters to logfile
\*********************************************************************************************/
void doStatisticLogging(taskIndex_t taskIndex, String logPrefix, byte logLevel)
{
  if (loglevelActiveFor(logLevel)) {
    // Statistic to logfile. E.g: ... [123/1|111|100/5|80/3/4|40] [12243|3244]
    String log; log.reserve(125);
    log  = logPrefix;
    log += F("Stats (taskId) [step0|1|2|3|tot(ok/nok/ign)] [lo|hi]= (");
    log += taskIndex;                           log += F(") [");
    log += P003_Step0counter[taskIndex];        log += F("|");
    log += P003_Step1counter[taskIndex];        log += F("|");
    log += P003_Step2OKcounter[taskIndex];      log += F("/");
    log += P003_Step2NOKcounter[taskIndex];     log += F("|");
    log += P003_Step3OKcounter[taskIndex];      log += F("/");
    log += P003_Step3NOKcounter[taskIndex];     log += F("/");
    log += P003_Step3IGNcounter[taskIndex];     log += F("|");
    log += P003_pulseTotalCounter[taskIndex];   log += F("] [");
    log += P003_pulseLowTime[taskIndex] / 1000L;  log += F("|");
    log += P003_pulseHighTime[taskIndex] / 1000L; log += F("]");
    addLog(logLevel, log);
  }
}

/*********************************************************************************************\
*  write collected timing values to logfile
\*********************************************************************************************/
void doTimingLogging(taskIndex_t taskIndex, String logPrefix, byte logLevel)
{
  if (loglevelActiveFor(logLevel)) {
    // Timer to logfile. E.g: ... [4|12000|13444|12243|3244]
    String log; log.reserve(120);
    log  = logPrefix;
    log += F("OverDueStats (taskId) [dbTim] {step0OdCnt} [maxOdTimeStep0|1|2|3]= (");
    log += taskIndex;  log += F(") [");
    log += Settings.TaskDevicePluginConfig[taskIndex][P003_IDX_DEBOUNCETIME];  log += F("] {");
    log += P003_Step0ODcounter[taskIndex];  log += F("} [");

    for (int pStep = 0; pStep <= P003_PSTEP_MAX; pStep++) {
      log += P003_StepOverdueMax[taskIndex][pStep];

      if (pStep < P003_PSTEP_MAX) { log += F("|"); }
    }
    log += F("]");
    addLog(logLevel, log);
  }
}

# endif // P003_PULSE_STATISTIC

/*********************************************************************************************\
* Check if task Index is valid for P003
\*********************************************************************************************/
bool validP003TaskIndex(taskIndex_t taskIndex) {
  if (loglevelActiveFor(LOG_LEVEL_ERROR) && (taskIndex >= P003_TASKS_MAX)) {
    addLog(LOG_LEVEL_ERROR, F("PULSE: Error, only the first 4 tasks can be pulse counters."));
  }
  return taskIndex < P003_TASKS_MAX;
}

/*********************************************************************************************\
* Check Pulse Counters (called from irq handler)
\*********************************************************************************************/
void Plugin_003_pulsecheck(taskIndex_t taskID)
{
  noInterrupts(); // s0170071: avoid nested interrups due to bouncing.

  if ((Settings.TaskDevicePluginConfig[taskID][P003_IDX_MODETYPE] & PULSE_MODE_MASK) == 0)

  // legathy edge Mode types
  {
    // KP: we use here P003_currentStableStartTime[taskID] to persist the PulseTime (pulseTimePrevious)
    const uint64_t timeSinceLastTrigger = getMicros64() - P003_currentStableStartTime[taskID];

    if (timeSinceLastTrigger > P003_debounceTime[taskID]) // check with debounce time for this task
    {
      P003_pulseCounter[taskID]++;
      P003_pulseTotalCounter[taskID]++;
      P003_pulseTime[taskID]              = timeSinceLastTrigger;
      P003_currentStableStartTime[taskID] = getMicros64(); // reset when counted to determine interval between counted pulses
    }
  }
  else

  // processing for new PULSE mode types
  {
    # ifdef P003_PULSE_STATISTIC
    P003_Step0counter[taskID]++;
    # endif // P003_PULSE_STATISTIC

    // check if processing is allowed (not blocked) for this task (taskID)
    if (!bitRead(P003_processingFlags, taskID))
    {
      // initiate processing
      bitSet(P003_processingFlags, taskID); // block further initiations as long as async processing is taking place
      bitSet(P003_initStepsFlags,  taskID); // PLUGIN_FIFTY_PER_SECOND is polling for this flag set
      P003_triggerTimestamp[taskID] = getMicros64();
    }
  }
  interrupts(); // enable interrupts again.
}

/*********************************************************************************************\
* Pulse Counter IRQ handlers
\*********************************************************************************************/
void Plugin_003_pulse_interrupt1()
{
  Plugin_003_pulsecheck(0);
}

void Plugin_003_pulse_interrupt2()
{
  Plugin_003_pulsecheck(1);
}

void Plugin_003_pulse_interrupt3()
{
  Plugin_003_pulsecheck(2);
}

void Plugin_003_pulse_interrupt4()
{
  Plugin_003_pulsecheck(3);
}

// void Plugin_003_pulse_interrupt5()
// {
//   Plugin_003_pulsecheck(4);
// }
//
// void Plugin_003_pulse_interrupt6()
// {
//   Plugin_003_pulsecheck(5);
// }
//
// void Plugin_003_pulse_interrupt7()
// {
//   Plugin_003_pulsecheck(6);
// }
//
// void Plugin_003_pulse_interrupt8()
// {
//   Plugin_003_pulsecheck(7);
// }

/*********************************************************************************************\
* Init Pulse Counters
\*********************************************************************************************/
bool Plugin_003_pulseinit(byte Par1, taskIndex_t taskID, byte Mode)
{
  switch (taskID)
  {
    case 0:
      attachInterrupt(Par1, Plugin_003_pulse_interrupt1, Mode);
      break;
    case 1:
      attachInterrupt(Par1, Plugin_003_pulse_interrupt2, Mode);
      break;
    case 2:
      attachInterrupt(Par1, Plugin_003_pulse_interrupt3, Mode);
      break;
    case 3:
      attachInterrupt(Par1, Plugin_003_pulse_interrupt4, Mode);
      break;

    // case 4:
    //   attachInterrupt(Par1, Plugin_003_pulse_interrupt5, Mode);
    //   break;
    // case 5:
    //   attachInterrupt(Par1, Plugin_003_pulse_interrupt6, Mode);
    //   break;
    // case 6:
    //   attachInterrupt(Par1, Plugin_003_pulse_interrupt7, Mode);
    //   break;
    // case 7:
    //   attachInterrupt(Par1, Plugin_003_pulse_interrupt8, Mode);
    //   break;
    default:
      addLog(LOG_LEVEL_ERROR, F("PULSE: Error, only the first 4 tasks can be pulse counters."));
      return false;
  }

  return true;
}

#endif // USES_P003
