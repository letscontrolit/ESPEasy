#pragma once

#include "../../ESPEasy_common.h"

#if FEATURE_SPI
void initializeSPIBuses();

#if FEATURE_SD
bool initSDcard();
#endif // if FEATURE_SD
#endif