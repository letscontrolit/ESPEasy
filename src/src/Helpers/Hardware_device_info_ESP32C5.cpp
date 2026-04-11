#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32C5

# include <soc/soc.h>
# include <soc/efuse_reg.h>
# include <soc/spi_reg.h>
# include <soc/spi_pins.h>
# include <soc/rtc.h>
# include <esp_chip_info.h>
# include <bootloader_common.h>


bool isFlashInterfacePin_ESPEasy(int gpio) {
  // FIXME TD-er: Must know whether we have internal or external flash

  // For chip variants with an in-package flash, this pin can not be used.


  switch (gpio) {
    case MSPI_IOMUX_PIN_NUM_HD:
    case MSPI_IOMUX_PIN_NUM_WP:
    case MSPI_IOMUX_PIN_NUM_CS0:
    case MSPI_IOMUX_PIN_NUM_CLK:
    case MSPI_IOMUX_PIN_NUM_MOSI:
    case MSPI_IOMUX_PIN_NUM_MISO:
      return true;

    case 19:
      // special pin to power flash
      return true;
  }
  return false;

  //  return (gpio) >= 24 && (gpio) <= 30 && gpio != 27;
}

bool flashVddPinCanBeUsedAsGPIO()
{
  return false;
}

int32_t getEmbeddedFlashSize()
{
  /*
  // See: framework-arduinoespressif32\tools\esp32-arduino-libs\esp32c5\include\soc\esp32c5\include\soc\efuse_reg.h
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
  */
  return 8;
}

int32_t getEmbeddedPSRAMSize()
{
  // Doesn't have PSRAM
  return 0;
}


# ifndef isPSRAMInterfacePin
bool isPSRAMInterfacePin(int gpio) {
  return FoundPSRAM() ? ((gpio) >= MSPI_IOMUX_PIN_NUM_CS1 && (gpio) <= MSPI_IOMUX_PIN_NUM_MOSI) : false;
}

# endif // ifndef isPSRAMInterfacePin



const __FlashStringHelper* getChipModel(uint32_t chip_model, uint32_t chip_revision, uint32_t pkg_version, bool single_core)
{
  return F("ESP32-C5");
}

#endif // ifdef ESP32C5
