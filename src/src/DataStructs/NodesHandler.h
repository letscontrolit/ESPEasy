#ifndef DATASTRUCTS_NODESHANDLER_H
#define DATASTRUCTS_NODESHANDLER_H

#include "NodeStruct.h"

#include "MAC_address.h"

class NodesHandler {
public:

  void                     addNode(const NodeStruct& node);

  bool                     hasNode(uint8_t unit_nr) const;

  bool                     hasNode(const uint8_t *mac) const;

  const NodeStruct       * getNode(uint8_t unit_nr) const;

  NodeStruct             * getNodeByMac(const MAC_address& mac);
  const NodeStruct       * getNodeByMac(const MAC_address& mac) const;
  const NodeStruct       * getNodeByMac(const MAC_address& mac,
                                        bool             & match_STA) const;

  NodesMap::const_iterator begin() const;
  NodesMap::const_iterator end() const;
  NodesMap::const_iterator find(uint8_t unit_nr) const;

  // Remove nodes in list older than max_age_allowed (msec)
  // Returns oldest age, max_age (msec) not removed from the list.
  // Return true if a node has been removed.
  bool              refreshNodeList(unsigned long  max_age_allowed,
                                    unsigned long& max_age);

  
  const NodeStruct* getPreferredNode() const;
  const NodeStruct* getPreferredNode_notMatching(const MAC_address& not_matching) const;

  // Update the node referring to this unit with the most recent info.
  void updateThisNode();

  const NodeStruct * getThisNode();

  uint8_t getDistance() const {
    return _distance;
  }

  bool lastTimeValidDistanceExpired() const;

  unsigned long get_lastTimeValidDistance() const {
    return _lastTimeValidDistance;
  }


private:

  bool isEndpoint() const;

  unsigned long _lastTimeValidDistance = 0;

  uint8_t _distance = 255;  // Cached value

  NodesMap _nodes;
};


#endif // DATASTRUCTS_NODESHANDLER_H
