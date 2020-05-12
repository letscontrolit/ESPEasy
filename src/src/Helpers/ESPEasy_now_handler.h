#ifndef HELPERS_ESPEASY_NOW_HANDLER_H
#define HELPERS_ESPEASY_NOW_HANDLER_H

#include "../Globals/ESPEasy_now_state.h"

#ifdef USES_ESPEASY_NOW

# include "../DataStructs/ESPEasy_now_hdr.h"
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
  void sendDiscoveryAnnounce(byte channel);

  bool sendToMQTT(controllerIndex_t controllerIndex,
                  const String    & topic,
                  const String    & payload);


private:

  peerInfoMap_t _peerInfoMap;
  
};


#endif // ifdef USES_ESPEASY_NOW

#endif // HELPERS_ESPEASY_NOW_HANDLER_H
