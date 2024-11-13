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


# if CONFIG_IDF_TARGET_ESP32S3   // ESP32-S3
  #  define HAS_HALL_EFFECT_SENSOR  0
  #  define HAS_TOUCH_GPIO 1
# elif CONFIG_IDF_TARGET_ESP32S2 // ESP32-S2
  #  define HAS_HALL_EFFECT_SENSOR  0
  #  define HAS_TOUCH_GPIO 1
# elif CONFIG_IDF_TARGET_ESP32C6 // ESP32-C6
  #  define HAS_HALL_EFFECT_SENSOR  0
  #  define HAS_TOUCH_GPIO  0
# elif CONFIG_IDF_TARGET_ESP32C3 // ESP32-C3
  #  define HAS_HALL_EFFECT_SENSOR  0
  #  define HAS_TOUCH_GPIO  0
# elif CONFIG_IDF_TARGET_ESP32C2 // ESP32-C2
  #  define HAS_HALL_EFFECT_SENSOR  0
  #  define HAS_TOUCH_GPIO  0
# elif CONFIG_IDF_TARGET_ESP32   // ESP32/PICO-D4
  #  if ESP_IDF_VERSION_MAJOR < 5
  #   define HAS_HALL_EFFECT_SENSOR  1
  #  else // if ESP_IDF_VERSION_MAJOR < 5

// Support for Hall Effect sensor was removed in ESP_IDF 5.x
  #   define HAS_HALL_EFFECT_SENSOR  0
  #  endif // if ESP_IDF_VERSION_MAJOR < 5
  #  define HAS_TOUCH_GPIO 1
# else // if CONFIG_IDF_TARGET_ESP32S3
  #  error Target CONFIG_IDF_TARGET is not supported
# endif // if CONFIG_IDF_TARGET_ESP32S3

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


bool isFlashInterfacePin_ESPEasy(int gpio) {
#if CONFIG_IDF_TARGET_ESP32

  if (getChipFeatures().embeddedFlash ||
      getChipFeatures().embeddedPSRAM) {

    // See page 20: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
    if (getChipFeatures().embeddedFlash) {
      //  ESP32-U4WDH In-Package Flash (4 MB)
      //  SD_DATA_1   IO0/DI      (GPIO-8)
      //  GPIO17      IO1/DO      (GPIO-17)
      //  SD_DATA_0   IO2/WP#     (GPIO-7)
      //  SD_CMD      IO3/HOLD#   (GPIO-11)
      //  SD_CLK      CLK         (GPIO-6)
      //  GPIO16      CS#         (GPIO-16)
      //  GND         VSS
      //  VDD_SDIO1   VDD

      if (gpio >= 6 && gpio <= 8) return true;
      if (gpio == 17 || gpio == 11 || gpio == 16) return true;
    }

    if (getChipFeatures().embeddedPSRAM) {
      //  ESP32-D0WDR2-V3 In-Package PSRAM (2 MB)
      //  SD_DATA_1       SIO0/SI (GPIO-8)
      //  SD_DATA_0       SIO1/SO (GPIO-7)
      //  SD_DATA_3       SIO2    (GPIO-10)
      //  SD_DATA_2       SIO3    (GPIO-9)
      //  SD_CLK          SCLK    (GPIO-6)
      //  GPIO16          CE#     (GPIO-16)
      //  GND             VSS
      //  VDD_SDIO1       VDD
      if (gpio >= 6 && gpio <= 10) return true;
      if (gpio == 16) return true;

    }
    return false;
  }


  // GPIO-6 ... 11: SPI flash and PSRAM
  // GPIO-16 & 17: CS for PSRAM, thus only unuable when PSRAM is present
  return (gpio) >= 6 && (gpio) <= 11;

#elif CONFIG_IDF_TARGET_ESP32S3

  // GPIO-26 ... 32: SPI flash and PSRAM
  // GPIO-33 ... 37: SPI 8 ­line mode (OPI) pins for flash or PSRAM, like ESP32-S3R8 / ESP32-S3R8V.
  return (gpio) >= 26 && (gpio) <= 32;

#elif CONFIG_IDF_TARGET_ESP32S2

  // GPIO-22 ... 25: SPI flash and PSRAM
  // GPIO-26: CS for PSRAM, thus only unuable when PSRAM is present
  // GPIO-27 ... 32: SPI 8 ­line mode (OPI) pins for flash or PSRAM (e.g. ESP32-S2FH2 and ESP32-S2FH4)
  return (gpio) >= 22 && (gpio) <= 25;

#elif CONFIG_IDF_TARGET_ESP32C6

  // FIXME TD-er: Must know whether we have internal or external flash

  // For chip variants with an in-package flash, this pin can not be used.
  if ((gpio == 10) || (gpio == 11)) {
    return true;
  }

  // For chip variants without an in-package flash, this pin can not be used.
  //  if (gpio == 14)
  //    return true;

  // GPIO-27: Flash voltage selector
  // GPIO-24 ... 30: Connected to internal flash (might be available when using external flash???)
  return (gpio) >= 24 && (gpio) <= 30 && gpio != 27;

#elif CONFIG_IDF_TARGET_ESP32C3

  // GPIO-11: Flash voltage selector
  // GPIO-12 ... 17: Connected to flash
  return (gpio) >= 12 && (gpio) <= 17;

#elif CONFIG_IDF_TARGET_ESP32C2

  // GPIO-11: Flash voltage selector
  // For chip variants with a SiP flash built in, GPIO11~ GPIO17 are dedicated to connecting SiP flash, not for other uses
  return (gpio) >= 12 && (gpio) <= 17;

#elif defined(ESP8266)

  if (isESP8285()) {
    return (gpio) == 6 || (gpio) == 7 || (gpio) == 8 || (gpio) == 11;
  }
  return (gpio) >= 6 && (gpio) <= 11;

#endif // if CONFIG_IDF_TARGET_ESP32
}

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

