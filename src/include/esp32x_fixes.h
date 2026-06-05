// Borrowed from the Tasmota branch, to support ESP32-C3 and ESP32-S3
// Many thanks to Theo Arends and the rest of the Tasmota team!

/*
   esp32x_fixes.h - fix esp32x toolchain

   Copyright (C) 2021  Theo Arends

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Xtensa toolchain declares `int32_t` as `int` but RISC-V toolchain
 * declares `int32_t` as `long int` which causes compilation errors.
 *
 * See:
 *   https://github.com/espressif/esp-idf/issues/6906
 *   https://github.com/espressif/arduino-esp32/issues/5086
 *
 * You need to add the following lines in `build_flags`:
 *                            -I$PROJECT_DIR/include
 *                            -include "esp32x_fixes.h"
 */

// #include <sdkconfig.h>

#ifdef __riscv

# undef __INT32_TYPE__
# define __INT32_TYPE__      int

# undef __UINT32_TYPE__
# define __UINT32_TYPE__     unsigned int

#endif // __riscv

#ifdef ESP32
# include <soc/soc_caps.h>
#endif

// alias, deprecated for the chips after esp32s2
#ifdef CONFIG_IDF_TARGET_ESP32
# define SPI_HOST    SPI1_HOST // SPI 1 bus attached to the flash (can use the same data lines but different SS)
# define HSPI_HOST   SPI2_HOST // SPI 2 bus normally mapped to pins 12 - 15, but can be matrixed to any pins
# define VSPI_HOST   SPI3_HOST // SPI 3 bus normally attached to pins 5, 18, 19 and 23, but can be matrixed to any pins

#else // ifdef CONFIG_IDF_TARGET_ESP32

// SPI_HOST (SPI1_HOST) is not supported by the SPI Master and SPI Slave driver on ESP32-S2 and later
# define SPI_HOST    SPI1_HOST
# define FSPI_HOST   SPI2_HOST  // ESP32C2, C3, C5, C6, C61, H2, S2, S3, P4 - SPI 2 bus
# if SOC_SPI_PERIPH_NUM > 2
#  define HSPI_HOST   SPI3_HOST // ESP32S2, S3, P4 - SPI 3 bus
# else
# endif
# define VSPI_HOST    FSPI_HOST // Alias for older code
#endif // TARGET


#ifndef CONFIG_SOC_WIFI_HE_SUPPORT
# if CONFIG_IDF_TARGET_ESP32C5 || CONFIG_IDF_TARGET_ESP32C6 || CONFIG_IDF_TARGET_ESP32C61
#  define CONFIG_SOC_WIFI_HE_SUPPORT 1
# else
#  define CONFIG_SOC_WIFI_HE_SUPPORT 0
# endif // if CONFIG_IDF_TARGET_ESP32C5 || CONFIG_IDF_TARGET_ESP32C6 || CONFIG_IDF_TARGET_ESP32C61
#endif // ifndef CONFIG_SOC_WIFI_HE_SUPPORT

#ifndef CONFIG_SOC_WIFI_SUPPORT_5G
# if CONFIG_IDF_TARGET_ESP32C5
#  define CONFIG_SOC_WIFI_SUPPORT_5G 1
# else
#  define CONFIG_SOC_WIFI_SUPPORT_5G 0
# endif // if CONFIG_IDF_TARGET_ESP32C5
#endif // ifndef CONFIG_SOC_WIFI_SUPPORT_5G
