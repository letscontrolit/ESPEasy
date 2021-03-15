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

#include "src/Helpers/ESPEasy_time_calc.h"

// additional debug/tuning messages from PULSE mode into the logfile
#define P003_PULSE_STATISTIC
#define P003_PULSE_STATS_LOG_LEVEL     LOG_LEVEL_DEBUG

#define PLUGIN_003
#define PLUGIN_ID_003                  3
#define PLUGIN_NAME_003                "Generic - Pulse counter"
// number and name of counted values 
#define PLUGIN_NR_VALUENAMES_003       3
#define PLUGIN_VALUENAME1_003          "Count"
#define PLUGIN_VALUENAME2_003          "Total"
#define PLUGIN_VALUENAME3_003          "Time"
// ... their index in UserVar and TaskDeviceValueNames
#define P003_IDX_pulseCounter           0
#define P003_IDX_pulseTotalCounter      1
#define P003_IDX_pulseTime              2
// ... and the following index into UserVar for storing the persisted TotalCounter
#define P003_IDX_persistedTotalCounter  3

// indexes for config parameters
#define P003_IDX_DEBOUNCETIME   0 
#define P003_IDX_COUNTERTYPE    1
#define P003_IDX_MODETYPE       2
// special Mode Type. Note: Lower 3 bits are significant for GPIO Interupt type. The upper bits distinguish the Mode Types
#define PULSE_LOW               (0x10|CHANGE)
#define PULSE_HIGH              (0x20|CHANGE)
#define PULSE_CHANGE            (0x30|CHANGE)
#define PULSE_MODE_MASK         0x30
#define MODE_INTERRUPT_MASK     0x03

// values for WEBFORM Counter Types
#define P003_NR_COUNTERTYPES               4
#define P003_COUNTERTYPE_LIST { F("Delta"), F("Delta/Total/Time"), F("Total"), F("Delta/Total"), }
#define P003_CT_INDEX_COUNTER              0
#define P003_CT_INDEX_COUNTER_TOTAL_TIME   1
#define P003_CT_INDEX_TOTAL                2
#define P003_CT_INDEX_COUNTER_TOTAL        3
// processing Steps in PLUGIN_TIMER_IN
#define P003_PROCESSING_STEP_1             1
#define P003_PROCESSING_STEP_2             2
#define P003_PROCESSING_STEP_3             3

void Plugin_003_pulse_interrupt1() ICACHE_RAM_ATTR;
void Plugin_003_pulse_interrupt2() ICACHE_RAM_ATTR;
void Plugin_003_pulse_interrupt3() ICACHE_RAM_ATTR;
void Plugin_003_pulse_interrupt4() ICACHE_RAM_ATTR;
void Plugin_003_pulsecheck(byte taskID) ICACHE_RAM_ATTR;

// this takes 20 bytes of IRAM per handler
// void Plugin_003_pulse_interrupt5() ICACHE_RAM_ATTR;
// void Plugin_003_pulse_interrupt6() ICACHE_RAM_ATTR;
// void Plugin_003_pulse_interrupt7() ICACHE_RAM_ATTR;
// void Plugin_003_pulse_interrupt8() ICACHE_RAM_ATTR;

// volatile counter variables for use in ISR
volatile unsigned long P003_pulseCounter[TASKS_MAX];           // number of counted pulses within most recent data collection/sent interval 
volatile unsigned long P003_pulseTotalCounter[TASKS_MAX];      // total number of pulses counted since last reset
volatile uint64_t      P003_pulseTime[TASKS_MAX];              // time between previous and most recently counted edge/pulse
volatile uint64_t      P003_currentStableStartTime[TASKS_MAX]; // stores the start time of the current stable pulse.
volatile uint64_t      P003_debounceTime[TASKS_MAX];           // 64 bit version of PCONFIG(P003_IDX_DEBOUNCETIME) in micoseconds 
volatile int 	         P003_initStepsFlags;                    // indicates that the pulse processing steps shall be initiated. One bit per task. 
volatile int 	         P003_processingFlags;                   // indicates pulse processing is running and interrupts must be ignored. One bit per task.
volatile uint64_t      P003_triggerTimestamp [TASKS_MAX];      // timestamp, when the signal change was detected in the ISR

