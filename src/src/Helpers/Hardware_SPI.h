#pragma once

#include "../../ESPEasy_common.h"

#if FEATURE_SPI

#include "../DataTypes/TaskIndex.h"

// Forward declaration
class SPIClass;


void initializeSPIBuses();

SPIClass* getSPI(uint8_t spi_bus);
bool getSPI(SPIClass& spi, uint8_t spi_bus);
SPIClass* getSPIBusForTask(taskIndex_t TaskIndex);

#if FEATURE_SD
bool initSDcard();
#endif // if FEATURE_SD
#endif