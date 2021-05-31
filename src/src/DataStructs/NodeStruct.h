#ifndef DATASTRUCTS_NODESTRUCT_H
#define DATASTRUCTS_NODESTRUCT_H

#include "../../ESPEasy_common.h"
#include <map>
#include <IPAddress.h>


#define NODE_TYPE_ID_ESP_EASY_STD           1
#define NODE_TYPE_ID_RPI_EASY_STD           5  // https://github.com/enesbcs/rpieasy
#define NODE_TYPE_ID_ESP_EASYM_STD         17
#define NODE_TYPE_ID_ESP_EASY32_STD        33
#define NODE_TYPE_ID_ARDUINO_EASY_STD      65
#define NODE_TYPE_ID_NANO_EASY_STD         81

const __FlashStringHelper * getNodeTypeDisplayString(byte nodeType);

/*********************************************************************************************\
* NodeStruct
\*********************************************************************************************/
struct NodeStruct
{
  NodeStruct();

  String    nodeName;
  IPAddress ip;
  uint16_t  build;
  byte      age;
  byte      nodeType;
  uint16_t  webgui_portnumber;
};
typedef std::map<byte, NodeStruct> NodesMap;


#endif // DATASTRUCTS_NODESTRUCT_H
