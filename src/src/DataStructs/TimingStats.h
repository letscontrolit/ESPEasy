#ifndef DATASTRUCTS_TIMINGSTATS_H
#define DATASTRUCTS_TIMINGSTATS_H

#include <Arduino.h>
#include <map>


/*********************************************************************************************\
* TimingStats
\*********************************************************************************************/


#define LOADFILE_STATS          0
#define SAVEFILE_STATS          1
#define LOOP_STATS              2
#define PLUGIN_CALL_50PS        3
#define PLUGIN_CALL_10PS        4
#define PLUGIN_CALL_10PSU       5
#define PLUGIN_CALL_1PS         6
#define SENSOR_SEND_TASK        7
#define SEND_DATA_STATS         8
#define COMPUTE_FORMULA_STATS   9
#define PROC_SYS_TIMER          10
#define SET_NEW_TIMER           11
#define TIME_DIFF_COMPUTE       12
#define MQTT_DELAY_QUEUE        13
#define C001_DELAY_QUEUE        14
#define C002_DELAY_QUEUE        15
#define C003_DELAY_QUEUE        16
#define C004_DELAY_QUEUE        17
#define C005_DELAY_QUEUE        18
#define C006_DELAY_QUEUE        19
#define C007_DELAY_QUEUE        20
#define C008_DELAY_QUEUE        21
#define C009_DELAY_QUEUE        22
#define C010_DELAY_QUEUE        23
#define C011_DELAY_QUEUE        24
#define C012_DELAY_QUEUE        25
#define C013_DELAY_QUEUE        26
#define C014_DELAY_QUEUE        27
#define C015_DELAY_QUEUE        28
#define C016_DELAY_QUEUE        29
#define C017_DELAY_QUEUE        30
#define C018_DELAY_QUEUE        31
#define C019_DELAY_QUEUE        32
#define C020_DELAY_QUEUE        33
#define TRY_CONNECT_HOST_TCP    34
#define TRY_CONNECT_HOST_UDP    35
#define HOST_BY_NAME_STATS      36
#define CONNECT_CLIENT_STATS    37
#define LOAD_CUSTOM_TASK_STATS  38
#define WIFI_ISCONNECTED_STATS  39
#define WIFI_NOTCONNECTED_STATS 40
#define LOAD_TASK_SETTINGS      41
#define TRY_OPEN_FILE           42
#define SPIFFS_GC_SUCCESS       43
#define SPIFFS_GC_FAIL          44
#define PARSE_SYSVAR            45
#define PARSE_SYSVAR_NOCHANGE   46
#define PARSE_TEMPLATE          47
#define RULES_PROCESSING        48
#define GRAT_ARP_STATS          49
#define BACKGROUND_TASKS        50
#define HANDLE_SCHEDULER_IDLE   51
#define HANDLE_SCHEDULER_TASK   52
#define HANDLE_SERVING_WEBPAGE  53

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
String getCPluginCFunctionName(int function);
bool   mustLogCFunction(int function);
String getMiscStatsName(int stat);


extern std::map<int, TimingStats> pluginStats;
extern std::map<int, TimingStats> controllerStats;
extern std::map<int, TimingStats> miscStats;
extern unsigned long timingstats_last_reset;

#define START_TIMER const unsigned statisticsTimerStart(micros());
#define STOP_TIMER_TASK(T, F) \
  if (mustLogFunction(F)) pluginStats[T * 256 + F].add(usecPassedSince(statisticsTimerStart));
#define STOP_TIMER_CONTROLLER(T, F) \
  if (mustLogCFunction(F)) controllerStats[T * 256 + F].add(usecPassedSince(statisticsTimerStart));

// #define STOP_TIMER_LOADFILE miscStats[LOADFILE_STATS].add(usecPassedSince(statisticsTimerStart));
#define STOP_TIMER(L) miscStats[L].add(usecPassedSince(statisticsTimerStart));

#endif // DATASTRUCTS_TIMINGSTATS_H
