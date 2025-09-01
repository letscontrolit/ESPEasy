#include "../Helpers/EEPROMExternal.h"
#include "../../../src/Globals/Settings.h"
#include "../../../src/Helpers/I2C_access.h"
#include "../../../ESPEasy_common.h"
#include "../../../src/Helpers/StringConverter.h"

#if FEATURE_EEPROM_EXTERNAL

namespace ESPEasy {
namespace eeprom {
AT24CX *EEPROMExternal                                   = nullptr;
EEPROMExternal_WriteProtect_e EEPROMExternalWriteProtect = EEPROMExternal_WriteProtect_e::Undefined;
bool EEPROMParamsOkState{};
LongTermTimer EEPROMParamsOkTimer;

constexpr uint32_t sizeof_uint32_t = sizeof(uint32_t);

/**
 * Initialize the external EEPROM device and variables
 */
void initializeEEPROMExternal() {
  const uint8_t eepromAddress = Settings.EEPROMExternalI2CAddress();

  if ((nullptr != ESPEasy::eeprom::EEPROMExternal) || (eepromAddress == 0)) { // Cleanup when turning off EEPROM
    delete ESPEasy::eeprom::EEPROMExternal;
    ESPEasy::eeprom::EEPROMExternal             = nullptr;
    ESPEasy::eeprom::EEPROMExternalWriteProtect = ESPEasy::eeprom::EEPROMExternal_WriteProtect_e::Undefined;
  }

  if ((nullptr == ESPEasy::eeprom::EEPROMExternal) && (eepromAddress > 0)) {
    const ESPEasy::eeprom::EEPROMExternal_Type_e eepromType =
      static_cast<ESPEasy::eeprom::EEPROMExternal_Type_e>(Settings.EEPROMExternalType());

    if (0 != ESPEasy::eeprom::selectEEPROMI2CBusAndMultiplexer()) { // Switch to I2C Bus and multiplexer channel of External EEPROM
      // We have an I2C device at this address, let's assume it's an EEPROM...
      uint8_t pageSize          = 0;
      const uint32_t eepromSize = ESPEasy::eeprom::getEEPROMSize(eepromType, pageSize);
      ESPEasy::eeprom::EEPROMExternal = new (std::nothrow) AT24CX(eepromAddress, pageSize, eepromSize);

      if (nullptr != ESPEasy::eeprom::EEPROMExternal) {
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, strformat(F("EEPROM: %s initialized at address 0x%02x"),
                                           FsP(ESPEasy::eeprom::getEEPROMName(eepromType)),
                                           eepromAddress));
        }

        ESPEasy::eeprom::checkEEPROMExternalWriteProtected();

        if (ESPEasy::eeprom::isEEPROMExternalWriteProtected()) {
          addLog(LOG_LEVEL_INFO, concat(F("EEPROM: Write-protected! Status: "),
                                        static_cast<uint8_t>(ESPEasy::eeprom::checkEEPROMExternalWriteProtected())));
        }
      } else {
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          addLog(LOG_LEVEL_ERROR, strformat(F("EEPROM: Initialization of %s failed"),
                                            FsP(ESPEasy::eeprom::getEEPROMName(eepromType))));
        }
      }
    } else {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        addLog(LOG_LEVEL_ERROR, strformat(F("EEPROM: No %s found at address 0x%02x"),
                                          FsP(ESPEasy::eeprom::getEEPROMName(eepromType)),
                                          eepromAddress));
      }
    }

    # if FEATURE_I2CMULTIPLEXER
    I2CMultiplexerOff(
      #  if FEATURE_I2C_MULTIPLE
      Settings.getI2CInterfaceEEPROM()
      #  else // if FEATURE_I2C_MULTIPLE
      0
      #  endif // if FEATURE_I2C_MULTIPLE
      ); // Restore the Multiplexer channel
    # endif // if FEATURE_I2CMULTIPLEXER
  }
}

