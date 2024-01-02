#ifndef DATASTRUCTS_NODESTRUCT_H
#define DATASTRUCTS_NODESTRUCT_H

#include "../../ESPEasy_common.h"

#if FEATURE_ESPEASY_P2P
#include "../Helpers/ESPEasy_time.h"
#include "../DataStructs/MAC_address.h"

#include <IPAddress.h>
#include <map>


/*********************************************************************************************\
* NodeStruct
\*********************************************************************************************/
struct __attribute__((__packed__)) NodeStruct
{
  NodeStruct();

  bool          valid() const;
  bool          validate(const IPAddress& remoteIP);

  // Compare nodes.
  // Return true when this node has better credentials to be used as ESPEasy-NOW neighbor
  // - Shorter distance to a network connected gateway node.
  // - confirmed ESPEasy-NOW peer
  // - better RSSI
  // - lower load (TODO TD-er)
  bool operator<(const NodeStruct &other) const;

  const __FlashStringHelper * getNodeTypeDisplayString() const;

  String        getNodeName() const;

  IPAddress     IP() const;

  #if FEATURE_USE_IPV6
  IPAddress     IPv6_link_local(bool stripZone = false) const;
  IPAddress     IPv6_global() const;

  bool          hasIPv6() const;
  #endif

  MAC_address   STA_MAC() const;

  MAC_address   ESPEasy_Now_MAC() const;

  unsigned long getAge() const;

  bool          isExpired() const;

  float         getLoad() const;

  String        getSummary() const;

  bool          setESPEasyNow_mac(const MAC_address& received_mac);

  int8_t        getRSSI() const;

  void          setRSSI(int8_t rssi);

  bool          markedAsPriorityPeer() const;

  bool          match(const MAC_address& mac) const;

  bool          isThisNode() const;

  void          setAP_MAC(const MAC_address& mac);


  // Do not change the order of this data, as it is being sent via P2P UDP.
  // 6 byte mac  (STA or ETH interface)
  // 4 byte ip
  // 1 byte unit
  // 2 byte build
  // 25 char name
  // 1 byte node type id

  // Added starting build '20107':
  // 2 bytes webserver port
  // 6 bytes AP MAC
  // 1 byte system load
  // 1 byte administrative distance


  uint8_t  sta_mac[6]        = { 0 }; // STA mode MAC (or MAC from ETH device)
  uint8_t  ip[4]             = { 0 };
  uint8_t  unit              = 0;
  uint16_t build             = 0;
  uint8_t  nodeName[25]      = { 0 };
  uint8_t  nodeType          = 0;
  uint16_t webgui_portnumber = 80;
  uint8_t  ap_mac[6]         = { 0 }; // AP mode MAC
  uint8_t  load              = 127;   // Default to average load
  uint8_t  distance          = 255;   // Administrative distance for routing
  uint8_t  timeSource        = static_cast<uint8_t>(timeSource_t::No_time_source);
  uint8_t  channel           = 0;     // The WiFi channel used
  uint8_t  ESPEasyNowPeer   : 1;      // Signalling if the node is an ESPEasy-NOW peer
  uint8_t  useAP_ESPEasyNow : 1;      // ESPEasy-NOW can either use STA or AP for communications.
  uint8_t  scaled_rssi      : 6;      // "shortened" RSSI value

  // When sending system info, this value contains the time since last time sync.
  // When kept as node info, this is the last time stamp the node info was updated.
  unsigned long lastUpdated = (1 << 30);
  uint8_t       version     = 1;
  #if FEATURE_USE_IPV6
  uint8_t hasIPv4                       : 1;
  // Whether the IPv6 address can be derived from the given sta_mac member
  uint8_t hasIPv6_mac_based_link_local  : 1;
  uint8_t hasIPv6_mac_based_link_global : 1;

  uint8_t unused : 5;
  #else
  uint8_t unused = 0;
  #endif
  uint32_t unix_time_sec  = 0;
  uint32_t unix_time_frac = 0;
};
typedef std::map<uint8_t, NodeStruct> NodesMap;
#endif // if FEATURE_ESPEASY_P2P
#endif // DATASTRUCTS_NODESTRUCT_H
