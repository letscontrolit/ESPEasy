#include "_Plugin_Helper.h"

#include "ESPEasy_common.h"

#include "src/CustomBuild/ESPEasyLimits.h"
#include "src/DataStructs/PluginTaskData_base.h"
#include "src/DataStructs/SettingsStruct.h"
#include "src/DataStructs/TimingStats.h"
#include "src/Globals/Cache.h"
#include "src/Globals/Settings.h"
#include "src/Helpers/Misc.h"
#include "src/Helpers/StringParser.h"
#include "src/Helpers/TaskValuesWriterHelper.h"



PluginTaskData_base *Plugin_task_data[TASKS_MAX] = {};

#if DEBUG_PCONFIG_RANGE_CHECK

# if DEBUG_PCONFIG_RANGE_CHECK > 2
#  define DEBUG_PCONFIG_RANGE_CHECK_args   max_n, event, n, linenr, filename
# elif DEBUG_PCONFIG_RANGE_CHECK > 1
#  define DEBUG_PCONFIG_RANGE_CHECK_args   max_n, event, n, linenr
# else
#  define DEBUG_PCONFIG_RANGE_CHECK_args   max_n, event, n
# endif

bool PCONFIGxxx_outOfBounds(
  const __FlashStringHelper   *prefix,
  const uint8_t                max_n,
  DEBUG_PCONFIG_RANGE_CHECK_args_decl)
{
  if (validTaskIndex(event->TaskIndex) && (n < max_n)) { return false; }

  if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
    const auto pluginID = getPluginID_from_TaskIndex(event->TaskIndex);
# if DEBUG_PCONFIG_RANGE_CHECK > 1
    addLog(LOG_LEVEL_ERROR, strformat(
             F("%s(%u) out of range (range: 0..%u) for taskIndex %u (%s: %s) (%s:%u)"),
             FsP(prefix),
             n,
             max_n - 1,
             event->TaskIndex + 1,
             pluginID.toDisplayString().c_str(),
             getPluginNameFromPluginID(pluginID).c_str(),
#  if DEBUG_PCONFIG_RANGE_CHECK > 2
             FsP(filename),
#  else
             FsP(F("line")),
#  endif
             linenr));
# else 
    addLog(LOG_LEVEL_ERROR, strformat(
             F("%s(%u) out of range (range: 0..%u) for taskIndex %u (%s: %s)"),
             FsP(prefix),
             n,
             max_n - 1,
             event->TaskIndex + 1,
             pluginID.toDisplayString().c_str(),
             getPluginNameFromPluginID(pluginID).c_str()));
# endif 
  }
  return true;
}

int16_t& do_PCONFIG(DEBUG_PCONFIG_RANGE_CHECK_args_decl)
{
  constexpr uint8_t max_n = NR_ELEMENTS(Settings.TaskDevicePluginConfig[0]);

  if (!PCONFIGxxx_outOfBounds(F("PCONFIG"), DEBUG_PCONFIG_RANGE_CHECK_args)) {
    return Settings.TaskDevicePluginConfig[event->TaskIndex][n];
  }
  static int16_t invalid{};
  invalid = 0;
  return invalid;
}

float& do_PCONFIG_FLOAT(DEBUG_PCONFIG_RANGE_CHECK_args_decl)
{
  constexpr uint8_t max_n = NR_ELEMENTS(Settings.TaskDevicePluginConfigFloat[0]);

  if (!PCONFIGxxx_outOfBounds(F("PCONFIG_FLOAT"), DEBUG_PCONFIG_RANGE_CHECK_args)) {
    return Settings.TaskDevicePluginConfigFloat[event->TaskIndex][n];
  }

  static float invalid{};
  invalid = 0;
  return invalid;
}

int32_t& do_PCONFIG_LONG(DEBUG_PCONFIG_RANGE_CHECK_args_decl)
{
  constexpr uint8_t max_n = NR_ELEMENTS(Settings.TaskDevicePluginConfigLong[0]);

  if (!PCONFIGxxx_outOfBounds(F("PCONFIG_LONG"), DEBUG_PCONFIG_RANGE_CHECK_args)) {
    return Settings.TaskDevicePluginConfigLong[event->TaskIndex][n];
  }
  static int32_t invalid{};
  invalid = 0;
  return invalid;
}

uint32_t& do_PCONFIG_ULONG(DEBUG_PCONFIG_RANGE_CHECK_args_decl)
{
  constexpr uint8_t max_n = NR_ELEMENTS(Settings.TaskDevicePluginConfigULong[0]);

  if (!PCONFIGxxx_outOfBounds(F("PCONFIG_ULONG"), DEBUG_PCONFIG_RANGE_CHECK_args)) {
    return Settings.TaskDevicePluginConfigULong[event->TaskIndex][n];
  }
  static uint32_t invalid{};
  invalid = 0;
  return invalid;
}

int8_t& do_PIN(DEBUG_PCONFIG_RANGE_CHECK_args_decl)
{
  constexpr uint8_t max_n = 3;

  if (!PCONFIGxxx_outOfBounds(F("PIN"), DEBUG_PCONFIG_RANGE_CHECK_args)) {
    // N.B. order of array indices taskIndex_t and n differs from the other PCONFIGxxx
    return Settings.TaskDevicePin[n][event->TaskIndex];
  }
  static int8_t invalid{};
  invalid = -1;
  return invalid;
}