/**
 * Check the stored parameters in the EEPROM with current data and settings
 * - Version
 * - Max. tasks
 * - Vars per tasks
 * - RTC Cache address
 * - Pinstate address
 */
bool validateEEPROMExternalParameters(bool force) {
  if (!force && (EEPROMParamsOkTimer.millisPassedSince() < EEPROM_PARAMSOK_STATE_TIMEOUT)) { // When called within timeout return cached
                                                                                             // result
    return EEPROMParamsOkState;
  }
  const uint16_t eepromVersionParam     = EEPROMExternal->readInt(EEPROM_PARAMS_VERSION_ADDRESS);
  const uint16_t eepromTasksMaxParam    = EEPROMExternal->readInt(EEPROM_PARAMS_TASKS_MAX);
  const uint16_t eepromVarsPerTaskParam = EEPROMExternal->readInt(EEPROM_PARAMS_VARS_PER_TASK);
  const uint32_t eepromPinstateParam    = EEPROMExternal->readLong(EEPROM_PARAMS_PINSTATE_ADDRESS);

  EEPROMParamsOkTimer.setNow();
  EEPROMParamsOkState = false;

  # if FEATURE_RTC_CACHE_STORAGE
  const uint32_t eepromRtcCacheParam = EEPROMExternal->readLong(EEPROM_PARAMS_RTC_CACHE_ADDRESS);
  # endif // if FEATURE_RTC_CACHE_STORAGE

  if ((EEPROM_PARAMS_CURRENT_VERSION == eepromVersionParam) &&
      (TASKS_MAX == eepromTasksMaxParam) &&
      (VARS_PER_TASK == eepromVarsPerTaskParam) &&
      # if FEATURE_RTC_CACHE_STORAGE
      (EEPROM_RTC_CACHE_START_OFFSET == eepromRtcCacheParam) &&
      # endif // if FEATURE_RTC_CACHE_STORAGE
      (EEPROM_GPIO_PINSTATE_START_OFFSET == eepromPinstateParam) &&
      (EEPROM_GPIO_PINSTATE_END_OFFSET <= EEPROM_CUSTOM_START_OFFSET)) {
    EEPROMParamsOkState = true;
  }

  return EEPROMParamsOkState;
}

/**
 * Update the stored parameters in EEPROM
 * - Version
 * - Max. tasks
 * - Vars per tasks
 * - RTC Cache address
 * - Pinstate address
 */
void updateEEPROMExternalParameters() {
  const uint16_t eepromVersionParam     = EEPROMExternal->readInt(EEPROM_PARAMS_VERSION_ADDRESS);
  const uint16_t eepromTasksMaxParam    = EEPROMExternal->readInt(EEPROM_PARAMS_TASKS_MAX);
  const uint16_t eepromVarsPerTaskParam = EEPROMExternal->readInt(EEPROM_PARAMS_VARS_PER_TASK);
  const uint32_t eepromPinstateParam    = EEPROMExternal->readLong(EEPROM_PARAMS_PINSTATE_ADDRESS);

  # if FEATURE_RTC_CACHE_STORAGE
  const uint32_t eepromRtcCacheParam = EEPROMExternal->readLong(EEPROM_PARAMS_RTC_CACHE_ADDRESS);
  # endif // if FEATURE_RTC_CACHE_STORAGE

  if (EEPROM_PARAMS_CURRENT_VERSION != eepromVersionParam) {
    EEPROMExternal->writeInt(EEPROM_PARAMS_VERSION_ADDRESS, EEPROM_PARAMS_CURRENT_VERSION);
  }

  if (TASKS_MAX != eepromTasksMaxParam) {
    EEPROMExternal->writeInt(EEPROM_PARAMS_TASKS_MAX, TASKS_MAX);
  }

  if (VARS_PER_TASK != eepromVarsPerTaskParam) {
    EEPROMExternal->writeInt(EEPROM_PARAMS_VARS_PER_TASK, VARS_PER_TASK);
  }

  # if FEATURE_RTC_CACHE_STORAGE

  if (EEPROM_RTC_CACHE_START_OFFSET != eepromRtcCacheParam) {
    EEPROMExternal->writeLong(EEPROM_PARAMS_RTC_CACHE_ADDRESS, EEPROM_RTC_CACHE_START_OFFSET);
  }
  # endif // if FEATURE_RTC_CACHE_STORAGE

  if (EEPROM_GPIO_PINSTATE_START_OFFSET != eepromPinstateParam) {
    EEPROMExternal->writeLong(EEPROM_PARAMS_PINSTATE_ADDRESS, EEPROM_GPIO_PINSTATE_START_OFFSET);
  }
}

