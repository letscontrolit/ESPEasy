#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32S2

// See: https://github.com/espressif/esptool/blob/master/esptool/targets/esp32s2.py

# include <soc/soc.h>
# include <soc/efuse_reg.h>
# include <soc/spi_reg.h>
# include <soc/rtc.h>
# include <esp_chip_info.h>
# include <bootloader_common.h>


// Flash datalines:
// https://github.com/tasmota/esp-idf/blob/206ce4b7f875bf5568ba47aba23f4b28e81b0574/components/efuse/esp32s2/esp_efuse_table.csv#L155


/** EFUSE_FLASH_CAP : R; bitpos: [24:21]; default: 0;
 * register: EFUSE_RD_MAC_SPI_SYS_3_REG
 */
# define EFUSE_FLASH_CAP    0x0000000FU
# define EFUSE_FLASH_CAP_M  (EFUSE_FLASH_CAP_V << EFUSE_FLASH_CAP_S)
# define EFUSE_FLASH_CAP_V  0x0000000FU
# define EFUSE_FLASH_CAP_S  21

bool isFlashInterfacePin_ESPEasy(int gpio) {
  // GPIO-22 ... 25: SPI flash and PSRAM
  // GPIO-26: CS for PSRAM, thus only unuable when PSRAM is present
  // GPIO-27 ... 32: SPI 8 ­line mode (OPI) pins for flash or PSRAM (e.g. ESP32-S2FH2 and ESP32-S2FH4)
  return (gpio) >= 22 && (gpio) <= 25;
}

bool flashVddPinCanBeUsedAsGPIO()
{
  return false;
}

int32_t getEmbeddedFlashSize()
{
  const uint32_t flash_cap = REG_GET_FIELD(EFUSE_RD_MAC_SPI_SYS_3_REG, EFUSE_FLASH_CAP);

  switch (flash_cap) {
    case 0: return 0;
    case 1: return 2;
    case 2: return 4;
  }

  // Unknown value, thus mark as negative value
  return -1 *  static_cast<int32_t>(flash_cap);
}

/** EFUSE_PSRAM_CAP : R; bitpos: [31:28]; default: 0;
 * register: EFUSE_RD_MAC_SPI_SYS_3_REG
 */
# define EFUSE_PSRAM_CAP    0x0000000FU
# define EFUSE_PSRAM_CAP_M  (EFUSE_PSRAM_CAP_V << EFUSE_PSRAM_CAP_S)
# define EFUSE_PSRAM_CAP_V  0x0000000FU
# define EFUSE_PSRAM_CAP_S  28

int32_t getEmbeddedPSRAMSize()
{
  const uint32_t psram_cap = REG_GET_FIELD(EFUSE_RD_MAC_SPI_SYS_3_REG, EFUSE_PSRAM_CAP);

  switch (psram_cap) {
    case 0: return 0;
    case 1: return 2;
    case 2: return 4;
  }

  // Unknown value, thus mark as negative value
  return -1 * static_cast<int32_t>(psram_cap);
}

# ifndef isPSRAMInterfacePin
bool isPSRAMInterfacePin(int gpio) {
  // GPIO-22 ... 25: SPI flash and PSRAM
  // GPIO-26: CS for PSRAM, thus only unuable when PSRAM is present
  // GPIO-27 ... 32: SPI 8 ­line mode (OPI) pins for flash or PSRAM (e.g. ESP32-S2FH2 and ESP32-S2FH4)
  // GPIO-27 ... 32: are never made accessible
  return FoundPSRAM() ? ((gpio) >= 26 && (gpio) <= 32) : false;
}

# endif // ifndef isPSRAMInterfacePin

const __FlashStringHelper* getChipModel(uint32_t chip_model, uint32_t chip_revision, uint32_t pkg_version, bool single_core)
{
  if (CHIP_ESP32S2 == chip_model) { // ESP32-S2
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
      case 0: return F("ESP32-S2");      // Max 240MHz, Single core, QFN 7*7, ESP32-S2-WROOM, ESP32-S2-WROVER, ESP32-S2-Saola-1, ESP32-S2-Kaluga-1
      case 1: return F("ESP32-S2FH2");   // Max 240MHz, Single core, QFN 7*7, 2MB embedded flash, ESP32-S2-MINI-1, ESP32-S2-DevKitM-1
      case 2: return F("ESP32-S2FH4");   // Max 240MHz, Single core, QFN 7*7, 4MB embedded flash
      case 3: return F("ESP32-S2FN4R2"); // Max 240MHz, Single core, QFN 7*7, 4MB embedded flash, 2MB embedded PSRAM, ESP32-S2-MINI-1U, ESP32-S2-DevKitM-1U
      case 100: return F("ESP32-S2R2");
      case 102: return F("ESP32-S2FNR2");  // Max 240MHz, Single core, QFN 7*7, 4MB embedded flash, 2MB embedded PSRAM, Lolin S2 mini
    }

    return F("ESP32-S2");
  }

  return F("Unknown");
}

#endif // ifdef ESP32S2
