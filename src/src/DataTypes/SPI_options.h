#ifndef DATASTRUCTS_SPI_OPTIONS_H
#define DATASTRUCTS_SPI_OPTIONS_H

#include <Arduino.h>

enum class SPI_Options_e {
  None = 0,
  Vspi,
  Hspi,
  UserDefined
};

#ifdef ESP32
const __FlashStringHelper* getSPI_optionToString(SPI_Options_e option);
const __FlashStringHelper* getSPI_optionToShortString(SPI_Options_e option);
#endif // ifdef ESP32

#endif // ifndef DATASTRUCTS_SPI_OPTIONS_H
