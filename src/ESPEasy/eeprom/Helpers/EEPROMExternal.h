#pragma once
#include "../../../ESPEasy_common.h"

#if FEATURE_EEPROM_EXTERNAL

# include "../../../src/DataTypes/TaskIndex.h"
# include "../../../src/Helpers/LongTermTimer.h"

# include <AT24CX.h>

namespace ESPEasy {
namespace eeprom {
enum class EEPROMExternal_WriteProtect_e : uint8_t {
  Undefined = 0,
  ReadWrite = 1,
  ReadOnly  = 2,

};

extern AT24CX *EEPROMExternal;
extern EEPROMExternal_WriteProtect_e EEPROMExternalWriteProtect;
extern bool EEPROMParamsOkState;
extern LongTermTimer EEPROMParamsOkTimer;

# define EEPROM_PARAMS_CURRENT_VERSION    (1) // Let's start with version 1

// Start from this offset
# define EEPROM_BASERTC_START_OFFSET      (0)

// Some system parameters to check before restoring anything
# define EEPROM_PARAMS_VERSION_ADDRESS    (32)

// Start writing the Custom slot values from this offset so we have some room for settings, if needed
# define EEPROM_CUSTOM_START_OFFSET       (EEPROM_BASERTC_START_OFFSET + 128)
# define EEPROM_CUSTOM_DIVISOR            (1u) // Use all for slots

// Enable/disable some models
# define EEPROM_SUPPORT_AT24C1024 1
# define EEPROM_SUPPORT_AT24C2048 0

# define EEPROM_PARAMSOK_STATE_TIMEOUT    (180000) // 3 minutes

// Supported AT24Cxxx and MB85RCxxx devices
enum class EEPROMExternal_Type_e : uint8_t {
  AT24C256 = 0,   // Default, 32 kB
  AT24C512 = 1,   // 64 kB
  # if EEPROM_SUPPORT_AT24C1024
  AT24C1024 = 2,  // 128 kB
  # endif // if EEPROM_SUPPORT_AT24C1024
  # if EEPROM_SUPPORT_AT24C2048
  AT24C2048 = 3,  // 256 kB (not supported yet)
  # endif // if EEPROM_SUPPORT_AT24C2048
  AT24C32   = 4,  // 4 kB, not endorsed, but widely available
  AT24C64   = 5,  // 8 kB
  AT24C128  = 6,  // 16 kB
  MB85RC256 = 7,  // 32 kB
  MB85RC512 = 8,  // 64 kB
  # if EEPROM_SUPPORT_AT24C1024
  MB85RC1M = 9,   // 128 kB
  # endif // if EEPROM_SUPPORT_AT24C1024
  # if EEPROM_SUPPORT_AT24C2048
  MB85RC2M = 10,  // 256 kB (not supported yet)
  # endif // if EEPROM_SUPPORT_AT24C2048
  MB85RC32  = 11, // 4 kB, not endorsed, possibly not available
  MB85RC64  = 12, // 8 kB
  MB85RC128 = 13, // 16 kB

};

void                          initializeEEPROMExternal();

bool                          validateEEPROMExternalParameters(bool force = false);
void                          updateEEPROMExternalParameters();

uint8_t                       checkEEPROMEnabled();
EEPROMExternal_WriteProtect_e checkEEPROMExternalWriteProtected(bool forced = false);
bool                          isEEPROMExternalWriteProtected();

uint8_t                       selectEEPROMI2CBusAndMultiplexer();

uint32_t                      getEEPROMSize(EEPROMExternal_Type_e type);
uint32_t                      getEEPROMSize(EEPROMExternal_Type_e type,
                                            uint8_t             & pageSize);
const __FlashStringHelper*    getEEPROMName(EEPROMExternal_Type_e type);

uint32_t                      getEEPROMAddressForSlot(uint32_t slot);

uint32_t                      getEEPROMMaxSlots();

bool                          writeEEPROMSlot(uint32_t                 slot,
                                              ESPEASY_RULES_FLOAT_TYPE data);
ESPEASY_RULES_FLOAT_TYPE      readEEPROMSlot(uint32_t slot);

} // namespace eeprom
} // namespace ESPEasy
#endif // if FEATURE_EEPROM_EXTERNAL
