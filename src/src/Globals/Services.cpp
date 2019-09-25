#include "../Globals/Services.h"

#ifdef FEATURE_ARDUINO_OTA
  bool ArduinoOTAtriggered=false;
#endif


#ifdef ESP8266  
  ESP8266WebServer WebServer(80);
  ESP8266HTTPUpdateServer httpUpdater(true);
#endif

/*
#ifdef ESP32
  WebServer WebServer(80);
#endif
*/