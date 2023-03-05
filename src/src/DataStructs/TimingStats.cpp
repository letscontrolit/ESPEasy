#include "../DataStructs/TimingStats.h"

#if FEATURE_TIMING_STATS

# include "../DataTypes/ESPEasy_plugin_functions.h"
# include "../Globals/CPlugins.h"
# include "../Helpers/_CPlugin_Helper.h"
# include "../Helpers/StringConverter.h"

std::map<int, TimingStats> pluginStats;
std::map<int, TimingStats> controllerStats;
std::map<TimingStatsElements, TimingStats> miscStats;
unsigned long timingstats_last_reset(0);


TimingStats::TimingStats() : _timeTotal(0.0f), _count(0), _maxVal(0), _minVal(4294967295) {}

void TimingStats::add(int64_t time) {
  _timeTotal += static_cast<float>(time);
  ++_count;

  if (time > static_cast<int64_t>(_maxVal)) { _maxVal = time; }

  if (time < static_cast<int64_t>(_minVal)) { _minVal = time; }
}

void TimingStats::reset() {
  _timeTotal = 0.0f;
  _count     = 0;
  _maxVal    = 0;
  _minVal    = 4294967295;
}

bool TimingStats::isEmpty() const {
  return _count == 0;
}

float TimingStats::getAvg() const {
  if (_count == 0) { return 0.0f; }
  return _timeTotal / static_cast<float>(_count);
}

uint32_t TimingStats::getMinMax(uint64_t& minVal, uint64_t& maxVal) const {
  if (_count == 0) {
    minVal = 0;
    maxVal = 0;
    return 0;
  }
  minVal = _minVal;
  maxVal = _maxVal;
  return _count;
}

bool TimingStats::thresholdExceeded(const uint64_t& threshold) const {
  if (_count == 0) {
    return false;
  }
  return _maxVal > threshold;
}

/********************************************************************************************\
   Functions used for displaying timing stats
 \*********************************************************************************************/
const __FlashStringHelper* getPluginFunctionName(int function) {
  switch (function) {
    case PLUGIN_INIT_ALL:              return F("INIT_ALL");
    case PLUGIN_INIT:                  return F("INIT");
    case PLUGIN_READ:                  return F("READ");
    case PLUGIN_ONCE_A_SECOND:         return F("ONCE_A_SECOND");
    case PLUGIN_TEN_PER_SECOND:        return F("TEN_PER_SECOND");
    case PLUGIN_DEVICE_ADD:            return F("DEVICE_ADD");
    case PLUGIN_EVENTLIST_ADD:         return F("EVENTLIST_ADD");
    case PLUGIN_WEBFORM_SAVE:          return F("WEBFORM_SAVE");
    case PLUGIN_WEBFORM_LOAD:          return F("WEBFORM_LOAD");
    case PLUGIN_WEBFORM_SHOW_VALUES:   return F("WEBFORM_SHOW_VALUES");
    case PLUGIN_FORMAT_USERVAR:        return F("FORMAT_USERVAR");
    case PLUGIN_GET_DEVICENAME:        return F("GET_DEVICENAME");
    case PLUGIN_GET_DEVICEVALUENAMES:  return F("GET_DEVICEVALUENAMES");
    case PLUGIN_WRITE:                 return F("WRITE");
    case PLUGIN_EVENT_OUT:             return F("EVENT_OUT");
    case PLUGIN_WEBFORM_SHOW_CONFIG:   return F("WEBFORM_SHOW_CONFIG");
    case PLUGIN_SERIAL_IN:             return F("SERIAL_IN");
    case PLUGIN_UDP_IN:                return F("UDP_IN");
    case PLUGIN_CLOCK_IN:              return F("CLOCK_IN");
    case PLUGIN_TASKTIMER_IN:          return F("TASKTIMER_IN");
    case PLUGIN_FIFTY_PER_SECOND:      return F("FIFTY_PER_SECOND");
    case PLUGIN_SET_CONFIG:            return F("SET_CONFIG");
    case PLUGIN_GET_DEVICEGPIONAMES:   return F("GET_DEVICEGPIONAMES");
    case PLUGIN_EXIT:                  return F("EXIT");
    case PLUGIN_GET_CONFIG_VALUE:      return F("GET_CONFIG");
    case PLUGIN_UNCONDITIONAL_POLL:    return F("UNCONDITIONAL_POLL");
    case PLUGIN_REQUEST:               return F("REQUEST");
    case PLUGIN_PROCESS_CONTROLLER_DATA: return F("PROCESS_CONTROLLER_DATA");
    case PLUGIN_I2C_GET_ADDRESS:       return F("I2C_CHECK_DEVICE");
  }
  return F("Unknown");
}

