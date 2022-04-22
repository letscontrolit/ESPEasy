#ifndef DATASTRUCTS_CACHES_H
#define DATASTRUCTS_CACHES_H

#include <map>
#include "../../ESPEasy_common.h"
#include "../Globals/Plugins.h"

#include "../Helpers/RulesHelper.h"

typedef std::map<String, taskIndex_t>TaskIndexNameMap;
typedef std::map<String, uint8_t>       TaskIndexValueNameMap;
typedef std::map<String, bool>       FilePresenceMap;

struct Caches {
  void clearAllCaches();

  void updateTaskCaches();

  void updateActiveTaskUseSerial0();

  TaskIndexNameMap      taskIndexName;
  TaskIndexValueNameMap taskIndexValueName;
  FilePresenceMap       fileExistsMap;
  RulesHelperClass      rulesHelper;
  bool                  activeTaskUseSerial0 = false;
};


#endif // DATASTRUCTS_CACHES_H
