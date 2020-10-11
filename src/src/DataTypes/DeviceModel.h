#ifndef DATASTRUCTS_DEVICEMODEL_H
#define DATASTRUCTS_DEVICEMODEL_H

/********************************************************************************************\
  Pre defined settings for off-the-shelf hardware
  \*********************************************************************************************/

// This enum will be stored, so do not change order or at least the values.
enum DeviceModel {
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
  DeviceMode_Olimex_ESP32_PoE,
  DeviceMode_Olimex_ESP32_EVB,
  DeviceMode_Olimex_ESP32_GATEWAY,
  

  DeviceModel_MAX
};


#endif // DATASTRUCTS_DEVICEMODEL_H