/**
 * Check if the EEPROM is properly initialized and enabled.
 * Returns the I2C address if all is OK
 */
uint8_t checkEEPROMEnabled() {
  const uint8_t eepromAddress = Settings.EEPROMExternalI2CAddress();

  if ((nullptr != EEPROMExternal) && (eepromAddress > 0)) { // EEPROM Configured?
    return eepromAddress;
  }
  return 0;
}

/**
 * Check if the EEPROM is write-protected
 * when forced = false only detect if current state is Undefined
 * - read a random byte in the first half of the address space (some chips ony WP the first half of the space!)
 * - write 0xAA and read back -> if unequal: read-only
 * - write 0x55 and read back -> if unequal: read-only
 * - Still OK:
 *   - restore original byte
 *   - Set status read-write
 */
EEPROMExternal_WriteProtect_e checkEEPROMExternalWriteProtected(bool forced) {
  if ((nullptr != EEPROMExternal) && ((EEPROMExternal_WriteProtect_e::Undefined == EEPROMExternalWriteProtect) || forced)) {
    const uint32_t addr     = random(0, getEEPROMSize(static_cast<EEPROMExternal_Type_e>(Settings.EEPROMExternalType())) / 2);
    const uint8_t  original = EEPROMExternal->read(addr);
    # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLog(LOG_LEVEL_DEBUG, strformat(F("EEPROM: Writeable check, addr: 0x%04x data: 0x%02X"), addr, original));
    }
    # endif // ifndef BUILD_NO_DEBUG
    EEPROMExternal->write(addr, 0xAA);
    uint8_t newdata = EEPROMExternal->read(addr);

    if (0xAA != newdata) { // write failed
      EEPROMExternalWriteProtect = EEPROMExternal_WriteProtect_e::ReadOnly;
    } else {
      EEPROMExternal->write(addr, 0x55);
      newdata = EEPROMExternal->read(addr);

      if (0x55 != newdata) { // write failed
        EEPROMExternalWriteProtect = EEPROMExternal_WriteProtect_e::ReadOnly;
      } else {
        EEPROMExternal->write(addr, original);
        EEPROMExternalWriteProtect = EEPROMExternal_WriteProtect_e::ReadWrite;
      }
    }
  }
  return EEPROMExternalWriteProtect;
}

/**
 * Is the EEPROM WriteProtected?
 */
bool isEEPROMExternalWriteProtected() {
  return EEPROMExternal_WriteProtect_e::ReadWrite != checkEEPROMExternalWriteProtected();
}

/**
 * Switch to I2C Bus and multiplexer channel of External EEPROM
 */
