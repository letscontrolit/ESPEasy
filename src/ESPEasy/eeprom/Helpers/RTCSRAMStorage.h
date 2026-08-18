#pragma once
#include "../../../ESPEasy_common.h"

#if FEATURE_RTC_SRAM_STORAGE

# include "../../../src/DataTypes/TaskIndex.h"

namespace ESPEasy {
namespace eeprom {

# if FEATURE_SRAM_STORAGE_DOUBLE
  #  define SRAM_STORAGE_FLOAT_TYPE double
# else
  #  define SRAM_STORAGE_FLOAT_TYPE float
# endif // if FEATURE_SRAM_STORAGE_DOUBLE

uint8_t                 checkRTCSRAMEnabled();
uint8_t                 selectRTCSRAMI2CBus();

uint32_t                getRTCSRAMSize();
uint32_t                getRTCSRAMAddressForSlot(uint32_t slot);
uint32_t                getRTCSRAMMaxSlots();

bool                    writeRTCSRAMSlot(uint32_t                slot,
                                         SRAM_STORAGE_FLOAT_TYPE data);
SRAM_STORAGE_FLOAT_TYPE readRTCSRAMSlot(uint32_t slot);

} // namespace eeprom
} // namespace ESPEasy
#endif // if FEATURE_RTC_SRAM_STORAGE
