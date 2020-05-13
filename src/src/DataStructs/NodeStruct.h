#ifndef DATASTRUCTS_NODESTRUCT_H
#define DATASTRUCTS_NODESTRUCT_H

#include "../../ESPEasy_common.h"
#include <map>
#include <IPAddress.h>


#define NODE_TYPE_ID_ESP_EASY_STD           1
#define NODE_TYPE_ID_RPI_EASY_STD           5 // https://github.com/enesbcs/rpieasy
#define NODE_TYPE_ID_ESP_EASYM_STD         17
#define NODE_TYPE_ID_ESP_EASY32_STD        33
#define NODE_TYPE_ID_ARDUINO_EASY_STD      65
#define NODE_TYPE_ID_NANO_EASY_STD         81

String getNodeTypeDisplayString(byte nodeType);

/*********************************************************************************************\
* NodeStruct
\*********************************************************************************************/
struct __attribute__((__packed__)) NodeStruct
{
  NodeStruct();

  void   setLocalData();
  String getNodeTypeDisplayString() const;

  String getNodeName() const;

  IPAddress IP() const;

  unsigned long getAge() const;

  String getSummary() const;




  // Do not change the order of this data, as it is being sent via P2P UDP.
  // 6 byte mac  (STA interface)
  // 4 byte ip
  // 1 byte unit
  // 2 byte build
  // 25 char name
  // 1 byte node type id

  // Added starting build '20107':
  // 2 bytes webserver port
  // 6 bytes AP MAC



  uint8_t   mac[6] = { 0 };  // STA mode MAC
  uint8_t   ip[4] = { 0 };
  byte      unit = 0;
  uint16_t  build        = 0;
  byte      nodeName[25] = { 0 };
  byte      nodeType     = 0;
  uint16_t  webserverPort = 80;
  uint8_t   ap_mac[6] = { 0 };  // AP mode MAC

  // Data not being sent to other nodes.
  unsigned long lastSeenTimestamp  = 0;



};
typedef std::map<byte, NodeStruct> NodesMap;


#endif // DATASTRUCTS_NODESTRUCT_H
