#pragma once

# include "../WebServer/common.h"
#if FEATURE_EEPROM_EXTERNAL || FEATURE_RTC_SRAM_STORAGE

void handle_eepromvars();

#endif // if FEATURE_EEPROM_EXTERNAL || FEATURE_RTC_SRAM_STORAGE
