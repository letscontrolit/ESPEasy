#include "../Helpers/CUL_stats.h"

#ifdef USES_P094

# include "../ESPEasyCore/ESPEasy_Log.h"
# include "../Globals/ESPEasy_time.h"
# include "../Helpers/CRC_functions.h"
# include "../Helpers/ESPEasy_time_calc.h"
# include "../Helpers/StringConverter.h"

# include "../WebServer/Markup.h"
# include "../WebServer/HTML_wrappers.h"

String CUL_Stats::toString(const CUL_Stats_struct& element) const
{
  uint8_t LQI        = 0;
  const int16_t rssi = mBusPacket_t::decode_LQI_RSSI(element._lqi_rssi, LQI);

  // e.g.: THC.02.12345678;1674030412;1674031412;123;101,-36
  static size_t estimated_length = 52;

  String res;

  res.reserve(estimated_length);
  {
    auto it = _mBusStatsSourceMap.find(element._sourceHash);
    if (element._sourceHash != 0u && it != _mBusStatsSourceMap.end()) {
      res += it->second;
    } else {
      res += '-';
    }
  }
  res += ';';

  if (element._id1 != 0u) {
    mBusPacket_header_t deviceID;
    deviceID.decode_fromUint64(element._id1);
    res += deviceID.toString();
  } else {
    res += '-';
  }
  res += ';';

  if (element._id2 != 0u) {
    mBusPacket_header_t deviceID;
    deviceID.decode_fromUint64(element._id2);
    res += deviceID.toString();
  } else {
    res += '-';
  }
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

  if (res.length() > estimated_length) {
    estimated_length = res.length();
  }
  return res;
}

bool CUL_Stats::add(const mBusPacket_t& packet)
{
  const CUL_stats_hash sourceHash{};
  return add(packet, packet.deviceID_to_map_key_no_length(), sourceHash);
}


bool CUL_Stats::add(const mBusPacket_t& packet, const String& source)
{
  CUL_stats_hash key = packet.deviceID_to_map_key_no_length();
  CUL_stats_hash sourceHash{};

  if (!source.isEmpty()) {
    sourceHash = calc_CRC32((const uint8_t *)(source.c_str()), source.length());
    _mBusStatsSourceMap[sourceHash] = source;
    key ^= sourceHash;
  }

  return add(packet, key, sourceHash);
}

bool CUL_Stats::add(const mBusPacket_t& packet, CUL_stats_hash key, CUL_stats_hash sourceHash)
{
  if (key == 0) { return false; }

  auto it = _mBusStatsMap.find(key);

  if (it == _mBusStatsMap.end()) {
    CUL_Stats_struct tmp;
    tmp._count              = 1;
    tmp._id1                = packet._deviceId1.encode_toUInt64();
    tmp._id2                = packet._deviceId2.encode_toUInt64();
    tmp._lqi_rssi           = packet._lqi_rssi;
    tmp._UnixTimeFirstSeen  = node_time.now();
    tmp._UnixTimeLastSeen   = tmp._UnixTimeFirstSeen;
    tmp._sourceHash         = sourceHash;
    _mBusStatsMap[key]      = tmp;
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
  const String res = toString(it->second);

  _mBusStatsMap.erase(it);
  return res;
}

void CUL_Stats::toHtml() const
{
  addRowLabel(F("CUL stats"));

  for (auto it = _mBusStatsMap.begin(); it != _mBusStatsMap.end(); ++it) {
    addHtml(toString(it->second));
    addHtml(F("<BR>"));
  }
}

#endif // ifdef USES_P094
