#ifndef DATASTRUCTS_ESPEASYCONTROLLERCACHE_CSV_DUMPER_H
#define DATASTRUCTS_ESPEASYCONTROLLERCACHE_CSV_DUMPER_H


#include "../../ESPEasy_common.h"

#if FEATURE_RTC_CACHE_STORAGE

# include "../ControllerQueue/C016_queue_element.h"
struct ESPEasyControllerCache_CSV_dumper {
  enum class Target {
    CSV_file,
    MQTT
  };

  ESPEasyControllerCache_CSV_dumper(bool joinTimestamp,
                                    bool onlySetTasks,
                                    char separator);

  ~ESPEasyControllerCache_CSV_dumper();

  size_t generateCSVHeader(bool send) const;

  bool   createCSVLine();

  size_t writeCSVLine(bool send) const {
    return writeToTarget(_outputLine, send);
  }

  void setTarget(Target target) {
    _target = target;
  }

private:

  uint32_t writeToTarget(const String& str,
                         bool          send = true) const;

  uint32_t writeToTarget(const char& c,
                         bool        send = true) const;

  String  _csv_values[VARS_PER_TASK * TASKS_MAX];
  uint8_t _nrDecimals[VARS_PER_TASK * TASKS_MAX] = { 0 };
  bool    _includeTask[TASKS_MAX]                = { 0 };
  bool    _joinTimestamp                         = true;
  bool    _onlySetTasks                          = true;
  char    _separator                             = ',';

  C016_binary_element element;
  bool                _element_processed = false;
  String              _outputLine;


  int _backup_peekFileNr  = 0;
  int _backup_peekFilePos = 0;

  Target _target = Target::CSV_file;
};
#endif // if FEATURE_RTC_CACHE_STORAGE

#endif // ifndef DATASTRUCTS_ESPEASYCONTROLLERCACHE_CSV_DUMPER_H
