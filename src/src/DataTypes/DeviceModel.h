#ifndef DATASTRUCTS_DEVICEMODEL_H
#define DATASTRUCTS_DEVICEMODEL_H

/********************************************************************************************\
  Pre defined settings for off-the-shelf hardware
  \*********************************************************************************************/

#include <Arduino.h>

// This enum will be stored, so do not change order or at least the values.
enum class DeviceModel : uint8_t {
  DeviceModel_default = 0,
  DeviceModel_Sonoff_Basic,
  DeviceModel_Sonoff_TH1x,
  DeviceModel_Sonoff_S2x,
  DeviceModel_Sonoff_TouchT1,
  DeviceModel_Sonoff_TouchT2,
  DeviceModel_Sonoff_TouchT3,
  DeviceModel_Sonoff_4ch,
  DeviceModel_Sonoff_POW,
  DeviceModel_Sonoff_POWr2,
  DeviceModel_Shelly1,
  DeviceModel_ShellyPLUG_S,
  DeviceModel_Olimex_ESP32_PoE,
  DeviceModel_Olimex_ESP32_EVB,
  DeviceModel_Olimex_ESP32_GATEWAY,
  DeviceModel_wESP32,
  DeviceModel_WT32_ETH01,
  

  DeviceModel_MAX
};


#endif // DATASTRUCTS_DEVICEMODEL_H