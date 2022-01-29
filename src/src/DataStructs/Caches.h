#ifndef DATASTRUCTS_CACHES_H
#define DATASTRUCTS_CACHES_H

#include <map>
#include "../../ESPEasy_common.h"
#include "../Globals/Plugins.h"

typedef std::map<String, taskIndex_t>TaskIndexNameMap;
typedef std::map<String, uint8_t>       TaskIndexValueNameMap;
typedef std::map<String, bool>       FilePresenceMap;

struct Caches {
  void clearAllCaches();

  void updateTaskCaches();

  void updateActiveTaskUseSerial0();

  static int taskValueIndex(taskIndex_t taskIndex, uint8_t rel_index); 

  int8_t getTaskValueDecimals(taskIndex_t taskIndex, uint8_t rel_index) const;

  void setTaskValueDecimals(taskIndex_t taskIndex, uint8_t rel_index, int8_t decimals);

  TaskIndexNameMap      taskIndexName;
  TaskIndexValueNameMap taskIndexValueName;
  FilePresenceMap       fileExistsMap;
  bool                  activeTaskUseSerial0 = false;
  int8_t taskValueDecimals[TASKS_MAX * VARS_PER_TASK];
};


#endif // DATASTRUCTS_CACHES_H
