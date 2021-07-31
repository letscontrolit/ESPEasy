#include "../Globals/Services.h"

#ifdef FEATURE_ARDUINO_OTA
  bool ArduinoOTAtriggered=false;
#endif


#ifdef ESP8266  
  ESP8266WebServer web_server(80);
  #ifndef NO_HTTP_UPDATER
  ESP8266HTTPUpdateServer httpUpdater(true);
  #endif
#endif

#ifdef ESP32
  #include <WiFi.h>
  #include <WebServer.h>
  WebServer web_server(80);
  #ifndef NO_HTTP_UPDATER
  ESP32HTTPUpdateServer httpUpdater(true);
  #endif
#endif


#ifdef FEATURE_DNS_SERVER
  #include <DNSServer.h>
  DNSServer  dnsServer;
  bool dnsServerActive = false;
#endif
