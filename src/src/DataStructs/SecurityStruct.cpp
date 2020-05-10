#include "../DataStructs/SecurityStruct.h"

#include "../../ESPEasy_common.h"
#include "../DataStructs/ESPEasyLimits.h"
#include "../Globals/CPlugins.h"

SecurityStruct::SecurityStruct() {
  ZERO_FILL(WifiSSID);
  ZERO_FILL(WifiKey);
  ZERO_FILL(WifiSSID2);
  ZERO_FILL(WifiKey2);
  ZERO_FILL(WifiAPKey);

  for (controllerIndex_t i = 0; i < CONTROLLER_MAX; ++i) {
    ZERO_FILL(ControllerUser[i]);
    ZERO_FILL(ControllerPassword[i]);
  }
  for (byte i = 0; i < ESPEASY_NOW_PEER_MAX; ++i) {
    ZERO_FILL(EspEasyNowPeerMAC[i]);
  }
  ZERO_FILL(Password);
}

void SecurityStruct::validate() {
  ZERO_TERMINATE(WifiSSID);
  ZERO_TERMINATE(WifiKey);
  ZERO_TERMINATE(WifiSSID2);
  ZERO_TERMINATE(WifiKey2);
  ZERO_TERMINATE(WifiAPKey);

  for (controllerIndex_t i = 0; i < CONTROLLER_MAX; ++i) {
    ZERO_TERMINATE(ControllerUser[i]);
    ZERO_TERMINATE(ControllerPassword[i]);
  }
  ZERO_TERMINATE(Password);
}


bool SecurityStruct::peerMacSet(byte peer_index) const {
  if (peer_index >= ESPEASY_NOW_PEER_MAX) {
    return false;
  }
  for (int i = 0; i < 6; ++i) {
    if (EspEasyNowPeerMAC[peer_index][i] != 0) {
      return true;
    }
  }
  return false;
}