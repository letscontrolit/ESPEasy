#ifndef HELPERS__INTERNAL_GPIO_PULSEHELPER_H
#define HELPERS__INTERNAL_GPIO_PULSEHELPER_H

#include "../../ESPEasy_common.h"

#include "../DataTypes/TaskIndex.h"


// additional debug/tuning messages from PULSE mode into the logfile
#ifndef LIMIT_BUILD_SIZE
  # define PULSE_STATISTIC
#endif // ifndef LIMIT_BUILD_SIZE

#ifdef PULSE_STATISTIC
  # include "../ESPEasyCore/ESPEasy_Log.h"
  # define PULSE_STATS_DEFAULT_LOG_LEVEL  LOG_LEVEL_DEBUG
  # define PULSE_STATS_ADHOC_LOG_LEVEL    LOG_LEVEL_INFO
#endif // ifdef PULSE_STATISTIC

// processing Steps in PLUGIN_TIMER_IN
#define GPIO_PULSE_HELPER_PROCESSING_STEP_0             0
#define GPIO_PULSE_HELPER_PROCESSING_STEP_1             1
#define GPIO_PULSE_HELPER_PROCESSING_STEP_2             2
#define GPIO_PULSE_HELPER_PROCESSING_STEP_3             3
#define P003_PSTEP_MAX                     GPIO_PULSE_HELPER_PROCESSING_STEP_3

// special Mode Type. Note: Lower 3 bits are significant for GPIO Interupt type. The upper bits distinguish the Mode Types
#define PULSE_LOW               (0x10 | CHANGE)
#define PULSE_HIGH              (0x20 | CHANGE)
#define PULSE_CHANGE            (0x30 | CHANGE)
#define PULSE_MODE_MASK         0x30
#define MODE_INTERRUPT_MASK     0x03


// volatile counter variables for use in ISR
struct pulseCounterISRdata_t {
  uint64_t      pulseTime              = 0; // time between previous and most recently counted edge/pulse
  uint64_t      currentStableStartTime = 0; // stores the start time of the current stable pulse.
  uint64_t      triggerTimestamp       = 0; // timestamp, when the signal change was detected in the ISR
  unsigned long pulseCounter           = 0; // number of counted pulses within most recent data collection/sent interval
  unsigned long pulseTotalCounter      = 0; // total number of pulses counted since last reset
  #ifdef PULSE_STATISTIC

  // debug/tuning variables for PULSE mode statistical logging
  unsigned int Step0counter = 0; // counts how often step 0 was entered (volatile <- in ISR)
  #endif // ifdef PULSE_STATISTIC

  bool initStepsFlags  = false;  // indicates that the pulse processing steps shall be initiated. One bit per task.
  bool processingFlags = false;  // indicates pulse processing is running and interrupts must be ignored. One bit per task.
};

// internal variables for PULSE mode, not used by ISR functions
struct pulseModeData_t {
  unsigned long pulseLowTime       = 0; // indicates the length of the most recent stable low pulse (in ms)
  unsigned long pulseHighTime      = 0; // indicates the length of the most recent stable high pulse (in ms)
  int           currentStableState = 0; // stores current stable pin state. Set in Step 3 when new stable pulse started
  int           lastCheckState     = 0; // most recent pin state, that was read. Set in Step1,2,3


#ifdef PULSE_STATISTIC

  void setStepOverdueMax(int pStep, long overdueTime) {
    if (StepOverdueMax[pStep] < overdueTime) {
      StepOverdueMax[pStep] = overdueTime;
    }
  }

  // debug/tuning variables for PULSE mode statistical logging
  unsigned int Step1counter    = 0; // counts how often step 1 was entered
  unsigned int Step2OKcounter  = 0; // counts how often step 2 detected the expected pin state (first verification)
  unsigned int Step2NOKcounter = 0; // counts how often step 2 detected the wrong pin state (first verification failed)
  unsigned int Step3OKcounter  = 0; // counts how often step 3 detected the expected pin state (2nd verification)
  unsigned int Step3NOKcounter = 0; // counts how often step 3 detected the wrong pin state (2nd verification failed)
  unsigned int Step3IGNcounter = 0; // counts how often step 3 detected the wrong pin state (2nd verification failed)
  unsigned int Step0ODcounter  = 0; // counts how often the debounce time timed out before step 0 was reached
  long         StepOverdueMax[P003_PSTEP_MAX + 1] = { 0 }; // longest recognised overdue time per step in ms
  byte         StatsLogLevel = PULSE_STATS_ADHOC_LOG_LEVEL; // log level for regular statistics logging

#endif
};

