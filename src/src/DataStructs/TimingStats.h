#ifndef DATASTRUCTS_TIMINGSTATS_H
#define DATASTRUCTS_TIMINGSTATS_H

#include "ESPEasy_plugin_functions.h"
#include "../../ESPEasy_common.h"
#include "../../ESPEasy_fdwdecl.h"

#ifdef USES_TIMING_STATS

#include "../Helpers/ESPEasy_time_calc.h"

# include <Arduino.h>
# include <map>


/*********************************************************************************************\
* TimingStats
\*********************************************************************************************/


# define LOADFILE_STATS          0
# define SAVEFILE_STATS          1
# define LOOP_STATS              2
# define PLUGIN_CALL_50PS        3
# define PLUGIN_CALL_10PS        4
# define PLUGIN_CALL_10PSU       5
# define PLUGIN_CALL_1PS         6
# define SENSOR_SEND_TASK        7
# define CPLUGIN_CALL_10PS       8
# define CPLUGIN_CALL_50PS       9
# define SEND_DATA_STATS         10
# define COMPUTE_FORMULA_STATS   11
# define PROC_SYS_TIMER          12
# define SET_NEW_TIMER           13
# define TIME_DIFF_COMPUTE       14
# define MQTT_DELAY_QUEUE        15
# define C001_DELAY_QUEUE        16
# define C002_DELAY_QUEUE        17
# define C003_DELAY_QUEUE        18
# define C004_DELAY_QUEUE        19
# define C005_DELAY_QUEUE        20
# define C006_DELAY_QUEUE        21
# define C007_DELAY_QUEUE        22
# define C008_DELAY_QUEUE        23
# define C009_DELAY_QUEUE        24
# define C010_DELAY_QUEUE        25
# define C011_DELAY_QUEUE        26
# define C012_DELAY_QUEUE        27
# define C013_DELAY_QUEUE        28
# define C014_DELAY_QUEUE        29
# define C015_DELAY_QUEUE        30
# define C016_DELAY_QUEUE        31
# define C017_DELAY_QUEUE        32
# define C018_DELAY_QUEUE        33
# define C019_DELAY_QUEUE        34
# define C020_DELAY_QUEUE        35
# define TRY_CONNECT_HOST_TCP    36
# define TRY_CONNECT_HOST_UDP    37
# define HOST_BY_NAME_STATS      38
# define CONNECT_CLIENT_STATS    39
# define LOAD_CUSTOM_TASK_STATS  40
# define WIFI_ISCONNECTED_STATS  41
# define WIFI_NOTCONNECTED_STATS 42
# define LOAD_TASK_SETTINGS      43
# define TRY_OPEN_FILE           44
# define FS_GC_SUCCESS           45
# define FS_GC_FAIL              46
# define PARSE_SYSVAR            47
# define PARSE_SYSVAR_NOCHANGE   48
# define PARSE_TEMPLATE_PADDED   49
# define RULES_PROCESSING        50
# define GRAT_ARP_STATS          51
# define BACKGROUND_TASKS        52
# define HANDLE_SCHEDULER_IDLE   53
# define HANDLE_SCHEDULER_TASK   54
# define HANDLE_SERVING_WEBPAGE  55

class TimingStats {
public:

  TimingStats();

  void         add(unsigned long time);
  void         reset();
  bool         isEmpty() const;
  float        getAvg() const;
  unsigned int getMinMax(unsigned long& minVal,
                         unsigned long& maxVal) const;
  bool         thresholdExceeded(unsigned long threshold) const;

private:

  float _timeTotal;
  unsigned int _count;
  unsigned long _maxVal;
  unsigned long _minVal;
};


String getPluginFunctionName(int function);
bool   mustLogFunction(int function);
String getCPluginCFunctionName(CPlugin::Function function);
bool   mustLogCFunction(CPlugin::Function function);
String getMiscStatsName(int stat);


extern std::map<int, TimingStats> pluginStats;
extern std::map<int, TimingStats> controllerStats;
extern std::map<int, TimingStats> miscStats;
extern unsigned long timingstats_last_reset;

# define START_TIMER const unsigned statisticsTimerStart(micros());
# define STOP_TIMER_TASK(T, F) \
  if (mustLogFunction(F)) pluginStats[(T) * 256 + (F)].add(usecPassedSince(statisticsTimerStart));
# define STOP_TIMER_CONTROLLER(T, F) \
  if (mustLogCFunction(F)) controllerStats[(T) * 256 + static_cast<int>(F)].add(usecPassedSince(statisticsTimerStart));

// #define STOP_TIMER_LOADFILE miscStats[LOADFILE_STATS].add(usecPassedSince(statisticsTimerStart));
# define STOP_TIMER(L) miscStats[L].add(usecPassedSince(statisticsTimerStart));

#else // ifdef USES_TIMING_STATS

# define START_TIMER
# define STOP_TIMER_TASK(T, F) ;
# define STOP_TIMER_CONTROLLER(T, F) ;
# define STOP_TIMER(L) ;


// FIXME TD-er: This class is used as a parameter in functions defined in .ino files.
// The Arduino build process tries to forward declare all functions it can find, regardless of defines.
// Meaning we must make sure the forward declaration of the TimingStats class is made, since it is used as an argument in some function.
class TimingStats;

#endif // ifdef USES_TIMING_STATS

#endif // DATASTRUCTS_TIMINGSTATS_H