uint8_t selectEEPROMI2CBusAndMultiplexer() {
  const uint8_t eepromAddress = Settings.EEPROMExternalI2CAddress();

  if (eepromAddress > 0) { // EEPROM Configured?
    # if FEATURE_I2C_MULTIPLE
    const uint8_t i2cBus = Settings.getI2CInterfaceEEPROM();
    # else // if FEATURE_I2C_MULTIPLE
    constexpr uint8_t i2cBus = 0;
    # endif // if FEATURE_I2C_MULTIPLE

    I2CSelectHighClockSpeed(i2cBus);

    # if FEATURE_I2CMULTIPLEXER
    const uint16_t eepromFlags = Settings.EEPROMExternalI2CMultiplexerFlags();
    const int  eepromMuxPort   = get8BitFromUL(eepromFlags, EEPROM_MUX_FLAGS_PORT);
    const bool eepromMulti     = bitRead(eepromFlags, EEPROM_MUX_FLAGS_MULTI);
    I2CMultiplexerSelectByBusAndMux(i2cBus, eepromMulti, eepromMuxPort);
    # endif // if FEATURE_I2CMULTIPLEXER

    if (0 == I2C_wakeup(eepromAddress)) {
      return eepromAddress;
    }
  }
  return 0;
}

/**
 * EEPROM size in bytes
 */
uint32_t getEEPROMSize(EEPROMExternal_Type_e type) {
  switch (type) {
    case EEPROMExternal_Type_e::AT24C256:
    case EEPROMExternal_Type_e::MB85RC256:
      return 32768ul;
    case EEPROMExternal_Type_e::AT24C512:
    case EEPROMExternal_Type_e::MB85RC512:
      return 65536ul;
    # if EEPROM_SUPPORT_AT24C1024
    case EEPROMExternal_Type_e::AT24C1024:
    case EEPROMExternal_Type_e::MB85RC1M:
      return 131072ul;
    # endif // if EEPROM_SUPPORT_AT24C1024
    # if EEPROM_SUPPORT_AT24C2048
    case EEPROMExternal_Type_e::AT24C2048:
    case EEPROMExternal_Type_e::MB85RC2M:
      return 262144ul;
    # endif // if EEPROM_SUPPORT_AT24C2048
    case EEPROMExternal_Type_e::AT24C32:
    case EEPROMExternal_Type_e::MB85RC32:
      return 4096ul;
    case EEPROMExternal_Type_e::AT24C64:
    case EEPROMExternal_Type_e::MB85RC64:
      return 8192ul;
    case EEPROMExternal_Type_e::AT24C128:
    case EEPROMExternal_Type_e::MB85RC128:
      return 16384ul;
  }
  return 0;
}

/**
 * EEPROM pagesize in bytes
 */
uint32_t getEEPROMSize(EEPROMExternal_Type_e type,
                       uint8_t             & pageSize) {
  pageSize = 0;

  switch (type) {
    case EEPROMExternal_Type_e::AT24C256:
    case EEPROMExternal_Type_e::MB85RC256:
      pageSize = 64u;
    case EEPROMExternal_Type_e::AT24C512:
    case EEPROMExternal_Type_e::MB85RC512:
      pageSize = 128u;
    # if EEPROM_SUPPORT_AT24C1024
    case EEPROMExternal_Type_e::AT24C1024:
    case EEPROMExternal_Type_e::MB85RC1M:
      pageSize = 128u;
    # endif // if EEPROM_SUPPORT_AT24C1024
    # if EEPROM_SUPPORT_AT24C2048
    case EEPROMExternal_Type_e::AT24C2048:
    case EEPROMExternal_Type_e::MB85RC2M:
      pageSize = 128u;
    # endif // if EEPROM_SUPPORT_AT24C2048
    case EEPROMExternal_Type_e::AT24C32:
    case EEPROMExternal_Type_e::MB85RC32:
      pageSize = 32u;
    case EEPROMExternal_Type_e::AT24C64:
    case EEPROMExternal_Type_e::MB85RC64:
      pageSize = 32u;
    case EEPROMExternal_Type_e::AT24C128:
    case EEPROMExternal_Type_e::MB85RC128:
      pageSize = 64u;
  }
  return getEEPROMSize(type);
}

/**
 * EEPROM/FRAM name
 */
