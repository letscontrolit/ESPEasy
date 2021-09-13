#include "../DataTypes/SPI_options.h"

#ifdef ESP32
const __FlashStringHelper* getSPI_optionToString(SPI_Options_e option) {
  switch (option) {
    case SPI_Options_e::None:
      return F("Disabled");
    case SPI_Options_e::Vspi:
      return F("VSPI: CLK=GPIO-18, MISO=GPIO-19, MOSI=GPIO-23");
    case SPI_Options_e::Hspi:
      return F("HSPI: CLK=GPIO-14, MISO=GPIO-12, MOSI=GPIO-13");
    case SPI_Options_e::UserDefined:
      return F("User-defined: CLK, MISO, MOSI GPIO-pins");
  }
}

const __FlashStringHelper* getSPI_optionToShortString(SPI_Options_e option) {
  switch (option) {
    case SPI_Options_e::None:
      return F("Disabled");
    case SPI_Options_e::Vspi:
      return F("VSPI");
    case SPI_Options_e::Hspi:
      return F("HSPI");
    case SPI_Options_e::UserDefined:
      return F("User-defined SPI");
  }
}

#endif // ifdef ESP32
