#ifndef DATASTRUCTS_P094_CUL_STATS_H
#define DATASTRUCTS_P094_CUL_STATS_H

#include "../../ESPEasy_common.h"
#ifdef USES_P094

# include "../DataStructs/mBusPacket.h"

# include <map>


struct CUL_Stats_struct {
  uint32_t _UnixTimeFirstSeen{};
  uint32_t _UnixTimeLastSeen{};
  uint16_t _lqi_rssi{};
  uint16_t _count{};
};

typedef uint64_t mBus_EncodedDeviceID;

typedef std::map<mBus_EncodedDeviceID, CUL_Stats_struct> mBusStatsMap;


struct CUL_Stats {
  // Create a string like this:
  // mBus device ID;UNIX time first;UNIX time last;count;LQI;RSSI
  // THC.02.1234567;1674030412;1674031412;123;101,-36
  static String toString(const CUL_Stats_struct& element,
                         mBus_EncodedDeviceID    enc_deviceID);


  // Return true when packet wasn't already present.
  bool   add(const mBusPacket_t& packet);

  // Create string from front element and remove from map
  String getFront();

  mBusStatsMap _mBusStatsMap;
};

#endif // ifdef USES_P094
#endif // ifndef DATASTRUCTS_P094_CUL_STATS_H
