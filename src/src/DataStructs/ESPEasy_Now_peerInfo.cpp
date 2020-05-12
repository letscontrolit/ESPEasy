#include "ESPEasy_Now_peerInfo.h"


static uint64_t mac_to_key(const uint8_t *mac)
{
  uint64_t key = 0;

  for (byte i = 0; i < 6; ++i) {
    key  = key << 8;
    key += mac[i];
  }
  return key;
}

void ESPEasy_Now_peerInfo::addPeer(const uint8_t *mac, const ESPEasy_Now_peerInfo_meta& meta)
{
  uint64_t key = mac_to_key(mac);

  peer_map[key] = meta;
}

bool ESPEasy_Now_peerInfo::hasPeer(const uint8_t *mac) const
{
  uint64_t key = mac_to_key(mac);

  return peer_map.find(key) != peer_map.end();
}

bool ESPEasy_Now_peerInfo::getPeer(const uint8_t             *mac,
                                   ESPEasy_Now_peerInfo_meta& meta) const
{
  uint64_t key = mac_to_key(mac);
  auto     it  = peer_map.find(key);

  if (it == peer_map.end()) { return false; }
  meta = it->second;

  return true;
}
