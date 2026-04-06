#include "../DataTypes/SPI_options.h"


#ifdef ESP32

#include "../Helpers/Hardware_device_info.h"

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
#define VSPI_FSPI_SHORT_STRING "FSPI (" STRINGIFY(FSPI_HOST) ")"
#  elif CONFIG_IDF_TARGET_ESP32S2   // ESP32-S2
#define VSPI_FSPI_SHORT_STRING "FSPI (" STRINGIFY(FSPI_HOST) ")"
#elif CONFIG_IDF_TARGET_ESP32C2 || CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C5 || CONFIG_IDF_TARGET_ESP32C6 || CONFIG_IDF_TARGET_ESP32C61 || CONFIG_IDF_TARGET_ESP32P4
#  if SOC_SPI_PERIPH_NUM > 2
#define VSPI_FSPI_SHORT_STRING "FSPI (" STRINGIFY(VSPI_HOST) ")"
#  else
#define VSPI_FSPI_SHORT_STRING "SPI" // STRINGIFY(VSPI_HOST)
#  endif

#  elif CONFIG_IDF_TARGET_ESP32   // ESP32/PICO-D4
#define VSPI_FSPI_SHORT_STRING "VSPI (" STRINGIFY(VSPI_HOST) ")"

#  else // if CONFIG_IDF_TARGET_ESP32S2
#   error Target CONFIG_IDF_TARGET is not supported
#  endif // if CONFIG_IDF_TARGET_ESP32S2

#if SOC_SPI_PERIPH_NUM > 2
#ifdef ESP32_CLASSIC
#define HSPI_SHORT_STRING "HSPI (" STRINGIFY(HSPI_HOST) ")"
#else
#define HSPI_SHORT_STRING  STRINGIFY(HSPI_HOST)
#endif
#endif


const __FlashStringHelper* getSPI_optionToString(SPI_Options_e option) {
  switch (option) {
    case SPI_Options_e::None:
      return F("Disabled");
    case SPI_Options_e::Vspi_Fspi:
      return F(
        VSPI_FSPI_SHORT_STRING 
        ": CLK=" STRINGIFY(VSPI_FSPI_SCK) 
        ", MISO=" STRINGIFY(VSPI_FSPI_MISO) 
        ", MOSI=" STRINGIFY(VSPI_FSPI_MOSI) );
#ifdef ESP32_CLASSIC
    case SPI_Options_e::Hspi:
      return F(
        HSPI_SHORT_STRING
        ": CLK=" STRINGIFY(HSPI_SCLK) 
        ", MISO=" STRINGIFY(HSPI_MISO) 
        ", MOSI=" STRINGIFY(HSPI_MOSI) );

      
#endif
    case SPI_Options_e::UserDefined_VSPI:
      return F("User-defined " VSPI_FSPI_SHORT_STRING);
#if SOC_SPI_PERIPH_NUM > 2
    case SPI_Options_e::UserDefined_HSPI:
      return F("User-defined " HSPI_SHORT_STRING);
#endif
  }
  return F("Unknown");
}

const __FlashStringHelper* get_vspi_fspi_str()
{
  return F(VSPI_FSPI_SHORT_STRING);
}

const String getSPI_optionToShortString(SPI_Options_e option, uint8_t spi_bus) {
#ifdef ESP32
  String res;
  switch (option) {
    case SPI_Options_e::None:
      return F("Disabled");
    case SPI_Options_e::Vspi_Fspi:
      res = F(VSPI_FSPI_SHORT_STRING);
      break;
#ifdef ESP32_CLASSIC
    case SPI_Options_e::Hspi:
      res = F("HSPI");
      break;
#endif
    case SPI_Options_e::UserDefined_VSPI:
      res = F("User-defined " VSPI_FSPI_SHORT_STRING);
      break;
#if SOC_SPI_PERIPH_NUM > 2
    case SPI_Options_e::UserDefined_HSPI:
      res = F("User-defined " HSPI_SHORT_STRING);
      break;
#endif
  }
  if (!res.isEmpty()) {
    if (getSPIBusCount() > 1) {
      return concat(res + F(" bus "), spi_bus);
    }
    return res;
  }
#else
  switch (option) {
    case SPI_Options_e::None:
      return F("Disabled");
    case SPI_Options_e::Vspi_Fspi:
      return F(VSPI_FSPI_SHORT_STRING);
    case SPI_Options_e::UserDefined_VSPI:
      return F("User-defined SPI");
  }
#endif
  return F("Unknown");
}

#endif // ifdef ESP32
