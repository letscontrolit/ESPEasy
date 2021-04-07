#ifndef HELPERS_ESPEASY_NOW_PEERMANAGER_H
#define HELPERS_ESPEASY_NOW_PEERMANAGER_H

#include "../../ESPEasy_common.h"
#include "../Globals/ESPEasy_now_state.h"
#ifdef USES_ESPEASY_NOW
# include "../DataStructs/MAC_address.h"

# include <list>

struct ESPEasy_now_peermanager_t {
  ESPEasy_now_peermanager_t();

  static MAC_address getBroadcastMAC();

  bool               addPeer(const MAC_address& mac,
                                 int                channel,
                                 const uint8_t key[WIFIESPNOW_KEYLEN] = nullptr);

  void               removeAllPeers();

private:

  // Keep track of active peers so we can remove the oldest when the max. nr. of peers is reached.
  std::list<MAC_address> activePeers;
};
#endif // ifdef USES_ESPEASY_NOW
#endif // ifndef HELPERS_ESPEASY_NOW_PEERMANAGER_H
