#ifndef DATASTRUCTS_P094_CUL_STATS_H
#define DATASTRUCTS_P094_CUL_STATS_H

#include "../../ESPEasy_common.h"
#ifdef USES_P094

# include "../DataStructs/mBusPacket.h"

# include <map>


typedef uint64_t mBus_EncodedDeviceID;

typedef uint32_t CUL_stats_hash;


struct CUL_Stats_struct {
  mBus_EncodedDeviceID _id1{};
  mBus_EncodedDeviceID _id2{};
  uint32_t _UnixTimeFirstSeen{};
  uint32_t _UnixTimeLastSeen{};
  uint16_t _lqi_rssi{};
  uint16_t _count{};
  CUL_stats_hash _sourceHash{};
};


typedef std::map<CUL_stats_hash, CUL_Stats_struct> mBusStatsMap;
typedef std::map<CUL_stats_hash, String> mBusStatsSourceMap;


struct CUL_Stats {
  // Create a string like this:
  // mBus device ID;UNIX time first;UNIX time last;count;LQI;RSSI
  // THC.02.12345678;1674030412;1674031412;123;101,-36
  String toString(const CUL_Stats_struct& element) const;


  // Return true when packet wasn't already present.
  bool   add(const mBusPacket_t& packet);
  bool   add(const mBusPacket_t& packet, const String& source);

private:

  bool   add(const mBusPacket_t& packet, CUL_stats_hash key, CUL_stats_hash sourceHash);

public:

  // Create string from front element and remove from map
  String getFront();

  void toHtml() const;

  mBusStatsMap _mBusStatsMap;
  mBusStatsSourceMap _mBusStatsSourceMap;
};

#endif // ifdef USES_P094
#endif // ifndef DATASTRUCTS_P094_CUL_STATS_H
