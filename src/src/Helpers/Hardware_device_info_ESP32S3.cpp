#include "../Helpers/Hardware_device_info.h"

#ifdef ESP32S3

// See:
// - https://github.com/espressif/esptool/blob/master/esptool/targets/esp32s3.py
// - https://github.com/tasmota/esp-idf/blob/206ce4b7f875bf5568ba47aba23f4b28e81b0574/components/efuse/esp32s3/esp_efuse_table.csv#L203-L208
# include <soc/soc.h>
# include <soc/efuse_reg.h>
# include <soc/spi_reg.h>
# include <soc/spi_pins.h>
# include <soc/rtc.h>
# include <esp_chip_info.h>
# include <bootloader_common.h>

// Flash data lines:
// https://github.com/tasmota/esp-idf/blob/206ce4b7f875bf5568ba47aba23f4b28e81b0574/components/efuse/esp32s3/esp_efuse_table.csv#L175

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
bool isFlashInterfacePin_ESPEasy(int gpio) {
  // GPIO-26 ... 32: SPI flash and PSRAM
  // GPIO-33 ... 37: SPI 8 ­line mode (OPI) pins for flash or PSRAM, like ESP32-S3R8 / ESP32-S3R8V.
//  return (gpio) >= 26 && (gpio) <= 32;
  switch (gpio) {
    case 26: // SPICS1   Only when PSRAM is present???
    case SPI_IOMUX_PIN_NUM_HD:
    case SPI_IOMUX_PIN_NUM_CS:
    case SPI_IOMUX_PIN_NUM_MOSI:
    case SPI_IOMUX_PIN_NUM_CLK:
    case SPI_IOMUX_PIN_NUM_MISO:
    case SPI_IOMUX_PIN_NUM_WP:
      return true;
  }
  return false;
}

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

# ifndef isPSRAMInterfacePin
bool isPSRAMInterfacePin(int gpio) {
  // GPIO-26 ... 32: SPI flash and PSRAM
  // GPIO-33 ... 37: SPI 8 ­line mode (OPI) pins for flash or PSRAM, like ESP32-S3R8 / ESP32-S3R8V.
  // See Appendix A, page 71: https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf
  return FoundPSRAM() ? ((gpio) >= 33 && (gpio) <= 37) : false;
}

# endif // ifndef isPSRAMInterfacePin


const __FlashStringHelper* getChipModel(uint32_t chip_model, uint32_t chip_revision, uint32_t pkg_version, bool single_core)
{
  if ((CHIP_ESP32S3 == chip_model) || // ESP32-S3
      (4 == chip_model) ||            // ESP32-S3(beta2)
      (6 == chip_model)) {            // ESP32-S3(beta3)
    /*
       ESP32-S3 Series
       - 32-bit MCU & 2.4 GHz Wi-Fi & Bluetooth 5 (LE)
       - Xtensa® 32-bit LX7 dual-core processor that operates at up to 240 MHz
       - 512 KB of SRAM and 384 KB of ROM on the chip, and SPI, Dual SPI, Quad SPI, Octal SPI, QPI, and OPI interfaces that allow connection
         to flash and external RAM
       - Additional support for vector instructions in the MCU, which provides acceleration for neural network computing and signal
         processing workloads
       - Peripherals include 45 programmable GPIOs, SPI, I2S, I2C, PWM, RMT, ADC and UART, SD/MMC host and TWAITM
       - Reliable security features ensured by RSA-based secure boot, AES-XTS-based flash encryption, the innovative digital signature and
         the HMAC peripheral, “World Controller”
     */

    /*

       efuse_reg.h:
       EFUSE_RD_MAC_SPI_SYS_0_REG = block1_addr
       EFUSE_RD_MAC_SPI_SYS_3_REG = block1_addr + (4 * num_word)) // (num_word = 3)

     */


# if (ESP_IDF_VERSION_MAJOR >= 5)
    pkg_version = REG_GET_FIELD(EFUSE_RD_MAC_SPI_SYS_3_REG, EFUSE_PKG_VERSION);

    switch (pkg_version) {
      case 0:              return F("ESP32-S3");        // QFN56
      case 1:              return F("ESP32-S3-PICO-1"); // LGA56
    }
# endif // if (ESP_IDF_VERSION_MAJOR >= 5)

    return F("ESP32-S3"); // Max 240MHz, Dual core, QFN 7*7, ESP32-S3-WROOM-1, ESP32-S3-DevKitC-1
  }

  return F("Unknown");
}

#endif // ifdef ESP32S3
