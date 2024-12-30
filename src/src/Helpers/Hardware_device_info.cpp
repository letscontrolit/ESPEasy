#include "../Helpers/Hardware_device_info.h"

#include "../Helpers/Hardware_defines.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/FS_Helper.h"

#ifdef ESP32
  # include <soc/soc.h>
  # include <soc/efuse_reg.h>
  # include <soc/spi_reg.h>
  # include <soc/rtc.h>

  # if ESP_IDF_VERSION_MAJOR == 4
    #  if CONFIG_IDF_TARGET_ESP32S3 // ESP32-S3
      #   include <esp32s3/rom/spi_flash.h>
      #   include <esp32s3/spiram.h>
      #   include <esp32s3/rom/rtc.h>
    #  elif CONFIG_IDF_TARGET_ESP32S2 // ESP32-S2
      #   include <esp32s2/rom/spi_flash.h>
      #   include <esp32s2/spiram.h>
      #   include <esp32s2/rom/rtc.h>
    #  elif CONFIG_IDF_TARGET_ESP32C3 // ESP32-C3
      #   include <esp32c3/rom/spi_flash.h>
      #   include <esp32c3/rom/rtc.h>
    #  elif CONFIG_IDF_TARGET_ESP32   // ESP32/PICO-D4
      #   include <esp32/rom/spi_flash.h>
      #   include <esp32/rom/rtc.h>
      #   include <esp32/spiram.h>
    #  else // if CONFIG_IDF_TARGET_ESP32S3
      #   error Target CONFIG_IDF_TARGET is not supported
    #  endif // if CONFIG_IDF_TARGET_ESP32S3
  # else    // ESP32 IDF 5.x and later
    #  include <rom/spi_flash.h>
    #  include <rom/rtc.h>
    #  include <bootloader_common.h>
  # endif // if ESP_IDF_VERSION_MAJOR == 4


#if CONFIG_IDF_TARGET_ESP32   // ESP32/PICO-D4
  #  if ESP_IDF_VERSION_MAJOR < 5
  #   define HAS_HALL_EFFECT_SENSOR  1
  #  else // if ESP_IDF_VERSION_MAJOR < 5

// Support for Hall Effect sensor was removed in ESP_IDF 5.x
  #   define HAS_HALL_EFFECT_SENSOR  0
  #  endif // if ESP_IDF_VERSION_MAJOR < 5
# else 
  #  define HAS_HALL_EFFECT_SENSOR  0
# endif

# if ESP_IDF_VERSION_MAJOR >= 5

#  include <esp_chip_info.h>
#  include <soc/soc.h>
#  include <driver/ledc.h>
#  include <esp_psram.h>

// #include <hal/ledc_hal.h>

# endif // if ESP_IDF_VERSION_MAJOR >= 5
#endif  // ifdef ESP32

/********************************************************************************************\
   Hardware information
 \*********************************************************************************************/
#ifdef ESP8266
int32_t getPartitionInfo(ESP8266_partition_type ptype, uint32_t& address, int32_t& size)
{
  address = 0;
  size    = -1;
  const uint32_t addr_offset = 0x40200000;
  const uint32_t realSize    = getFlashRealSizeInBytes();

  switch (ptype) {
    case ESP8266_partition_type::sketch:
      address = 0;
      size    = getSketchSize();
      break;
    case ESP8266_partition_type::ota:
      address = getSketchSize();
      size    = getFreeSketchSpace();
      break;
    case ESP8266_partition_type::fs:
      address = ((uint32_t)&_FS_start - addr_offset);
      size    = ((uint32_t)((uint32_t)&_FS_end - (uint32_t)&_FS_start));
      break;
    case ESP8266_partition_type::eeprom:
      address = ((uint32_t)&_EEPROM_start - addr_offset);
      size    = realSize - address - 16384;
      break;
    case ESP8266_partition_type::rf_cal:
      address = realSize - 16384;
      size    = 4096;
      break;
    case ESP8266_partition_type::wifi:
      address = realSize - 12288;
      size    = 12288;
      break;
  }

  if (size > 0) {
    return address / SPI_FLASH_SEC_SIZE;
  }
  return -1;
}

#endif // ifdef ESP8266


uint32_t getFlashChipId() {
  // Cache since size does not change
  static uint32_t flashChipId = 0;

  if (flashChipId == 0) {
  #ifdef ESP32
    uint32_t tmp = g_rom_flashchip.device_id;

    for (int i = 0; i < 3; ++i) {
      flashChipId  = flashChipId << 8;
      flashChipId |= (tmp & 0xFF);
      tmp          = tmp >> 8;
    }

    //    esp_flash_read_id(nullptr, &flashChipId);
  #elif defined(ESP8266)
    flashChipId = ESP.getFlashChipId();
  #endif // ifdef ESP32
  }
  return flashChipId;
}

