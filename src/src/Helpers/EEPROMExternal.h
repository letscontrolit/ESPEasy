#pragma once
#include "../../ESPEasy_common.h"

#if FEATURE_EEPROM_EXTERNAL

#include "../DataTypes/TaskIndex.h"
#include "../DataStructs/RTCCacheStruct.h"

# include <AT24CX.h>

enum class EEPROMExternal_WriteProtect_e : uint8_t {
  Undefined = 0,
  ReadWrite = 1,
  ReadOnly  = 2,
};

extern AT24CX *EEPROMExternal;
extern EEPROMExternal_WriteProtect_e EEPROMExternalWriteProtect;

// Start writing the base RTC struct from this offset (not currently saving this to EEPROM) // TODO
# define EEPROM_BASERTC_START_OFFSET      (0)

// Start writing the UserVar values from this offset, should be > sizeof(RTCStruct) that is 32 currently
# define EEPROM_USERVAR_START_OFFSET      (EEPROM_BASERTC_START_OFFSET + 128)

// Write the UserVar-checksum from this offset, right after the UserVar values
# define EEPROM_USERVAR_CHECKSUM_OFFSET   (EEPROM_USERVAR_START_OFFSET + (TASKS_MAX * VARS_PER_TASK * sizeof(uint32_t)))

// Even when not enabled, we still reserve the space
# define EEPROM_RTC_CACHE_START_OFFSET    (EEPROM_USERVAR_CHECKSUM_OFFSET + sizeof(uint32_t))
# define EEPROM_RTC_CACHE_WRITEPOS_OFFSET (EEPROM_RTC_CACHE_START_OFFSET + RTC_CACHE_DATA_SIZE)
# define EEPROM_RTC_CACHE_META_OFFSET     (EEPROM_RTC_CACHE_WRITEPOS_OFFSET + sizeof(uint16_t))
# define EEPROM_RTC_CACHE_META_CRC_OFFSET (EEPROM_RTC_CACHE_META_OFFSET + sizeof(RTC_cache_struct))
# define EEPROM_RTC_CACHE_CHECKSUM_OFFSET (EEPROM_RTC_CACHE_META_CRC_OFFSET + sizeof(uint32_t))

// Offset for storing GPIO states, directly following the RTC Cache storage and checksum
# define EEPROM_GPIO_MCPPCF_START_OFFSET  (EEPROM_RTC_CACHE_CHECKSUM_OFFSET + sizeof(uint32_t))

// Choose an arbitrary but fixed offset
# define EEPROM_CUSTOM_START_OFFSET       (2048)

// NB: Only first half of EEPROM_CUSTOM_START_OFFSET available for slots when String Variables feature enabled!
# if FEATURE_STRING_VARIABLES
#  define EEPROM_CUSTOM_DIVISOR           (2) // Split in slots- and strings- halves
# else // if FEATURE_STRING_VARIABLES
#  define EEPROM_CUSTOM_DIVISOR           (1) // Use all for slots
# endif // if FEATURE_STRING_VARIABLES

// Enable/disable some models
# define EEPROM_SUPPORT_AT24C1024 1
# define EEPROM_SUPPORT_AT24C2048 0

// Supported AT24Cxxx devices
enum class EEPROMExternal_Type_e : uint8_t {
  AT24C256 = 0,  // Default, 32 kB
  AT24C512 = 1,  // 64 kB
  # if EEPROM_SUPPORT_AT24C1024
  AT24C1024 = 2, // 128 kB
  # endif // if EEPROM_SUPPORT_AT24C1024
  # if EEPROM_SUPPORT_AT24C2048
  AT24C2048 = 3, // 256 kB (not supported yet)
  # endif // if EEPROM_SUPPORT_AT24C2048
  AT24C32  = 4,  // 4 kB, not endorsed, but widely available types:
  AT24C64  = 5,  // 8 kB
  AT24C128 = 6,  // 16 kB
};

uint8_t                       checkEEPROMEnabled();
EEPROMExternal_WriteProtect_e checkEEPROMExternalWriteProtected(bool forced = false);
bool                          isEEPROMExternalWriteProtected();

uint8_t                       selectEEPROMI2CBusAndMultiplexer();

uint32_t                      getEEPROMSize(EEPROMExternal_Type_e type);
uint32_t                      getEEPROMSize(EEPROMExternal_Type_e type,
                                            uint8_t             & pageSize);
const __FlashStringHelper*    getEEPROMName(EEPROMExternal_Type_e type);

uint32_t                      getEEPROMAddressForSlot(uint32_t slot);
uint32_t                      getEEPROMAddressForTaskValue(taskIndex_t    task,
                                                           taskVarIndex_t varNr);
uint32_t                      getEEPROMMaxSlots();
bool                          writeEEPROMSlot(uint32_t slot,
                                              float    data);
float                         readEEPROMSlot(uint32_t slot);
#endif // if FEATURE_EEPROM_EXTERNAL
