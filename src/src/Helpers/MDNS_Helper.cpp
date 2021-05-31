#include "../Helpers/MDNS_Helper.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyEth.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"

#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/NetworkState.h"
#include "../Globals/Services.h"
#include "../Globals/Settings.h"

#include "../Helpers/StringProvider.h"

void set_mDNS() {
  #ifdef FEATURE_MDNS

  if (!WiFiEventData.WiFiServicesInitialized()) { return; }

  if (webserverRunning) {
    if (!mDNS_init) {
      addLog(LOG_LEVEL_INFO, F("WIFI : Starting mDNS..."));
      mDNS_init = MDNS.begin(NetworkGetHostname().c_str());
      MDNS.setInstanceName(NetworkGetHostname()); // Needed for when the hostname has changed.

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("WIFI : ");

        if (mDNS_init) {
          log += F("mDNS started, with name: ");
          log += getValue(LabelType::M_DNS);
        }
        else {
          log += F("mDNS failed");
        }
        addLog(LOG_LEVEL_INFO, log);
      }
      if (mDNS_init) {
        MDNS.addService(F("http"), F("tcp"), Settings.WebserverPort);
      }
    }
  } else {
    #ifdef ESP8266
    if (mDNS_init) {
      MDNS.close();
    }
    mDNS_init = false;
    #endif
  }
  #endif // ifdef FEATURE_MDNS
}