// internal variables for PULSE mode
unsigned long           P003_pulseLowTime[TASKS_MAX];           // indicates the length of the most recent stable low pulse (in ms)
unsigned long           P003_pulseHighTime[TASKS_MAX];          // indicates the length of the most recent stable high pulse (in ms)
int                     P003_currentStableState[TASKS_MAX];     // stores current stable pin state. Set in Step 3 when new stable pulse started
int                     P003_lastCheckState[TASKS_MAX];         // most recent pin state, that was read. Set in Step1,2,3

#ifdef P003_PULSE_STATISTIC
// debug/tuning variables for PULSE mode statistical logging
volatile unsigned int P003_Step0counter[TASKS_MAX]; // counts how often step 0 was entered (volatile <- in ISR)
unsigned int P003_Step1counter[TASKS_MAX];          // counts how often step 1 was entered
unsigned int P003_Step2OKcounter[TASKS_MAX];        // counts how often step 2 detected the expected pin state (first verification)
unsigned int P003_Step2NOKcounter[TASKS_MAX];       // counts how often step 2 detected the wrong pin state (first verification failed)
unsigned int P003_Step3OKcounter[TASKS_MAX];        // counts how often step 3 detected the expected pin state (2nd verification)
unsigned int P003_Step3NOKcounter[TASKS_MAX];       // counts how often step 3 detected the wrong pin state (2nd verification failed)
unsigned int P003_Step3IGNcounter[TASKS_MAX];       // counts how often step 3 detected the wrong pin state (2nd verification failed)
#endif // P003_PULSE_STATISTIC


