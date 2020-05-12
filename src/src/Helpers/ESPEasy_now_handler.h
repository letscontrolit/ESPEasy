#ifndef HELPERS_ESPEASY_NOW_HANDLER_H
#define HELPERS_ESPEASY_NOW_HANDLER_H

#include "../Globals/ESPEasy_now_state.h"

#ifdef USES_ESPEASY_NOW

# include "../DataStructs/ESPEasy_now_hdr.h"
# include "../DataStructs/ESPEasy_Now_packet.h"
# include "../DataStructs/ESPEasy_Now_peerInfo.h"
# include "../Globals/CPlugins.h"


class ESPEasy_now_handler_t {
public:

  ESPEasy_now_handler_t() {}

  bool begin();

  void end();

  bool loop();

  // Send out the discovery announcement via broadcast.
  // This may be picked up by others
  void sendDiscoveryAnnounce(byte channel = 0);

  bool sendToMQTT(controllerIndex_t controllerIndex,
                  const String    & topic,
                  const String    & payload);

  bool getPeerInfo(const uint8_t             *mac,
                   ESPEasy_Now_peerInfo_meta& meta) const;

private:

  bool                 send(const ESPEasy_Now_packet& packet);
  WifiEspNowSendStatus send(const ESPEasy_Now_packet& packet,
                            size_t                    timeout);

  WifiEspNowSendStatus waitForSendStatus(size_t timeout) const;


  bool                 handle_DiscoveryAnnounce(const ESPEasy_Now_packet& packet);

  bool                 handle_MQTTControllerMessage(const ESPEasy_Now_packet& packet);


  ESPEasy_Now_peerInfo _peerInfoMap;
};


#endif // ifdef USES_ESPEASY_NOW

#endif // HELPERS_ESPEASY_NOW_HANDLER_H
