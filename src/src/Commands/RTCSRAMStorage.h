#pragma once

#include "../../ESPEasy_common.h"

#if FEATURE_RTC_SRAM_STORAGE
const __FlashStringHelper* Command_writeRTC(struct EventStruct *event,
                                            const char         *Line);
#endif // if FEATURE_RTC_SRAM_STORAGE
