#include "../Helpers/MDNS_Helper.h"

#if FEATURE_MDNS

# include "../ESPEasyCore/ESPEasy_Log.h"
# include "../../ESPEasy/net/eth/ESPEasyEth.h"
# include "../../ESPEasy/net/ESPEasyNetwork.h"

# include "../../ESPEasy/net/Globals/ESPEasyWiFiEvent.h"
# include "../../ESPEasy/net/Globals/NetworkState.h"
# include "../Globals/Services.h"
# include "../Globals/Settings.h"

# include "../Helpers/StringProvider.h"
#endif // if FEATURE_MDNS


/*
Resources for testing DNS Service Discovery:
- Windows: https://www.ibm.com/docs/en/snips/4.6.0?topic=uzcn-using-bonjour-from-windows-command-line-discover-services


*/

  #if FEATURE_MDNS
# ifdef ESP8266
    #  define UPDATE_MDNS_RUNNING mDNS_init = MDNS.isRunning()
    #  define MDNS_RUNNING  MDNS.isRunning()
  static esp8266::MDNSImplementation::MDNSResponder::hMDNSService service_http{};
#  ifndef LIMIT_BUILD_SIZE
  static esp8266::MDNSImplementation::MDNSResponder::hMDNSService service_devinfo{};
#  endif
# else // ifdef ESP8266
    #  define UPDATE_MDNS_RUNNING mDNS_init = false
    #  define MDNS_RUNNING  mDNS_init
# endif // ifdef ESP8266
#endif


void set_mDNS() {
  #if FEATURE_MDNS

  if (ESPEasy::net::NetworkConnected(true)) { update_mDNS(); }
  #endif // if FEATURE_MDNS
}

void end_mDNS() {
#if FEATURE_MDNS
  if (MDNS_RUNNING) {
# ifdef ESP8266

    if (service_http) { 
      MDNS.removeService(service_http); 
      service_http = nullptr;
    }
#  ifndef LIMIT_BUILD_SIZE

    if (service_devinfo) { 
      MDNS.removeService(service_devinfo); 
      service_devinfo = nullptr;
    }
#  endif // ifndef LIMIT_BUILD_SIZE
    MDNS.announce();
# endif // ifdef ESP8266
    MDNS.end();
    UPDATE_MDNS_RUNNING;
  }
#endif
}

void update_mDNS() {
  #if FEATURE_MDNS

  end_mDNS();

  if (webserverRunning) {
    if (!MDNS_RUNNING) {
      addLog(LOG_LEVEL_INFO, F("mDNS : Starting mDNS..."));
      const String hostname = ESPEasy::net::NetworkGetHostname();
      mDNS_init = MDNS.begin(hostname.c_str());
      MDNS.setInstanceName(hostname); // Needed for when the hostname has changed.

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
        // Adding Service TXT fields differs between ESP8266 and ESP32
# ifdef ESP8266
        service_http =
          MDNS.addService(nullptr, FsP("http"), FsP("tcp"), Settings.WebserverPort);
# else // ifdef ESP8266
        MDNS.addService(F("http"), F("tcp"), Settings.WebserverPort);
# endif // ifdef ESP8266

# ifndef LIMIT_BUILD_SIZE
#  ifdef ESP8266
        service_devinfo =
          MDNS.addService(nullptr, FsP("device-info"), FsP("tcp"), Settings.UDPPort);

        //        if (service_devinfo) {
        MDNS.addServiceTxt(service_devinfo, "fn", hostname.c_str());
        MDNS.addServiceTxt(service_devinfo, "md", FsP(get_binary_filename()));

        //        }
#  endif // ifdef ESP8266
#  ifdef ESP32
        MDNS.addService(F("device-info"), F("tcp"), Settings.UDPPort);
        MDNS.addServiceTxt(F("device-info"), F("tcp"), F("fn"), hostname.c_str());
        MDNS.addServiceTxt(F("device-info"), F("tcp"), F("md"), FsP(get_binary_filename()));
#  endif // ifdef ESP32
# endif // ifndef LIMIT_BUILD_SIZE
      }
    }
  } else {
    # ifdef ESP8266

    if (MDNS_RUNNING) {
      MDNS.close();
    }
    UPDATE_MDNS_RUNNING;
    # endif // ifdef ESP8266
  }
  #endif // if FEATURE_MDNS
}
