#pragma once

#include "../../ESPEasy_common.h"

#if FEATURE_EEPROM_EXTERNAL
const __FlashStringHelper* Command_writeEE(struct EventStruct *event,
                                           const char         *Line);
#endif // if FEATURE_EEPROM_EXTERNAL