const __FlashStringHelper* getEEPROMName(EEPROMExternal_Type_e type) {
  # ifndef BUILD_NO_DEBUG

  switch (type) {
    case EEPROMExternal_Type_e::AT24C256:
      return F("AT24C256");
    case EEPROMExternal_Type_e::AT24C512:
      return F("AT24C512");
    #  if EEPROM_SUPPORT_AT24C1024
    case EEPROMExternal_Type_e::AT24C1024:
      return F("AT24C1024");
    #  endif // if EEPROM_SUPPORT_AT24C1024
    #  if EEPROM_SUPPORT_AT24C2048
    case EEPROMExternal_Type_e::AT24C2048:
      return F("AT24C2048");
    #  endif // if EEPROM_SUPPORT_AT24C2048
    case EEPROMExternal_Type_e::AT24C32:
      return F("AT24C32");
    case EEPROMExternal_Type_e::AT24C64:
      return F("AT24C64");
    case EEPROMExternal_Type_e::AT24C128:
      return F("AT24C128");
    case EEPROMExternal_Type_e::MB85RC256:
      return F("MB85RC256");
    case EEPROMExternal_Type_e::MB85RC512:
      return F("MB85RC512");
    #  if EEPROM_SUPPORT_AT24C1024
    case EEPROMExternal_Type_e::MB85RC1M:
      return F("MB85RC1M");
    #  endif // if EEPROM_SUPPORT_AT24C1024
    #  if EEPROM_SUPPORT_AT24C2048
    case EEPROMExternal_Type_e::MB85RC2M:
      return F("MB85RC2M");
    #  endif // if EEPROM_SUPPORT_AT24C2048
    case EEPROMExternal_Type_e::MB85RC32:
      return F("MB85RC32");
    case EEPROMExternal_Type_e::MB85RC64:
      return F("MB85RC64");
    case EEPROMExternal_Type_e::MB85RC128:
      return F("MB85RC128");
  }
  return F("");
  # else // ifndef BUILD_NO_DEBUG
  return F("EEPROM/FRAM");
  # endif // ifndef BUILD_NO_DEBUG
}

/**
 * EEPROM address for slot or 0xFFFF when error
 */
uint32_t getEEPROMAddressForSlot(uint32_t slot) {
  if (checkEEPROMEnabled() > 0) {
    const uint32_t eepromSize = getEEPROMSize(static_cast<EEPROMExternal_Type_e>(Settings.EEPROMExternalType()));

    if ((eepromSize > 0) && (slot < getEEPROMMaxSlots())) {
      const uint32_t slotAddr = EEPROM_CUSTOM_START_OFFSET + (slot * sizeof_uint32_t);

      if (slotAddr < eepromSize) {
        return slotAddr;
      }
    }
  }
  return std::numeric_limits<uint32_t>::max();
}

/**
 * EEPROM address for task and varnr or 0xFFFF when error
 */
uint32_t getEEPROMAddressForTaskValue(taskIndex_t task, taskVarIndex_t varNr) {
  if (checkEEPROMEnabled() > 0) {
    const uint32_t eepromSize = getEEPROMSize(static_cast<EEPROMExternal_Type_e>(Settings.EEPROMExternalType()));

    if ((eepromSize > 0) && validTaskIndex(task) && validTaskVarIndex(varNr)) {
      const uint32_t slotAddr = EEPROM_USERVAR_START_OFFSET + (((task * VARS_PER_TASK) + varNr) * sizeof_uint32_t);

      if (slotAddr < eepromSize) {
        return slotAddr;
      }
    }
  }
  return std::numeric_limits<uint32_t>::max();
}

/**
 * EEPROM available number of slots, max 1024
 * NB: Only first half of EEPROM_CUSTOM_START_OFFSET available for slots when String Variables feature enabled!
 */
