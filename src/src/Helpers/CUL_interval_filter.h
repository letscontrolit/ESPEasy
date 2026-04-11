#ifndef DATASTRUCTS_P094_CUL_TIME_FILTER_STRUCT_H
#define DATASTRUCTS_P094_CUL_TIME_FILTER_STRUCT_H

#include "../../ESPEasy_common.h"
#ifdef USES_P094

# include "../DataStructs/mBusPacket.h"
# include "../PluginStructs/P094_Filter.h"

# include <map>


struct CUL_time_filter_struct {
  CUL_time_filter_struct() = default;
  CUL_time_filter_struct(uint32_t      checksum,
                         unsigned long UnixTimeExpiration);

  uint32_t      _checksum{};
  unsigned long _UnixTimeExpiration{};
};

typedef uint32_t mBusSerial;

typedef std::map<mBusSerial, CUL_time_filter_struct> mBusFilterMap;


struct CUL_interval_filter {
  // Return true when packet wasn't already present.
  bool filter(const mBusPacket_t& packet,
              const P094_filter & filter);

  // Remove packets that have expired.
  void purgeExpired();


  mBusFilterMap _mBusFilterMap;

  bool enabled = false;
};

#endif // ifdef USES_P094
#endif // ifndef DATASTRUCTS_P094_CUL_TIME_FILTER_STRUCT_H