bool mustLogFunction(int function) {
  if (!Settings.EnableTimingStats()) { return false; }

  switch (function) {
    case PLUGIN_INIT_ALL:              return false;
    case PLUGIN_INIT:                  return false;
    case PLUGIN_READ:                  return true;
    case PLUGIN_ONCE_A_SECOND:         return true;
    case PLUGIN_TEN_PER_SECOND:        return true;
    case PLUGIN_DEVICE_ADD:            return false;
    case PLUGIN_EVENTLIST_ADD:         return false;
    case PLUGIN_WEBFORM_SAVE:          return false;
    case PLUGIN_WEBFORM_LOAD:          return false;
    case PLUGIN_WEBFORM_SHOW_VALUES:   return false;
    case PLUGIN_FORMAT_USERVAR:        return false;
    case PLUGIN_GET_DEVICENAME:        return false;
    case PLUGIN_GET_DEVICEVALUENAMES:  return false;
    case PLUGIN_WRITE:                 return true;
    case PLUGIN_EVENT_OUT:             return true;
    case PLUGIN_WEBFORM_SHOW_CONFIG:   return false;
    case PLUGIN_SERIAL_IN:             return true;
    case PLUGIN_UDP_IN:                return true;
    case PLUGIN_CLOCK_IN:              return false;
    case PLUGIN_TASKTIMER_IN:          return true;
    case PLUGIN_FIFTY_PER_SECOND:      return true;
    case PLUGIN_SET_CONFIG:            return false;
    case PLUGIN_GET_DEVICEGPIONAMES:   return false;
    case PLUGIN_EXIT:                  return false;
    case PLUGIN_GET_CONFIG_VALUE:      return false;
    case PLUGIN_UNCONDITIONAL_POLL:    return false;
    case PLUGIN_REQUEST:               return true;
    case PLUGIN_I2C_GET_ADDRESS:       return true;
    case PLUGIN_PROCESS_CONTROLLER_DATA: return true;
  }
  return false;
}

const __FlashStringHelper* getCPluginCFunctionName(CPlugin::Function function) {
  switch (function) {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:              return F("CPLUGIN_PROTOCOL_ADD");
    case CPlugin::Function::CPLUGIN_PROTOCOL_TEMPLATE:         return F("CPLUGIN_PROTOCOL_TEMPLATE");
    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:             return F("CPLUGIN_PROTOCOL_SEND");
    case CPlugin::Function::CPLUGIN_PROTOCOL_RECV:             return F("CPLUGIN_PROTOCOL_RECV");
    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:            return F("CPLUGIN_GET_DEVICENAME");
    case CPlugin::Function::CPLUGIN_WEBFORM_SAVE:              return F("CPLUGIN_WEBFORM_SAVE");
    case CPlugin::Function::CPLUGIN_WEBFORM_LOAD:              return F("CPLUGIN_WEBFORM_LOAD");
    case CPlugin::Function::CPLUGIN_GET_PROTOCOL_DISPLAY_NAME: return F("CPLUGIN_GET_PROTOCOL_DISPLAY_NAME");
    case CPlugin::Function::CPLUGIN_TASK_CHANGE_NOTIFICATION:  return F("CPLUGIN_TASK_CHANGE_NOTIFICATION");
    case CPlugin::Function::CPLUGIN_INIT:                      return F("CPLUGIN_INIT");
    case CPlugin::Function::CPLUGIN_UDP_IN:                    return F("CPLUGIN_UDP_IN");
    case CPlugin::Function::CPLUGIN_FLUSH:                     return F("CPLUGIN_FLUSH");
    case CPlugin::Function::CPLUGIN_TEN_PER_SECOND:            return F("CPLUGIN_TEN_PER_SECOND");
    case CPlugin::Function::CPLUGIN_FIFTY_PER_SECOND:          return F("CPLUGIN_FIFTY_PER_SECOND");
    case CPlugin::Function::CPLUGIN_INIT_ALL:                  return F("CPLUGIN_INIT_ALL");
    case CPlugin::Function::CPLUGIN_EXIT:                      return F("CPLUGIN_EXIT");
    case CPlugin::Function::CPLUGIN_WRITE:                     return F("CPLUGIN_WRITE");

    case CPlugin::Function::CPLUGIN_GOT_CONNECTED:
    case CPlugin::Function::CPLUGIN_GOT_INVALID:
    case CPlugin::Function::CPLUGIN_INTERVAL:
    case CPlugin::Function::CPLUGIN_ACKNOWLEDGE:
    case CPlugin::Function::CPLUGIN_WEBFORM_SHOW_HOST_CONFIG:
      break;
  }
  return F("Unknown");
}

