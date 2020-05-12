#include "NodesHandler.h"


static bool mac_equal(const uint8_t *mac1, const uint8_t *mac2)
{
  for (byte i = 0; i < 6; ++i) {
    if (mac1[i] != mac2[i]) { return false; }
  }
  return true;
}

void NodesHandler::addNode(const NodeStruct& node)
{
  _nodes[node.unit] = node;
  _nodes[node.unit].lastSeenTimestamp = millis();
}

bool NodesHandler::hasNode(uint8_t unit_nr) const
{
  return _nodes.find(unit_nr) != _nodes.end();
}

bool NodesHandler::hasNode(const uint8_t *mac) const
{
  return getNode(mac) != nullptr;
}

const NodeStruct * NodesHandler::getNode(uint8_t unit_nr) const
{
  auto it = _nodes.find(unit_nr);

  if (it == _nodes.end()) {
    return nullptr;
  }
  return &(it->second);
}

const NodeStruct * NodesHandler::getNode(const uint8_t *mac) const
{
  for (auto it = _nodes.begin(); it != _nodes.end(); ++it)
  {
    if (mac_equal(mac, it->second.mac) || mac_equal(mac, it->second.ap_mac)) {
      return &(it->second);
    }
  }
  return nullptr;
}

NodeStruct* NodesHandler::getNode(const uint8_t *mac)
{
  const NodeStruct *node = getNode(mac);

  return const_cast<NodeStruct *>(node);
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
  for (auto it = _nodes.begin(); it != _nodes.end(); ) {
    unsigned long age = it->second.getAge();
    if (age > max_age_allowed) {
      it = _nodes.erase(it);
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