uint32_t getEEPROMMaxSlots() {
  if (checkEEPROMEnabled() > 0) {
    const uint32_t eepromSize = getEEPROMSize(static_cast<EEPROMExternal_Type_e>(Settings.EEPROMExternalType()));

    if (eepromSize > 0) {
      const uint32_t slotMax = min((unsigned long)(((eepromSize - EEPROM_CUSTOM_START_OFFSET) / EEPROM_CUSTOM_DIVISOR) / sizeof_uint32_t),
                                   1024ul);

      return slotMax;
    }
  }
  return 0;
}

/**
 * EEPROM write value to slot if the slot is valid
 */
bool writeEEPROMSlot(uint32_t slot,
                     float    data) {
  const uint32_t addr = getEEPROMAddressForSlot(slot);

  if ((addr != std::numeric_limits<uint32_t>::max()) && !isEEPROMExternalWriteProtected()) {
    const float oldData = EEPROMExternal->readLong(addr);

    if (!essentiallyEqual(oldData, data)) {
      EEPROMExternal->writeFloat(addr, data);
    }
    return true;
  }
  return false;
}

/**
 * EEPROM read value from slot or 0 when invalid
 */
float readEEPROMSlot(uint32_t slot) {
  const uint32_t addr = getEEPROMAddressForSlot(slot);

  if (addr != std::numeric_limits<uint32_t>::max()) {
    return EEPROMExternal->readFloat(addr);
  }
  return 0.0f;
}

# if FEATURE_EEPROM_BACKGROUND

// A map of tasks to be executed
std::map<EEPROMExternalTaskType_e, EEPROMExternalTaskData> EEPROMTaskMap;
uint16_t EEPROMSaveDelaySeconds{};

/**
 * Convert TaskType to text
 */
const __FlashStringHelper* TaskDataTypeToString(EEPROMExternalTaskType_e type) {
  switch (type) {
    case EEPROMExternalTaskType_e::None: return F("");
    case EEPROMExternalTaskType_e::UserVars: return F("UserVars");
    case EEPROMExternalTaskType_e::ValueSlots: return F("ValueSlots");
    case EEPROMExternalTaskType_e::C016Caches: return F("C016 Cache elements");
    case EEPROMExternalTaskType_e::PinStates: return F("Pinstates");
  }
  return F("");
}

/**
 * Perform the actual task, bij calling the provided function
 */
void EEPROM_execute_task(void *parameter) {
  EEPROMExternalTaskData*_task_data = static_cast<EEPROMExternalTaskData *>(parameter);

  if ((_task_data->status == EEPROMExternalTaskState_e::Starting) && (nullptr != _task_data->function)) {
    _task_data->status = EEPROMExternalTaskState_e::Processing;

    selectEEPROMI2CBusAndMultiplexer();

    _task_data->timer.setNow();

    // Blocking operation
    _task_data->data = _task_data->function();

    // Results are in
    _task_data->duration = _task_data->timer.millisPassedSince();

    #  if FEATURE_I2CMULTIPLEXER
    I2CMultiplexerOff(
      #   if FEATURE_I2C_MULTIPLE
      Settings.getI2CInterfaceEEPROM()
      #   else // if FEATURE_I2C_MULTIPLE
      0
      #   endif // if FEATURE_I2C_MULTIPLE
      ); // Restore the Multiplexer channel
    #  endif // if FEATURE_I2CMULTIPLEXER

    _task_data->status = EEPROMExternalTaskState_e::Ready;
  }

  _task_data->function = nullptr; // Don't run again accidently
  #  if FEATURE_EEPROM_RTOS_TASK
  _task_data->taskHandle = NULL;
  vTaskDelete(_task_data->taskHandle);
  #  endif // if FEATURE_EEPROM_RTOS_TASK
}

/**
 * Insert a task into the taskmap, 1 per task type, ignore new task when same type is already scheduled to run, or currently running
 */
