#include "../DataTypes/SPI_options.h"


#ifdef ESP32

// ESP32 VSPI:
//  SCK  = 18
//  MISO = 19
//  MOSI = 23
// ESP32 HSPI:
//  SCK  = 14
//  MISO = 12
//  MOSI = 13


// ESP32-S2 FSPI:
//  SCK  = 36
//  MISO = 37
//  MOSI = 35

// ESP32-S3 FSPI:
//  SCK  = 36
//  MISO = 37
//  MOSI = 35

// ESP32-C2 SPI:
//  SCK  = 4
//  MISO = 5
//  MOSI = 6

// ESP32-C3 SPI:
//  SCK  = 4
//  MISO = 5
//  MOSI = 6


#  if CONFIG_IDF_TARGET_ESP32S3   // ESP32-S3
#define VSPI_FSPI_SHORT_STRING "FSPI"
#  elif CONFIG_IDF_TARGET_ESP32S2   // ESP32-S2
#define VSPI_FSPI_SHORT_STRING "FSPI"
#  elif CONFIG_IDF_TARGET_ESP32C2 // ESP32-C2
#define VSPI_FSPI_SHORT_STRING "FSPI"
#  elif CONFIG_IDF_TARGET_ESP32C3 // ESP32-C3
#define VSPI_FSPI_SHORT_STRING "SPI"
#  elif CONFIG_IDF_TARGET_ESP32C6 // ESP32-C6
#define VSPI_FSPI_SHORT_STRING "FSPI"
#  elif CONFIG_IDF_TARGET_ESP32   // ESP32/PICO-D4
#define VSPI_FSPI_SHORT_STRING "VSPI"

#  else // if CONFIG_IDF_TARGET_ESP32S2
#   error Target CONFIG_IDF_TARGET is not supported
#  endif // if CONFIG_IDF_TARGET_ESP32S2


const __FlashStringHelper* getSPI_optionToString(SPI_Options_e option) {
  switch (option) {
    case SPI_Options_e::None:
      return F("Disabled");
    case SPI_Options_e::Vspi_Fspi:
      return F(
        VSPI_FSPI_SHORT_STRING 
        ": CLK=GPIO-" STRINGIFY(VSPI_FSPI_SCK) 
        ", MISO=GPIO-" STRINGIFY(VSPI_FSPI_MISO) 
        ", MOSI=GPIO-" STRINGIFY(VSPI_FSPI_MOSI) );
#ifdef ESP32_CLASSIC
    case SPI_Options_e::Hspi:
      return F(
        "HSPI"
        ": CLK=GPIO-" STRINGIFY(HSPI_SCLK) 
        ", MISO=GPIO-" STRINGIFY(HSPI_MISO) 
        ", MOSI=GPIO-" STRINGIFY(HSPI_MOSI) );

      
#endif
    case SPI_Options_e::UserDefined:
      return F("User-defined: CLK, MISO, MOSI GPIO-pins");
  }
  return F("Unknown");
}

const __FlashStringHelper* getSPI_optionToShortString(SPI_Options_e option) {
  switch (option) {
    case SPI_Options_e::None:
      return F("Disabled");
    case SPI_Options_e::Vspi_Fspi:
      return F(VSPI_FSPI_SHORT_STRING);
#ifdef ESP32_CLASSIC
    case SPI_Options_e::Hspi:
      return F("HSPI");
#endif
    case SPI_Options_e::UserDefined:
      return F("User-defined SPI");
  }
  return F("Unknown");
}

#endif // ifdef ESP32