bool mustLogCFunction(CPlugin::Function function) {
  if (!Settings.EnableTimingStats()) { return false; }

  switch (function) {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:              return false;
    case CPlugin::Function::CPLUGIN_PROTOCOL_TEMPLATE:         return false;
    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:             return true;
    case CPlugin::Function::CPLUGIN_PROTOCOL_RECV:             return true;
    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:            return false;
    case CPlugin::Function::CPLUGIN_WEBFORM_SAVE:              return false;
    case CPlugin::Function::CPLUGIN_WEBFORM_LOAD:              return false;
    case CPlugin::Function::CPLUGIN_GET_PROTOCOL_DISPLAY_NAME: return false;
    case CPlugin::Function::CPLUGIN_TASK_CHANGE_NOTIFICATION:  return false;
    case CPlugin::Function::CPLUGIN_INIT:                      return false;
    case CPlugin::Function::CPLUGIN_UDP_IN:                    return true;
    case CPlugin::Function::CPLUGIN_FLUSH:                     return false;
    case CPlugin::Function::CPLUGIN_TEN_PER_SECOND:            return true;
    case CPlugin::Function::CPLUGIN_FIFTY_PER_SECOND:          return true;
    case CPlugin::Function::CPLUGIN_INIT_ALL:                  return false;
    case CPlugin::Function::CPLUGIN_EXIT:                      return false;
    case CPlugin::Function::CPLUGIN_WRITE:                     return true;

    case CPlugin::Function::CPLUGIN_GOT_CONNECTED:
    case CPlugin::Function::CPLUGIN_GOT_INVALID:
    case CPlugin::Function::CPLUGIN_INTERVAL:
    case CPlugin::Function::CPLUGIN_ACKNOWLEDGE:
    case CPlugin::Function::CPLUGIN_WEBFORM_SHOW_HOST_CONFIG:
      break;
  }
  return false;
}

