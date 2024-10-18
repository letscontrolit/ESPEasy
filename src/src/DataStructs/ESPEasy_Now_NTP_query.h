#ifndef DATASTRUCT_ESPEASY_NOW_NTP_QUERY_H
#define DATASTRUCT_ESPEASY_NOW_NTP_QUERY_H

#include "../../ESPEasy_common.h"

#ifdef USES_ESPEASY_NOW

#include <Arduino.h>
#include <stdint.h>


#include "../Globals/ESPEasy_now_state.h"

#include "../DataTypes/ESPEasyTimeSource.h"

class MAC_address;

class ESPEasy_Now_NTP_query {
public:

  ESPEasy_Now_NTP_query();

  bool getMac(MAC_address& mac) const;

  void find_best_NTP(const MAC_address& mac,
                     timeSource_t       timeSource,
                     unsigned long      timePassedSinceLastTimeSync);

  void                 reset(bool success);

  bool                 hasLowerWander() const;

  bool                 isBroadcast() const;

  void                 markSendTime();

  void                 createBroadcastNTP();

  void                 createReply(unsigned long queryReceiveTimestamp);

  bool                 processReply(const ESPEasy_Now_NTP_query& received,
                                    unsigned long                receiveTimestamp);

  // FIXME TD-er: These members could all be private, but the processReply function must then be a friend function.
  // However this gives lots of warnings as the class is then a friend of itself.

  double _unixTime_d;
  unsigned long _millis_in;
  unsigned long _millis_out;
  unsigned long _expectedWander_ms;
  timeSource_t _timeSource = timeSource_t::No_time_source;
  uint8_t _mac[6]          = { 0 };

  // if previous best choice failed, keep track of it.
  // so we will not get stuck retrying the one that does not reply
  uint8_t _mac_prev_fail[6] = { 0 };
};

#endif
#endif // DATASTRUCT_ESPEASY_NOW_NTP_QUERY_H
