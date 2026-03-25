#pragma once

#include "../../ESPEasy_common.h"

#if FEATURE_SPI

// Forward declaration
class SPIClass;


void initializeSPIBuses();

SPIClass* getSPI(uint8_t spi_bus);

#if FEATURE_SD
bool initSDcard();
#endif // if FEATURE_SD
#endif