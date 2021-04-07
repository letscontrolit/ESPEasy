#include "../Helpers/ESPEasy_now_peermanager.h"

#ifdef USES_ESPEASY_NOW

# include "../ESPEasyCore/ESPEasyWifi.h"
# include "../Globals/Nodes.h"


ESPEasy_now_peermanager_t::ESPEasy_now_peermanager_t() {
  removeAllPeers();
}

MAC_address ESPEasy_now_peermanager_t::getBroadcastMAC()
{
  static MAC_address ESPEasy_now_broadcast_MAC;

  if (ESPEasy_now_broadcast_MAC.mac[0] != 0xFF) {
    for (int i = 0; i < 6; ++i) {
      ESPEasy_now_broadcast_MAC.mac[i] = 0xFF;
    }
  }
  return ESPEasy_now_broadcast_MAC;
}

bool ESPEasy_now_peermanager_t::addPeer(const MAC_address& mac, int channel, const uint8_t key[WIFIESPNOW_KEYLEN])
{
  bool res = true;

  if (mac != getBroadcastMAC()) {
    const NodeStruct *nodeInfo = Nodes.getNodeByMac(mac);

    if (nodeInfo != nullptr) {
      if (channel == 0) {
        channel = nodeInfo->channel;
      }
    }

    if (!WifiEspNow.hasPeer(mac.mac)) {
      res = WifiEspNow.addPeer(mac.mac, channel);

      if (!res && (activePeers.size() != 0)) {
        WifiEspNow.removePeer(activePeers.front().mac);
        activePeers.pop_front();
        res = WifiEspNow.addPeer(mac.mac, channel, key);
      }

      if (res) {
        activePeers.push_back(mac);
      }
    } else {
      // Move the MAC address to the back of the list as it is actively used
      if (activePeers.back() != mac) {
        auto it    = activePeers.begin();
        bool found = false;

        while (it != activePeers.end() && !found) {
          if (*it == mac) {
            found = true;
            activePeers.erase(it);
          }
          ++it;
        }
        activePeers.push_back(mac);
      }
    }
  }
  return res;
}

void ESPEasy_now_peermanager_t::removeAllPeers() {
  const int MAX_PEERS = 20;
  WifiEspNowPeerInfo oldPeers[MAX_PEERS];
  int nOldPeers = std::min(WifiEspNow.listPeers(oldPeers, MAX_PEERS), MAX_PEERS);

  for (int i = 0; i < nOldPeers; ++i) {
    WifiEspNow.removePeer(oldPeers[i].mac);
  }
  activePeers.clear();
}

#endif // ifdef USES_ESPEASY_NOW
