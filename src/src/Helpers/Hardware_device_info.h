#ifndef HELPERS_HARDWARE_DEVICE_INFO_H
#define HELPERS_HARDWARE_DEVICE_INFO_H

#include "../../ESPEasy_common.h"



/********************************************************************************************\
   Hardware information
 \*********************************************************************************************/
#ifdef ESP8266
enum class ESP8266_partition_type {
  sketch,
  ota,
  fs,
  eeprom,
  rf_cal,
  wifi
};

// Get info on the partition type
// @retval The flash sector. (negative on unknown ptype)
int32_t getPartitionInfo(ESP8266_partition_type ptype, uint32_t& address, int32_t& size);


#endif

// Arduino has a different macro isFlashInterfacePin for ESP8266 as ESP8285
// This has been fixed in SDK3.x, but since we still need to support SDK 2.7.x,
// had to rename the function to isFlashInterfacePin_ESPEasy
bool isFlashInterfacePin_ESPEasy(int gpio);

uint32_t                   getFlashChipId();

uint32_t                   getFlashRealSizeInBytes();

uint32_t                   getFlashChipSpeed();

#ifdef ESP32
uint32_t                   getXtalFrequencyMHz();

struct esp32_chip_features {
  bool embeddedFlash{};
  bool wifi_bgn{};
  bool bluetooth_ble{};
  bool bluetooth_classic{};
  bool ieee_802_15_4{};
  bool embeddedPSRAM{};
};

esp32_chip_features        getChipFeatures();
String                     getChipFeaturesString();

bool                      flashVddPinCanBeUsedAsGPIO();

int32_t                   getEmbeddedFlashSize();
int32_t                   getEmbeddedPSRAMSize();

// @retval true:   octal (8 data lines)
// @retval false:  quad (4 data lines)
bool                       getFlashChipOPI_wired();
#endif // ifdef ESP32

const __FlashStringHelper* getFlashChipMode();

bool                       puyaSupport();

uint8_t                    getFlashChipVendorId();

bool                       flashChipVendorPuya();

// Last 24 bit of MAC address as integer, to be used in rules.
uint32_t                   getChipId();

uint8_t                    getChipCores();

const __FlashStringHelper* getChipModel();

#ifdef ESP32
const __FlashStringHelper* getChipModel(
  uint32_t chip_model, 
  uint32_t chip_revision, 
  uint32_t pkg_version, 
  bool single_core);
#endif


bool                       isESP8285(uint32_t& pkg_version, bool& high_temp_version);
bool                       isESP8285();

String                     getChipRevision();

uint32_t                   getSketchSize();

uint32_t                   getFreeSketchSpace();


/********************************************************************************************\
   PSRAM support
 \*********************************************************************************************/
#ifdef ESP32

// this function is a replacement for `psramFound()`.
// `psramFound()` can return true even if no PSRAM is actually installed
// This new version also checks `esp_spiram_is_initialized` to know if the PSRAM is initialized
// Original Tasmota:
// https://github.com/arendst/Tasmota/blob/1e6b78a957be538cf494f0e2dc49060d1cb0fe8b/tasmota/support_esp.ino#L470
bool FoundPSRAM();

// new function to check whether PSRAM is present and supported (i.e. required pacthes are present)
bool UsePSRAM();

/*
 * ESP32 v1 and v2 needs some special patches to use PSRAM.
 * Original function used from Tasmota:
 * https://github.com/arendst/Tasmota/blob/1e6b78a957be538cf494f0e2dc49060d1cb0fe8b/tasmota/support_esp.ino#L762
 *
 * If using ESP32 v1, please add: `-mfix-esp32-psram-cache-issue -lc-psram-workaround -lm-psram-workaround`
 *
 * This function returns true if the chip supports PSRAM natively (v3) or if the
 * patches are present.
 */
bool CanUsePSRAM();

#ifndef isPSRAMInterfacePin
bool isPSRAMInterfacePin(int gpio);
#endif

#endif // ESP32

#endif