// Return flash string type to reduce bin size
const __FlashStringHelper* getMiscStatsName_F(TimingStatsElements stat) {
  switch (stat) {
    case TimingStatsElements::LOADFILE_STATS:             return F("Load File");
    case TimingStatsElements::SAVEFILE_STATS:             return F("Save File");
    case TimingStatsElements::LOOP_STATS:                 return F("Loop");
    case TimingStatsElements::PLUGIN_CALL_50PS:           return F("Plugin call 50 p/s");
    case TimingStatsElements::PLUGIN_CALL_10PS:           return F("Plugin call 10 p/s");
    case TimingStatsElements::PLUGIN_CALL_10PSU:          return F("Plugin call 10 p/s U");
    case TimingStatsElements::PLUGIN_CALL_1PS:            return F("Plugin call  1 p/s");
    case TimingStatsElements::CPLUGIN_CALL_50PS:          return F("CPlugin call 50 p/s");
    case TimingStatsElements::CPLUGIN_CALL_10PS:          return F("CPlugin call 10 p/s");
    case TimingStatsElements::SENSOR_SEND_TASK:           return F("SensorSendTask()");
    case TimingStatsElements::SEND_DATA_STATS:            return F("sendData()");
    case TimingStatsElements::COMPUTE_FORMULA_STATS:      return F("Compute formula");
    case TimingStatsElements::COMPUTE_STATS:              return F("Compute()");
    case TimingStatsElements::PLUGIN_CALL_DEVICETIMER_IN: return F("PLUGIN_DEVICETIMER_IN");
    case TimingStatsElements::SET_NEW_TIMER:              return F("setNewTimerAt()");
    case TimingStatsElements::MQTT_DELAY_QUEUE:           return F("Delay queue MQTT");
    case TimingStatsElements::TRY_CONNECT_HOST_TCP:       return F("try_connect_host() (TCP)");
    case TimingStatsElements::TRY_CONNECT_HOST_UDP:       return F("try_connect_host() (UDP)");
    case TimingStatsElements::HOST_BY_NAME_STATS:         return F("hostByName()");
    case TimingStatsElements::CONNECT_CLIENT_STATS:       return F("connectClient()");
    case TimingStatsElements::LOAD_CUSTOM_TASK_STATS:     return F("LoadCustomTaskSettings()");
    case TimingStatsElements::WIFI_ISCONNECTED_STATS:     return F("WiFi.isConnected()");
    case TimingStatsElements::WIFI_NOTCONNECTED_STATS:    return F("WiFi.isConnected() (fail)");
    case TimingStatsElements::LOAD_TASK_SETTINGS:         return F("LoadTaskSettings()");
    case TimingStatsElements::SAVE_TASK_SETTINGS:         return F("SaveTaskSettings()");
    case TimingStatsElements::LOAD_CONTROLLER_SETTINGS:   return F("LoadControllerSettings()");
    #ifdef ESP32
    case TimingStatsElements::LOAD_CONTROLLER_SETTINGS_C: return F("LoadControllerSettings() (cached)");
    #endif
    case TimingStatsElements::SAVE_CONTROLLER_SETTINGS:   return F("SaveControllerSettings()");
    case TimingStatsElements::TRY_OPEN_FILE:              return F("TryOpenFile()");
    case TimingStatsElements::FS_GC_SUCCESS:              return F("ESPEASY_FS GC success");
    case TimingStatsElements::FS_GC_FAIL:                 return F("ESPEASY_FS GC fail");
    case TimingStatsElements::RULES_PROCESSING:           return F("rulesProcessing()");
    case TimingStatsElements::RULES_PARSE_LINE:           return F("parseCompleteNonCommentLine()");
    case TimingStatsElements::RULES_PROCESS_MATCHED:      return F("processMatchedRule()");
    case TimingStatsElements::RULES_MATCH:                return F("rulesMatch()");
    case TimingStatsElements::GRAT_ARP_STATS:             return F("sendGratuitousARP()");
    case TimingStatsElements::SAVE_TO_RTC:                return F("saveToRTC()");
    case TimingStatsElements::BACKGROUND_TASKS:           return F("backgroundtasks()");
    case TimingStatsElements::PROCESS_SYSTEM_EVENT_QUEUE: return F("process_system_event_queue()");
    case TimingStatsElements::HANDLE_SCHEDULER_IDLE:      return F("handle_schedule() idle");
    case TimingStatsElements::HANDLE_SCHEDULER_TASK:      return F("handle_schedule() task");
    case TimingStatsElements::PARSE_TEMPLATE_PADDED:      return F("parseTemplate_padded()");
    case TimingStatsElements::PARSE_SYSVAR:               return F("parseSystemVariables()");
    case TimingStatsElements::PARSE_SYSVAR_NOCHANGE:      return F("parseSystemVariables() No change");
    case TimingStatsElements::HANDLE_SERVING_WEBPAGE:     return F("handle webpage");
    case TimingStatsElements::HANDLE_SERVING_WEBPAGE_JSON: return F("handle webpage JSON");
    case TimingStatsElements::WIFI_SCAN_ASYNC:            return F("WiFi Scan Async");
    case TimingStatsElements::WIFI_SCAN_SYNC:             return F("WiFi Scan Sync (blocking)");
    case TimingStatsElements::NTP_SUCCESS:                return F("NTP Success");
    case TimingStatsElements::NTP_FAIL:                   return F("NTP Fail");
    case TimingStatsElements::SYSTIME_UPDATED:            return F("Systime Set");
    case TimingStatsElements::C018_AIR_TIME:              return F("C018 LoRa TTN - Air Time");
#ifdef LIMIT_BUILD_SIZE
    default: break;
#else
    // Include all elements of the enum, to allow the compiler to check if we missed some
    case TimingStatsElements::C001_DELAY_QUEUE:
    case TimingStatsElements::C002_DELAY_QUEUE:
    case TimingStatsElements::C003_DELAY_QUEUE:
    case TimingStatsElements::C004_DELAY_QUEUE:
    case TimingStatsElements::C005_DELAY_QUEUE:
    case TimingStatsElements::C006_DELAY_QUEUE:
    case TimingStatsElements::C007_DELAY_QUEUE:
    case TimingStatsElements::C008_DELAY_QUEUE:
    case TimingStatsElements::C009_DELAY_QUEUE:
    case TimingStatsElements::C010_DELAY_QUEUE:
    case TimingStatsElements::C011_DELAY_QUEUE:
    case TimingStatsElements::C012_DELAY_QUEUE:
    case TimingStatsElements::C013_DELAY_QUEUE:
    case TimingStatsElements::C014_DELAY_QUEUE:
    case TimingStatsElements::C015_DELAY_QUEUE:
    case TimingStatsElements::C016_DELAY_QUEUE:
    case TimingStatsElements::C017_DELAY_QUEUE:
    case TimingStatsElements::C018_DELAY_QUEUE:
    case TimingStatsElements::C019_DELAY_QUEUE:
    case TimingStatsElements::C020_DELAY_QUEUE:
    case TimingStatsElements::C021_DELAY_QUEUE:
    case TimingStatsElements::C022_DELAY_QUEUE:
    case TimingStatsElements::C023_DELAY_QUEUE:
    case TimingStatsElements::C024_DELAY_QUEUE:
    case TimingStatsElements::C025_DELAY_QUEUE:
      break;

#endif
  }
  return F("Unknown");
}