const __FlashStringHelper* getChipModel() {
#ifdef ESP32

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

  uint32_t chip_model    = chip_info.model;
  const uint32_t chip_revision =
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    chip_info.revision / 100;
#else
    chip_info.revision;
#endif

  uint32_t pkg_version = 0;
# if (ESP_IDF_VERSION_MAJOR >= 5)
  pkg_version = bootloader_common_get_chip_ver_pkg();
# endif // if (ESP_IDF_VERSION_MAJOR >= 5)

  //  uint32_t chip_revision = ESP.getChipRevision();
  bool rev3 = (3 == chip_revision);

  //  bool single_core = (1 == ESP.getChipCores());
  bool single_core = (1 == chip_info.cores);

  if (chip_model < 2) { // ESP32
# if CONFIG_IDF_TARGET_ESP32

    /* esptool:
        def get_pkg_version(self):
            word3 = self.read_efuse(3)
            pkg_version = (word3 >> 9) & 0x07
            pkg_version += ((word3 >> 2) & 0x1) << 3
            return pkg_version
     */
#  if ESP_IDF_VERSION_MAJOR < 5
    uint32_t chip_ver = REG_GET_FIELD(EFUSE_BLK0_RDATA3_REG, EFUSE_RD_CHIP_VER_PKG);
#  else // if ESP_IDF_VERSION_MAJOR < 5
    uint32_t chip_ver = REG_GET_FIELD(EFUSE_BLK0_RDATA3_REG, EFUSE_RD_CHIP_PACKAGE);
#  endif // if ESP_IDF_VERSION_MAJOR < 5
    pkg_version = chip_ver & 0x7;

    //    AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("HDW: ESP32 Model %d, Revision %d, Core %d, Package %d"), chip_info.model, chip_revision,
    // chip_info.cores, chip_ver);

    switch (pkg_version) {
      case EFUSE_RD_CHIP_VER_PKG_ESP32D0WDQ6:

        if (single_core) { return F("ESP32-S0WDQ6"); }    // Max 240MHz, Single core, QFN 6*6
        else if (rev3)   { return F("ESP32-D0WDQ6-V3"); } // Max 240MHz, Dual core, QFN 6*6
        else {             return F("ESP32-D0WDQ6"); }    // Max 240MHz, Dual core, QFN 6*6
      case EFUSE_RD_CHIP_VER_PKG_ESP32D0WDQ5:

        if (single_core) { return F("ESP32-S0WD"); }   // Max 160MHz, Single core, QFN 5*5, ESP32-SOLO-1, ESP32-DevKitC
        else if (rev3)   { return F("ESP32-D0WDQ5-V3"); } // Max 240MHz, Dual core, QFN 5*5, ESP32-WROOM-32E, ESP32_WROVER-E, ESP32-DevKitC
        else {             return F("ESP32-D0WDQ5"); }    // Max 240MHz, Dual core, QFN 5*5, ESP32-WROOM-32D, ESP32_WROVER-B, ESP32-DevKitC
      case EFUSE_RD_CHIP_VER_PKG_ESP32D2WDQ5:
        return F("ESP32-D2WDQ5");                      // Max 160MHz, Dual core, QFN 5*5, 2MB embedded flash
      case 3:

        if (single_core) { return F("ESP32-S0WD-OEM"); } // Max 160MHz, Single core, QFN 5*5, Xiaomi Yeelight
        else {             return F("ESP32-D0WD-OEM"); } // Max 240MHz, Dual core, QFN 5*5
      case EFUSE_RD_CHIP_VER_PKG_ESP32U4WDH:
        return F("ESP32-U4WDH");                         // Max 160MHz, Single core, QFN 5*5, 4MB embedded flash, ESP32-MINI-1,
      // ESP32-DevKitM-1
      case EFUSE_RD_CHIP_VER_PKG_ESP32PICOD4:

        if (rev3)        { return F("ESP32-PICO-V3"); } // Max 240MHz, Dual core, LGA 7*7, ESP32-PICO-V3-ZERO, ESP32-PICO-V3-ZERO-DevKit
        else {             return F("ESP32-PICO-D4"); } // Max 240MHz, Dual core, LGA 7*7, 4MB embedded flash, ESP32-PICO-KIT
      case EFUSE_RD_CHIP_VER_PKG_ESP32PICOV302:
        return F("ESP32-PICO-V3-02");                    // Max 240MHz, Dual core, LGA 7*7, 8MB embedded flash, 2MB embedded PSRAM,
                                                         // ESP32-PICO-MINI-02, ESP32-PICO-DevKitM-2
      case EFUSE_RD_CHIP_VER_PKG_ESP32D0WDR2V3:
        return F("ESP32-D0WDR2-V3");
    }
# endif // if CONFIG_IDF_TARGET_ESP32
    return F("ESP32");
  }
  else if (CHIP_ESP32S2 == chip_model) { // ESP32-S2
# ifdef CONFIG_IDF_TARGET_ESP32S2

    /* esptool:
        def get_flash_version(self):
            num_word = 3
            block1_addr = self.EFUSE_BASE + 0x044
            word3 = self.read_reg(block1_addr + (4 * num_word))
            pkg_version = (word3 >> 21) & 0x0F
            return pkg_version
        def get_psram_version(self):
            num_word = 3
            block1_addr = self.EFUSE_BASE + 0x044
            word3 = self.read_reg(block1_addr + (4 * num_word))
                pkg_version = (word3 >> 28) & 0x0F
                return pkg_version
     */
    uint32_t chip_ver  = REG_GET_FIELD(EFUSE_RD_MAC_SPI_SYS_3_REG, EFUSE_FLASH_VERSION);
    uint32_t psram_ver = REG_GET_FIELD(EFUSE_RD_MAC_SPI_SYS_3_REG, EFUSE_PSRAM_VERSION);
    pkg_version = (chip_ver & 0xF) + ((psram_ver & 0xF) * 100);

    //    AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("HDW: ESP32 Model %d, Revision %d, Core %d, Package %d"), chip_info.model, chip_revision,
    // chip_info.cores, chip_ver);

    switch (pkg_version) {
      case 0:              return F("ESP32-S2");      // Max 240MHz, Single core, QFN 7*7, ESP32-S2-WROOM, ESP32-S2-WROVER,
      // ESP32-S2-Saola-1, ESP32-S2-Kaluga-1
      case 1:              return F("ESP32-S2FH2");   // Max 240MHz, Single core, QFN 7*7, 2MB embedded flash, ESP32-S2-MINI-1,
      // ESP32-S2-DevKitM-1
      case 2:              return F("ESP32-S2FH4");   // Max 240MHz, Single core, QFN 7*7, 4MB embedded flash
      case 3:              return F("ESP32-S2FN4R2"); // Max 240MHz, Single core, QFN 7*7, 4MB embedded flash, 2MB embedded PSRAM, ,
      // ESP32-S2-MINI-1U, ESP32-S2-DevKitM-1U
      case 100:            return F("ESP32-S2R2");
      case 102:            return F("ESP32-S2FNR2");  // Max 240MHz, Single core, QFN 7*7, 4MB embedded flash, 2MB embedded PSRAM, , Lolin
                                                      // S2 mini
    }

# endif // CONFIG_IDF_TARGET_ESP32S2
    return F("ESP32-S2");
  }
  else if (CHIP_ESP32C3 == chip_model) { // ESP32-C3
# ifdef CONFIG_IDF_TARGET_ESP32C3

    /* esptool:
        def get_pkg_version(self):
            num_word = 3
            block1_addr = self.EFUSE_BASE + 0x044
            word3 = self.read_reg(block1_addr + (4 * num_word))
            pkg_version = (word3 >> 21) & 0x0F
            return pkg_version
     */
    uint32_t chip_ver = REG_GET_FIELD(EFUSE_RD_MAC_SPI_SYS_3_REG, EFUSE_PKG_VERSION);
    pkg_version = chip_ver & 0x7;

    //    pkg_version = esp_efuse_get_pkg_ver();

    //    AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("HDW: ESP32 Model %d, Revision %d, Core %d, Package %d"), chip_info.model, chip_revision,
    // chip_info.cores, chip_ver);

    switch (pkg_version) {
      case 0:              return F("ESP32-C3");    // Max 160MHz, Single core, QFN 5*5, ESP32-C3-WROOM-02, ESP32-C3-DevKitC-02
      //        case 1:              return F("ESP32-C3FH4");        // Max 160MHz, Single core, QFN 5*5, 4MB embedded flash,
      // ESP32-C3-MINI-1, ESP32-C3-DevKitM-1
      case 1:              return F("ESP8685");     // Max 160MHz, Single core, QFN 5*5, 4MB embedded flash, ESP32-C3-MINI-1,
      // ESP32-C3-DevKitM-1
      case 2:              return F("ESP32-C3 AZ"); // QFN32
      case 3:              return F("ESP8686");     // QFN24
    }
# endif // CONFIG_IDF_TARGET_ESP32C3
    return F("ESP32-C3");
  }
  else if ((CHIP_ESP32S3 == chip_model) || // ESP32-S3
           (4 == chip_model) ||            // ESP32-S3(beta2)
           (6 == chip_model)) {            // ESP32-S3(beta3)
    /*
       ESP32-S3 Series
       - 32-bit MCU & 2.4 GHz Wi-Fi & Bluetooth 5 (LE)
       - Xtensa® 32-bit LX7 dual-core processor that operates at up to 240 MHz
       - 512 KB of SRAM and 384 KB of ROM on the chip, and SPI, Dual SPI, Quad SPI, Octal SPI, QPI, and OPI interfaces that allow connection
          to flash and external RAM
       - Additional support for vector instructions in the MCU, which provides acceleration for neural network computing and signal
          processing workloads
       - Peripherals include 45 programmable GPIOs, SPI, I2S, I2C, PWM, RMT, ADC and UART, SD/MMC host and TWAITM
       - Reliable security features ensured by RSA-based secure boot, AES-XTS-based flash encryption, the innovative digital signature and
          the HMAC peripheral, “World Controller”
     */

    /*

            efuse_reg.h:
            EFUSE_RD_MAC_SPI_SYS_0_REG = block1_addr
            EFUSE_RD_MAC_SPI_SYS_3_REG = block1_addr + (4 * num_word)) // (num_word = 3)

     */

# ifdef CONFIG_IDF_TARGET_ESP32S3
#  if (ESP_IDF_VERSION_MAJOR >= 5)
    pkg_version = REG_GET_FIELD(EFUSE_RD_MAC_SPI_SYS_3_REG, EFUSE_PKG_VERSION);

    switch (pkg_version) {
      case 0:              return F("ESP32-S3");        // QFN56
      case 1:              return F("ESP32-S3-PICO-1"); // LGA56
    }
#  endif // if (ESP_IDF_VERSION_MAJOR >= 5)
# endif // CONFIG_IDF_TARGET_ESP32S3

    return F("ESP32-S3");      // Max 240MHz, Dual core, QFN 7*7, ESP32-S3-WROOM-1, ESP32-S3-DevKitC-1
  }
  else if (12 == chip_model) { // ESP32-C2 = ESP8684 if embedded flash
    /*
       ESP32-C2 Series
       - 32-bit RISC-V MCU & 2.4 GHz Wi-Fi & Bluetooth 5 (LE)
       - 32-bit RISC-V single-core processor that operates at up to 120 MHz
       - State-of-the-art power and RF performance
       - 576 KB ROM, 272 KB SRAM (16 KB for cache) on the chip
       - 14 programmable GPIOs: SPI, UART, I2C, LED PWM controller, General DMA controller (GDMA), SAR ADC, Temperature sensor
     */
# ifdef CONFIG_IDF_TARGET_ESP32C2

    switch (pkg_version) {
      case 0:              return F("ESP32-C2");
      case 1:              return F("ESP32-C2");
    }
# endif // CONFIG_IDF_TARGET_ESP32C2
    return F("ESP32-C2");
  }
  else if ((7 == chip_model) ||  // ESP32-C6(beta)
           (13 == chip_model)) { // ESP32-C6
# ifdef CONFIG_IDF_TARGET_ESP32C6

    /* esptool:
        def get_pkg_version(self):
            num_word = 3
            block1_addr = self.EFUSE_BASE + 0x044
            word3 = self.read_reg(block1_addr + (4 * num_word))
            pkg_version = (word3 >> 21) & 0x0F
            return pkg_version
     */
    uint32_t chip_ver = REG_GET_FIELD(EFUSE_RD_MAC_SPI_SYS_3_REG, EFUSE_PKG_VERSION);
    pkg_version = chip_ver & 0x7;

    //    pkg_version = esp_efuse_get_pkg_ver();

    //    AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("HDW: ESP32 Model %d, Revision %d, Core %d, Package %d"), chip_info.model, chip_revision,
    // chip_info.cores, chip_ver);

    switch (pkg_version) {
      case 0: return F("ESP32-C6");
      case 1: return F("ESP32-C6FH4 (QFN32)");
    }
# endif // CONFIG_IDF_TARGET_ESP32C6
    return F("ESP32-C6");
  }
  else if ((10 == chip_model) ||            // ESP32-H2(beta1)
           (14 == chip_model) ||           // ESP32-H2(beta2)
           (CHIP_ESP32H2 == chip_model)) { // ESP32-H2
# ifdef CONFIG_IDF_TARGET_ESP32H2

    /* esptool:
        def get_pkg_version(self):
            num_word = 3
            block1_addr = self.EFUSE_BASE + 0x044
            word3 = self.read_reg(block1_addr + (4 * num_word))
            pkg_version = (word3 >> 21) & 0x0F
            return pkg_version
     */
    uint32_t chip_ver = REG_GET_FIELD(EFUSE_RD_MAC_SPI_SYS_3_REG, EFUSE_PKG_VERSION);
    pkg_version = chip_ver & 0x7;

    //    pkg_version = esp_efuse_get_pkg_ver();

    //    AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("HDW: ESP32 Model %d, Revision %d, Core %d, Package %d"), chip_info.model, chip_revision,
    // chip_info.cores, chip_ver);

    switch (pkg_version) {
      case 0:              return F("ESP32-H2");
    }
# endif // CONFIG_IDF_TARGET_ESP32H2
    return F("ESP32-H2");
  }
  else if (18 == chip_model) {  // ESP32-P4
# ifdef CONFIG_IDF_TARGET_ESP32P4

    switch (pkg_version) {
      case 0:              return F("ESP32-P4");
    }
# endif // CONFIG_IDF_TARGET_ESP32P4
    return F("ESP32-P4");
  }
  else if (17 == chip_model) {  // ESP32-C5 beta3 (MPW)
    return F("ESP32-C5 beta3");
  }
  else if (23 == chip_model) {  // ESP32-C5 MP
    return F("ESP32-C5");
  }
  else if (20 == chip_model) {  // ESP32-C61
    return F("ESP32-C61");
  }

  return F("ESP32");
#elif defined(ESP8266)
  uint32_t pkg_version{};
  bool     high_temp_version{};

  if (isESP8285(pkg_version, high_temp_version)) {
    switch (pkg_version) {
      case 1:
        return (high_temp_version)
          ? F("ESP8285H08") // 1M flash
          : F("ESP8285N08");
      case 2:
        return (high_temp_version)
          ? F("ESP8285H16") // 2M flash
          : F("ESP8285N16");
      case 4:
        return (high_temp_version)
          ? F("ESP8285H32") // 4M flash
          : F("ESP8285N32");
    }
    return F("ESP8285");
  }
  return F("ESP8266EX");
#endif // ifdef ESP32
  return F("Unknown");
}

