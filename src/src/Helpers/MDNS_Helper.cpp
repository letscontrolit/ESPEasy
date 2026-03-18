#include "../Helpers/MDNS_Helper.h"

#if FEATURE_MDNS

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../../ESPEasy/net/eth/ESPEasyEth.h"
#include "../../ESPEasy/net/ESPEasyNetwork.h"

#include "../../ESPEasy/net/Globals/ESPEasyWiFiEvent.h"
#include "../../ESPEasy/net/Globals/NetworkState.h"
#include "../Globals/Services.h"
#include "../Globals/Settings.h"

#include "../Helpers/StringProvider.h"
#endif

void set_mDNS() {
  #if FEATURE_MDNS
  if (ESPEasy::net::NetworkConnected(true)) update_mDNS();
  #endif // if FEATURE_MDNS
}

void update_mDNS() {
  #if FEATURE_MDNS

#ifdef ESP8266
    #define UPDATE_MDNS_RUNNING mDNS_init = MDNS.isRunning()
    #define MDNS_RUNNING  MDNS.isRunning()
#else
    #define UPDATE_MDNS_RUNNING mDNS_init = false
    #define MDNS_RUNNING  mDNS_init
#endif




  if (MDNS_RUNNING) {
    MDNS.end();
    UPDATE_MDNS_RUNNING;
  }
  if (webserverRunning) {
    if (!MDNS_RUNNING) {
      addLog(LOG_LEVEL_INFO, F("mDNS : Starting mDNS..."));
      mDNS_init = MDNS.begin(ESPEasy::net::NetworkGetHostname().c_str());
      MDNS.setInstanceName(ESPEasy::net::NetworkGetHostname()); // Needed for when the hostname has changed.

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("mDNS : ");

        if (mDNS_init) {
          log += F("Started, with name: ");
          log += getValue(LabelType::M_DNS);
        }
        else {
          log += F("Failed");
        }
        addLogMove(LOG_LEVEL_INFO, log);
      }
      if (mDNS_init) {
        MDNS.addService(F("http"), F("tcp"), Settings.WebserverPort);
        MDNS.addService(F("espeasyp2p"), F("udp"), Settings.UDPPort);
      }
    }
  } else {
    #ifdef ESP8266
    if (MDNS_RUNNING) {
      MDNS.close();
    }
    UPDATE_MDNS_RUNNING;
    #endif
  }
  #endif
}