uint32_t getFlashRealSizeInBytes() {
  // Cache since size does not change
  static uint32_t res = 0;

  if (res == 0) {
    #if defined(ESP32)
    res = (1 << ((getFlashChipId() >> 16) & 0xFF));
    #else // if defined(ESP32)
    res = ESP.getFlashChipRealSize(); // ESP.getFlashChipSize();
    #endif // if defined(ESP32)
  }
  return res;
}

#ifdef ESP32
uint32_t getXtalFrequencyMHz() {
  return rtc_clk_xtal_freq_get();
}

esp32_chip_features getChipFeatures() {
  static esp32_chip_features res;
  static bool loaded = false;

  if (!loaded) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    res.embeddedFlash     = chip_info.features & CHIP_FEATURE_EMB_FLASH;
    res.wifi_bgn          = chip_info.features & CHIP_FEATURE_WIFI_BGN;
    res.bluetooth_ble     = chip_info.features & CHIP_FEATURE_BLE;
    res.bluetooth_classic = chip_info.features & CHIP_FEATURE_BT;
    res.ieee_802_15_4     = chip_info.features & CHIP_FEATURE_IEEE802154;
    res.embeddedPSRAM     = chip_info.features & CHIP_FEATURE_EMB_PSRAM;

    if (!res.embeddedFlash) {
        const int32_t flash_cap = getEmbeddedFlashSize();
        if (flash_cap > 0) {
          res.embeddedFlash = true;
        }
    }

    loaded = true;
  }
  return res;
}

String getChipFeaturesString() {
  String features;

  if (getChipFeatures().wifi_bgn) { features += F("Wi-Fi bgn / "); }

  if (getChipFeatures().bluetooth_ble) { features += F("BLE / "); }

  if (getChipFeatures().ieee_802_15_4) { features += F("IEEE 802.15.4 / "); }

  const int32_t flash_cap = getEmbeddedFlashSize();

  if (getChipFeatures().embeddedFlash || (flash_cap != 0)) {
    if (flash_cap > 0) {
      features += strformat(F("%dMB "), flash_cap);
    } else if (flash_cap < 0) {
      features += strformat(F("(%d) "), flash_cap);
    }
    features += F("Emb. Flash");
    features += F(" / ");
  }

  const int32_t psram_cap = getEmbeddedPSRAMSize();

  if (getChipFeatures().embeddedPSRAM || (psram_cap != 0)) {
    if (psram_cap > 0) {
      features += strformat(F("%dMB "), psram_cap);
    } else if (psram_cap < 0) {
      features += strformat(F("(%d) "), psram_cap);
    }
    features += F("Emb. PSRAM");
  }
  features.trim();

  if (features.endsWith(F("/"))) { features = features.substring(0, features.length() - 1); }
  return features;
}

bool getFlashChipOPI_wired() {
  # if defined(ESP32_CLASSIC) || defined(ESP32C2)
  return false;

  # else // ifdef ESP32_CLASSIC

  // Source: https://github.com/espressif/esptool/commit/b25606b95920bd06df87aff9202c7a15377d4a30
  const uint32_t data = REG_GET_BIT(EFUSE_RD_REPEAT_DATA3_REG, BIT(9));
  return data != 0;
  # endif // ifdef ESP32_CLASSIC
}

#endif    // ifdef ESP32


uint32_t getFlashChipSpeed() {
  #ifdef ESP8266
  return ESP.getFlashChipSpeed();
  #else // ifdef ESP8266
# if ESP_IDF_STILL_NEEDS_SPI_REGISTERS_FIXED

  // ESP IDF still needs to patch those SPI registers
  // for which patches have been submitted and somehow they managed to merge it completely wrong.
  return ESP.getFlashChipSpeed();
# else // if ESP_IDF_STILL_NEEDS_SPI_REGISTERS_FIXED

  // All ESP32-variants have the SPI flash wired to SPI peripheral 1
  const uint32_t spi_clock = REG_READ(SPI_CLOCK_REG(1));

  /*
     addLog(LOG_LEVEL_INFO,   strformat(
      F("SPI_clock: %x  FSPI: %d SPI_CLOCK_REG(1): %x"),
      spi_clock, FSPI, SPI_CLOCK_REG(1)));
   */

  if (spi_clock & BIT(31)) {
    // spi_clk is equal to system clock
    return getApbFrequency();
  }
  return spiClockDivToFrequency(spi_clock);
# endif // if ESP_IDF_STILL_NEEDS_SPI_REGISTERS_FIXED
  #endif // ifdef ESP8266
}

