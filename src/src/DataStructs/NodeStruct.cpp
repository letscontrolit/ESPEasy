#include "NodeStruct.h"

#include "../Globals/Settings.h"
#include "../../ESPEasy-Globals.h"

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

  NodeStruct::NodeStruct() :
    build(0), age(0), nodeType(0), webgui_portnumber(0)
  {
    for (byte i = 0; i < 4; ++i) { ip[i] = 0; }
  }

bool NodeStruct::validate() {
  if (build < 20107) {
    // webserverPort introduced in 20107
    webserverPort = 80;
    load = 0;
    distance = 255;
  }

  // FIXME TD-er: Must make some sanity checks to see if it is a valid message
  return true;
}

void NodeStruct::setLocalData() {
  WiFi.macAddress(mac);
  WiFi.softAPmacAddress(ap_mac);
  {
    IPAddress localIP = WiFi.localIP();

    for (byte i = 0; i < 4; ++i) {
      ip[i] = localIP[i];
    }
  }

  unit  = Settings.Unit;
  build = Settings.Build;
  memcpy(nodeName, Settings.Name, 25);
  nodeType = NODE_TYPE_ID;

  //  webserverPort = Settings.WebserverPort; // PR #3053
  int load_int = getCPUload() * 2.55;
  if (load_int > 255) {
    load = 255;
  } else {
    load = load_int;
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

unsigned long NodeStruct::getAge() const {
  return timePassedSince(lastSeenTimestamp);
}

float NodeStruct::getLoad() const {
  return load / 2.55;
}

String NodeStruct::getSummary() const {
  String res;
  res.reserve(48);
  res = F("Unit: ");
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