bool isESP8285(uint32_t& pkg_version, bool& high_temp_version)
{
  // Original code from Tasmota:
  // https://github.com/arendst/Tasmota/blob/62675a37a0e7b46283e2fdfe459bb8fd29d1cc2a/tasmota/tasmota_support/support_esp.ino#L151

  /*
     ESP8266 SoCs
     - 32-bit MCU & 2.4 GHz Wi-Fi
     - High-performance 160 MHz single-core CPU
     - +19.5 dBm output power ensures a good physical range
     - Sleep current is less than 20 μA, making it suitable for battery-powered and wearable-electronics applications
     - Peripherals include UART, GPIO, I2C, I2S, SDIO, PWM, ADC and SPI
   */

  // esptool.py get_efuses
  uint32_t efuse0 = *(uint32_t *)(0x3FF00050);

  //  uint32_t efuse1 = *(uint32_t*)(0x3FF00054);
  uint32_t efuse2 = *(uint32_t *)(0x3FF00058);
  uint32_t efuse3 = *(uint32_t *)(0x3FF0005C);

  bool r0_4  = efuse0 & (1 << 4);    // ESP8285
  bool r2_16 = efuse2 & (1 << 16);   // ESP8285

  if (r0_4 || r2_16) {               // ESP8285
    //                                                              1M 2M 2M 4M flash size
    //   r0_4                                                       1  1  0  0
    bool r3_25 = efuse3 & (1 << 25); // flash matrix  0  0  1  1
    bool r3_26 = efuse3 & (1 << 26); // flash matrix  0  1  0  1
    bool r3_27 = efuse3 & (1 << 27); // flash matrix  0  0  0  0
    pkg_version = 0;

    if (!r3_27) {
      if (r0_4 && !r3_25) {
        pkg_version = (r3_26) ? 2 : 1;
      }
      else if (!r0_4 && r3_25) {
        pkg_version = (r3_26) ? 4 : 2;
      }
    }
    high_temp_version = efuse0 & (1 << 5); // Max flash temperature (0 = 85C, 1 = 105C)
    return true;
  }
  return false;
}

