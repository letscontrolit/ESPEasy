#ifndef DATATYPES_ESPEASYTIMESOURCE_H
#define DATATYPES_ESPEASYTIMESOURCE_H


#include <stdint.h>
#include <Arduino.h>

class String;

#define EXT_TIME_SOURCE_MIN_UPDATE_INTERVAL_MSEC 3600000
#define EXT_TIME_SOURCE_MIN_UPDATE_INTERVAL_SEC 3600

// Time Source type, sort by priority.
// Enum values are sent via NodeStruct, so only add new ones and don't change existing values
// typical time wander of an ESP module is 40 ppm, or 0.04 msec/sec, or roughly 3.5 seconds per 24h.
enum class timeSource_t : uint8_t {
  // External time source  (considered more reliable)
  GPS_PPS_time_source = 5,      // 1 - 10 msec accuracy
  GPS_time_source     = 10,     // 10 - 100 msec accuracy
  NTP_time_source     = 15,     // 20 - 100 msec accuracy

  // Manual override has higher priority because it is some kind of external sync
  Manual_set          = 20,     // Unknown accuracy

  // Sources which may drift over time due to lack of external synchronization.
  ESP_now_peer        = 40,     // < 5 msec accuracy between nodes, but time on the whole network may drift
  ESPEASY_p2p_UDP     = 41,
  External_RTC_time_source = 45, // Typically +/- 500 msec off.

  Restore_RTC_time_source = 50, // > 1 sec difference per reboot
  No_time_source          = 255 // No time set
};

const __FlashStringHelper* toString(timeSource_t timeSource);
bool isExternalTimeSource(timeSource_t timeSource);

// Only use peers if there is no external source available.
// A network without external synced source may drift as a whole
// All nodes in the network may be in sync with each other, but get out of sync with the rest of the world.
// Therefore use a strong bias for external synced nodes.
// But also must make sure the same NTP synced node will be held responsible for the entire network.
unsigned long computeExpectedWander(timeSource_t  timeSource,
                                    unsigned long timePassedSinceLastTimeSync);

#endif /* DATATYPES_ESPEASYTIMESOURCE_H */