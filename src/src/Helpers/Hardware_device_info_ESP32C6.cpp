#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32C6

# include <soc/soc.h>
# include <soc/efuse_reg.h>
# include <soc/spi_reg.h>
# include <soc/rtc.h>

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
#endif
