#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32S2

// See: https://github.com/espressif/esptool/blob/master/esptool/targets/esp32s2.py

  # include <soc/soc.h>
  # include <soc/efuse_reg.h>
  # include <soc/spi_reg.h>
  # include <soc/rtc.h>


// Flash datalines: https://github.com/tasmota/esp-idf/blob/206ce4b7f875bf5568ba47aba23f4b28e81b0574/components/efuse/esp32s2/esp_efuse_table.csv#L155



/** EFUSE_FLASH_CAP : R; bitpos: [24:21]; default: 0;
 * register: EFUSE_RD_MAC_SPI_SYS_3_REG
 */
  # define EFUSE_FLASH_CAP    0x0000000FU
  # define EFUSE_FLASH_CAP_M  (EFUSE_FLASH_CAP_V << EFUSE_FLASH_CAP_S)
  # define EFUSE_FLASH_CAP_V  0x0000000FU
  # define EFUSE_FLASH_CAP_S  21


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


#endif
