#include "OTA.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/Serial.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/Services.h"
#include "../Globals/Settings.h"
#include "../Helpers/Misc.h"

bool OTA_possible(uint32_t& maxSketchSize, bool& use2step) {
#if defined(ESP8266)

  // Compute the current free space and sketch size, rounded to 4k blocks.
  // These block bounaries are needed for erasing a full block on flash.
  const uint32_t freeSketchSpace            = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
  const uint32_t currentSketchSize          = (ESP.getSketchSize() + 0x1000) & 0xFFFFF000;
  const uint32_t smallestOtaImageSizeNeeded = (((SMALLEST_OTA_IMAGE + 16) + 0x1000) & 0xFFFFF000);
  const bool     otaPossible                = freeSketchSpace >= smallestOtaImageSizeNeeded;
  use2step = freeSketchSpace < currentSketchSize; // Assume the new image has the same size.

  if (use2step) {
    const uint32_t totalSketchSpace = freeSketchSpace + currentSketchSize;
    maxSketchSize = totalSketchSpace - smallestOtaImageSizeNeeded;
  } else {
    maxSketchSize = freeSketchSpace;
  }
  maxSketchSize -= 16; // Must leave 16 bytes at the end.

  if (maxSketchSize > MAX_SKETCH_SIZE) { maxSketchSize = MAX_SKETCH_SIZE; }
  return otaPossible;
#elif defined(ESP32)
  maxSketchSize = MAX_SKETCH_SIZE;
  use2step      = false;
  return true;
#else // if defined(ESP8266)
  return false;
#endif // if defined(ESP8266)
}

#ifdef FEATURE_ARDUINO_OTA

/********************************************************************************************\
   Allow updating via the Arduino OTA-protocol. (this allows you to upload directly from platformio)
 \*********************************************************************************************/
void ArduinoOTAInit()
{
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("ArduinoOTAInit"));
  # endif // ifndef BUILD_NO_RAM_TRACKER

  ArduinoOTA.setPort(ARDUINO_OTA_PORT);
  ArduinoOTA.setHostname(Settings.getHostname().c_str());

  if (SecuritySettings.Password[0] != 0) {
    ArduinoOTA.setPassword(SecuritySettings.Password);
  }

  ArduinoOTA.onStart([]() {
    serialPrintln(F("OTA  : Start upload"));
    ArduinoOTAtriggered = true;
    ESPEASY_FS.end(); // important, otherwise it fails
  });

  ArduinoOTA.onEnd([]() {
    serialPrintln(F("\nOTA  : End"));

    // "dangerous": if you reset during flash you have to reflash via serial
    // so dont touch device until restart is complete
    serialPrintln(F("\nOTA  : DO NOT RESET OR POWER OFF UNTIL BOOT+FLASH IS COMPLETE."));

    // delay(100);
    // reboot(); //Not needed, node reboots automaticall after calling onEnd and succesfully flashing
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    if (Settings.UseSerial) {
      Serial.printf("OTA  : Progress %u%%\r", (progress / (total / 100)));
    }
  });

  ArduinoOTA.onError([](ota_error_t error) {
    serialPrint(F("\nOTA  : Error (will reboot): "));

    if (error == OTA_AUTH_ERROR) { serialPrintln(F("Auth Failed")); }
    else if (error == OTA_BEGIN_ERROR) { serialPrintln(F("Begin Failed")); }
    else if (error == OTA_CONNECT_ERROR) { serialPrintln(F("Connect Failed")); }
    else if (error == OTA_RECEIVE_ERROR) { serialPrintln(F("Receive Failed")); }
    else if (error == OTA_END_ERROR) { serialPrintln(F("End Failed")); }

    delay(100);
    reboot(ESPEasy_Scheduler::IntendedRebootReason_e::OTA_error);
  });

  #if defined(ESP8266) && defined(FEATURE_MDNS)
  ArduinoOTA.begin(true);
  #else
  ArduinoOTA.begin();
  #endif

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("OTA  : Arduino OTA enabled on port ");
    log += ARDUINO_OTA_PORT;
    addLog(LOG_LEVEL_INFO, log);
  }
}

#endif // ifdef FEATURE_ARDUINO_OTA