bool isESP8285() {
  #ifdef ESP8266
  uint32_t pkg_version{};
  bool     high_temp_version{};
  return isESP8285(pkg_version, high_temp_version);
  #else // ifdef ESP8266
  return false;
  #endif // ifdef ESP8266
}

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

# ifndef isPSRAMInterfacePin
bool isPSRAMInterfacePin(int gpio) {
#  if CONFIG_IDF_TARGET_ESP32

  // GPIO-6 ... 11: SPI flash and PSRAM
  // GPIO-16 & 17: CS for PSRAM, thus only unuable when PSRAM is present
  return FoundPSRAM() ? ((gpio) == 16 || (gpio) == 17) : false;

#  elif CONFIG_IDF_TARGET_ESP32S2

  // GPIO-22 ... 25: SPI flash and PSRAM
  // GPIO-26: CS for PSRAM, thus only unuable when PSRAM is present
  // GPIO-27 ... 32: SPI 8 ­line mode (OPI) pins for flash or PSRAM (e.g. ESP32-S2FH2 and ESP32-S2FH4)
  // GPIO-27 ... 32: are never made accessible
  return FoundPSRAM() ? ((gpio) >= 26 && (gpio) <= 32) : false;

#  elif CONFIG_IDF_TARGET_ESP32S3

  // GPIO-26 ... 32: SPI flash and PSRAM
  // GPIO-33 ... 37: SPI 8 ­line mode (OPI) pins for flash or PSRAM, like ESP32-S3R8 / ESP32-S3R8V.
  // See Appendix A, page 71: https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf
  return FoundPSRAM() ? ((gpio) >= 33 && (gpio) <= 37) : false;

#  elif CONFIG_IDF_TARGET_ESP32C3

  // GPIO-11: Flash voltage selector
  // GPIO-12 ... 17: Connected to flash
  return false;

#  endif // if CONFIG_IDF_TARGET_ESP32
  return false;
}

# endif // ifndef isPSRAMInterfacePin

#endif  // ESP32
