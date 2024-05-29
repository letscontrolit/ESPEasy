#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32S3

// See: 
// - https://github.com/espressif/esptool/blob/master/esptool/targets/esp32s3.py
// - https://github.com/tasmota/esp-idf/blob/206ce4b7f875bf5568ba47aba23f4b28e81b0574/components/efuse/esp32s3/esp_efuse_table.csv#L203-L208
  # include <soc/soc.h>
  # include <soc/efuse_reg.h>
  # include <soc/spi_reg.h>
  # include <soc/rtc.h>

// Flash data lines: https://github.com/tasmota/esp-idf/blob/206ce4b7f875bf5568ba47aba23f4b28e81b0574/components/efuse/esp32s3/esp_efuse_table.csv#L175

/** EFUSE_FLASH_CAP : R; bitpos: [29:27]; default: 0;
 * register: EFUSE_RD_MAC_SPI_SYS_3_REG
 */
  # define EFUSE_FLASH_CAP    0x00000007U
  # define EFUSE_FLASH_CAP_M  (EFUSE_FLASH_CAP_V << EFUSE_FLASH_CAP_S)
  # define EFUSE_FLASH_CAP_V  0x00000007U
  # define EFUSE_FLASH_CAP_S  27


/** EFUSE_FLASH_VENDOR : R; bitpos: [2:0]; default: 0;
 * register: EFUSE_RD_MAC_SPI_SYS_4_REG
 */
  # define EFUSE_FLASH_VENDOR    0x00000007U
  # define EFUSE_FLASH_VENDOR_M  (EFUSE_FLASH_VENDOR_V << EFUSE_FLASH_VENDOR_S)
  # define EFUSE_FLASH_VENDOR_V  0x00000007U
  # define EFUSE_FLASH_VENDOR_S  0


/*
    switch (flash_vendor)
    {
      case 1: features += F("(XMC)"); break;
      case 2: features += F("(GD)"); break;
      case 3: features += F("(FM)"); break;
      case 4: features += F("(TT)"); break;
      case 5: features += F("(BY)"); break;
    }
    */


int32_t getEmbeddedFlashSize()
{
  const uint32_t flash_cap = REG_GET_FIELD(EFUSE_RD_MAC_SPI_SYS_3_REG, EFUSE_FLASH_CAP);

  switch (flash_cap) {
    case 0: return 0;
    case 1: return 8;
    case 2: return 4;
  }

  // Unknown value, thus mark as negative value
  return -1 *  static_cast<int32_t>(flash_cap);
}

/** EFUSE_PSRAM_CAP : R; bitpos: [4:3]; default: 0;
 * register: EFUSE_RD_MAC_SPI_SYS_4_REG
 */
  # define EFUSE_PSRAM_CAP    0x00000003U
  # define EFUSE_PSRAM_CAP_M  (EFUSE_PSRAM_CAP_V << EFUSE_PSRAM_CAP_S)
  # define EFUSE_PSRAM_CAP_V  0x00000003U
  # define EFUSE_PSRAM_CAP_S  3

/** EFUSE_PSRAM_VENDOR : R; bitpos: [8:7]; default: 0;
 * register: EFUSE_RD_MAC_SPI_SYS_4_REG
 */
  # define EFUSE_PSRAM_VENDOR    0x00000003U
  # define EFUSE_PSRAM_VENDOR_M  (EFUSE_PSRAM_VENDOR_V << EFUSE_PSRAM_VENDOR_S)
  # define EFUSE_PSRAM_VENDOR_V  0x00000003U
  # define EFUSE_PSRAM_VENDOR_S  7


/*
    switch (psram_vendor)
    {
      case 1: features += F("(AP_3v3)"); break;
      case 2: features += F("(AP_1v8)"); break;
    }
*/


int32_t getEmbeddedPSRAMSize()
{
  const uint32_t psram_cap = REG_GET_FIELD(EFUSE_RD_MAC_SPI_SYS_4_REG, EFUSE_PSRAM_CAP);

  switch (psram_cap) {
    case 0: return 0;
    case 1: return 8;
    case 2: return 2;
  }

  // Unknown value, thus mark as negative value
  return -1 * static_cast<int32_t>(psram_cap);
}

#endif // ifdef ESP32S3
