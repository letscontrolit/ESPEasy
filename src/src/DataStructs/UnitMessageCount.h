#ifndef DATASTRUCTS_UNITMESSAGECOUNT_H
#define DATASTRUCTS_UNITMESSAGECOUNT_H

#include "../../ESPEasy_common.h"

#include <map>

// For deduplication, some controllers may add a unit ID and current counter.
// This count will wrap around, so it is just to detect if a message is received multiple times.
// The unit ID is the unit where the message originates from and thus should be kept along when forwarding.
struct UnitMessageCount_t {
  UnitMessageCount_t() {}

  UnitMessageCount_t(byte unitnr, byte messageCount) : unit(unitnr), count(messageCount) {}

  byte unit  = 0; // Initialize to "not set"
  byte count = 0;
};

struct UnitLastMessageCount_map {
  bool isNew(const UnitMessageCount_t *count) const;

  void add(const UnitMessageCount_t *count);

private:

  std::map<byte, byte>_map;
};

#endif // ifndef DATASTRUCTS_UNITMESSAGECOUNT_H
