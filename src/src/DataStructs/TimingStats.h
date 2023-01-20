#ifndef DATASTRUCTS_TIMINGSTATS_H
#define DATASTRUCTS_TIMINGSTATS_H

#include "../../ESPEasy_common.h"

#if FEATURE_TIMING_STATS

# include "../DataTypes/DeviceIndex.h"
# include "../DataTypes/ESPEasy_plugin_functions.h"
# include "../DataTypes/ProtocolIndex.h"
# include "../Globals/Settings.h"
# include "../Helpers/ESPEasy_time_calc.h"

# include <Arduino.h>
# include <map>
#endif // if FEATURE_TIMING_STATS


/*********************************************************************************************\
* TimingStats
\*********************************************************************************************/

// These TimingStatsElements must not be excluded when FEATURE_TIMING_STATS is not defined.
// The Cxxx_DELAY_QUEUE defines are used in the macros to process the controller queues.
enum class TimingStatsElements {

  // Controller queue
  MQTT_DELAY_QUEUE,
  C001_DELAY_QUEUE,
  C002_DELAY_QUEUE,
  C003_DELAY_QUEUE,
  C004_DELAY_QUEUE,
  C005_DELAY_QUEUE,
  C006_DELAY_QUEUE,
  C007_DELAY_QUEUE,
  C008_DELAY_QUEUE,
  C009_DELAY_QUEUE,
  C010_DELAY_QUEUE,
  C011_DELAY_QUEUE,
  C012_DELAY_QUEUE,
  C013_DELAY_QUEUE,
  C014_DELAY_QUEUE,
  C015_DELAY_QUEUE,
  C016_DELAY_QUEUE,
  C017_DELAY_QUEUE,
  C018_DELAY_QUEUE,
  C018_AIR_TIME,
  C019_DELAY_QUEUE,
  C020_DELAY_QUEUE,
  C021_DELAY_QUEUE,
  C022_DELAY_QUEUE,
  C023_DELAY_QUEUE,
  C024_DELAY_QUEUE,
  C025_DELAY_QUEUE,

  
  // Related to Task runs & sending data + rules
  PLUGIN_CALL_50PS,
  PLUGIN_CALL_10PS,
  PLUGIN_CALL_10PSU,
  PLUGIN_CALL_1PS,
  CPLUGIN_CALL_10PS,
  CPLUGIN_CALL_50PS,
  SENSOR_SEND_TASK,
  SEND_DATA_STATS,
  COMPUTE_FORMULA_STATS,
  COMPUTE_STATS,
  PARSE_SYSVAR,
  PARSE_SYSVAR_NOCHANGE,
  PARSE_TEMPLATE_PADDED,
  PROCESS_SYSTEM_EVENT_QUEUE,
  RULES_MATCH,
  RULES_PROCESSING,
  RULES_PROCESS_MATCHED,
  RULES_PARSE_LINE,
  
  // Related to file access
  LOADFILE_STATS,
  LOAD_TASK_SETTINGS,
  LOAD_CUSTOM_TASK_STATS,
  LOAD_CONTROLLER_SETTINGS,
  #ifdef ESP32
  LOAD_CONTROLLER_SETTINGS_C,
  #endif
  SAVEFILE_STATS,
  SAVE_TASK_SETTINGS,
  SAVE_CONTROLLER_SETTINGS,
  TRY_OPEN_FILE,
  FS_GC_SUCCESS,
  FS_GC_FAIL,

  // Scheduler related
  SAVE_TO_RTC,
  PLUGIN_CALL_DEVICETIMER_IN,
  SET_NEW_TIMER,
  HANDLE_SCHEDULER_TASK,
  HANDLE_SCHEDULER_IDLE,
  BACKGROUND_TASKS,

  // Web serving
  HANDLE_SERVING_WEBPAGE,
  HANDLE_SERVING_WEBPAGE_JSON,

  // Network related
  TRY_CONNECT_HOST_TCP,
  TRY_CONNECT_HOST_UDP,
  HOST_BY_NAME_STATS,
  GRAT_ARP_STATS,
  WIFI_ISCONNECTED_STATS,
  WIFI_NOTCONNECTED_STATS,
  CONNECT_CLIENT_STATS,
  WIFI_SCAN_ASYNC,
  WIFI_SCAN_SYNC,

  // Time sync (also network related)
  NTP_SUCCESS,
  NTP_FAIL,
  SYSTIME_UPDATED,

  // Close to the lifetime stats shown on the timing stats page
  LOOP_STATS
};

#if FEATURE_TIMING_STATS

class TimingStats {
public:

  TimingStats();

  void     add(int64_t time);
  void     reset();
  bool     isEmpty() const;
  float    getAvg() const;
  uint32_t getMinMax(uint64_t& minVal,
                     uint64_t& maxVal) const;
  bool     thresholdExceeded(const uint64_t& threshold) const;

private:

  float _timeTotal;
  uint32_t _count;
  uint64_t _maxVal;
  uint64_t _minVal;
};


const __FlashStringHelper* getPluginFunctionName(int function);
bool                       mustLogFunction(int function);
const __FlashStringHelper* getCPluginCFunctionName(CPlugin::Function function);
bool                       mustLogCFunction(CPlugin::Function function);
String                     getMiscStatsName(TimingStatsElements stat);

void                       stopTimerTask(deviceIndex_t T,
                                         int           F,
                                         uint64_t      statisticsTimerStart);
void                       stopTimerController(protocolIndex_t   T,
                                               CPlugin::Function F,
                                               uint64_t          statisticsTimerStart);
void                       stopTimer(TimingStatsElements L,
                                     uint64_t            statisticsTimerStart);
void                       addMiscTimerStat(TimingStatsElements L,
                                            int64_t             T);

extern std::map<int, TimingStats> pluginStats;
extern std::map<int, TimingStats> controllerStats;
extern std::map<TimingStatsElements, TimingStats> miscStats;
extern unsigned long timingstats_last_reset;

# define START_TIMER const uint64_t statisticsTimerStart(getMicros64());
# define STOP_TIMER_TASK(T, F) stopTimerTask(T, F, statisticsTimerStart);
# define STOP_TIMER_CONTROLLER(T, F) stopTimerController(T, F, statisticsTimerStart);

// #define STOP_TIMER_LOADFILE miscStats[LOADFILE_STATS].add(usecPassedSince(statisticsTimerStart));
# define STOP_TIMER(L) stopTimer(TimingStatsElements::L, statisticsTimerStart);
# define STOP_TIMER_VAR(L) stopTimer(L, statisticsTimerStart);

// Add a timer statistic value in usec.
# define ADD_TIMER_STAT(L, T) addMiscTimerStat(TimingStatsElements::L, T);

#else // if FEATURE_TIMING_STATS

# define START_TIMER ;
# define STOP_TIMER_TASK(T, F) ;
# define STOP_TIMER_CONTROLLER(T, F) ;
# define STOP_TIMER(L) ;
# define ADD_TIMER_STAT(L, T) ;


// FIXME TD-er: This class is used as a parameter in functions defined in .ino files.
// The Arduino build process tries to forward declare all functions it can find, regardless of defines.
// Meaning we must make sure the forward declaration of the TimingStats class is made, since it is used as an argument in some function.
class TimingStats;

#endif // if FEATURE_TIMING_STATS

#endif // DATASTRUCTS_TIMINGSTATS_H
