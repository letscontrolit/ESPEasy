#ifndef HELPERS_ESPEASY_NOW_HANDLER_H
#define HELPERS_ESPEASY_NOW_HANDLER_H

#include "../Globals/ESPEasy_now_state.h"

#ifdef USES_ESPEASY_NOW

# include "../DataStructs/ESPEasy_now_hdr.h"
# include "../DataStructs/ESPEasy_Now_DuplicateCheck.h"
# include "../DataStructs/ESPEasy_Now_packet.h"
# include "../DataStructs/ESPEasy_now_merger.h"
# include "../DataStructs/ESPEasy_Now_NTP_query.h"
# include "../Globals/CPlugins.h"


class ESPEasy_now_handler_t {
public:

  ESPEasy_now_handler_t() {}

  bool begin();

  void end();

  bool loop();

private:

  bool processMessage(const ESPEasy_now_merger& message);

public:

  // Send out the discovery announcement via broadcast.
  // This may be picked up by others
  void sendDiscoveryAnnounce(byte channel = 0);

  void sendNTPquery();

  void sendNTPbroadcast();

  bool sendToMQTT(controllerIndex_t controllerIndex,
                  const String    & topic,
                  const String    & payload);

  void sendSendData_DuplicateCheck(uint32_t                              key,
                                   ESPEasy_Now_DuplicateCheck::message_t message_type,
                                   uint8_t                               mac[6]);

private:

  bool handle_DiscoveryAnnounce(const ESPEasy_now_merger& message);

  bool handle_NTPquery(const ESPEasy_now_merger& message);

  bool handle_MQTTControllerMessage(const ESPEasy_now_merger& message);

  bool handle_SendData_DuplicateCheck(const ESPEasy_now_merger& message);

  ESPEasy_Now_NTP_query _best_NTP_candidate;
};


#endif // ifdef USES_ESPEASY_NOW

#endif // HELPERS_ESPEASY_NOW_HANDLER_H
