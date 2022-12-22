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
#include "../Helpers/ESPEasy_math.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Misc.h"



// ********************************************************************************
// URLs needed for C016_CacheController
// to help dump the content of the binary log files
// ********************************************************************************
void handle_dumpcache() {
  if (!isLoggedIn()) { return; }

  C016_startCSVdump();
  unsigned long timestamp;
  uint8_t  controller_idx;
  uint8_t  TaskIndex;
  Sensor_VType  sensorType;
  uint8_t  valueCount;
  float val1;
  float val2;
  float val3;
  float val4;

  TXBuffer.startStream();
  addHtml(F("UNIX timestamp;contr. idx;sensortype;taskindex;value count"));

  for (taskIndex_t i = 0; i < TASKS_MAX; ++i) {
    for (int j = 0; j < VARS_PER_TASK; ++j) {
      addHtml(';');
      addHtml(getTaskDeviceName(i));
      addHtml('#');
      addHtml(getTaskValueName(i, j));
    }
  }
  html_BR();
  float csv_values[VARS_PER_TASK * TASKS_MAX];

  for (int i = 0; i < VARS_PER_TASK * TASKS_MAX; ++i) {
    csv_values[i] = 0.0f;
  }

  while (C016_getCSVline(timestamp, controller_idx, TaskIndex, sensorType,
                         valueCount, val1, val2, val3, val4)) {
    {
      String html;
      html.reserve(64);
      html += timestamp;
      html += ';';
      html += controller_idx;
      html += ';';
      html += static_cast<uint8_t>(sensorType);
      html += ';';
      html += TaskIndex;
      html += ';';
      html += valueCount;
      addHtml(html);
    }
    int valindex = TaskIndex * VARS_PER_TASK;
    csv_values[valindex++] = val1;
    csv_values[valindex++] = val2;
    csv_values[valindex++] = val3;
    csv_values[valindex++] = val4;

    for (int i = 0; i < VARS_PER_TASK * TASKS_MAX; ++i) {
      if (essentiallyZero(csv_values[i])) {
        addHtml(';', '0');
      } else {
        addHtml(';');
        addHtmlFloat(csv_values[i], 6);
      }
    }
    html_BR();
    delay(0);
  }
  TXBuffer.endStream();
}

void handle_cache_json() {
  if (!isLoggedIn()) { return; }

  // Flush any data still in RTC memory to the cache files.
  C016_startCSVdump();

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
