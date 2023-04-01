#ifndef GLOBALS_CACHE_H
#define GLOBALS_CACHE_H

#include "../DataStructs/Caches.h"

void clearAllCaches();

void clearAllButTaskCaches();

void clearTaskCache(taskIndex_t TaskIndex);

void clearFileCaches();

void updateActiveTaskUseSerial0();

bool activeTaskUseSerial0();

extern Caches Cache;

#endif // GLOBALS_CACHE_H
