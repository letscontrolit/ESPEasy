#ifndef DATATYPES_SPI_OPTIONS_H
#define DATATYPES_SPI_OPTIONS_H

#include <Arduino.h>

enum class SPI_Options_e { // Do not change values as this is stored in the settings!
  None        = 0,
  Vspi        = 1,
  Hspi        = 2,
  UserDefined = 9 // Leave some room for other, possible future, hardware-related options
};

#ifdef ESP32
const __FlashStringHelper* getSPI_optionToString(SPI_Options_e option);
const __FlashStringHelper* getSPI_optionToShortString(SPI_Options_e option);
#endif // ifdef ESP32

#endif // ifndef DATATYPES_SPI_OPTIONS_H
