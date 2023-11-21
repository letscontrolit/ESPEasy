#ifndef DATATYPES_SPI_OPTIONS_H
#define DATATYPES_SPI_OPTIONS_H

#include "../../ESPEasy_common.h"


// ESP32 classic has default pins for HSPI.
// All others don't, as any pin can be used.

// Default SPI host:
// ESP32 classic: VSPI
// ESP32-C3     : FSPI
// ESP32-S2/S3  : FSPI

// ESP32 classic:
//  SPI_HOST  = SPI1_HOST // Only usable on ESP32
//  HSPI_HOST = SPI2_HOST
//  VSPI_HOST = SPI3_HOST
// 
// ESP32-S2:
//  FSPI_HOST = SPI2_HOST
//  HSPI_HOST = SPI3_HOST
// 
// ESP32-S3:
//  FSPI_HOST = SPI2_HOST
//  HSPI_HOST = SPI3_HOST
// 
// ESP32-C6:
//  FSPI_HOST = SPI1_HOST
// ESP32-C3:
//  FSPI_HOST = SPI2_HOST
// ESP32-C2:
//  FSPI_HOST = SPI1_HOST


// SPIx_HOST defined in enum spi_host_device_t in hal/spi_types.h
// Aliases for each platform defined in ESPEasy: src/include/esp32x_fixes.h

// HSPI option is disabled for now on other than ESP32_CLASSIC.
// Should be added later as "Custom HSPI" when we support multiple SPI busses.
#ifdef ESP32
#  if CONFIG_IDF_TARGET_ESP32S3   // ESP32-S3
#define VSPI_FSPI_SCK  36
#define VSPI_FSPI_MISO 37
#define VSPI_FSPI_MOSI 35
#  elif CONFIG_IDF_TARGET_ESP32S2   // ESP32-S2
#define VSPI_FSPI_SCK  36
#define VSPI_FSPI_MISO 37
#define VSPI_FSPI_MOSI 35
#  elif CONFIG_IDF_TARGET_ESP32C6 // ESP32-C6
#define VSPI_FSPI_SCK  21
#define VSPI_FSPI_MISO 20
#define VSPI_FSPI_MOSI 19
#  elif CONFIG_IDF_TARGET_ESP32C3 // ESP32-C3
#define VSPI_FSPI_SCK  4
#define VSPI_FSPI_MISO 5
#define VSPI_FSPI_MOSI 6
#  elif CONFIG_IDF_TARGET_ESP32C2 // ESP32-C2
#define VSPI_FSPI_SCK  4  
#define VSPI_FSPI_MISO 5
#define VSPI_FSPI_MOSI 6
#  elif CONFIG_IDF_TARGET_ESP32   // ESP32/PICO-D4
#define VSPI_FSPI_SCK  18
#define VSPI_FSPI_MISO 19
#define VSPI_FSPI_MOSI 23

# define HSPI_SCLK   14
# define HSPI_MISO   12
# define HSPI_MOSI   13
# define HSPI_SS     15

#  else // if CONFIG_IDF_TARGET_ESP32S2
#   error Target CONFIG_IDF_TARGET is not supported
#  endif // if CONFIG_IDF_TARGET_ESP32S2

#endif

enum class SPI_Options_e { // Do not change values as this is stored in the settings!
  None        = 0,
  Vspi_Fspi   = 1, // Default SPI bus
#ifdef ESP32_CLASSIC
  Hspi        = 2,
#endif

  // UserDefined is using the default SPI bus.
  // For ESP32 classic, this is called VSPI
  // For later versions it is called FSPI
  // N.B. the ESP32-C3 does not seem to name these as there is no SPI3_HOST.
  UserDefined = 9 // Leave some room for other, possible future, hardware-related options
};

#ifdef ESP32
const __FlashStringHelper* getSPI_optionToString(SPI_Options_e option);
const __FlashStringHelper* getSPI_optionToShortString(SPI_Options_e option);
#endif // ifdef ESP32

#endif // ifndef DATATYPES_SPI_OPTIONS_H
