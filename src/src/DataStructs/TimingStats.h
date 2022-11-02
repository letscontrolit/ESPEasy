#ifndef DATASTRUCTS_TIMINGSTATS_H
#define DATASTRUCTS_TIMINGSTATS_H

#include "../../ESPEasy_common.h"

#if FEATURE_TIMING_STATS

# include "../DataTypes/ESPEasy_plugin_functions.h"
# include "../Globals/Settings.h"
# include "../Helpers/ESPEasy_time_calc.h"

# include <Arduino.h>
# include <map>


/*********************************************************************************************\
* TimingStats
\*********************************************************************************************/


# define LOADFILE_STATS                 0
# define SAVEFILE_STATS                 1
# define LOOP_STATS                     2
# define PLUGIN_CALL_50PS               3
# define PLUGIN_CALL_10PS               4
# define PLUGIN_CALL_10PSU              5
# define PLUGIN_CALL_1PS                6
# define SENSOR_SEND_TASK               7
# define CPLUGIN_CALL_10PS              8
# define CPLUGIN_CALL_50PS              9
# define SEND_DATA_STATS                10
# define COMPUTE_FORMULA_STATS          11
# define PLUGIN_CALL_DEVICETIMER_IN     12
# define SET_NEW_TIMER                  13
# define TIME_DIFF_COMPUTE              14
# define MQTT_DELAY_QUEUE               15
# define C001_DELAY_QUEUE               16
# define C002_DELAY_QUEUE               17
# define C003_DELAY_QUEUE               18
# define C004_DELAY_QUEUE               19
# define C005_DELAY_QUEUE               20
# define C006_DELAY_QUEUE               21
# define C007_DELAY_QUEUE               22
# define C008_DELAY_QUEUE               23
# define C009_DELAY_QUEUE               24
# define C010_DELAY_QUEUE               25
# define C011_DELAY_QUEUE               26
# define C012_DELAY_QUEUE               27
# define C013_DELAY_QUEUE               28
# define C014_DELAY_QUEUE               29
# define C015_DELAY_QUEUE               30
# define C016_DELAY_QUEUE               31
# define C017_DELAY_QUEUE               32
# define C018_DELAY_QUEUE               33
# define C019_DELAY_QUEUE               34
# define C020_DELAY_QUEUE               35
# define C021_DELAY_QUEUE               36
# define C022_DELAY_QUEUE               37
# define C023_DELAY_QUEUE               38
# define C024_DELAY_QUEUE               39
# define C025_DELAY_QUEUE               40
# define C018_AIR_TIME                  41
# define TRY_CONNECT_HOST_TCP           42
# define TRY_CONNECT_HOST_UDP           43
# define HOST_BY_NAME_STATS             44
# define CONNECT_CLIENT_STATS           45
# define LOAD_CUSTOM_TASK_STATS         46
# define WIFI_ISCONNECTED_STATS         47
# define WIFI_NOTCONNECTED_STATS        48
# define LOAD_TASK_SETTINGS             49
# define TRY_OPEN_FILE                  50
# define FS_GC_SUCCESS                  51
# define FS_GC_FAIL                     52
# define PARSE_SYSVAR                   53
# define PARSE_SYSVAR_NOCHANGE          54
# define PARSE_TEMPLATE_PADDED          55
# define RULES_PROCESSING               56
# define RULES_PARSE_LINE               57
# define RULES_PROCESS_MATCHED          58
# define RULES_MATCH                    59
# define GRAT_ARP_STATS                 60
# define SAVE_TO_RTC                    61
# define BACKGROUND_TASKS               62
# define PROCESS_SYSTEM_EVENT_QUEUE     63
# define HANDLE_SCHEDULER_IDLE          64
# define HANDLE_SCHEDULER_TASK          65
# define HANDLE_SERVING_WEBPAGE         66
#ifdef USES_ESPEASY_NOW
# define HANDLE_ESPEASY_NOW_LOOP        67
# define EXPIRED_ESPEASY_NOW_LOOP       68
# define INVALID_ESPEASY_NOW_LOOP       69
# define RECEIVE_ESPEASY_NOW_LOOP       70
# define ESPEASY_NOW_SEND_MSG_SUC       71
# define ESPEASY_NOW_SEND_MSG_FAIL      72
# define ESPEASY_NOW_SEND_PCKT          73
# define ESPEASY_NOW_DEDUP_LOOP         74
#endif

# define WIFI_SCAN_ASYNC                75
# define WIFI_SCAN_SYNC                 76
# define NTP_SUCCESS                    77
# define NTP_FAIL                       78
# define SYSTIME_UPDATED                79


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
String                     getMiscStatsName(int stat);

void                       stopTimerTask(int      T,
                                         int      F,
                                         uint64_t statisticsTimerStart);
void                       stopTimerController(int               T,
                                               CPlugin::Function F,
                                               uint64_t          statisticsTimerStart);
void                       stopTimer(int      L,
                                     uint64_t statisticsTimerStart);
void                       addMiscTimerStat(int     L,
                                            int64_t T);

extern std::map<int, TimingStats> pluginStats;
extern std::map<int, TimingStats> controllerStats;
extern std::map<int, TimingStats> miscStats;
extern unsigned long timingstats_last_reset;

# define START_TIMER const uint64_t statisticsTimerStart(getMicros64());
# define STOP_TIMER_TASK(T, F) stopTimerTask(T, F, statisticsTimerStart);
# define STOP_TIMER_CONTROLLER(T, F) stopTimerController(T, F, statisticsTimerStart);

// #define STOP_TIMER_LOADFILE miscStats[LOADFILE_STATS].add(usecPassedSince(statisticsTimerStart));
# define STOP_TIMER(L) stopTimer(L, statisticsTimerStart);

// Add a timer statistic value in usec.
# define ADD_TIMER_STAT(L, T) addMiscTimerStat(L, T);

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
