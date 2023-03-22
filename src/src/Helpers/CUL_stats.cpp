#include "../Helpers/CUL_stats.h"

#ifdef USES_P094

# include "../ESPEasyCore/ESPEasy_Log.h"
# include "../Globals/ESPEasy_time.h"
# include "../Helpers/ESPEasy_time_calc.h"
# include "../Helpers/StringConverter.h"

String CUL_Stats::toString(const CUL_Stats_struct& element, mBus_EncodedDeviceID enc_deviceID)
{
  uint8_t LQI        = 0;
  const int16_t rssi = mBusPacket_t::decode_LQI_RSSI(element._lqi_rssi, LQI);
  mBusPacket_header_t deviceID;

  deviceID.decode_fromUint64(enc_deviceID);

  String res = deviceID.toString();

  res += ';';
  res += element._UnixTimeFirstSeen;
  res += ';';
  res += element._UnixTimeLastSeen;
  res += ';';
  res += element._count;
  res += ';';
  res += LQI;
  res += ';';
  res += rssi;
  return res;
}

bool CUL_Stats::add(const mBusPacket_t& packet)
{
  const mBus_EncodedDeviceID deviceID = packet.deviceID_toUInt64();

  if (deviceID == 0) { return false; }

  auto it = _mBusStatsMap.find(deviceID);

  if (it == _mBusStatsMap.end()) {
    CUL_Stats_struct tmp;
    tmp._count              = 1;
    tmp._lqi_rssi           = packet._lqi_rssi;
    tmp._UnixTimeFirstSeen  = node_time.now();
    tmp._UnixTimeLastSeen   = tmp._UnixTimeFirstSeen;
    _mBusStatsMap[deviceID] = tmp;
    return true;
  }
  it->second._count++;
  it->second._lqi_rssi         = packet._lqi_rssi;
  it->second._UnixTimeLastSeen = node_time.now();
  return false;
}

String CUL_Stats::getFront()
{
  auto it = _mBusStatsMap.begin();

  if (it == _mBusStatsMap.end()) { return EMPTY_STRING; }
  const String res = toString(it->second, it->first);

  _mBusStatsMap.erase(it);
  return res;
}

#endif // ifdef USES_P094
