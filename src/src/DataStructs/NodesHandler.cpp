#include "NodesHandler.h"

#include "../../ESPEasy-Globals.h"
#include "../../ESPEasyWifi.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/PeriodicalActions.h"
#include "../Globals/MQTT.h"

void NodesHandler::addNode(const NodeStruct& node)
{
  int8_t rssi = 0;

  // Erase any existing node with matching MAC address
  for (auto it = _nodes.begin(); it != _nodes.end(); )
  {
    if (node.match(it->second.sta_mac) || node.match(it->second.ap_mac)) {
      rssi = it->second.getRSSI();
      it = _nodes.erase(it);
    } else {
      ++it;
    }
  }
  _nodes[node.unit]             = node;
  _nodes[node.unit].lastUpdated = millis();
  if (node.getRSSI() >= 0) {
    _nodes[node.unit].setRSSI(rssi);
  }
}

bool NodesHandler::hasNode(uint8_t unit_nr) const
{
  return _nodes.find(unit_nr) != _nodes.end();
}

bool NodesHandler::hasNode(const uint8_t *mac) const
{
  return getNodeByMac(mac) != nullptr;
}

const NodeStruct * NodesHandler::getNode(uint8_t unit_nr) const
{
  auto it = _nodes.find(unit_nr);

  if (it == _nodes.end()) {
    return nullptr;
  }
  return &(it->second);
}

NodeStruct * NodesHandler::getNodeByMac(const MAC_address& mac)
{
  if (mac.all_zero()) {
    return nullptr;
  }
  delay(0);

  for (auto it = _nodes.begin(); it != _nodes.end(); ++it)
  {
    if (mac == it->second.sta_mac) {
      return &(it->second);
    }

    if (mac == it->second.ap_mac) {
      return &(it->second);
    }
  }
  return nullptr;
}

const NodeStruct * NodesHandler::getNodeByMac(const MAC_address& mac) const
{
  bool match_STA;

  return getNodeByMac(mac, match_STA);
}

const NodeStruct * NodesHandler::getNodeByMac(const MAC_address& mac, bool& match_STA) const
{
  if (mac.all_zero()) {
    return nullptr;
  }
  delay(0);

  for (auto it = _nodes.begin(); it != _nodes.end(); ++it)
  {
    if (mac == it->second.sta_mac) {
      match_STA = true;
      return &(it->second);
    }

    if (mac == it->second.ap_mac) {
      match_STA = false;
      return &(it->second);
    }
  }
  return nullptr;
}

const NodeStruct * NodesHandler::getPreferredNode() const {
  MAC_address dummy;

  return getPreferredNode_notMatching(dummy);
}

const NodeStruct * NodesHandler::getPreferredNode_notMatching(const MAC_address& not_matching) const {
  MAC_address this_mac;

  WiFi.macAddress(this_mac.mac);
  const NodeStruct *thisNode = getNodeByMac(this_mac);
  const NodeStruct *reject   = getNodeByMac(not_matching);

  const NodeStruct *res = nullptr;

  for (auto it = _nodes.begin(); it != _nodes.end(); ++it)
  {
    if ((&(it->second) != reject) && (&(it->second) != thisNode)) {
      if (res == nullptr) {
        res = &(it->second);
      } else {
        if (it->second < *res) {
          res = &(it->second);
        }
      }
    }
  }
  return res;
}

void NodesHandler::updateThisNode() {
  NodeStruct thisNode;

  // Set local data
  WiFi.macAddress(thisNode.sta_mac);
  WiFi.softAPmacAddress(thisNode.ap_mac);
  {
    IPAddress localIP = WiFi.localIP();

    for (byte i = 0; i < 4; ++i) {
      thisNode.ip[i] = localIP[i];
    }
  }
  thisNode.channel = WiFi.channel();

  thisNode.unit  = Settings.Unit;
  thisNode.build = Settings.Build;
  memcpy(thisNode.nodeName, Settings.Name, 25);
  thisNode.nodeType = NODE_TYPE_ID;

  thisNode.webgui_portnumber = Settings.WebserverPort;
  int load_int = getCPUload() * 2.55;

  if (load_int > 255) {
    thisNode.load = 255;
  } else {
    thisNode.load = load_int;
  }
  thisNode.timeSource = static_cast<uint8_t>(node_time.timeSource);

  switch (node_time.timeSource) {
    case timeSource_t::No_time_source:
      thisNode.lastUpdated = (1 << 30);
      break;
    default:
    {
      thisNode.lastUpdated = timePassedSince(node_time.lastSyncTime);
      break;
    }
  }

  if (isEndpoint()) {
    _distance = 0;
  } else {
    _distance = 255;
    const NodeStruct *preferred = getPreferredNode_notMatching(thisNode.sta_mac);

    if (preferred != nullptr) {
      if (preferred->distance < 255 && !preferred->isExpired()) {
        _distance = preferred->distance + 1;
        _lastTimeValidDistance = millis();
      }
    }
  }
  thisNode.distance = _distance;
  addNode(thisNode);
}

const NodeStruct * NodesHandler::getThisNode() {
  updateThisNode();
  MAC_address this_mac;
  WiFi.macAddress(this_mac.mac);
  return getNodeByMac(this_mac.mac);
}

NodesMap::const_iterator NodesHandler::begin() const {
  return _nodes.begin();
}

NodesMap::const_iterator NodesHandler::end() const {
  return _nodes.end();
}

NodesMap::const_iterator NodesHandler::find(uint8_t unit_nr) const
{
  return _nodes.find(unit_nr);
}

bool NodesHandler::refreshNodeList(unsigned long max_age_allowed, unsigned long& max_age)
{
  max_age = 0;
  bool nodeRemoved = false;

  for (auto it = _nodes.begin(); it != _nodes.end();) {
    unsigned long age = it->second.getAge();

    if (age > max_age_allowed) {
      it          = _nodes.erase(it);
      nodeRemoved = true;
    } else {
      ++it;

      if (age > max_age) {
        max_age = age;
      }
    }
  }
  return nodeRemoved;
}

// FIXME TD-er: should be a check per controller to see if it will accept messages
bool NodesHandler::isEndpoint() const
{
  // FIXME TD-er: Must check controller to see if it needs wifi (e.g. LoRa or cache controller do not need it)
  #ifdef USES_MQTT
  controllerIndex_t enabledMqttController = firstEnabledMQTT_ControllerIndex();
  if (validControllerIndex(enabledMqttController)) {
    // FIXME TD-er: Must call updateMQTTclient_connected() and see what effect
    // the MQTTclient_connected state has when using ESPEasy-now.
    return MQTTclient.connected();
  }
  #endif

  if (!WiFiConnected()) return false;

  return false;
}

bool NodesHandler::lastTimeValidDistanceExpired() const
{
//  if (_lastTimeValidDistance == 0) return false;
  return timePassedSince(_lastTimeValidDistance) > 120000; // 2 minutes
}
