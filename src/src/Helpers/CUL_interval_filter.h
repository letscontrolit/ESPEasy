#ifndef DATASTRUCTS_P094_CUL_TIME_FILTER_STRUCT_H
#define DATASTRUCTS_P094_CUL_TIME_FILTER_STRUCT_H

#include "../../ESPEasy_common.h"
#ifdef USES_P094

# include "../DataStructs/mBusPacket.h"

# include <map>


struct CUL_time_filter_struct {
  CUL_time_filter_struct() = default;
  CUL_time_filter_struct(uint32_t checksum,
                         uint32_t timeout_msec);

  uint32_t _checksum{};
  uint32_t _timeout{};
};

typedef uint32_t mBusSerial;

typedef std::map<mBusSerial, CUL_time_filter_struct> mBusFilterMap;


struct CUL_interval_filter {
  // Return true when packet wasn't already present.
  bool add(const mBusPacket_t& packet);

  // Remove packets that have expired.
  void purgeExpired();


  mBusFilterMap _mBusFilterMap;
};

#endif // ifdef USES_P094
#endif // ifndef DATASTRUCTS_P094_CUL_TIME_FILTER_STRUCT_H
