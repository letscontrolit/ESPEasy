#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32C3

// See: https://github.com/espressif/esptool/blob/master/esptool/targets/esp32c3.py


# include <soc/soc.h>
# include <soc/efuse_reg.h>
# include <soc/spi_reg.h>
# include <soc/rtc.h>
# include <esp_chip_info.h>
# include <bootloader_common.h>


/** EFUSE_FLASH_CAP : R; bitpos: [29:27]; default: 0;
 * register: EFUSE_RD_MAC_SPI_SYS_3_REG
 */
# define EFUSE_FLASH_CAP    0x00000007U
# define EFUSE_FLASH_CAP_M  (EFUSE_FLASH_CAP_V << EFUSE_FLASH_CAP_S)
# define EFUSE_FLASH_CAP_V  0x00000007U
# define EFUSE_FLASH_CAP_S  27


/** EFUSE_VDD_SPI_AS_GPIO : RO; bitpos: [26]; default: 0;
 *  Set this bit to vdd spi pin function as gpio.
 *  register: EFUSE_RD_REPEAT_ERR0_REG
 */
# define EFUSE_VDD_SPI_AS_GPIO    (BIT(26))
# define EFUSE_VDD_SPI_AS_GPIO_M  (EFUSE_VDD_SPI_AS_GPIO_V << EFUSE_VDD_SPI_AS_GPIO_S)
# define EFUSE_VDD_SPI_AS_GPIO_V  0x00000001U
# define EFUSE_VDD_SPI_AS_GPIO_S  26

bool isFlashInterfacePin_ESPEasy(int gpio) {
  // GPIO-11: Flash voltage selector
  // GPIO-12 ... 17: Connected to flash
  return (gpio) >= 12 && (gpio) <= 17;
}

bool flashVddPinCanBeUsedAsGPIO()
{
  const bool efuse_vdd_spi_as_gpio = REG_GET_FIELD(EFUSE_RD_REPEAT_ERR0_REG, EFUSE_VDD_SPI_AS_GPIO) != 0;

  return efuse_vdd_spi_as_gpio;
}

int32_t getEmbeddedFlashSize()
{
  const uint32_t flash_cap = REG_GET_FIELD(EFUSE_RD_MAC_SPI_SYS_3_REG, EFUSE_FLASH_CAP);

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
 if (CHIP_ESP32C3 == chip_model) { // ESP32-C3
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
      case 0: return F("ESP32-C3 (QFN32)");    // Max 160MHz, Single core, QFN 5*5, ESP32-C3-WROOM-02, ESP32-C3-DevKitC-02
      // case 1: return F("ESP32-C3FH4");      // Max 160MHz, Single core, QFN 5*5, 4MB embedded flash, ESP32-C3-MINI-1, ESP32-C3-DevKitM-1
      case 1: return F("ESP8685 (QFN28)");     // Max 160MHz, Single core, QFN 5*5, 4MB embedded flash, ESP32-C3-MINI-1, ESP32-C3-DevKitM-1
      case 2: return F("ESP32-C3 AZ (QFN32)"); // QFN32
      case 3: return F("ESP8686 (QFN24)");     // QFN24
    }
    return F("ESP32-C3");
  }

  return F("Unknown");
}

#endif // ifdef ESP32C3
