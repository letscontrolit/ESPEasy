#include "ESPEasy_now_handler.h"

#ifdef USES_ESPEASY_NOW

#include "../../ESPEasy_Log.h"
#include "../../ESPEasy_fdwdecl.h"

#include "../Globals/SecuritySettings.h"

bool ESPEasy_now_handler_t::begin()
{
  if (!WifiEspNow.begin()) { return false; }

  for (byte peer = 0; peer < ESPEASY_NOW_PEER_MAX; ++peer) {
    if (SecuritySettings.peerMacSet(peer)) {
      if (!WifiEspNow.addPeer(SecuritySettings.EspEasyNowPeerMAC[peer])) {
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log;
          log.reserve(48);
          log  = F("ESPEasy_Now: Failed to add peer ");
          log += formatMAC(SecuritySettings.EspEasyNowPeerMAC[peer]);
          addLog(LOG_LEVEL_ERROR, log);
        }
      }
    }
  }
  use_EspEasy_now = true;
  return true;
}

void ESPEasy_now_handler_t::end()
{
  use_EspEasy_now = false;
 WifiEspNow.end();
}

#endif // ifdef USES_ESPEASY_NOW
