#pragma once

#if FEATURE_EEPROM_EXTERNAL || FEATURE_RTC_SRAM_STORAGE
# include "../WebServer/common.h"

void handle_eepromvars();

#endif // if FEATURE_EEPROM_EXTERNAL || FEATURE_RTC_SRAM_STORAGE