bool EEPROMAddTask(EEPROMExternalTaskType_e type,
                   EEPROMExternalTaskData   taskData) {
  auto task = EEPROMTaskMap.find(type);

  // Is a task is already in progress, skip until the next AddTask
  if ((task == EEPROMTaskMap.end()) || (EEPROMExternalTaskState_e::Available == task->second.status)) {
    EEPROMTaskMap[type] = taskData;    // Insert task
    task                = EEPROMTaskMap.find(type);

    if (task != EEPROMTaskMap.end()) { // Upserted successfully
      task->second.status = EEPROMExternalTaskState_e::Starting;

      #  ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        addLog(LOG_LEVEL_DEBUG,
               strformat(F("EEPROM: AddTask upserted for %s, status %u (tasks: %u)"
                           #   if FEATURE_EEPROM_RTOS_TASK
                           " (RTOS)"
                           #   endif // if FEATURE_EEPROM_RTOS_TASK
                           ),
                         FsP(TaskDataTypeToString(task->second.type)), static_cast<uint8_t>(task->second.status), EEPROMTaskMap.size()));
      }
      #  endif // ifndef BUILD_NO_DEBUG
    } else {
      addLog(LOG_LEVEL_ERROR, F("EEPROM: Task not inserted"));
    }

    #  if FEATURE_EEPROM_RTOS_TASK
    EEPROMExternalLoop(); // Kick off if it's a RTOS task
    #  endif // if FEATURE_EEPROM_RTOS_TASK
    return true;
  }
  return false;
}

/**
 * Check the task map for tasks to run and start that, also report about finished tasks, and clean those for re-use
 */
bool EEPROMExternalLoop() {
  if (0 != EEPROMSaveDelaySeconds) {
    EEPROMSaveDelaySeconds--;
    return false;
  }
  EEPROMSaveDelaySeconds = Settings.EEPROMSaveDelaySeconds();

  for (auto task = EEPROMTaskMap.begin(); task != EEPROMTaskMap.end(); ++task) {
    if ((EEPROMExternalTaskState_e::Ready == task->second.status) ||
        (EEPROMExternalTaskState_e::Error == task->second.status)) {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        addLog(LOG_LEVEL_INFO, strformat(F("EEPROM: Write %s %s %d in %u msec."
                                           #  if FEATURE_EEPROM_RTOS_TASK
                                           " (RTOS)"
                                           #  endif // if FEATURE_EEPROM_RTOS_TASK
                                           ),
                                         FsP(TaskDataTypeToString(task->second.type)),
                                         FsP(EEPROMExternalTaskState_e::Ready == task->second.status ? F("success") : F("failed")),
                                         task->second.data,
                                         task->second.duration));
      }
      task->second.status = EEPROMExternalTaskState_e::Available;
    }

    if (EEPROMExternalTaskState_e::Starting == task->second.status) {
      if (nullptr != task->second.function) {
        #  ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          addLog(LOG_LEVEL_DEBUG, strformat(F("EEPROM: Loop starting task: %s"), FsP(TaskDataTypeToString(task->second.type))));
        }
        #  endif // ifndef BUILD_NO_DEBUG
        #  if FEATURE_EEPROM_RTOS_TASK
        xTaskCreatePinnedToCore(
          EEPROM_execute_task,      // Function that should be called
          "EEPROM.write()",         // Name of the task (for debugging)
          4000,                     // Stack size (bytes)
          &task->second,            // Parameter to pass
          1,                        // Task priority
          &task->second.taskHandle, // Task handle
          xPortGetCoreID()          // Core you want to run the task on (0 or 1)
          );
        #  else // if FEATURE_EEPROM_RTOS_TASK
        EEPROM_execute_task(&task->second);
        #  endif // if FEATURE_EEPROM_RTOS_TASK
      } else {
        task->second.status = EEPROMExternalTaskState_e::Available; // Recycle
      }
    }
  }
  return false;
}

# endif // if FEATURE_EEPROM_BACKGROUND
} // namespace eeprom
} // namespace ESPEasy
#endif // if FEATURE_EEPROM_EXTERNAL
