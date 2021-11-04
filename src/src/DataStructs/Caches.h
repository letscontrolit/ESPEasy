#ifndef DATASTRUCTS_CACHES_H
#define DATASTRUCTS_CACHES_H

#include <map>
#include "../../ESPEasy_common.h"
#include "../Globals/Plugins.h"

typedef std::map<String, taskIndex_t> TaskIndexNameMap;
typedef std::map<String, uint8_t>     TaskIndexValueNameMap;
typedef std::map<String, int>         FilePresenceMap; 

struct Caches {
  void clearAllCaches();

  void updateTaskCaches();

  void updateActiveTaskUseSerial0();

  TaskIndexNameMap      taskIndexName;
  TaskIndexValueNameMap taskIndexValueName;
  FilePresenceMap       fileExistsMap;  // Filesize. -1 if not present
  bool                  activeTaskUseSerial0 = false;
};


#endif // DATASTRUCTS_CACHES_H
