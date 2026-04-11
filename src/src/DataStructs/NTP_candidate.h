#ifndef DATASTRUCT_NTP_CANDIDATE_H
#define DATASTRUCT_NTP_CANDIDATE_H

#include "../../ESPEasy_common.h"
#if FEATURE_ESPEASY_P2P
# include "../DataStructs/NodeStruct.h"

struct NTP_candidate_struct {
  bool set(const NodeStruct& node);

  void clear();

  // Returns whether a node has reported to have its system time set.
  // Time since first node reported its time should be at least 30 seconds
  // to allow for the node with the most accurate time estimate to have
  // reported its system time.
  // Reported Unix time is compensated for the time passed since it was received.
  timeSource_t getUnixTime(
    double & unix_time_d,
    int32_t& wander,
    uint8_t& unit) const;

  uint32_t     _unix_time_sec         = 0;
  uint32_t     _unix_time_frac        = 0;
  int32_t      _time_wander           = -1;
  uint32_t     _received_moment       = 0;
  uint32_t     _first_received_moment = 0;
  uint8_t      _unit                  = 0;
  timeSource_t _timeSource            = timeSource_t::No_time_source;
};
#endif // if FEATURE_ESPEASY_P2P
#endif // ifndef DATASTRUCT_NTP_CANDIDATE_H
