#include "../DataStructs/RTCStruct.h"

  void RTCStruct::init() {
    ID1 = 0xAA;
    ID2 = 0x55;
    clearLastWiFi();
    factoryResetCounter = 0;
    deepSleepState = 0;
    bootFailedCount = 0;
    flashDayCounter = 0;
    lastWiFiSettingsIndex = 0;
    flashCounter = 0;
    bootCounter = 0;
    lastMixedSchedulerId = 0;
    unused1 = 0;
    unused2 = 0;
    lastSysTime = 0;
  }

  void RTCStruct::clearLastWiFi() {
    for (byte i = 0; i < 6; ++i) {
      lastBSSID[i] = 0;
    }
    lastWiFiChannel = 0;
    lastWiFiSettingsIndex = 0;
  }

  bool RTCStruct::lastWiFi_set() const {
    return lastBSSID[0] != 0 && lastWiFiChannel != 0 && lastWiFiSettingsIndex != 0;
  }