#endif // if DEBUG_PCONFIG_RANGE_CHECK

String PCONFIG_LABEL(int n) {
  if (n < PLUGIN_CONFIGVAR_MAX) {
    return concat(F("pconf_"), n);
  }
  return F("error");
}

void resetPluginTaskData() {
  for (taskIndex_t i = 0; i < TASKS_MAX; ++i) {
    Plugin_task_data[i] = nullptr;
  }
}

void clearPluginTaskData(taskIndex_t taskIndex) {
  if (validTaskIndex(taskIndex)) {
    if (Plugin_task_data[taskIndex] != nullptr) {
      delete Plugin_task_data[taskIndex];
      Plugin_task_data[taskIndex] = nullptr;
    }
  }
}

bool initPluginTaskData(taskIndex_t taskIndex, PluginTaskData_base *data) {
  if (!validTaskIndex(taskIndex)) {
    if (data != nullptr) {
      delete data;
    }
    return false;
  }

  // 2nd heap may have been active to allocate the PluginTaskData, but here we need to keep the default heap active
#ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
#endif // ifdef USE_SECOND_HEAP


  clearPluginTaskData(taskIndex);

  if (data != nullptr) {
    if (Settings.TaskDeviceEnabled[taskIndex]) {
      Plugin_task_data[taskIndex]                     = data;
      Plugin_task_data[taskIndex]->_taskdata_pluginID = Settings.getPluginID_for_task(taskIndex);

#if FEATURE_PLUGIN_STATS
      const uint8_t valueCount = getValueCountForTask(taskIndex);

      for (size_t i = 0; i < valueCount; ++i) {
        if (Cache.enabledPluginStats(taskIndex, i)) {
          Plugin_task_data[taskIndex]->initPluginStats(i);
        }
      }
#endif // if FEATURE_PLUGIN_STATS
#if FEATURE_PLUGIN_FILTER

      // TODO TD-er: Implement init

#endif // if FEATURE_PLUGIN_FILTER

    } else {
      delete data;
    }
  }
  return getPluginTaskData(taskIndex) != nullptr;
}

PluginTaskData_base* getPluginTaskData(taskIndex_t taskIndex) {
  if (pluginTaskData_initialized(taskIndex)) {

    if (!Plugin_task_data[taskIndex]->baseClassOnly()) {
      return Plugin_task_data[taskIndex];
    }
  }
  return nullptr;
}

PluginTaskData_base* getPluginTaskDataBaseClassOnly(taskIndex_t taskIndex) {
  if (pluginTaskData_initialized(taskIndex)) {
    return Plugin_task_data[taskIndex];
  }
  return nullptr;
}

bool pluginTaskData_initialized(taskIndex_t taskIndex) {
  if (!validTaskIndex(taskIndex)) {
    return false;
  }
  return Plugin_task_data[taskIndex] != nullptr &&
         (Plugin_task_data[taskIndex]->_taskdata_pluginID == Settings.getPluginID_for_task(taskIndex));
}

String getPluginCustomArgName(int varNr)                                   { return getPluginCustomArgName(F("pc_arg"), varNr); }

String getPluginCustomArgName(const __FlashStringHelper *label, int varNr) { return concat(label, varNr + 1); }

int    getFormItemIntCustomArgName(int varNr)                              { return getFormItemInt(getPluginCustomArgName(varNr)); }

bool pluginOptionalTaskIndexArgumentMatch(taskIndex_t taskIndex, const String& string, uint8_t paramNr) {
  if (!validTaskIndex(taskIndex)) {
    return false;
  }
  const taskIndex_t found_taskIndex = parseCommandArgumentTaskIndex(string, paramNr);

  if (!validTaskIndex(found_taskIndex)) {
    // Optional parameter not present
    return true;
  }
  return found_taskIndex == taskIndex;
}

bool pluginWebformShowGPIOdescription(taskIndex_t                taskIndex,
                                      const __FlashStringHelper *newline,
                                      String                   & description)
{
  struct EventStruct TempEvent(taskIndex);

  TempEvent.String1 = newline;
  return PluginCall(PLUGIN_WEBFORM_SHOW_GPIO_DESCR, &TempEvent, description);
}

int getValueCountForTask(taskIndex_t taskIndex) {
  struct EventStruct TempEvent(taskIndex);
  String dummy;

  PluginCall(PLUGIN_GET_DEVICEVALUECOUNT, &TempEvent, dummy);
  return TempEvent.Par1;
}

int checkDeviceVTypeForTask(struct EventStruct *event) {
  // TD-er:  Do not use event->getSensorType() here
  if (event->sensorType == Sensor_VType::SENSOR_TYPE_NOT_SET) {
    if (validTaskIndex(event->TaskIndex)) {
      String dummy;

      event->idx = -1;

      if (PluginCall(PLUGIN_GET_DEVICEVTYPE, event, dummy)) {
        return event->idx; // pconfig_index
      }
    }
  }
  return -1;
}
