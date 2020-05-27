#include "ESPEasy_Now_peer.h"

#ifdef USES_ESPEASY_NOW

ESPEasy_Now_peer::ESPEasy_Now_peer()
{
  _peerInfo.channel = 0;
}

ESPEasy_Now_peer::ESPEasy_Now_peer(uint8_t mac[6], uint8_t channel, int8_t rssi)
  : _rssi(rssi)
{
  setMac(mac);
  _peerInfo.channel = channel;
}

void ESPEasy_Now_peer::setMac(uint8_t mac[6])
{
  for (byte i = 0; i < 6; ++i) {
    _peerInfo.mac[i] = mac[i];
  }
}

bool ESPEasy_Now_peer::equalMac(const ESPEasy_Now_peer& other) const
{
  for (byte i = 0; i < 6; ++i) {
    if (_peerInfo.mac[i] != other._peerInfo.mac[i]) {
      return false;
    }
  }
  return true;
}

bool ESPEasy_Now_peer::update(const ESPEasy_Now_peer& other)
{
  if (equalMac(other)) {
    if (other._rssi < 0) {
      _rssi = other._rssi;
    }

    if (other._distance != 255) {
      _distance = other._distance;
    }
    _peerInfo.channel = other._peerInfo.channel;

    if (other._confirmedESPEasyNowPeer) {
      _confirmedESPEasyNowPeer = true;
    }
    return true;
  }
  return false;
}

bool ESPEasy_Now_peer::operator<(const ESPEasy_Now_peer& other) const
{
  if (_distance != other._distance) {
    return _distance < other._distance;
  }

  if (_confirmedESPEasyNowPeer != other._confirmedESPEasyNowPeer) {
    // One is confirmed, so prefer that one.
    return _confirmedESPEasyNowPeer;
  }

  if (_rssi != other._rssi) {
    if (_rssi >= 0) {
      // This one has no set RSSI, so the other one is better
      return false;
    }

    if (other._rssi >= 0) {
      // This other has no set RSSI, so the this one is better
      return true;
    }
    return _rssi > other._rssi;
  }
  return true;
}

bool ESPEasy_Now_peer::operator==(const ESPEasy_Now_peer& other) const
{
  return equalMac(other);
}

#endif // ifdef USES_ESPEASY_NOW
