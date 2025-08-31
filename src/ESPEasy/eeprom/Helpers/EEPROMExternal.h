#pragma once
#include "../../../ESPEasy_common.h"

#if FEATURE_EEPROM_EXTERNAL

# include "../../../src/DataTypes/TaskIndex.h"
# include "../../../src/DataStructs/PortStatusStruct.h"
# include "../../../src/DataStructs/RTCCacheStruct.h"
# include "../../../src/DataStructs/RTCStruct.h"
# include "../../../src/Helpers/LongTermTimer.h"
# include <map>

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

# define EEPROM_PARAMS_CURRENT_VERSION    (1)  // Let's start with version 1
# define EEPROM_PINSTATE_ELEMENT_COUNT    (64) // Number of PinState elements (16 bytes each) to store in EEPROM

// Start writing the base RTC struct from this offset (not currently saving this to EEPROM) // TODO
# define EEPROM_BASERTC_START_OFFSET      (0)

// Some system parameters to check before restoring anything
# define EEPROM_PARAMS_VERSION_ADDRESS    (32)
# define EEPROM_PARAMS_TASKS_MAX          (EEPROM_PARAMS_VERSION_ADDRESS + sizeof(uint16_t))
# define EEPROM_PARAMS_VARS_PER_TASK      (EEPROM_PARAMS_TASKS_MAX + sizeof(uint16_t))
# define EEPROM_PARAMS_RTC_CACHE_ADDRESS  (EEPROM_PARAMS_VARS_PER_TASK + sizeof(uint16_t))
# define EEPROM_PARAMS_PINSTATE_ADDRESS   (EEPROM_PARAMS_RTC_CACHE_ADDRESS + sizeof(uint32_t))

// Start writing the UserVar values from this offset, should be > sizeof(RTCStruct) that is 32 currently
# define EEPROM_USERVAR_START_OFFSET      (EEPROM_BASERTC_START_OFFSET + 128)

// Write the UserVar-checksum from this offset, right after the UserVar values
# define EEPROM_USERVAR_CHECKSUM_OFFSET   (EEPROM_USERVAR_START_OFFSET + (TASKS_MAX * VARS_PER_TASK * sizeof(uint32_t)))

# if FEATURE_RTC_CACHE_STORAGE

// Even when not enabled, we still reserve the space
#  define EEPROM_RTC_CACHE_START_OFFSET    (EEPROM_USERVAR_CHECKSUM_OFFSET + sizeof(uint32_t))
#  define EEPROM_RTC_CACHE_WRITEPOS_OFFSET (EEPROM_RTC_CACHE_START_OFFSET + RTC_CACHE_DATA_SIZE)
#  define EEPROM_RTC_CACHE_META_OFFSET     (EEPROM_RTC_CACHE_WRITEPOS_OFFSET + sizeof(uint16_t))
#  define EEPROM_RTC_CACHE_META_CRC_OFFSET (EEPROM_RTC_CACHE_META_OFFSET + sizeof(RTC_cache_struct))
#  define EEPROM_RTC_CACHE_CHECKSUM_OFFSET (EEPROM_RTC_CACHE_META_CRC_OFFSET + sizeof(uint32_t))
# endif // if FEATURE_RTC_CACHE_STORAGE

// Offset for storing GPIO pinstates, directly following the RTC Cache storage and checksum
# if FEATURE_RTC_CACHE_STORAGE
#  define EEPROM_GPIO_PINSTATE_START_OFFSET  (EEPROM_RTC_CACHE_CHECKSUM_OFFSET + sizeof(uint32_t))
# else // if FEATURE_RTC_CACHE_STORAGE
#  define EEPROM_GPIO_PINSTATE_START_OFFSET  (EEPROM_USERVAR_CHECKSUM_OFFSET + sizeof(uint32_t))
# endif // if FEATURE_RTC_CACHE_STORAGE
# define EEPROM_PINSTATE_ELEMENT_SIZE     (sizeof(uint32_t) + sizeof(portStatusStruct))
# define EEPROM_GPIO_PINSTATE_END_OFFSET  (EEPROM_GPIO_PINSTATE_START_OFFSET + (EEPROM_PINSTATE_ELEMENT_COUNT * EEPROM_PINSTATE_ELEMENT_SIZE))

// Choose an arbitrary but fixed offset
# define EEPROM_CUSTOM_START_OFFSET       (2048)

// NB: Only first half of EEPROM_CUSTOM_START_OFFSET available for slots when String Variables feature enabled!
# if FEATURE_STRING_VARIABLES
#  define EEPROM_CUSTOM_DIVISOR           (2u) // Split in slots- and strings- halves
# else // if FEATURE_STRING_VARIABLES
#  define EEPROM_CUSTOM_DIVISOR           (1u) // Use all for slots
# endif // if FEATURE_STRING_VARIABLES

// Enable/disable some models
# define EEPROM_SUPPORT_AT24C1024 1
# define EEPROM_SUPPORT_AT24C2048 0

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

bool                          validateEEPROMExternalParameters();
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
uint32_t                      getEEPROMAddressForTaskValue(taskIndex_t    task,
                                                           taskVarIndex_t varNr);
uint32_t                      getEEPROMMaxSlots();
bool                          writeEEPROMSlot(uint32_t slot,
                                              float    data);
float                         readEEPROMSlot(uint32_t slot);

# if FEATURE_EEPROM_BACKGROUND
enum class EEPROMExternalTaskState_e : uint8_t {
  Available  = 0,
  Starting   = 1,
  Processing = 2,
  Ready      = 3,
  Error      = 4,
};

enum class EEPROMExternalTaskType_e : uint8_t {
  None       = 0,
  UserVars   = 1,
  ValueSlots = 2,
  C016Caches = 3,
  PinStates  = 4,
};

struct EEPROMExternalTaskData {
  EEPROMExternalTaskState_e status = EEPROMExternalTaskState_e::Available;
  EEPROMExternalTaskType_e  type   = EEPROMExternalTaskType_e::None;
  uint32_t                  (*function)() = nullptr;
  LongTermTimer             timer;
  uint32_t                  duration{};
  int32_t                   data{};

  #  if FEATURE_EEPROM_RTOS_TASK

  // This is C-code, so not set to nullptr, but to NULL
  TaskHandle_t taskHandle = NULL;
  #  endif // if FEATURE_EEPROM_RTOS_TASK
};

const __FlashStringHelper* TaskDataTypeToString(EEPROMExternalTaskType_e type);
bool                       EEPROMAddTask(EEPROMExternalTaskType_e type,
                                         EEPROMExternalTaskData   taskData);
bool                       EEPROMExternalLoop();

extern std::map<EEPROMExternalTaskType_e, EEPROMExternalTaskData> EEPROMTaskMap;
extern uint16_t EEPROMSaveDelaySeconds;

# endif // if FEATURE_EEPROM_BACKGROUND
} // namespace eeprom
} // namespace ESPEasy
#endif // if FEATURE_EEPROM_EXTERNAL
