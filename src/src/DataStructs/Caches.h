#ifndef DATASTRUCTS_CACHES_H
#define DATASTRUCTS_CACHES_H

#include <map>
#include "../../ESPEasy_common.h"
#include "../Globals/Plugins.h"

typedef std::map<String, taskIndex_t> TaskIndexNameMap;
typedef std::map<String, byte> TaskIndexValueNameMap;

struct Caches {
  void clearAllCaches();


  TaskIndexNameMap taskIndexName;
  TaskIndexValueNameMap taskIndexValueName;
};


#endif // DATASTRUCTS_CACHES_H
