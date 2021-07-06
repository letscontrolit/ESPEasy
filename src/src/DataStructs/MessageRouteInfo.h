#ifndef DATASTRUCTS_UNITMESSAGECOUNT_H
#define DATASTRUCTS_UNITMESSAGECOUNT_H

#include "../../ESPEasy_common.h"

#include <map>

// For deduplication, some controllers may add a unit ID and current counter.
// This count will wrap around, so it is just to detect if a message is received multiple times.
// The unit ID is the unit where the message originates from and thus should be kept along when forwarding.
struct MessageRouteInfo_t {
  typedef std::vector<uint8_t> uint8_t_vector;


  MessageRouteInfo_t() {}

  MessageRouteInfo_t(uint8_t unitnr, uint8_t messageCount) : unit(unitnr), count(messageCount) {}

  MessageRouteInfo_t(const uint8_t* serializedData, size_t size);

  MessageRouteInfo_t(const uint8_t_vector& serializedData);

  bool deserialize(const uint8_t* serializedData, size_t size);

  uint8_t_vector serialize() const;

  uint8_t unit  = 0; // Initialize to "not set"
  uint8_t count = 0;
  uint8_t dest_unit  = 0; // Initialize to "not set"

  uint8_t_vector trace;
};

struct UnitMessageRouteInfo_map {
  bool isNew(const MessageRouteInfo_t *count) const;

  void add(const MessageRouteInfo_t *count);

private:

  std::map<uint8_t, MessageRouteInfo_t> _map;
};

#endif // ifndef DATASTRUCTS_UNITMESSAGECOUNT_H
