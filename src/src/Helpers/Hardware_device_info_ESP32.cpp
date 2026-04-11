#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32_CLASSIC

# include <soc/soc.h>
# include <soc/efuse_reg.h>
# include <soc/spi_reg.h>
# include <soc/rtc.h>
# include <esp_chip_info.h>
# include <bootloader_common.h>

bool isFlashInterfacePin_ESPEasy(int gpio) {
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

      if ((gpio >= 6) && (gpio <= 8)) { return true; }

      if ((gpio == 17) || (gpio == 11) || (gpio == 16)) { return true; }
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
      if ((gpio >= 6) && (gpio <= 10)) { return true; }

      if (gpio == 16) { return true; }
    }
    return false;
  }


  // GPIO-6 ... 11: SPI flash and PSRAM
  // GPIO-16 & 17: CS for PSRAM, thus only unuable when PSRAM is present
  return (gpio) >= 6 && (gpio) <= 11;
}

bool flashVddPinCanBeUsedAsGPIO()
{
  return false;
}

int32_t getEmbeddedFlashSize()
{
  return 0;
}

int32_t getEmbeddedPSRAMSize()
{
  // FIXME TD-er: Need to implement
  return 0;
}

# ifndef isPSRAMInterfacePin
bool isPSRAMInterfacePin(int gpio) {
  // GPIO-6 ... 11: SPI flash and PSRAM
  // GPIO-16 & 17: CS for PSRAM, thus only unuable when PSRAM is present
  return FoundPSRAM() ? ((gpio) == 16 || (gpio) == 17) : false;
}

# endif // ifndef isPSRAMInterfacePin


const __FlashStringHelper* getChipModel(uint32_t chip_model, uint32_t chip_revision, uint32_t pkg_version, bool single_core)
{
  //  uint32_t chip_revision = ESP.getChipRevision();
  const bool rev3 = (3 == chip_revision);

  if (chip_model < 2) { // ESP32
    /* esptool:
       def get_pkg_version(self):
       word3 = self.read_efuse(3)
       pkg_version = (word3 >> 9) & 0x07
       pkg_version += ((word3 >> 2) & 0x1) << 3
       return pkg_version
     */
# if ESP_IDF_VERSION_MAJOR < 5
    uint32_t chip_ver = REG_GET_FIELD(EFUSE_BLK0_RDATA3_REG, EFUSE_RD_CHIP_VER_PKG);
# else // if ESP_IDF_VERSION_MAJOR < 5
    uint32_t chip_ver = REG_GET_FIELD(EFUSE_BLK0_RDATA3_REG, EFUSE_RD_CHIP_PACKAGE);
# endif // if ESP_IDF_VERSION_MAJOR < 5
    pkg_version = chip_ver & 0x7;


    //    AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("HDW: ESP32 Model %d, Revision %d, Core %d, Package %d"), chip_info.model, chip_revision,
    // chip_info.cores, chip_ver);

    switch (pkg_version) {
      case EFUSE_RD_CHIP_VER_PKG_ESP32D0WDQ6:

        if (single_core) { return F("ESP32-S0WDQ6"); }    // Max 240MHz, Single core, QFN 6*6
        else if (rev3)   { return F("ESP32-D0WDQ6-V3"); } // Max 240MHz, Dual core, QFN 6*6
        else {             return F("ESP32-D0WDQ6"); }    // Max 240MHz, Dual core, QFN 6*6
      case EFUSE_RD_CHIP_VER_PKG_ESP32D0WDQ5:

        if (single_core) { return F("ESP32-S0WD"); }      // Max 160MHz, Single core, QFN 5*5, ESP32-SOLO-1, ESP32-DevKitC
        else if (rev3)   { return F("ESP32-D0WDQ5-V3"); } // Max 240MHz, Dual core, QFN 5*5, ESP32-WROOM-32E, ESP32_WROVER-E, ESP32-DevKitC
        else {             return F("ESP32-D0WDQ5"); }    // Max 240MHz, Dual core, QFN 5*5, ESP32-WROOM-32D, ESP32_WROVER-B, ESP32-DevKitC
      case EFUSE_RD_CHIP_VER_PKG_ESP32D2WDQ5:
        return F("ESP32-D2WDQ5"); // Max 160MHz, Dual core, QFN 5*5, 2MB embedded flash
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
        return F("ESP32-PICO-V3-02");                   // Max 240MHz, Dual core, LGA 7*7, 8MB embedded flash, 2MB embedded PSRAM,
      // ESP32-PICO-MINI-02, ESP32-PICO-DevKitM-2
      case EFUSE_RD_CHIP_VER_PKG_ESP32D0WDR2V3:
        return F("ESP32-D0WDR2-V3"); // Max 240MHz, Dual core, QFN 5*5, ESP32-WROOM-32E, ESP32_WROVER-E, ESP32-DevKitC
    }
    return F("ESP32");
  }

  return F("Unknown");
}

#endif // ifdef ESP32_CLASSIC
