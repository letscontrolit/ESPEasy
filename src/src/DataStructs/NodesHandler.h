#ifndef DATASTRUCTS_NODESHANDLER_H
#define DATASTRUCTS_NODESHANDLER_H

#include "NodeStruct.h"

class NodesHandler {
public:
  void addNode(const NodeStruct& node);

  bool hasNode(uint8_t unit_nr) const;

  bool hasNode(const uint8_t* mac) const;

  const NodeStruct* getNode(uint8_t unit_nr) const;

  const NodeStruct* getNode(const uint8_t* mac) const;

  NodeStruct* getNode(const uint8_t* mac);

  NodesMap::const_iterator begin() const;

  NodesMap::const_iterator end() const;
  NodesMap::const_iterator find(uint8_t unit_nr) const;

  // Remove nodes in list older than max_age_allowed (msec)
  // Returns oldest age, max_age (msec) not removed from the list.
  // Return true if a node has been removed.
  bool refreshNodeList(unsigned long max_age_allowed, unsigned long& max_age);

private:

  NodesMap _nodes;

};


#endif // DATASTRUCTS_NODESHANDLER_H