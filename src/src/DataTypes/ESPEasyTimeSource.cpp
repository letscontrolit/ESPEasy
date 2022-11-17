#include "../DataTypes/ESPEasyTimeSource.h"

#include <Arduino.h>

#include "../../ESPEasy_common.h"

const __FlashStringHelper* toString(timeSource_t timeSource)
{
  switch (timeSource) {
    case timeSource_t::GPS_PPS_time_source:      return F("GPS PPS");
    case timeSource_t::GPS_time_source:          return F("GPS");
    case timeSource_t::NTP_time_source:          return F("NTP");
    case timeSource_t::Manual_set:               return F("Manual");
    case timeSource_t::ESP_now_peer:             return F(ESPEASY_NOW_NAME " peer");
    case timeSource_t::ESPEASY_p2p_UDP:          return F("ESPEasy p2p");
    case timeSource_t::External_RTC_time_source: return F("Ext. RTC at boot");
    case timeSource_t::Restore_RTC_time_source:  return F("RTC at boot");
    case timeSource_t::No_time_source:           return F("No time set");
  }
  return F("Unknown");
}

bool isExternalTimeSource(timeSource_t timeSource)
{
  // timeSource_t::ESP_now_peer or timeSource_t::ESPEASY_p2p_UDP
  // should NOT be considered "external"
  // It may be an unreliable source if no other source is present in the network.

  switch (timeSource) {
    case timeSource_t::GPS_PPS_time_source:
    case timeSource_t::GPS_time_source:
    case timeSource_t::NTP_time_source:
    case timeSource_t::External_RTC_time_source:
    case timeSource_t::Manual_set:
      return true;
    default:
      return false;
  }
}

// Typical time wander for ESP nodes is 0.04 ms/sec
// Meaning per 25 sec, the time may wander 1 msec.
#define TIME_WANDER_FACTOR  25000

unsigned long computeExpectedWander(timeSource_t  timeSource,
                                    unsigned long timePassedSinceLastTimeSync)
{
  unsigned long expectedWander_ms = timePassedSinceLastTimeSync / TIME_WANDER_FACTOR;

  switch (timeSource) {
    case timeSource_t::GPS_PPS_time_source:
    {
      expectedWander_ms += 1;
      break;
    }
    case timeSource_t::GPS_time_source:
    {
      // Not sure about the wander here, as GPS does not have a drift.
      // But the moment a message is received from a second's start may differ.
      expectedWander_ms += 10;
      break;
    }
    case timeSource_t::NTP_time_source:
    {
      // Typical time needed to perform a NTP request to an online NTP server
      expectedWander_ms += 30;
      break;
    }

    case timeSource_t::ESP_now_peer:
    case timeSource_t::ESPEASY_p2p_UDP:
    {
      // expected wander is 144 per hour.
      // Using a 'penalty' of 1000 makes it only preferrable over NTP after +/- 7 hour.
      expectedWander_ms += 1000;
      break;
    }

    case timeSource_t::External_RTC_time_source:
    {
      // Will be off by +/- 500 msec
      expectedWander_ms += 500;
      break;
    }
    case timeSource_t::Restore_RTC_time_source:
    {
      // May be off by the time needed to reboot + some time since the last update of the RTC
      // If a reboot was due to a watchdog reset, then it will be an additional 2 - 6 seconds.
      expectedWander_ms += 5000;
      break;
    }
    case timeSource_t::Manual_set:
    {
      expectedWander_ms += 10000;
      break;
    }
    case timeSource_t::No_time_source:
    {
      // Cannot sync from it.
      return 1 << 30;
    }
  }
  return expectedWander_ms;
}