boolean Plugin_003(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  String log;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:                  // ** Set my technical parameters as ESPEasy device **
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

    case PLUGIN_GET_DEVICENAME:               // ** deliver my own device name **
    {
      string = F(PLUGIN_NAME_003);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:         // ** define the names for the data that the plugin provides **
    {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_003));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_003));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_003));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:          // ** return the pin name as show in the configuration webform **
    {
      event->String1 = formatGpioName_input(F("Pulse"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:                 // ** add configuration entry boxes into webform html-code **
    {
      addFormNumericBox(F("Debounce Time (mSec)"), F("p003")
                        , PCONFIG(P003_IDX_DEBOUNCETIME));

      byte   choice     = PCONFIG(P003_IDX_COUNTERTYPE);
      byte   choice2    = PCONFIG(P003_IDX_MODETYPE);
      String options[P003_NR_COUNTERTYPES] = P003_COUNTERTYPE_LIST;
      addFormSelector(F("Counter Type"), F("p003_countertype"), P003_NR_COUNTERTYPES, options, NULL, choice);

      if (choice != 0) {
        addHtml(F("<span style=\"color:red\">Total count is not persistent!</span>"));
      }

      #define P003_NR_MODETYPES  7
      String modeRaise[P003_NR_MODETYPES];   // displayed texts
      // KP: correction for modeRaise[0]: LOW=0 did not generate interupts and does not make sense. Thus it changed to "none"
      //      Note: A correction to ONLOW = 0x04 (cf. Arduino.h) causes problems as it fires consecutive interupts, while GPIO is low 
      modeRaise[0] = F("none");
      modeRaise[1] = F("CHANGE");
      modeRaise[2] = F("RISING");
      modeRaise[3] = F("FALLING");
      modeRaise[4] = F("PULSE low");
      modeRaise[5] = F("PULSE high");
      modeRaise[6] = F("PULSE change");
      
      int modeValues[P003_NR_MODETYPES];  // trigger flags
      modeValues[0] = 0;    // KP: replaced LOW by 0, as it does nothing
      modeValues[1] = CHANGE;
      modeValues[2] = RISING;
      modeValues[3] = FALLING;
      modeValues[4] = PULSE_LOW;
      modeValues[5] = PULSE_HIGH;
      modeValues[6] = PULSE_CHANGE;

      addFormSelector(F("Mode Type"), F("p003_raisetype"), P003_NR_MODETYPES, modeRaise, modeValues, choice2);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:                 // ** save configuration date as provided by webform **
    {
      PCONFIG(P003_IDX_DEBOUNCETIME)  = getFormItemInt(F("p003"));
      PCONFIG(P003_IDX_COUNTERTYPE)   = getFormItemInt(F("p003_countertype"));
      PCONFIG(P003_IDX_MODETYPE)      = getFormItemInt(F("p003_raisetype"));
      success                     = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_VALUES:          // ** provide current plugin data to webform  **
    {
          pluginWebformShowValue(ExtraTaskSettings.TaskDeviceValueNames[P003_IDX_pulseCounter],      String(P003_pulseCounter[event->TaskIndex]));
          pluginWebformShowValue(ExtraTaskSettings.TaskDeviceValueNames[P003_IDX_pulseTotalCounter], String(P003_pulseTotalCounter[event->TaskIndex]));
          pluginWebformShowValue(ExtraTaskSettings.TaskDeviceValueNames[P003_IDX_pulseTime],         String(P003_pulseTime[event->TaskIndex]/1000.0f), false);
      success = true;
      break;
    }

    case PLUGIN_INIT:                         // ** initialize plugin **
    {
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
          P003_pulseTime[event->TaskIndex]         = UserVar[event->BaseVarIndex + P003_IDX_pulseTime]*1000L;
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
      P003_debounceTime[event->TaskIndex] = (uint64_t)Settings.TaskDevicePluginConfig[event->TaskIndex][P003_IDX_DEBOUNCETIME]*1000L;

      // task indexes larger than 32 should never happen
      if ( event->TaskIndex > sizeof(P003_initStepsFlags)*8 ) {
        // P003_initStepsFlags and P003_processingFlags can only serve as much tasks as their size in bits (32)
        log =  F("P003: Error! TaskIndex "); log += event->TaskIndex; log += F("is too large");
        addLog(LOG_LEVEL_ERROR, log);
        break;
      }    

      // initialize internal variables for PULSE mode handling
      bitClear(P003_initStepsFlags, event->TaskIndex);
      bitClear(P003_processingFlags, event->TaskIndex);
      P003_pulseLowTime[event->TaskIndex]           = 0;
      P003_pulseHighTime[event->TaskIndex]          = 0;
      P003_currentStableState[event->TaskIndex]     = ((PCONFIG(P003_IDX_MODETYPE) == PULSE_LOW) ? HIGH : LOW );
      P003_currentStableStartTime[event->TaskIndex] = 0;
      P003_lastCheckState[event->TaskIndex]         = HIGH;    

      #ifdef P003_PULSE_STATISTIC
      // initialize statistical step counters from TotalCounter and error counters with 0
      P003_Step0counter[event->TaskIndex]     = P003_pulseTotalCounter[event->TaskIndex];
      P003_Step1counter[event->TaskIndex]     = P003_pulseTotalCounter[event->TaskIndex];
      P003_Step2OKcounter[event->TaskIndex]   = P003_pulseTotalCounter[event->TaskIndex];
      P003_Step2NOKcounter[event->TaskIndex]  = 0;
      P003_Step3OKcounter[event->TaskIndex]   = P003_pulseTotalCounter[event->TaskIndex];
      P003_Step3NOKcounter[event->TaskIndex]  = 0;
      P003_Step3IGNcounter[event->TaskIndex]  = 0;
      #endif // P003_PULSE_STATISTIC

      log = F("INIT : PulsePin: ");
      log += Settings.TaskDevicePin1[event->TaskIndex];
      addLog(LOG_LEVEL_INFO, log);
      
      // set up device pin and estabish interupt handlers
      pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT_PULLUP);
      success =
        Plugin_003_pulseinit(Settings.TaskDevicePin1[event->TaskIndex], event->TaskIndex,
                             (PCONFIG(P003_IDX_MODETYPE) & MODE_INTERRUPT_MASK));

      break;
    }

    case PLUGIN_READ:                         // ** PLUGIN_READ **
    {
      // store the current counter values into UserVar (RTC-memory)
      // FIXME TD-er: Is it correct to write the first 3  UserVar values, regardless the set counter type?
      UserVar[event->BaseVarIndex + P003_IDX_pulseCounter]      = P003_pulseCounter[event->TaskIndex];
      UserVar[event->BaseVarIndex + P003_IDX_pulseTotalCounter] = P003_pulseTotalCounter[event->TaskIndex];
      UserVar[event->BaseVarIndex + P003_IDX_pulseTime]         = P003_pulseTime[event->TaskIndex]/1000.0f;
      
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

    case PLUGIN_WRITE:                        // ** execute functional command **
    {
      String command            = parseString(string, 1);
      bool   mustCallPluginRead = false;

      if (command == F("resetpulsecounter"))
      {
        // Valid commands:
        // - resetpulsecounter
        // - resetpulsecounter,taskindex

        // Allow for an optional taskIndex parameter. When not given it will take the first task with this plugin.
        if (!pluginOptionalTaskIndexArgumentMatch(event->TaskIndex, string, 1)) {
          break;
        }

        #ifdef P003_PULSE_STATISTIC
        // reduce the statistical step counters by the current value of TotalCounter in order to keep statistic correct
        P003_Step0counter[event->TaskIndex]     -= P003_pulseTotalCounter[event->TaskIndex];
        P003_Step1counter[event->TaskIndex]     -= P003_pulseTotalCounter[event->TaskIndex];
        P003_Step2OKcounter[event->TaskIndex]   -= P003_pulseTotalCounter[event->TaskIndex];
        P003_Step3OKcounter[event->TaskIndex]   -= P003_pulseTotalCounter[event->TaskIndex];
        #endif // P003_PULSE_STATISTIC

        P003_pulseCounter[event->TaskIndex]      = 0;
        P003_pulseTotalCounter[event->TaskIndex] = 0;
        P003_pulseTime[event->TaskIndex]         = 0;

        mustCallPluginRead                       = true;
        success                                  = true; // Command is handled.
      } 
      else if (command == F("setpulsecountertotal"))
      {
        // Valid commands:
        // - setpulsecountertotal,value
        // - setpulsecountertotal,value,taskindex

        // First check if (optional) task index matches.
        if (!pluginOptionalTaskIndexArgumentMatch(event->TaskIndex, string, 2)) {
          break;
        }

        int par1;

        if (validIntFromString(parseString(string, 2), par1))
        {
          #ifdef P003_PULSE_STATISTIC
          // adjust the statistical step counters relative to TotalCounter, in order to keep statistic correct
          P003_Step0counter[event->TaskIndex]     -= P003_pulseTotalCounter[event->TaskIndex] - par1;
          P003_Step1counter[event->TaskIndex]     -= P003_pulseTotalCounter[event->TaskIndex] - par1;
          P003_Step2OKcounter[event->TaskIndex]   -= P003_pulseTotalCounter[event->TaskIndex] - par1;
          P003_Step3OKcounter[event->TaskIndex]   -= P003_pulseTotalCounter[event->TaskIndex] - par1;
          #endif // P003_PULSE_STATISTIC

          P003_pulseTotalCounter[event->TaskIndex] = par1;

          mustCallPluginRead                       = true;
          success                                  = true; // Command is handled.
        }
      }
      else if (command == F("logpulsestatistic"))
      {
        #ifdef P003_PULSE_STATISTIC
          // Valid commands:
          // - logpulsestatistic
          // - logpulsestatistic,taskindex

          // Allow for an optional taskIndex parameter. When not given it will take the first task of this plugin.
          if (!pluginOptionalTaskIndexArgumentMatch(event->TaskIndex, string, 1)) {
            break;
          }

          doStatisticLogging(event->TaskIndex, F("P003+"));
          success = true; // Command is handled.
        #else
          success = false; // Command not available
        #endif // P003_PULSE_STATISTIC
      }

      if (mustCallPluginRead) {
        // Note that the set time is before the current moment, so we call the read as soon as possible.
        // The read does also use any set formula and stored the value in RTC.
        Scheduler.schedule_task_device_timer(event->TaskIndex, millis() - 10);
      }
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:             // ** called 50 times per second **
    {
      // if initialization step is needed for any task with Pulse Mode
      if( P003_initStepsFlags )
      {
        for ( int taskID = 0; taskID < TASKS_MAX; taskID++ )
        {
          if( bitRead(P003_initStepsFlags, taskID) )
          {
            // schedule step 1 in remaining milliseconds from debounce time
            Scheduler.setPluginTaskTimer((long)(P003_debounceTime[taskID]-(getMicros64()-P003_triggerTimestamp[taskID]))/1000L, \
                                         taskID, P003_PROCESSING_STEP_1);
            // initialization done 
            bitClear(P003_initStepsFlags, taskID);
          }
        }
      }
      break;
    }
 
    case PLUGIN_TIMER_IN:                     // ** process scheduled task timer event **
    {
      // this function is called when the next processing step is scheduled
      int pinState;
      int processingStep = event->Par1;

      switch (processingStep)
      {
        case P003_PROCESSING_STEP_1:   // read pin status
        {
          #ifdef P003_PULSE_STATISTIC
          P003_Step1counter[event->TaskIndex]++;
          #endif // P003_PULSE_STATISTIC
          //  read current state from this tasks's GPIO
          P003_lastCheckState[event->TaskIndex] = digitalRead(Settings.TaskDevicePin1[event->TaskIndex]);
          // after debounceTime/2, do step 2          
          Scheduler.setPluginTaskTimer(PCONFIG(P003_IDX_DEBOUNCETIME) >> 1, event->TaskIndex, P003_PROCESSING_STEP_2);
          break;  
        }
        
        case P003_PROCESSING_STEP_2:   // 1st validation of pin status
        {
          //  read current state from this tasks's GPIO
          pinState = digitalRead(Settings.TaskDevicePin1[event->TaskIndex]);

          if (pinState == (int)P003_lastCheckState[event->TaskIndex] )
          // we found stable state
          {
            #ifdef P003_PULSE_STATISTIC
            P003_Step2OKcounter[event->TaskIndex]++;
            #endif // P003_PULSE_STATISTIC
            // after debounceTime/2, do step 3          
            Scheduler.setPluginTaskTimer(PCONFIG(P003_IDX_DEBOUNCETIME >> 1), event->TaskIndex, P003_PROCESSING_STEP_3);
          }
          else
          // we found unexpected different pin state
          {
            #ifdef P003_PULSE_STATISTIC
            P003_Step2NOKcounter[event->TaskIndex]++;
            #endif // P003_PULSE_STATISTIC
            // lets ignore previous pin status. It might have been a spike. Try to detect stable signal
            P003_lastCheckState [event->TaskIndex] = pinState;    // now trust the new state
            // after debounceTime/2, do step 2 again     
            Scheduler.setPluginTaskTimer(PCONFIG(P003_IDX_DEBOUNCETIME >> 1), event->TaskIndex, P003_PROCESSING_STEP_2);
          }      
          break;  
        }

        case P003_PROCESSING_STEP_3:  // 2nd validation of pin status and counting
        {
          // determine earliest effective start time of current stable pulse (= NOW - 2 * DebounceTime )
          uint64_t pulseChangeTime = getMicros64() - (P003_debounceTime[event->TaskIndex] << 1);

          // determine how long the current stable pulse was lasting
          if ( P003_currentStableState[event->TaskIndex] == HIGH )  // pulse was HIGH
              { P003_pulseHighTime[event->TaskIndex] = pulseChangeTime - P003_currentStableStartTime[event->TaskIndex]; }
          else    // pulse was LOW
               { P003_pulseLowTime[event->TaskIndex]  = pulseChangeTime - P003_currentStableStartTime[event->TaskIndex]; }  
              
          //  read current state from this tasks's GPIO
          pinState = digitalRead(Settings.TaskDevicePin1[event->TaskIndex]);

          if (pinState == P003_lastCheckState[event->TaskIndex] )
          // we found the same state as in step 2. It is stable and valid.
          {
            if (pinState != P003_currentStableState[event->TaskIndex] )
            // The state changed. Previous sable pulse ends, new starts
            {
              #ifdef P003_PULSE_STATISTIC
              P003_Step3OKcounter[event->TaskIndex]++;
              #endif // P003_PULSE_STATISTIC

              // lets terminate the previous pulse and setup start point for new stable one
              P003_currentStableState[event->TaskIndex]     = !P003_currentStableState[event->TaskIndex];
              P003_currentStableStartTime[event->TaskIndex] = pulseChangeTime;       

              // now provide the counter result values for the ended pulse ( depending on mode type)
              switch (Settings.TaskDevicePluginConfig[event->TaskIndex][P003_IDX_MODETYPE])
              {
                case PULSE_CHANGE:
                {
                  if (P003_currentStableState[event->TaskIndex] == LOW )  // HIGH had ended
                    P003_pulseTime[event->TaskIndex] = P003_pulseHighTime[event->TaskIndex];
                  else    // LOW has ended
                    P003_pulseTime[event->TaskIndex] = P003_pulseLowTime[event->TaskIndex];

                  P003_pulseCounter[event->TaskIndex]++;
                  P003_pulseTotalCounter[event->TaskIndex]++;
                  break;
                }
                case PULSE_HIGH:
                {
                  if ( P003_currentStableState[event->TaskIndex] == LOW )  // HIGH had ended (else do nothing)
                  {
                    P003_pulseTime[event->TaskIndex] = P003_pulseLowTime[event->TaskIndex] + P003_pulseHighTime[event->TaskIndex];
                    P003_pulseCounter[event->TaskIndex]++;
                    P003_pulseTotalCounter[event->TaskIndex]++;
                  }
                  break;
                }
                case PULSE_LOW:
                {
                  if ( P003_currentStableState[event->TaskIndex] == HIGH )  // LOW had ended (else do nothing)
                  {
                    P003_pulseTime[event->TaskIndex] = P003_pulseLowTime[event->TaskIndex] + P003_pulseHighTime[event->TaskIndex];
                    P003_pulseCounter[event->TaskIndex]++;
                    P003_pulseTotalCounter[event->TaskIndex]++;
                  }
                  break;
                }
                default:
                {
                  log = F("_P003:PLUGIN_TIMER_IN: Invalid modeType: ");
                  log += Settings.TaskDevicePluginConfig[event->TaskIndex][P003_IDX_MODETYPE];
                  addLog(LOG_LEVEL_ERROR, log);
                  break;
                }
              }
            }
            else 
            // we found the same stable state as before
            {
              #ifdef P003_PULSE_STATISTIC
              P003_Step3IGNcounter[event->TaskIndex]++;
              #endif // P003_PULSE_STATISTIC
              // do nothing. Ignore interupt. previous stable state was confirmed probably after a spike
            }

            #ifdef P003_PULSE_STATISTIC
            doStatisticLogging(event->TaskIndex, F("P003:"));
            #endif // P003_PULSE_STATISTIC

            // allow next pulse check call from interrupt
            bitClear(P003_processingFlags, event->TaskIndex);
          }
          else
          // we found unexpected different pin state
          {
            #ifdef P003_PULSE_STATISTIC
            P003_Step3NOKcounter[event->TaskIndex]++;
            #endif // P003_PULSE_STATISTIC
            // ignore spike from previous step. It is regarded as spike within previous=current signal. Again try to detect stable signal
            P003_lastCheckState [event->TaskIndex] = pinState;    // now trust the previous=new state
            Scheduler.setPluginTaskTimer(PCONFIG(P003_IDX_DEBOUNCETIME) >> 1, event->TaskIndex, P003_PROCESSING_STEP_2);
          }
          break;  
        }
        default:
        {
          log = F("_P003:PLUGIN_TIMER_IN: Invalid processingStep: ");
          log += processingStep;
          addLog(LOG_LEVEL_ERROR, log);
          break;
        }
      }
      break;
    }

  }
  return success;
}

#ifdef P003_PULSE_STATISTIC
/*********************************************************************************************\
* write statistic counters to logfile
\*********************************************************************************************/
void doStatisticLogging(taskIndex_t TaskIndex, String logPrefix)
{
  // Statistic to logfile. E.g: ... [123|111|100/5|80/3/4|40] [12243|3244]
  String log = logPrefix;
  log += F("Stats[step0|1|2|3|tot(ok/nok/ign)] [lo|hi]: [");
  log += P003_Step0counter[TaskIndex];        log += "|";
  log += P003_Step1counter[TaskIndex];        log += "|";
  log += P003_Step2OKcounter[TaskIndex];      log += "/";
  log += P003_Step2NOKcounter[TaskIndex];     log += "|";
  log += P003_Step3OKcounter[TaskIndex];      log += "/";
  log += P003_Step3NOKcounter[TaskIndex];     log += "/";
  log += P003_Step3IGNcounter[TaskIndex];     log += "|";
  log += P003_pulseTotalCounter[TaskIndex];   log += "] [";
  log += P003_pulseLowTime[TaskIndex]/1000L;  log += "|";
  log += P003_pulseHighTime[TaskIndex]/1000L; log += "]";
  addLog(P003_PULSE_STATS_LOG_LEVEL, log);
}
#endif // P003_PULSE_STATISTIC

/*********************************************************************************************\
* Check Pulse Counters (called from irq handler)
\*********************************************************************************************/
void Plugin_003_pulsecheck(byte taskID)
{
  noInterrupts(); // s0170071: avoid nested interrups due to bouncing.
   
  if ( (Settings.TaskDevicePluginConfig[taskID][P003_IDX_MODETYPE] & PULSE_MODE_MASK) == 0 )
  // legathy edge Mode types
  {
    // KP: we use here P003_currentStableStartTime[taskID] to persist the PulseTime (pulseTimePrevious)
    const uint64_t timeSinceLastTrigger = getMicros64() - P003_currentStableStartTime[taskID];

    if (timeSinceLastTrigger > P003_debounceTime[taskID]) // check with debounce time for this task
    {
      P003_pulseCounter[taskID]++;
      P003_pulseTotalCounter[taskID]++;
      P003_pulseTime[taskID]               = timeSinceLastTrigger;
      P003_currentStableStartTime[taskID]  = getMicros64();  // reset when counted to determine interval between counted pulses
    }
  }
  else
  // processing for new PULSE mode types
  {
    #ifdef P003_PULSE_STATISTIC
    P003_Step0counter[taskID]++;
    #endif // P003_PULSE_STATISTIC

    // check if processing is allowed (not blocked) for this task (taskID)
    if( !bitRead(P003_processingFlags, taskID))
    {
      // initiate processing
      bitSet(P003_processingFlags, taskID); // block further initiations as long as async processing is taking place
      bitSet(P003_initStepsFlags, taskID);  // PLUGIN_FIFTY_PER_SECOND is polling for this flag set
      P003_triggerTimestamp [taskID]  = getMicros64();
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

void Plugin_003_pulse_interrupt5()
{
  Plugin_003_pulsecheck(4);
}

void Plugin_003_pulse_interrupt6()
{
  Plugin_003_pulsecheck(5);
}

void Plugin_003_pulse_interrupt7()
{
  Plugin_003_pulsecheck(6);
}

void Plugin_003_pulse_interrupt8()
{
  Plugin_003_pulsecheck(7);
}

/*********************************************************************************************\
* Init Pulse Counters
\*********************************************************************************************/
bool Plugin_003_pulseinit(byte Par1, byte taskID, byte Mode)
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
