#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32C3

// See: https://github.com/espressif/esptool/blob/master/esptool/targets/esp32c3.py


  # include <soc/soc.h>
  # include <soc/efuse_reg.h>
  # include <soc/spi_reg.h>
  # include <soc/rtc.h>


/** EFUSE_FLASH_CAP : R; bitpos: [29:27]; default: 0;
 * register: EFUSE_RD_MAC_SPI_SYS_3_REG
 */
  # define EFUSE_FLASH_CAP    0x00000007U
  # define EFUSE_FLASH_CAP_M  (EFUSE_FLASH_CAP_V << EFUSE_FLASH_CAP_S)
  # define EFUSE_FLASH_CAP_V  0x00000007U
  # define EFUSE_FLASH_CAP_S  27


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
#endif
