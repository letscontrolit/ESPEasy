#ifndef DATASTRUCTS_NODESTRUCT_H
#define DATASTRUCTS_NODESTRUCT_H

#include "ESPEasy_common.h"
#include <map>
#include <IPAddress.h>

/*********************************************************************************************\
* NodeStruct
\*********************************************************************************************/
struct NodeStruct
{
  NodeStruct() :
    build(0), age(0), nodeType(0)
  {
    for (byte i = 0; i < 4; ++i) { ip[i] = 0; }
  }

  String    nodeName;
  IPAddress ip;
  uint16_t  build;
  byte      age;
  byte      nodeType;
};
typedef std::map<byte, NodeStruct> NodesMap;
NodesMap Nodes;


#endif // DATASTRUCTS_NODESTRUCT_H
