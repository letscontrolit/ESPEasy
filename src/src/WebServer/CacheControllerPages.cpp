#include "../WebServer/CacheControllerPages.h"

#ifdef USES_C016

#include "../WebServer/ESPEasy_WebServer.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/JSON.h"
#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/DeviceStruct.h"
#include "../DataTypes/TaskIndex.h"
#include "../Globals/C016_ControllerCache.h"
#include "../Globals/Cache.h"
#include "../Helpers/ESPEasy_math.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Misc.h"



// ********************************************************************************
// URLs needed for C016_CacheController
// to help dump the content of the binary log files
// ********************************************************************************
void handle_dumpcache() {
  if (!isLoggedIn()) { return; }

  char separator = ';';
  if (hasArg(F("separator"))) {
    String sep = webArg(F("separator"));
    if (isWrappedWithQuotes(sep)) {
      removeChar(sep, sep[0]);
    }
    separator = sep[0];
  }
  
  // First backup the peek file positions.
  int peekFileNr;
  const int peekFilePos = ControllerCache.getPeekFilePos(peekFileNr);

  // Set peek file position to first entry:
  ControllerCache.setPeekFilePos(0, 0);

  C016_flush();

  TXBuffer.startStream();
  {
    String header(F("UNIX timestamp;UTC timestamp;taskindex;plugin ID"));
    if (separator != ';') { header.replace(';', separator); }
    addHtml(header);
  }

  for (taskIndex_t i = 0; i < TASKS_MAX; ++i) {
    for (int j = 0; j < VARS_PER_TASK; ++j) {
      addHtml(separator);
      addHtml(getTaskDeviceName(i));
      addHtml('#');
      addHtml(getTaskValueName(i, j));
    }
  }
  html_BR();

  // Allocate a String per taskvalue instead of per task
  // This way the small strings will hardly ever need heap allocation.
  constexpr size_t nrTaskValues = VARS_PER_TASK * TASKS_MAX;
  String csv_values[nrTaskValues];
  uint8_t nrDecimals[nrTaskValues] = {0};

  {
    // Initialize arrays
    const String sep_zero = String(separator) + '0';
    for (size_t i = 0; i < nrTaskValues; ++i) {
      csv_values[i] = sep_zero;
      nrDecimals[i] = Cache.getTaskDeviceValueDecimals(i / VARS_PER_TASK, i % VARS_PER_TASK);
    }
  }

  C016_binary_element element;

  while (C016_getTaskSample(element)) {
    addHtmlInt(static_cast<uint32_t>(element._timestamp));
    addHtml(separator);
    struct tm ts;
    breakTime(element._timestamp, ts);
    addHtml(formatDateTimeString(ts));
    addHtml(separator);
    addHtmlInt(element.TaskIndex);
    addHtml(separator);
    addHtmlInt(element.pluginID);
    size_t valindex = element.TaskIndex * VARS_PER_TASK;
    for (size_t i = 0; i < VARS_PER_TASK; ++i) {
      csv_values[valindex] = separator;
      if (essentiallyZero(element.values[i])) {
        csv_values[valindex] += '0';
      } else {
        const int decimals = static_cast<int>(nrDecimals[valindex]);
        if (decimals == 0) {
          csv_values[valindex] += static_cast<int>(element.values[i]);
        } else {
          csv_values[valindex] += String(element.values[i], decimals);
        }
      }
      ++valindex;
    }

    for (size_t i = 0; i < nrTaskValues; ++i) {
      addHtml(csv_values[i]);
    }
    html_BR();
//    delay(0);
  }
  TXBuffer.endStream();

  // Restore peek file positions.
  ControllerCache.setPeekFilePos(peekFileNr, peekFilePos);

}

void handle_cache_json() {
  if (!isLoggedIn()) { return; }

  // Flush any data still in RTC memory to the cache files.
  C016_flush();

  TXBuffer.startJsonStream();
  addHtml(F("{\"columns\": ["));

  //     addHtml(F("UNIX timestamp;contr. idx;sensortype;taskindex;value count"));
  addHtml(to_json_value(F("UNIX timestamp")));
  addHtml(',');
  addHtml(to_json_value(F("UTC timestamp")));
  addHtml(',');
  addHtml(to_json_value(F("task index")));
  if (hasArg(F("pluginID"))) {
    addHtml(',');
    addHtml(to_json_value(F("plugin ID")));
  }

  for (taskIndex_t i = 0; i < TASKS_MAX; ++i) {
    for (int j = 0; j < VARS_PER_TASK; ++j) {
      String label = getTaskDeviceName(i);
      label += '#';
      label += getTaskValueName(i, j);
      addHtml(',');
      addHtml(to_json_value(label));
    }
  }
  addHtml(F("],\n"));
  addHtml(F("\"files\": ["));
  bool islast = false;
  int  filenr = 0;
  int fileCount = 0;

  while (!islast) {
    const String currentFile = C016_getCacheFileName(filenr, islast);
    ++filenr;

    if (currentFile.length() > 0) {
      if (fileCount != 0) {
        addHtml(',');
      }
      addHtml(to_json_value(currentFile));
      ++fileCount;
    }
  }
  addHtml(F("],\n"));
  addHtml(F("\"pluginID\": ["));
  for (taskIndex_t taskIndex = 0; validTaskIndex(taskIndex); ++taskIndex) {
    if (taskIndex != 0) {
      addHtml(',');
    }
    addHtmlInt(getPluginID_from_TaskIndex(taskIndex));
  }
  addHtml(F("],\n"));
  stream_next_json_object_value(F("separator"), F(";"));
  stream_last_json_object_value(F("nrfiles"), fileCount);
  addHtml('\n');
  TXBuffer.endStream();
}

void handle_cache_csv() {
  if (!isLoggedIn()) { return; }
}

#endif // ifdef USES_C016
