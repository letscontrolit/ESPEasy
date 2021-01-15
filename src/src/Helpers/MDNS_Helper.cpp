#include "../Helpers/MDNS_Helper.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyEth.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"

#include "../Globals/NetworkState.h"
#include "../Globals/Services.h"
#include "../Globals/Settings.h"

#include "../Helpers/StringProvider.h"

void set_mDNS() {
  #ifdef FEATURE_MDNS

  if (webserverRunning) {
    addLog(LOG_LEVEL_INFO, F("WIFI : Starting mDNS..."));
    bool mdns_started = MDNS.begin(NetworkGetHostname().c_str());
    MDNS.setInstanceName(NetworkGetHostname()); // Needed for when the hostname has changed.

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("WIFI : ");

      if (mdns_started) {
        log += F("mDNS started, with name: ");
        log += getValue(LabelType::M_DNS);
      }
      else {
        log += F("mDNS failed");
      }
      addLog(LOG_LEVEL_INFO, log);
    }

    if (mdns_started) {
      MDNS.addService(F("http"), F("tcp"), Settings.WebserverPort);
    }
  }
  #endif // ifdef FEATURE_MDNS
}
