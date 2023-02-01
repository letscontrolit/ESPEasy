#ifndef DATASTRUCTS_ESPEASYCONTROLLERCACHE_CSV_DUMPER_H
#define DATASTRUCTS_ESPEASYCONTROLLERCACHE_CSV_DUMPER_H


#include "../../ESPEasy_common.h"

#if FEATURE_RTC_CACHE_STORAGE

# include "../ControllerQueue/C016_queue_element.h"

struct ESPEasyControllerCache_CSV_element {
  void markBegin();
  void markEnd();

  String line;
  int    startFileNr = 0;
  int    startPos    = 0;
  int    endFileNr   = 0;
  int    endPos      = 0;
};

struct ESPEasyControllerCache_CSV_dumper {
  enum class Target {
    CSV_file,
    MQTT
  };

  ESPEasyControllerCache_CSV_dumper(bool   joinTimestamp,
                                    bool   onlySetTasks,
                                    char   separator,
                                    Target target);

  ~ESPEasyControllerCache_CSV_dumper();

  size_t generateCSVHeader(bool send) const;

  bool   createCSVLine();

  size_t getCSVlineLength() const {
    return _outputLine.line.length();
  }

  const ESPEasyControllerCache_CSV_element& getCSVline() const {
    return _outputLine;
  }

  size_t writeCSVLine(bool send) const {
    return writeToTarget(_outputLine.line, send);
  }

  void setPeekFilePos(int peekFileNr,
                      int peekReadPos);

private:

  uint32_t writeToTarget(const String& str,
                         bool          send = true) const;

  uint32_t writeToTarget(const char& c,
                         bool        send = true) const;

  void     flushValuesLeft(uint32_t csv_values_left);

  String  _csv_values[VARS_PER_TASK * TASKS_MAX];
  uint8_t _nrDecimals[VARS_PER_TASK * TASKS_MAX] = { 0 };
  bool    _includeTask[TASKS_MAX]                = { 0 };
  bool    _joinTimestamp                         = true;
  bool    _onlySetTasks                          = true;
  char    _separator                             = ',';

  C016_binary_element                _element;
  bool                               _element_processed = true;
  ESPEasyControllerCache_CSV_element _outputLine;
  String                             _taskIndex_str, _pluginID_str;


  int _backup_peekFileNr  = 0;
  int _backup_peekFilePos = 0;

  Target _target = Target::CSV_file;
};
#endif // if FEATURE_RTC_CACHE_STORAGE

#endif // ifndef DATASTRUCTS_ESPEASYCONTROLLERCACHE_CSV_DUMPER_H
