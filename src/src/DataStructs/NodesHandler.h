#ifndef DATASTRUCTS_NODESHANDLER_H
#define DATASTRUCTS_NODESHANDLER_H

#include "../../ESPEasy_common.h"
#if FEATURE_ESPEASY_P2P

#include "../DataStructs/MAC_address.h"
#include "../DataStructs/NodeStruct.h"
#include "../DataStructs/NTP_candidate.h"


#ifdef USES_ESPEASY_NOW
# include "../DataStructs/ESPEasy_now_traceroute.h"
# include "../DataStructs/ESPEasy_now_Node_statistics.h"
# include "../DataStructs/ESPEasy_Now_MQTT_queue_check_packet.h"
# include "../DataTypes/ESPEasy_Now_MQTT_queue_check_state.h"
# include "../Globals/ESPEasy_now_peermanager.h"
#endif // ifdef USES_ESPEASY_NOW

#include "../Helpers/ESPEasyMutex.h"


class NodesHandler {
public:

  // Add node to the list of known nodes.
  // @retval true when the node was not yet present in the list.
  bool addNode(const NodeStruct& node);

#ifdef USES_ESPEASY_NOW
  bool addNode(const NodeStruct                   & node,
               const ESPEasy_now_traceroute_struct& traceRoute);
#endif // ifdef USES_ESPEASY_NOW


  bool                     hasNode(uint8_t unit_nr) const;

  bool                     hasNode(const uint8_t *mac) const;

  NodeStruct             * getNode(uint8_t unit_nr);
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
  bool refreshNodeList(unsigned long  max_age_allowed,
                       unsigned long& max_age);


  const NodeStruct                   * getPreferredNode() const;
  const NodeStruct                   * getPreferredNode_notMatching(uint8_t unit_nr) const;
  const NodeStruct                   * getPreferredNode_notMatching(const MAC_address& not_matching) const;

#ifdef USES_ESPEASY_NOW
  const ESPEasy_now_traceroute_struct* getTraceRoute(uint8_t unit) const;
  const ESPEasy_now_traceroute_struct* getDiscoveryRoute(uint8_t unit) const;

  void                                 setTraceRoute(const MAC_address                  & mac,
                                                     const ESPEasy_now_traceroute_struct& traceRoute);
#endif // ifdef USES_ESPEASY_NOW

  // Update the node referring to this unit with the most recent info.
  void              updateThisNode();

  const NodeStruct* getThisNode();

  uint8_t           getDistance() const;

  bool              lastTimeValidDistanceExpired() const;

  unsigned long     get_lastTimeValidDistance() const {
    return _lastTimeValidDistance;
  }

  bool    isEndpoint() const;

#ifdef USES_ESPEASY_NOW
  uint8_t getESPEasyNOW_channel() const;
#endif // ifdef USES_ESPEASY_NOW

  bool    recentlyBecameDistanceZero();

  void    setRSSI(const MAC_address& mac,
                  int                rssi);

  void    setRSSI(uint8_t unit,
                  int     rssi);

#ifdef USES_ESPEASY_NOW
  void                                            updateSuccessRate(uint8_t unit,
                                                                    bool    success);
  void                                            updateSuccessRate(const MAC_address& mac,
                                                                    bool               success);

  int                                             getRouteSuccessRate(uint8_t  unit,
                                                                      uint8_t& distance) const;

  uint8_t                                         getSuccessRate(uint8_t unit) const;

  ESPEasy_Now_MQTT_QueueCheckState::Enum getMQTTQueueState(uint8_t unit) const;

  void                                            setMQTTQueueState(uint8_t                                         unit,
                                                                    ESPEasy_Now_MQTT_QueueCheckState::Enum state);
  void                                            setMQTTQueueState(const MAC_address                             & mac,
                                                                    ESPEasy_Now_MQTT_QueueCheckState::Enum state);

#endif // ifdef USES_ESPEASY_NOW

  bool getUnixTime(double &unix_time, uint8_t& unit) const {
    return _ntp_candidate.getUnixTime(unix_time, unit);
  }


private:

  void setRSSI(NodeStruct *node,
               int         rssi);

  unsigned long _lastTimeValidDistance = 0;

  uint8_t _distance = 255; // Cached value

  NodesMap _nodes;
  ESPEasy_Mutex _nodes_mutex;

  NTP_candidate_struct _ntp_candidate;
  

#ifdef USES_ESPEASY_NOW
  ESPEasy_now_Node_statisticsMap _nodeStats;
  ESPEasy_Mutex _nodeStats_mutex;
#endif // ifdef USES_ESPEASY_NOW

  bool _recentlyBecameDistanceZero = false;
};

#endif

#endif // ifndef DATASTRUCTS_NODESHANDLER_H