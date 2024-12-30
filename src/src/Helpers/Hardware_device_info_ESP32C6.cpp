#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32C6

# include <soc/soc.h>
# include <soc/efuse_reg.h>
# include <soc/spi_reg.h>
# include <soc/rtc.h>
# include <esp_chip_info.h>
# include <bootloader_common.h>


bool isFlashInterfacePin_ESPEasy(int gpio) {
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
}

bool flashVddPinCanBeUsedAsGPIO()
{
  return false;
}

int32_t getEmbeddedFlashSize()
{
  // See: framework-arduinoespressif32\tools\esp32-arduino-libs\esp32c6\include\soc\esp32c6\include\soc\efuse_reg.h
  const uint32_t flash_cap = REG_GET_FIELD(EFUSE_RD_MAC_SPI_SYS_4_REG, EFUSE_FLASH_CAP);

  // FIXME TD-er: No idea about meaning of values
  switch (flash_cap) {
    case 0: return 0;
    case 1: return 4;
    case 2: return 2;
    case 3: return 1;
    case 4: return 8;
  }

  // Unknown value, thus mark as negative value
  return -1 *  static_cast<int32_t>(flash_cap);
}

int32_t getEmbeddedPSRAMSize()
{
  // Doesn't have PSRAM
  return 0;
}

# ifndef isPSRAMInterfacePin
bool isPSRAMInterfacePin(int gpio) {
  return false;
}

# endif // ifndef isPSRAMInterfacePin


const __FlashStringHelper* getChipModel(uint32_t chip_model, uint32_t chip_revision, uint32_t pkg_version, bool single_core)
{
  if ((7 == chip_model) ||  // ESP32-C6(beta)
      (13 == chip_model)) { // ESP32-C6
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

    return F("ESP32-C6");
  }

  return F("Unknown");
}

#endif // ifdef ESP32C6
