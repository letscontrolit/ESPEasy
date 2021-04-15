#include "../Globals/Cache.h"

#include "../DataStructs/Caches.h"

void clearAllCaches()
{
  Cache.clearAllCaches();
}

void updateTaskCaches()
{
  Cache.updateTaskCaches();
}

void updateActiveTaskUseSerial0()
{
  Cache.updateActiveTaskUseSerial0();
}

bool activeTaskUseSerial0()
{
  return Cache.activeTaskUseSerial0;
}

Caches Cache;
