#ifndef DATASTRUCT_NTP_CANDIDATE_H
#define DATASTRUCT_NTP_CANDIDATE_H

#include "../../ESPEasy_common.h"
#if FEATURE_ESPEASY_P2P
# include "../DataStructs/NodeStruct.h"

struct NTP_candidate_struct {
  void set(const NodeStruct& node);

  void clear();

  bool getUnixTime(uint32_t& unix_time) const;

  uint32_t _unix_time       = 0;
  int32_t  _time_wander     = -1;
  uint32_t _received_moment = 0;
};
#endif // if FEATURE_ESPEASY_P2P
#endif // ifndef DATASTRUCT_NTP_CANDIDATE_H
