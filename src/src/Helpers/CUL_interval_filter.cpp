#include "../Helpers/CUL_interval_filter.h"


#ifdef USES_P094

# include "../ESPEasyCore/ESPEasy_Log.h"
# include "../Helpers/ESPEasy_time_calc.h"
# include "../Helpers/StringConverter.h"


# define CUL_FILTER_TIMEOUT_MSEC 300000 // 5 minutes


CUL_time_filter_struct::CUL_time_filter_struct(uint32_t checksum, uint32_t timeout_msec) : _checksum(checksum)
{
  _timeout = millis() + timeout_msec;
}

bool CUL_interval_filter::add(const mBusPacket_t& packet)
{
  const mBusSerial serial = packet.getDeviceSerial();
  auto it                 = _mBusFilterMap.find(serial);

  if (it != _mBusFilterMap.end()) {
    // Already present
    if (!timeOutReached(it->second._timeout)) {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLogMove(LOG_LEVEL_INFO, concat(F("CUL   : Interval filtered: "), packet.toString()));
      }
      return false;
    }

    if (packet._checksum == it->second._checksum) {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLogMove(LOG_LEVEL_INFO, concat(F("CUL   : Interval Same Checksum: "), packet.toString()));
      }
      return false;
    }

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLogMove(LOG_LEVEL_INFO, concat(F("CUL   : Remove from Interval Filter: "), packet.toString()));
    }
    _mBusFilterMap.erase(it);
  }

  CUL_time_filter_struct item(packet._checksum, CUL_FILTER_TIMEOUT_MSEC);

  _mBusFilterMap[serial] = item;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO, concat(F("CUL   : Add to IntervalFilter: "), packet.toString()));
  }

  return true;
}

void CUL_interval_filter::purgeExpired()
{
  auto it = _mBusFilterMap.begin();

  for (; it != _mBusFilterMap.end();) {
    if (timeOutReached(it->second._timeout)) {
      it = _mBusFilterMap.erase(it);
    } else {
      ++it;
    }
  }
}

#endif // ifdef USES_P094
