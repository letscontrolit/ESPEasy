#include "../Helpers/CUL_interval_filter.h"


#ifdef USES_P094

# include "../ESPEasyCore/ESPEasy_Log.h"
# include "../Globals/ESPEasy_time.h"
# include "../Globals/TimeZone.h"
# include "../Helpers/ESPEasy_time_calc.h"
# include "../Helpers/StringConverter.h"


CUL_time_filter_struct::CUL_time_filter_struct(uint32_t checksum, unsigned long UnixTimeExpiration)
  : _checksum(checksum), _UnixTimeExpiration(UnixTimeExpiration) {}

String CUL_interval_filter_getExpiration_log_str(const P094_filter& filter)
{
  const unsigned long expiration = filter.computeUnixTimeExpiration();

  if ((expiration != 0) && (expiration != 0xFFFFFFFF)) {
    struct tm exp_tm;
    breakTime(time_zone.toLocal(expiration), exp_tm);

    return concat(F(" Expiration: "), formatDateTimeString(exp_tm));
  }
  return EMPTY_STRING;
}

bool CUL_interval_filter::filter(const mBusPacket_t& packet, const P094_filter& filter)
{
  if (!enabled) {
    return true;
  }

  if (filter.getFilterWindow() == P094_Filter_Window::None) {
    // Will always be rejected, so no need to keep track of the message
    return false;
  }

  if (filter.getFilterWindow() == P094_Filter_Window::All) {
    // Will always be allowed, so no need to keep track of the message
    return true;
  }

  const uint32_t key = packet.deviceID_to_map_key();
  auto it            = _mBusFilterMap.find(key);

  if (it != _mBusFilterMap.end()) {
    // Already present
    if (node_time.getUnixTime() < it->second._UnixTimeExpiration) {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = concat(F("CUL   : Interval filtered: "), packet.toString());
        log += CUL_interval_filter_getExpiration_log_str(filter);
        addLogMove(LOG_LEVEL_INFO, log);
      }
      return false;
    }

    if (packet._checksum == it->second._checksum) {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLogMove(LOG_LEVEL_INFO, concat(F("CUL   : Interval Same Checksum: "), packet.toString()));
      }
      return false;
    }

    // Has expired, so remove from filter map
    _mBusFilterMap.erase(it);
  }

  const unsigned long expiration = filter.computeUnixTimeExpiration();

  CUL_time_filter_struct item(packet._checksum, expiration);

  _mBusFilterMap[key] = item;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = concat(F("CUL   : Add to IntervalFilter: "), packet.toString());
    log += CUL_interval_filter_getExpiration_log_str(filter);

    addLogMove(LOG_LEVEL_INFO, log);
  }

  return true;
}

void CUL_interval_filter::purgeExpired()
{
  auto it = _mBusFilterMap.begin();

  const unsigned long currentTime = node_time.getUnixTime();

  for (; it != _mBusFilterMap.end();) {
    if (currentTime > it->second._UnixTimeExpiration) {
      it = _mBusFilterMap.erase(it);
    } else {
      ++it;
    }
  }
}

#endif // ifdef USES_P094