struct Internal_GPIO_pulseHelper {
  enum class GPIOtriggerMode {
    None        = 0,
    Change      = CHANGE,
    Rising      = RISING,
    Falling     = FALLING,
    PulseLow    = PULSE_LOW,
    PulseHigh   = PULSE_HIGH,
    PulseChange = PULSE_CHANGE,
  };

  static void addGPIOtriggerMode(const __FlashStringHelper *label,
                                 const __FlashStringHelper *id,
                                 GPIOtriggerMode            currentSelection);


  struct pulseCounterConfig {
    // Make sure the speed-optimized debounceTime_micros equals the 16 bit debounceTime
    void setDebounceTime(uint16_t debounceTime_u16) {
      debounceTime        = debounceTime_u16;
      debounceTime_micros = static_cast<uint64_t>(debounceTime_u16) * 1000L;
    }

    bool useEdgeMode() const {
      return (static_cast<int>(interruptPinMode) & PULSE_MODE_MASK) == 0;
    }

    uint64_t        debounceTime_micros = 0; // 64 bit version of debounceTime in micoseconds
    uint16_t        debounceTime        = 0;
    taskIndex_t     taskIndex           = INVALID_TASK_INDEX;
    byte            gpio                = -1;
    byte            pullupPinMode       = INPUT_PULLUP;
    GPIOtriggerMode interruptPinMode    = GPIOtriggerMode::Change;
  };


  Internal_GPIO_pulseHelper(pulseCounterConfig configuration);

  ~Internal_GPIO_pulseHelper();

  // Format GPIOtriggerMode to a flash string
  static const __FlashStringHelper* toString(GPIOtriggerMode mode);

  bool                              init();

  void                              getPulseCounters(unsigned long& pulseCounter,
                                                     unsigned long& pulseTotalCounter,
                                                     float        & pulseTime_msec);

  void setPulseCountTotal(unsigned long pulseTotalCounter);

  void setPulseCounter(unsigned long pulseCounter,
                       float         pulseTime_msec = 0.0f);

  void resetPulseCounter();

  // Process recorded pulse data on regular intervals.
  // Typically from PLUGIN_FIFTY_PER_SECOND or PLUGIN_TIMER_IN
  void doPulseStepProcessing(int pStep);

  pulseModeData_t pulseModeData;

private:

  uint64_t us_Since_triggerTimestamp() const;

  long     msec_Since_triggerTimestamp() const;

  /*********************************************************************************************\
  *  Processing for found stable pulse
  \*********************************************************************************************/
  void     processStablePulse(int      pinState,
                              uint64_t pulseChangeTime);


  volatile pulseCounterISRdata_t ISRdata;
  const pulseCounterConfig       config;

  static void ISR_pulseCheck(Internal_GPIO_pulseHelper *self);


#ifdef PULSE_STATISTIC

public:

  // adjust the statistical step counters relative to TotalCounter, in order to keep statistic correct
  void updateStatisticalCounters(int par1);

  void setStatsLogLevel(byte logLevel);

  /*********************************************************************************************\
  *  reset statistical error cunters and overview variables
  \*********************************************************************************************/
  void resetStatsErrorVars();

  /*********************************************************************************************\
  *  write statistic counters to logfile
  \*********************************************************************************************/
  void doStatisticLogging(byte logLevel);

  /*********************************************************************************************\
  *  write collected timing values to logfile
  \*********************************************************************************************/
  void doTimingLogging(byte logLevel);
  #endif // ifdef PULSE_STATISTIC
};


#endif // ifndef HELPERS__INTERNAL_GPIO_PULSEHELPER_H
