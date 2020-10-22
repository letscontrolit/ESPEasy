#include "../WebServer/CacheControllerPages.h"

#ifdef USES_C016

#include "../WebServer/WebServer.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/JSON.h"
#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/DeviceStruct.h"
#include "../DataTypes/TaskIndex.h"
#include "../Globals/C016_ControllerCache.h"
#include "../Globals/ExtraTaskSettings.h"
#include "../Helpers/ESPEasy_Storage.h"


// ********************************************************************************
// URLs needed for C016_CacheController
// to help dump the content of the binary log files
// ********************************************************************************
void handle_dumpcache() {
  if (!isLoggedIn()) { return; }

  C016_startCSVdump();
  unsigned long timestamp;
  byte  controller_idx;
  byte  TaskIndex;
  Sensor_VType  sensorType;
  byte  valueCount;
  float val1;
  float val2;
  float val3;
  float val4;

  TXBuffer.startStream();
  addHtml(F("UNIX timestamp;contr. idx;sensortype;taskindex;value count"));

  for (taskIndex_t i = 0; i < TASKS_MAX; ++i) {
    LoadTaskSettings(i);

    for (int j = 0; j < VARS_PER_TASK; ++j) {
      addHtml(";");
      addHtml(ExtraTaskSettings.TaskDeviceName);
      addHtml("#");
      addHtml(ExtraTaskSettings.TaskDeviceValueNames[j]);
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
      html += static_cast<byte>(sensorType);
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
      String html;
      html.reserve(12);
      html += ';';

      if (csv_values[i] == 0.0f) {
        html += '0';
      } else {
        html += String(csv_values[i], 6);
      }
      addHtml(html);
    }
    html_BR();
    delay(0);
  }
  TXBuffer.endStream();
}

void handle_cache_json() {
  if (!isLoggedIn()) { return; }

  TXBuffer.startJsonStream();
  addHtml(F("{\"columns\": ["));

  //     addHtml(F("UNIX timestamp;contr. idx;sensortype;taskindex;value count"));
  stream_to_json_value(F("UNIX timestamp"));
  addHtml(",");
  stream_to_json_value(F("UTC timestamp"));
  addHtml(",");
  stream_to_json_value(F("task index"));

  for (taskIndex_t i = 0; i < TASKS_MAX; ++i) {
    LoadTaskSettings(i);

    for (int j = 0; j < VARS_PER_TASK; ++j) {
      String label = ExtraTaskSettings.TaskDeviceName;
      label += '#';
      label += ExtraTaskSettings.TaskDeviceValueNames[j];
      addHtml(",");
      stream_to_json_value(label);
    }
  }
  addHtml(F("],\n"));
  C016_startCSVdump();
  addHtml(F("\"files\": ["));
  bool islast = false;
  int  filenr = 0;

  while (!islast) {
    String currentFile = C016_getCacheFileName(islast);

    if (currentFile.length() > 0) {
      if (filenr != 0) {
        addHtml(",");
      }
      stream_to_json_value(currentFile);
      ++filenr;
    }
  }
  addHtml(F("],\n"));
  stream_last_json_object_value(F("nrfiles"), String(filenr));
  addHtml(F("\n"));
  TXBuffer.endStream();
}

void handle_cache_csv() {
  if (!isLoggedIn()) { return; }
}

#endif // ifdef USES_C016
