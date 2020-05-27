#include "NodeStruct.h"

#include "../Globals/Settings.h"
#include "../../ESPEasy-Globals.h"
#include "../../ESPEasyTimeTypes.h"

String getNodeTypeDisplayString(byte nodeType) {
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

NodeStruct::NodeStruct() : ESPEasyNowPeer(0), useAP_ESPEasyNow(0), scaled_rssi(0),
    build(0), age(0), nodeType(0), webgui_portnumber(0)
{}

bool NodeStruct::validate() {
  if (build < 20107) {
    // webserverPort introduced in 20107
    webgui_portnumber = 80;
    load              = 0;
    distance          = 255;
  }

  // FIXME TD-er: Must make some sanity checks to see if it is a valid message
  return true;
}

void NodeStruct::setLocalData() {
  WiFi.macAddress(sta_mac);
  WiFi.softAPmacAddress(ap_mac);
  {
    IPAddress localIP = WiFi.localIP();

    for (byte i = 0; i < 4; ++i) {
      ip[i] = localIP[i];
    }
  }
  channel = WiFi.channel();

  unit  = Settings.Unit;
  build = Settings.Build;
  memcpy(nodeName, Settings.Name, 25);
  nodeType = NODE_TYPE_ID;

  webgui_portnumber = Settings.WebserverPort;
  int load_int = getCPUload() * 2.55;

  if (load_int > 255) {
    load = 255;
  } else {
    load = load_int;
  }
  timeSource = static_cast<uint8_t>(node_time.timeSource);

  switch (node_time.timeSource) {
    case timeSource_t::No_time_source:
      lastUpdated = (1 << 30);
      break;
    default:
    {
      lastUpdated = timePassedSince(node_time.lastSyncTime);
      break;
    }
  }
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
