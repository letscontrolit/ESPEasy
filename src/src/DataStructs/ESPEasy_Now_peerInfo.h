#ifndef DATASTRUCTS_ESPEASY_NOW_PEERINFOR_H
#define DATASTRUCTS_ESPEASY_NOW_PEERINFOR_H

#include <Arduino.h>

#include <map>
#include <IPAddress.h>

struct ESPEasy_Now_peerInfo_meta {
  String  nodeName;
  IPAddress ip;
  byte unit;
  uint8_t channel  = 0;
  uint8_t distance = 0;
  bool    encrypt  = false;
};

typedef std::map<uint64_t, ESPEasy_Now_peerInfo_meta> peerInfoMap_t;


struct ESPEasy_Now_peerInfo {
  void   addPeer(const uint8_t                   *mac,
                 const ESPEasy_Now_peerInfo_meta& meta);

  bool   hasPeer(const uint8_t *mac) const;

  bool   getPeer(const uint8_t             *mac,
                 ESPEasy_Now_peerInfo_meta& meta) const;

  String formatPeerInfo(const uint8_t *mac) const;

private:

  peerInfoMap_t peer_map;
};


#endif // DATASTRUCTS_ESPEASY_NOW_PEERINFOR_H
