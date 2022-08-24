#include "../Globals/Services.h"

#if FEATURE_ARDUINO_OTA
  bool ArduinoOTAtriggered = false;
#endif


#ifdef ESP8266  
  ESP8266WebServer web_server(80);
  #ifndef NO_HTTP_UPDATER
  ESP8266HTTPUpdateServer httpUpdater(true);
  #endif
#endif

#ifdef ESP32
  WebServer web_server(80);
  #ifndef NO_HTTP_UPDATER
  ESP32HTTPUpdateServer httpUpdater(true);
  #endif
#endif


#if FEATURE_DNS_SERVER
  DNSServer  dnsServer;
  bool dnsServerActive = false;
#endif // if FEATURE_DNS_SERVER