String getMiscStatsName(TimingStatsElements stat) {
  if ((stat >= TimingStatsElements::C001_DELAY_QUEUE) && 
      (stat <= TimingStatsElements::C025_DELAY_QUEUE)) {
    return concat(
      F("Delay queue "),
      get_formatted_Controller_number(static_cast<cpluginID_t>(static_cast<int>(stat) - static_cast<int>(TimingStatsElements::C001_DELAY_QUEUE) + 1)));
  }
  return getMiscStatsName_F(static_cast<TimingStatsElements>(stat));
}

void stopTimerTask(deviceIndex_t T, int F, uint64_t statisticsTimerStart)
{
  if (mustLogFunction(F)) { pluginStats[static_cast<int>(T) * 256 + (F)].add(usecPassedSince(statisticsTimerStart)); }
}

void stopTimerController(protocolIndex_t T, CPlugin::Function F, uint64_t statisticsTimerStart)
{
  if (mustLogCFunction(F)) { controllerStats[static_cast<int>(T) * 256 + static_cast<int>(F)].add(usecPassedSince(statisticsTimerStart)); }
}

void stopTimer(TimingStatsElements L, uint64_t statisticsTimerStart)
{
  if (Settings.EnableTimingStats()) { miscStats[L].add(usecPassedSince(statisticsTimerStart)); }
}

void addMiscTimerStat(TimingStatsElements L, int64_t T)
{
  if (Settings.EnableTimingStats()) { miscStats[L].add(T); }
}

#endif // if FEATURE_TIMING_STATS
