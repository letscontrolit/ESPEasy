#pragma once
#include "../../ESPEasy_common.h"

#if FEATURE_EEPROM_EXTERNAL

# include <AT24CX.h>

extern AT24CX *EEPROMExternal;

// Start writing the base RTC struct from this offset (not currently saving this to EEPROM) // TODO
# define EEPROM_BASERTC_START_OFFSET      (0)

// Start writing the UserVar values from this offset, should be > sizeof(RTCStruct) that is 32 currently
# define EEPROM_USERVAR_START_OFFSET      (100)

// Write the UserVar-checksum from this offset, right after the UserVar values
# define EEPROM_USERVAR_CHECKSUM_OFFSET   (EEPROM_USERVAR_START_OFFSET + (TASKS_MAX * VARS_PER_TASK * sizeof(uint32_t)))

// Offset for storing GPIO states // TODO
# define EEPROM_GPIO_MCPPCF_START_OFFSET  (1024)

// Choose an arbitrary but fixed offset
# define EEPROM_CUSTOM_START_OFFSET       (2048)

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

uint8_t                    selectEEPROMI2CBusAndMultiplexer();

uint32_t                   getEEPROMSize(EEPROMExternal_Type_e type);
uint32_t                   getEEPROMSize(EEPROMExternal_Type_e type,
                                         uint8_t             & pageSize);
const __FlashStringHelper* getEEPROMName(EEPROMExternal_Type_e type);
#endif // if FEATURE_EEPROM_EXTERNAL