const __FlashStringHelper* getFlashChipMode() {
  #ifdef ESP32

  if (getFlashChipOPI_wired()) {
    switch (ESP.getFlashChipMode()) {
      case FM_QIO:     return F("QIO (OPI Wired)");
      case FM_QOUT:    return F("QOUT (OPI Wired)");
      case FM_DIO:     return F("DIO (OPI Wired)");
      case FM_DOUT:    return F("DOUT (OPI Wired)");
  # ifdef ESP32
      case FM_FAST_READ: return F("Fast (OPI Wired)");
      case FM_SLOW_READ: return F("Slow (OPI Wired)");
  # endif // ifdef ESP32
      case FM_UNKNOWN: break;
    }
  }

  #endif // ifdef ESP32

  switch (ESP.getFlashChipMode()) {
    case FM_QIO:     return F("QIO");
    case FM_QOUT:    return F("QOUT");
    case FM_DIO:     return F("DIO");
    case FM_DOUT:    return F("DOUT");
#ifdef ESP32
    case FM_FAST_READ: return F("Fast");
    case FM_SLOW_READ: return F("Slow");
#endif // ifdef ESP32
    case FM_UNKNOWN: break;
  }
  return F("Unknown");
}

bool puyaSupport() {
  bool supported = false;

#ifdef PUYA_SUPPORT

  // New support starting core 2.5.0
  if (PUYA_SUPPORT) { supported = true; }
#endif // ifdef PUYA_SUPPORT
#ifdef PUYASUPPORT

  // Old patch
  supported = true;
#endif // ifdef PUYASUPPORT
  return supported;
}

uint8_t getFlashChipVendorId() {
#ifdef PUYA_SUPPORT
  return ESP.getFlashChipVendorId();
#else // ifdef PUYA_SUPPORT
  # if defined(ESP8266)

  // Cache since size does not change
  static uint32_t flashChipId = ESP.getFlashChipId();
  return flashChipId & 0x000000ff;
  # elif defined(ESP32)
  return 0xFF; // Not an existing function for ESP32
  # endif // if defined(ESP8266)
#endif // ifdef PUYA_SUPPORT
}

bool flashChipVendorPuya() {
  const uint8_t vendorId = getFlashChipVendorId();

  return vendorId == 0x85; // 0x146085 PUYA
}

uint32_t getChipId() {
  uint32_t chipId = 0;

#ifdef ESP8266
  chipId = ESP.getChipId();
#endif // ifdef ESP8266
#ifdef ESP32

  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
#endif // ifdef ESP32

  return chipId;
}

uint8_t getChipCores() {
  #ifdef ESP8266
  return 1;
  #else // ifdef ESP8266
  static uint8_t cores = 0;

  if (cores == 0) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    cores = chip_info.cores;
  }
  return cores;
  #endif // ifdef ESP8266
}

#ifdef ESP32
bool isESP8285() {
  return false;
}
#endif



String getChipRevision() {
  static uint16_t rev = 0;

  #ifdef ESP32

  // See: https://github.com/espressif/esp-idf/blob/master/examples/get-started/hello_world/main/hello_world_main.c
  if (rev == 0) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    # if ESP_IDF_VERSION_MAJOR < 5
    rev = chip_info.full_revision;
    # else // if ESP_IDF_VERSION_MAJOR < 5
    rev = chip_info.revision;
    # endif // if ESP_IDF_VERSION_MAJOR < 5
  }
  #endif // ifdef ESP32
  return strformat(F("%d.%02d"), rev / 100, rev % 100);
}

uint32_t getSketchSize() {
  // Cache the value as this never changes during run time.
  static uint32_t res = ESP.getSketchSize();

  return res;
}

uint32_t getFreeSketchSpace() {
  // Cache the value as this never changes during run time.
  static uint32_t res = ESP.getFreeSketchSpace();

  return res;
}

/********************************************************************************************\
   PSRAM support
 \*********************************************************************************************/

#ifdef ESP32

// this function is a replacement for `psramFound()`.
// `psramFound()` can return true even if no PSRAM is actually installed
// This new version also checks `esp_spiram_is_initialized` to know if the PSRAM is initialized
// Original Tasmota:
// https://github.com/arendst/Tasmota/blob/1e6b78a957be538cf494f0e2dc49060d1cb0fe8b/tasmota/support_esp.ino#L470
bool FoundPSRAM() {
  # if ESP_IDF_VERSION_MAJOR >= 5
  return psramFound();
  # else // if ESP_IDF_VERSION_MAJOR >= 5
#  if CONFIG_IDF_TARGET_ESP32C3
  return psramFound();
#  else  // if CONFIG_IDF_TARGET_ESP32C3
  return psramFound() && esp_spiram_is_initialized();
#  endif // if CONFIG_IDF_TARGET_ESP32C3
  # endif // if ESP_IDF_VERSION_MAJOR >= 5
}

