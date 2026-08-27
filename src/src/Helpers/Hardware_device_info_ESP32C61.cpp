#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32C61


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

  // FIXME TD-er: No idea where these pins come from. Disabled for now.
  // See: https://github.com/letscontrolit/ESPEasy/issues/5220

  /*
     if ((gpio == 10) || (gpio == 11)) {
     return true;
     }
   */

  // For chip variants without an in-package flash, this pin can not be used.
  //  if (gpio == 14)
  //    return true;

  // GPIO-27: Flash voltage selector
  // GPIO-24 ... 30: Connected to internal flash (might be available when using external flash???)

  switch (gpio) {
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 4, 0)
    case SPI_IOMUX_PIN_NUM_CS:
    case SPI_IOMUX_PIN_NUM_CLK:
    case SPI_IOMUX_PIN_NUM_MOSI:
    case SPI_IOMUX_PIN_NUM_MISO:
    case SPI_IOMUX_PIN_NUM_WP:
    case SPI_IOMUX_PIN_NUM_HD:
#else
    case MSPI_IOMUX_PIN_NUM_HD:
    case MSPI_IOMUX_PIN_NUM_WP:
    case MSPI_IOMUX_PIN_NUM_CS0:
    case MSPI_IOMUX_PIN_NUM_CLK:
    case MSPI_IOMUX_PIN_NUM_MOSI:
    case MSPI_IOMUX_PIN_NUM_MISO:
#endif
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
  return 8; // TODO TD-er: Must implement
  // See: framework-arduinoespressif32\tools\esp32-arduino-libs\esp32c61\include\soc\esp32c61\include\soc\efuse_reg.h
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
  if (20 == chip_model) { // ESP32-C61
    return F("ESP32-C61");
  }

  return F("Unknown");
}

#endif // ifdef ESP32C61
