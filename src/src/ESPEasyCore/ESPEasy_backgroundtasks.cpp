#include "../ESPEasyCore/ESPEasy_backgroundtasks.h"


#include "../../ESPEasy-Globals.h"
#include "../../ESPEasy_common.h"
#include "../DataStructs/TimingStats.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/Serial.h"
#include "../Globals/NetworkState.h"
#include "../Globals/Services.h"
#include "../Globals/Settings.h"
#include "../Helpers/Network.h"
#include "../Helpers/Networking.h"


/*********************************************************************************************\
* run background tasks
\*********************************************************************************************/
bool runningBackgroundTasks = false;
void backgroundtasks()
{
  // checkRAM(F("backgroundtasks"));
  // always start with a yield
  delay(0);

  /*
     // Remove this watchdog feed for now.
     // See https://github.com/letscontrolit/ESPEasy/issues/1722#issuecomment-419659193

   #ifdef ESP32
     // Have to find a similar function to call ESP32's esp_task_wdt_feed();
   #else
     ESP.wdtFeed();
   #endif
   */

  // prevent recursion!
  if (runningBackgroundTasks)
  {
    return;
  }
  START_TIMER
  const bool networkConnected = NetworkConnected();

  runningBackgroundTasks = true;

  /*
     // Not needed anymore, see: https://arduino-esp8266.readthedocs.io/en/latest/faq/readme.html#how-to-clear-tcp-pcbs-in-time-wait-state
     if (networkConnected) {
   #if defined(ESP8266)
      tcpCleanup();
   #endif
     }
   */

  process_serialWriteBuffer();

  if (!UseRTOSMultitasking) {
    serial();

    if (webserverRunning) {
      web_server.handleClient();
    }

    checkUDP();
  }

  #ifdef FEATURE_DNS_SERVER

  // process DNS, only used if the ESP has no valid WiFi config
  if (dnsServerActive) {
    dnsServer.processNextRequest();
  }
  #endif // ifdef FEATURE_DNS_SERVER

  #ifdef FEATURE_ARDUINO_OTA

  if (Settings.ArduinoOTAEnable) {
    ArduinoOTA.handle();
  }

  // once OTA is triggered, only handle that and dont do other stuff. (otherwise it fails)
  while (ArduinoOTAtriggered)
  {
    delay(0);

    ArduinoOTA.handle();
  }

  #endif // ifdef FEATURE_ARDUINO_OTA

  #ifdef FEATURE_MDNS

  // Allow MDNS processing
  if (networkConnected) {
    # ifdef ESP8266

    // ESP32 does not have an update() function
    MDNS.update();
    # endif // ifdef ESP8266
  }
  #endif // ifdef FEATURE_MDNS

  delay(0);

  statusLED(false);

  runningBackgroundTasks = false;
  STOP_TIMER(BACKGROUND_TASKS);
}