// new function to check whether PSRAM is present and supported (i.e. required pacthes are present)
bool UsePSRAM() {
  static bool can_use_psram = CanUsePSRAM();

  return FoundPSRAM() && can_use_psram;
}

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
bool CanUsePSRAM() {
  if (!FoundPSRAM()) { return false; }
# ifdef HAS_PSRAM_FIX
  return true;
# endif // ifdef HAS_PSRAM_FIX
# if CONFIG_IDF_TARGET_ESP32
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  if ((CHIP_ESP32 == chip_info.model) && 
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
  (chip_info.revision < 300)
#else
  (chip_info.revision < 3)
#endif
  ) {
    return false;
  }
#  if ESP_IDF_VERSION_MAJOR < 4
  uint32_t chip_ver    = REG_GET_FIELD(EFUSE_BLK0_RDATA3_REG, EFUSE_RD_CHIP_VER_PKG);
  uint32_t pkg_version = chip_ver & 0x7;

  if ((CHIP_ESP32 == chip_info.model) && (pkg_version >= 6)) {
    return false; // support for embedded PSRAM of ESP32-PICO-V3-02 requires esp-idf 4.4
  }
#  endif // ESP_IDF_VERSION_MAJOR < 4

# endif // if CONFIG_IDF_TARGET_ESP32
  return true;
}

# ifdef ESP32
const __FlashStringHelper* getChipModel()
{
  
  // https://www.espressif.com/en/products/socs
  // https://github.com/arendst/Tasmota/blob/1e6b78a957be538cf494f0e2dc49060d1cb0fe8b/tasmota/support_esp.ino#L579

  /*
     Source: esp_chip_info.h & esptool.py  ('IMAGE_CHIP_ID') https://github.com/espressif/esptool/search?q=IMAGE_CHIP_ID

     typedef enum {
     CHIP_ESP32      = 1,  //!< ESP32
     CHIP_ESP32S2    = 2,  //!< ESP32-S2
     CHIP_ESP32S3_b  = 4,  //!< ESP32-S3(beta2)
     CHIP_ESP32S3    = 9,  //!< ESP32-S3
     CHIP_ESP32C3    = 5,  //!< ESP32-C3
     CHIP_ESP32C2    = 12, //!< ESP32-C2
     CHIP_ESP32C5    = 17, //!< ESP32-C5 beta3 (MPW)
     CHIP_ESP32C5    = 23, //!< ESP32-C5 MP
     CHIP_ESP32C6_b  = 7,  //!< ESP32-C6(beta)
     CHIP_ESP32C6    = 13, //!< ESP32-C6
     CHIP_ESP32C61   = 20, //!< ESP32-C61
     CHIP_ESP32H2_b1 = 10, //!< ESP32-H2(beta1)
     CHIP_ESP32H2_b2 = 14, //!< ESP32-H2(beta2)
     CHIP_ESP32H2    = 16, //!< ESP32-H2
     CHIP_ESP32P4    = 18, //!< ESP32-P4
     } esp_chip_model_t;

     // Chip feature flags, used in esp_chip_info_t
   #define CHIP_FEATURE_EMB_FLASH      BIT(0)      //!< Chip has embedded flash memory
   #define CHIP_FEATURE_WIFI_BGN       BIT(1)      //!< Chip has 2.4GHz WiFi
   #define CHIP_FEATURE_BLE            BIT(4)      //!< Chip has Bluetooth LE
   #define CHIP_FEATURE_BT             BIT(5)      //!< Chip has Bluetooth Classic
   #define CHIP_FEATURE_IEEE802154     BIT(6)      //!< Chip has IEEE 802.15.4
   #define CHIP_FEATURE_EMB_PSRAM      BIT(7)      //!< Chip has embedded psram

     // The structure represents information about the chip
     typedef struct {
     esp_chip_model_t model;  //!< chip model, one of esp_chip_model_t
     uint32_t features;       //!< bit mask of CHIP_FEATURE_x feature flags
     uint8_t cores;           //!< number of CPU cores
     uint8_t revision;        //!< chip revision number
     } esp_chip_info_t;
   */

  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  uint32_t chip_model          = chip_info.model;
  const uint32_t chip_revision =
#  if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    chip_info.revision / 100;
#  else // if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    chip_info.revision;
#  endif // if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)

  uint32_t pkg_version = 0;
#  if (ESP_IDF_VERSION_MAJOR >= 5)
  pkg_version = bootloader_common_get_chip_ver_pkg();
#  endif // if (ESP_IDF_VERSION_MAJOR >= 5)

  //  bool single_core = (1 == ESP.getChipCores());
  bool single_core = (1 == chip_info.cores);


  return getChipModel(chip_model, chip_revision, pkg_version, single_core);
}
#endif


#endif  // ESP32
