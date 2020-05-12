#ifndef DATASTRUCTS_ESPEASY_NOW_PEERINFOR_H
#define DATASTRUCTS_ESPEASY_NOW_PEERINFOR_H

#include <Arduino.h>

#include <map>

struct ESPEasy_Now_peerInfo_meta {
  String  nodeName;
  uint8_t channel = 0;
  uint8_t distance = 0;
  bool    encrypt = false;
};


struct ESPEasy_Now_peerInfo {
  ESPEasy_Now_peerInfo_meta meta;
  uint8_t                   mac[6] = { 0 };
};

typedef std::map<uint8_t[6], ESPEasy_Now_peerInfo_meta> peerInfoMap_t;


#endif // DATASTRUCTS_ESPEASY_NOW_PEERINFOR_H
