#pragma once

#include "../../ESPEasy_common.h"


void initializeSPIBuses();

#if FEATURE_SD
bool initSDcard();
#endif // if FEATURE_SD
