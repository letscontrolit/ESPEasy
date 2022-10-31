#ifndef DATASTRUCT_NTP_CANDIDATE_H
#define DATASTRUCT_NTP_CANDIDATE_H

#include "../../ESPEasy_common.h"
#if FEATURE_ESPEASY_P2P
# include "../DataStructs/NodeStruct.h"

struct NTP_candidate_struct {
  bool set(const NodeStruct& node);

  void clear();

  bool getUnixTime(double& unix_time_d, uint8_t& unit) const;

  uint32_t _unix_time_sec         = 0;
  uint32_t _unix_time_frac        = 0;
  int32_t  _time_wander           = -1;
  uint32_t _received_moment       = 0;
  uint32_t _first_received_moment = 0;
  uint8_t  _unit                  = 0;
};
#endif // if FEATURE_ESPEASY_P2P
#endif // ifndef DATASTRUCT_NTP_CANDIDATE_H
