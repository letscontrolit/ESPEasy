#ifndef DATASTRUCTS_ESPEASY_NOW_PEER_H
#define DATASTRUCTS_ESPEASY_NOW_PEER_H

#include <Arduino.h>

#include "../Globals/ESPEasy_now_state.h"

#ifdef USES_ESPEASY_NOW

class __attribute__((__packed__)) ESPEasy_Now_peer {
public:

  ESPEasy_Now_peer();
  ESPEasy_Now_peer(uint8_t mac[6],
                   uint8_t channel,
                   int8_t  rssi);

  void setMac(uint8_t mac[6]);

  bool equalMac(const ESPEasy_Now_peer &other) const;

  bool update(const ESPEasy_Now_peer &other);

  // Compare peers.
  // Return true when this peer has better RSSI and/or is a confirmed ESPEasy-Now peer
  bool operator<(const ESPEasy_Now_peer &other) const;

  // A peer is equal, when it has the same MAC.
  bool operator==(const ESPEasy_Now_peer &other) const;

  WifiEspNowPeerInfo _peerInfo;
  int8_t _rssi = 0;
  uint8_t _distance = 255;
  bool _confirmedESPEasyNowPeer = false;
};
#endif // ifdef USES_ESPEASY_NOW

#endif // DATASTRUCTS_ESPEASY_NOW_PEER_H
