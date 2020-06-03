#include "NodeStruct.h"

#include "../Globals/Settings.h"
#include "../../ESPEasy-Globals.h"
#include "../../ESPEasyTimeTypes.h"
#include "../Globals/SecuritySettings.h"
#include "../Helpers/ESPEasy_time_calc.h"

NodeStruct::NodeStruct() : ESPEasyNowPeer(0), useAP_ESPEasyNow(0), scaled_rssi(0),
    build(0), age(0), nodeType(0), webgui_portnumber(0)
{}

bool NodeStruct::valid() const {
  // FIXME TD-er: Must make some sanity checks to see if it is a valid message
  return true;
}

bool NodeStruct::validate() {
  if (build < 20107) {
    // webserverPort introduced in 20107
    webgui_portnumber = 80;
    load              = 0;
    distance          = 255;
  }

  // FIXME TD-er: Must make some sanity checks to see if it is a valid message
  return valid();
}

bool NodeStruct::operator<(const NodeStruct &other) const {
  if (ESPEasyNowPeer != other.ESPEasyNowPeer) {
    // One is confirmed, so prefer that one.
    return ESPEasyNowPeer;
  }

  const bool markedAsPriority = markedAsPriorityPeer();
  if (markedAsPriority != other.markedAsPriorityPeer()) {
    return markedAsPriority;
  }

  if (distance != other.distance) {
    return distance < other.distance;
  }
  
  const int8_t thisRssi = getRSSI();
  const int8_t otherRssi = other.getRSSI();

  if (thisRssi != otherRssi) {
    if (thisRssi >= 0) {
      // This one has no set RSSI, so the other one is better
      return false;
    }

    if (otherRssi >= 0) {
      // This other has no set RSSI, so the this one is better
      return true;
    }
    return thisRssi > otherRssi;
  }
  return true;
}

String NodeStruct::getNodeTypeDisplayString() const {
  switch (nodeType)
  {
    case NODE_TYPE_ID_ESP_EASY_STD:     return F("ESP Easy");
    case NODE_TYPE_ID_ESP_EASYM_STD:    return F("ESP Easy Mega");
    case NODE_TYPE_ID_ESP_EASY32_STD:   return F("ESP Easy 32");
    case NODE_TYPE_ID_RPI_EASY_STD:     return F("RPI Easy");
    case NODE_TYPE_ID_ARDUINO_EASY_STD: return F("Arduino Easy");
    case NODE_TYPE_ID_NANO_EASY_STD:    return F("Nano Easy");
  }
  return "";
}

String NodeStruct::getNodeName() const {
  String res;
  size_t length = strnlen(reinterpret_cast<const char *>(nodeName), sizeof(nodeName));

  res.reserve(length);

  for (size_t i = 0; i < length; ++i) {
    res += static_cast<char>(nodeName[i]);
  }
  return res;
}

IPAddress NodeStruct::IP() const {
  return IPAddress(ip[0], ip[1], ip[2], ip[3]);
}

MAC_address NodeStruct::STA_MAC() const {
  return MAC_address(sta_mac);
}

MAC_address NodeStruct::ESPEasy_Now_MAC() const {
  if (useAP_ESPEasyNow) {
    return MAC_address(ap_mac);
  }
  return MAC_address(sta_mac);
}

unsigned long NodeStruct::getAge() const {
  return timePassedSince(lastUpdated);
}

float NodeStruct::getLoad() const {
  return load / 2.55;
}

String NodeStruct::getSummary() const {
  String res;

  res.reserve(48);
  res  = F("Unit: ");
  res += unit;
  res += F(" \"");
  res += getNodeName();
  res += '"';
  res += F(" load: ");
  res += String(getLoad(), 1);
  res += F(" RSSI: ");
  res += getRSSI();
  res += F(" ch: ");
  res += channel;
  res += F(" dst: ");
  res += distance;
  return res;
}

bool NodeStruct::setESPEasyNow_mac(const MAC_address& received_mac)
{
  if (received_mac == sta_mac) {
    ESPEasyNowPeer   = 1;
    useAP_ESPEasyNow = 0;
    return true;
  }

  if (received_mac == ap_mac) {
    ESPEasyNowPeer   = 1;
    useAP_ESPEasyNow = 1;
    return true;
  }
  return false;
}

int8_t NodeStruct::getRSSI() const
{
  if (scaled_rssi == 0) {
    return 0; // Not set
  }

  if (scaled_rssi == 0x3F) {
    return 31; // Error state
  }

  // scaled_rssi = 1 ... 62
  // output = -38 ... -99
  int8_t rssi = scaled_rssi + 37;
  return rssi * -1;
}

void NodeStruct::setRSSI(int8_t rssi)
{
  if (rssi == 0) {
    // Not set
    scaled_rssi = 0;
    return;
  }

  if (rssi > 0) {
    // Error state
    scaled_rssi = 0x3F;
    return;
  }
  rssi *= -1;
  rssi -= 37;

  if (rssi < 1) {
    scaled_rssi = 1;
    return;
  }

  if (rssi >= 0x3F) {
    scaled_rssi = 0x3F - 1;
    return;
  }
  scaled_rssi = rssi;
}

bool NodeStruct::markedAsPriorityPeer() const
{
  for (int i = 0; i < ESPEASY_NOW_PEER_MAX; ++i) {
    if (SecuritySettings.peerMacSet(i)) {
      if (match(SecuritySettings.EspEasyNowPeerMAC[i])) {
        return true;
      }
    }
  }
  return false;
}

bool NodeStruct::match(const MAC_address& mac) const
{
  return (mac == sta_mac || mac == ap_mac);
}