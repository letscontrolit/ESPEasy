
// ********************************************************************************
// ULRs needed for C016_CacheController 
// to help dump the content of the binary log files
// ********************************************************************************
void handle_dumpcache() {
  if (!isLoggedIn()) { return; }

  #ifdef USES_C016

  /*
      String str = F("attachment; filename=cache_");
      str += Settings.Name;
      str += "_U";
      str += Settings.Unit;
      str += F("_Build");
      str += BUILD;
      str += '_';
      if (systemTimePresent())
      {
        str += getDateTimeString('\0', '\0', '\0');
      }
      str += F(".csv");
      WebServer.sendHeader(F("Content-Disposition"), str);
     //    WebServer.streamFile(dataFile, F("application/octet-stream"));
   */
  C016_startCSVdump();
  unsigned long timestamp;
  byte  controller_idx;
  byte  TaskIndex;
  byte  sensorType;
  byte  valueCount;
  float val1;
  float val2;
  float val3;
  float val4;

  TXBuffer.startStream();
  TXBuffer += F("UNIX timestamp;contr. idx;sensortype;taskindex;value count");

  for (int i = 0; i < TASKS_MAX; ++i) {
    LoadTaskSettings(i);

    for (int j = 0; j < VARS_PER_TASK; ++j) {
      TXBuffer += ';';
      TXBuffer += ExtraTaskSettings.TaskDeviceName;
      TXBuffer += '#';
      TXBuffer += ExtraTaskSettings.TaskDeviceValueNames[j];
    }
  }
  TXBuffer += F("<BR>");
  float csv_values[VARS_PER_TASK * TASKS_MAX];

  for (int i = 0; i < VARS_PER_TASK * TASKS_MAX; ++i) {
    csv_values[i] = 0.0;
  }

  while (C016_getCSVline(timestamp, controller_idx, TaskIndex, sensorType,
                         valueCount, val1, val2, val3, val4)) {
    TXBuffer += timestamp;
    TXBuffer += ';';
    TXBuffer += controller_idx;
    TXBuffer += ';';
    TXBuffer += sensorType;
    TXBuffer += ';';
    TXBuffer += TaskIndex;
    TXBuffer += ';';
    TXBuffer += valueCount;
    int valindex = TaskIndex * VARS_PER_TASK;
    csv_values[valindex++] = val1;
    csv_values[valindex++] = val2;
    csv_values[valindex++] = val3;
    csv_values[valindex++] = val4;

    for (int i = 0; i < VARS_PER_TASK * TASKS_MAX; ++i) {
      TXBuffer += ';';

      if (csv_values[i] == 0.0) {
        TXBuffer += '0';
      } else {
        TXBuffer += String(csv_values[i], 6);
      }
    }
    TXBuffer += F("<BR>");
    delay(0);
  }
  TXBuffer.endStream();

  #endif // ifdef USES_C016
}

void handle_cache_json() {
  if (!isLoggedIn()) { return; }

  #ifdef USES_C016
  TXBuffer.startJsonStream();
  TXBuffer += F("{\"columns\": [");

  //     TXBuffer += F("UNIX timestamp;contr. idx;sensortype;taskindex;value count");
  stream_to_json_value(F("UNIX timestamp"));
  TXBuffer += ',';
  stream_to_json_value(F("UTC timestamp"));
  TXBuffer += ',';
  stream_to_json_value(F("task index"));

  for (int i = 0; i < TASKS_MAX; ++i) {
    LoadTaskSettings(i);

    for (int j = 0; j < VARS_PER_TASK; ++j) {
      String label = ExtraTaskSettings.TaskDeviceName;
      label    += '#';
      label    += ExtraTaskSettings.TaskDeviceValueNames[j];
      TXBuffer += ',';
      stream_to_json_value(label);
    }
  }
  TXBuffer += F("],\n");
  C016_startCSVdump();
  TXBuffer += F("\"files\": [");
  bool islast = false;
  int  filenr = 0;

  while (!islast) {
    String currentFile = C016_getCacheFileName(islast);

    if (currentFile.length() > 0) {
      if (filenr != 0) {
        TXBuffer += ',';
      }
      stream_to_json_value(currentFile);
      ++filenr;
    }
  }
  TXBuffer += F("],\n");
  stream_last_json_object_value(F("nrfiles"), String(filenr));
  TXBuffer += F("\n");
  TXBuffer.endStream();
  #endif // ifdef USES_C016
}

void handle_cache_csv() {
  if (!isLoggedIn()) { return; }
}
