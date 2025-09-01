#include "../Helpers/ESPEasyRTC.h"

#include "../Globals/RTC.h"
#include "../DataStructs/RTCStruct.h"
#include "../DataStructs/RTCCacheStruct.h"
#include "../DataStructs/RTC_cache_handler_struct.h"
#include "../DataStructs/TimingStats.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/Plugins.h"
#include "../Globals/RuntimeData.h"
#include "../Globals/Settings.h"
#include "../Helpers/CRC_functions.h"
#include "../../ESPEasy_common.h"

#if FEATURE_EEPROM_EXTERNAL
#include "../../ESPEasy/eeprom/Helpers/EEPROMExternal.h"
#include "../Globals/Cache.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Hardware_I2C.h"
#include "../Helpers/StringConverter.h"

uint8_t readDataForUserVars(size_t index); // Forward declaration
#endif // if FEATURE_EEPROM_EXTERNAL

#ifdef ESP8266
#include <user_interface.h>
#endif

#ifdef ESP32
#include <soc/rtc.h>
#endif

/*********************************************************************************************\
* RTC memory stored values
\*********************************************************************************************/

// ESP8266 RTC layout:
/*
   During deep sleep, only RTC still working, so maybe we need to save some user data in RTC memory.
   Only user data area can be used by user.
 |<--------system data--------->|<-----------------user data--------------->|
 | 256 bytes                    | 512 bytes                                 |
   Note:
   RTC memory is 4 bytes aligned for read and write operations.
   Address parameter refers to block number(4 bytes per block).
   So, if we want to access some data at the beginning of user data area,
   address: 256/4 = 64
   data   : data pointer
   size   : data length, uint8_t

   Prototype:
    bool system_rtc_mem_read (
      uint32 src_addr,
      void * data,
      uint32 save_size
    )

    bool system_rtc_mem_write (
      uint32 des_addr,
      void * data,
      uint32 save_size
    )
 */

// ESP8266 RTC layout ESPeasy:
// these offsets are in blocks, bytes = blocks * 4
// 64   RTCStruct  max 40 bytes: ( 74 - 64 ) * 4
// 74   UserVar
// 122  UserVar checksum:  RTC_BASE_USERVAR + (TASKS_MAX * VARS_PER_TASK)
// 128  Cache (C016) metadata  4 blocks
// 132  Cache (C016) data  6 blocks per sample => max 10 samples



// ESP32 has 2 types of RTC memory:
// RTC SLOW:
//   - 8 kB which can be accessed by both CPU cores and ULP core
//   - Persistent even after reset (not power loss)
//   - Needs RTC_NOINIT_ATTR attribute
// RTC FAST:
//   - 8 kB, only accessed by the "PRO_CPU" 
//   - Persistent during sleep, but not after reset
//   - Needs RTC_DATA_ATTR attribute

// Important to realize:
// Since allocation to RTC memory on ESP32 is done by the compiler, there is no
// guarantee the addresses will be the same after a recompile.
// Thus the data stored in RTC may not survive flashing a new build.

// Structs stored in RTC SLOW:
//   - RTCStruct to keep information on reboot reason, last used WiFi, etc.
//   - UserVar   to keep task values persistent just like on ESP8266

/**
 * With EEPROMExternal (AT24cxxx) enabled and configured:
 * - UserVar will be stored in external EEPROM
 * -
 */




//#define RTC_STRUCT_DEBUG


constexpr uint32_t sizeof_uint32_t = sizeof(uint32_t);

#ifdef ESP32
constexpr size_t UserVar_nrelements = VARS_PER_TASK * TASKS_MAX;


// Since the global UserVar and RTC objects are defined "extern", they cannot be located in the RTC memory.
// Thus we have to keep a copy here.
RTC_NOINIT_ATTR RTCStruct RTC_tmp;
RTC_NOINIT_ATTR uint32_t UserVar_RTC[UserVar_nrelements];
RTC_NOINIT_ATTR uint32_t UserVar_checksum;
#endif


/********************************************************************************************\
   Save RTC struct to RTC memory
 \*********************************************************************************************/
bool saveToRTC()
{
  START_TIMER
  // ESP8266 has the RTC struct stored in memory which we must actively fetch
  // ESP32 can use a compiler flag to mark a struct to be located in RTC_SLOW memory
  #if defined(ESP32)
  RTC_tmp = RTC;
  #else // if defined(ESP32)

  if (!system_rtc_mem_write(RTC_BASE_STRUCT, reinterpret_cast<const uint8_t *>(&RTC), sizeof(RTC)) || !readFromRTC())
  {
      # ifdef RTC_STRUCT_DEBUG
    addLog(LOG_LEVEL_ERROR, F("RTC  : Error while writing to RTC"));
      # endif // ifdef RTC_STRUCT_DEBUG
    return false;
  }
  #endif // if defined(ESP32)
  STOP_TIMER(SAVE_TO_RTC);
  return true;
}

/********************************************************************************************\
   Initialize RTC memory
 \*********************************************************************************************/
void initRTC()
{
  RTC.init();
  saveToRTC();

  UserVar.clear();
  saveUserVarToRTC(true);
}

/********************************************************************************************\
   Read RTC struct from RTC memory
 \*********************************************************************************************/
bool readFromRTC()
{
  // ESP8266 has the RTC struct stored in memory which we must actively fetch
  // ESP32 can use a compiler flag to mark a struct to be located in RTC_SLOW memory
  #ifdef ESP32
  RTC = RTC_tmp;
  #endif
  #ifdef ESP8266
  if (!system_rtc_mem_read(RTC_BASE_STRUCT, reinterpret_cast<uint8_t *>(&RTC), sizeof(RTC))) {
    return false;
  }
  #endif
  return RTC.ID1 == 0xAA && RTC.ID2 == 0x55;
}

#if FEATURE_EEPROM_EXTERNAL
uint32_t saveUserVarToEEPROM() {
  uint32_t eepromWritten{};
  // Update system parameters if not correct
  const bool paramsOk = ESPEasy::eeprom::validateEEPROMExternalParameters();

  if (!paramsOk) {
    ESPEasy::eeprom::updateEEPROMExternalParameters();
  }

  for (taskIndex_t task = 0; task < TASKS_MAX; ++task) {
    if (Settings.TaskDeviceEnabled[task] || !paramsOk) { // Only check enabled tasks or when re-writing the params
      const TaskValues_Data_t* taskValues = UserVar.getRawTaskValues_Data(task);
      if (taskValues != nullptr) {
        for (uint8_t varNr = 0; varNr < VARS_PER_TASK; ++varNr) {
          const bool storeValue = Cache.getTaskVarStoreInEEPROM(task, varNr);
          if (!paramsOk || storeValue) {
            const uint32_t newData = storeValue
                                      ? taskValues->getUint32(varNr)
                                      : std::numeric_limits<uint32_t>::max(); // NaN when read as float
            const uint32_t addr = ESPEasy::eeprom::getEEPROMAddressForTaskValue(task, varNr);
            if (newData != ESPEasy::eeprom::EEPROMExternal->readLong(addr)) { // Only update EEPROM if data differs
              ESPEasy::eeprom::EEPROMExternal->writeLong(addr, newData);
              eepromWritten += sizeof_uint32_t;
            }
          }
        }
      }
    }
  }

  if (eepromWritten) { // Did we change anything? Only then do the slow operation of CRC calculation by reading all bytes
    // Calculate checksum for all stored values, not equal to the UserVar checksum!
    const uint32_t calcsum = calc_CRC32(readDataForUserVars, TASKS_MAX * VARS_PER_TASK * sizeof_uint32_t);
    if (calcsum != ESPEasy::eeprom::EEPROMExternal->readLong(EEPROM_USERVAR_CHECKSUM_OFFSET)) {
      ESPEasy::eeprom::EEPROMExternal->writeLong(EEPROM_USERVAR_CHECKSUM_OFFSET, calcsum);
      eepromWritten += sizeof_uint32_t;
    }
  }
  return eepromWritten;
}
#endif // if FEATURE_EEPROM_EXTERNAL

/********************************************************************************************\
   Save values to RTC memory
 \*********************************************************************************************/
bool saveUserVarToRTC() {
  return saveUserVarToRTC(false);
}

bool saveUserVarToRTC(bool initial)
{
  #if FEATURE_EEPROM_EXTERNAL
  // Check if we have an external EEPROM available on the configured I2C bus & channel, and save all task values there
  const uint8_t eepromAddress = ESPEasy::eeprom::checkEEPROMEnabled();
  if (!initial && (eepromAddress > 0) && !ESPEasy::eeprom::isEEPROMExternalWriteProtected()) { // EEPROM Configured and writable?

    if (0 != ESPEasy::eeprom::selectEEPROMI2CBusAndMultiplexer()) { // Switch to I2C Bus and multiplexer channel of External EEPROM
      #if FEATURE_EEPROM_BACKGROUND
      ESPEasy::eeprom::EEPROMExternalTaskData taskdata;
      taskdata.type = ESPEasy::eeprom::EEPROMExternalTaskType_e::UserVars;
      taskdata.function = saveUserVarToEEPROM;
      ESPEasy::eeprom::EEPROMAddTask(ESPEasy::eeprom::EEPROMExternalTaskType_e::UserVars, taskdata);
      #else // if FEATURE_EEPROM_BACKGROUND
      #ifndef BUILD_NO_DEBUG
      uint32_t startmicros = micros();
      const uint32_t eepromWritten =
      #endif
      saveUserVarToEEPROM();
      #ifndef BUILD_NO_DEBUG
      startmicros = micros() - startmicros;

      if (loglevelActiveFor(LOG_LEVEL_INFO)) { // FIXME LOG_LEVEL_DEBUG
        const ESPEasy::eeprom::EEPROMExternal_Type_e eepromType =
              static_cast<ESPEasy::eeprom::EEPROMExternal_Type_e>(Settings.EEPROMExternalType());
        addLog(LOG_LEVEL_INFO, strformat(F("EEPROM: UserVar: %u bytes (%.2f ms) written to %s"),
                                          eepromWritten, startmicros / 1000.0f, FsP(ESPEasy::eeprom::getEEPROMName(eepromType))));
      }
      #endif // ifndef BUILD_NO_DEBUG
      #endif // if FEATURE_EEPROM_BACKGROUND
    }
    #if FEATURE_I2CMULTIPLEXER
    I2CMultiplexerOff(
      #if FEATURE_I2C_MULTIPLE
      Settings.getI2CInterfaceEEPROM()
      #else //if FEATURE_I2C_MULTIPLE
      0
      #endif // if FEATURE_I2C_MULTIPLE
    ); // Restore the Multiplexer channel
    #endif // if FEATURE_I2CMULTIPLEXER
  }
  #endif // if FEATURE_EEPROM_EXTERNAL

  // ESP8266 has the RTC struct stored in memory which we must actively fetch
  // ESP32   Uses a temp structure which is mapped to the RTC address range.
  #if defined(ESP32)
  for (taskIndex_t task = 0; task < TASKS_MAX; ++task) {
    const TaskValues_Data_t* taskValues = UserVar.getRawTaskValues_Data(task);
    if (taskValues != nullptr) {
      for (uint8_t varNr = 0; varNr < VARS_PER_TASK; ++varNr) {
        const size_t index = (task * VARS_PER_TASK) + varNr;
        UserVar_RTC[index] = taskValues->getUint32(varNr);
      }
    }
  }
  UserVar_checksum = UserVar.compute_CRC32();
  return true;
  #endif

  #ifdef ESP8266
  // addLog(LOG_LEVEL_DEBUG, F("RTCMEM: saveUserVarToRTC"));
  size_t   size{};
  uint8_t *buffer    = UserVar.get(size);
  const uint32_t sum = UserVar.compute_CRC32();
  bool  ret    = system_rtc_mem_write(RTC_BASE_USERVAR, buffer, size);
  ret &= system_rtc_mem_write(RTC_BASE_USERVAR + (size >> 2), reinterpret_cast<const uint8_t *>(&sum), 4);
  return ret;
  #endif
}

#if FEATURE_EEPROM_EXTERNAL
uint8_t readDataForUserVars(size_t index) {
  if (nullptr != ESPEasy::eeprom::EEPROMExternal) {
    return ESPEasy::eeprom::EEPROMExternal->read(EEPROM_USERVAR_START_OFFSET + index);
  }
  return 0;
}
#endif // if FEATURE_EEPROM_EXTERNAL

/********************************************************************************************\
   Read RTC struct from RTC memory
 \*********************************************************************************************/
bool readUserVarFromRTC()
{
  #if FEATURE_EEPROM_EXTERNAL
  const uint8_t eepromAddress = ESPEasy::eeprom::checkEEPROMEnabled();
  if (eepromAddress > 0) { // EEPROM Configured and restoring of Task Values enabled?
    bool result = false;

    if (0 != ESPEasy::eeprom::selectEEPROMI2CBusAndMultiplexer()) { // Switch to I2C Bus and multiplexer channel of External EEPROM
      // Check system parameters with last stored values
      const bool eepromParamsOK = ESPEasy::eeprom::validateEEPROMExternalParameters(true);
      // Check checksum and if correct, restore UserVar values
      const uint32_t checksum = ESPEasy::eeprom::EEPROMExternal->readLong(EEPROM_USERVAR_CHECKSUM_OFFSET);
      const uint32_t calcsum = calc_CRC32(readDataForUserVars, TASKS_MAX * VARS_PER_TASK * sizeof_uint32_t);
      #ifndef BUILD_NO_DEBUG
      if (loglevelActiveFor(LOG_LEVEL_INFO)) { // FIXME LOG_LEVEL_DEBUG
        addLog(LOG_LEVEL_INFO, strformat(F("EEPROM: readUserVarFromRTC calculated: %u, expected: %u equal: %c, params: %c"),
                                         calcsum, checksum, calcsum == checksum ? 'Y' : 'n', eepromParamsOK ? 'Y' : 'n'));
      }
      #endif // ifndef BUILD_NO_DEBUG
      if (eepromParamsOK && (calcsum == checksum)) {
        addLog(LOG_LEVEL_INFO, F("INIT : Restoring Task Values from EEPROM."));
        result = true;
        taskIndex_t lastTask = TASKS_MAX;
        for (size_t i = 0; i < (TASKS_MAX * VARS_PER_TASK) && result; ++i) {
          const taskIndex_t taskIndex = i / VARS_PER_TASK;
          const uint8_t varNr = i % VARS_PER_TASK;
          if (taskIndex != lastTask) {
            LoadTaskSettings(taskIndex);
            lastTask = taskIndex;
          }
          // Store in raw form, so we don't apply formula as we don't really know what type is required.
          const uint32_t addr = ESPEasy::eeprom::getEEPROMAddressForTaskValue(taskIndex, varNr);
          if (addr != std::numeric_limits<uint32_t>::max()) {
            if (Cache.getTaskVarStoreInEEPROM(taskIndex, varNr)) { // Restore only when enabled
              TaskValues_Data_t* taskValues = UserVar.getRawTaskValues_Data(taskIndex);
              taskValues->setUint32(varNr, ESPEasy::eeprom::EEPROMExternal->readLong(addr));
            }
          } else {
            result = false;
          }
        }
      }
    }
    #if FEATURE_I2CMULTIPLEXER
    I2CMultiplexerOff(
      #if FEATURE_I2C_MULTIPLE
      Settings.getI2CInterfaceEEPROM()
      #else //if FEATURE_I2C_MULTIPLE
      0
      #endif // if FEATURE_I2C_MULTIPLE
    ); // Restore the Multiplexer channel
    #endif // if FEATURE_I2CMULTIPLEXER
    if (result) {
      return true; // We did all that was needed
    }
  }
  #endif // if FEATURE_EEPROM_EXTERNAL

  // ESP8266 has the RTC struct stored in memory which we must actively fetch
  // ESP32   Uses a temp structure which is mapped to the RTC address range.
  #if defined(ESP32)
  if (calc_CRC32(reinterpret_cast<const uint8_t *>(&UserVar_RTC[0]), UserVar_nrelements * sizeof(float)) == UserVar_checksum) {
    for (size_t i = 0; i < UserVar_nrelements; ++i) {
      const taskIndex_t taskIndex = i / VARS_PER_TASK;
      const uint8_t varNr = i % VARS_PER_TASK;
      // Store in raw form, so we don't apply formula as we don't really know what type is required.
      TaskValues_Data_t* taskValues = UserVar.getRawTaskValues_Data(taskIndex);
      taskValues->setUint32(varNr, UserVar_RTC[i]);
    }
    return true;
  }
  return false;
  #endif

  #ifdef ESP8266
  // addLog(LOG_LEVEL_DEBUG, F("RTCMEM: readUserVarFromRTC"));
  size_t   size{};
  uint8_t *buffer = UserVar.get(size);
  bool ret        = system_rtc_mem_read(RTC_BASE_USERVAR, buffer, size);
  uint32_t sumRAM = UserVar.compute_CRC32();
  uint32_t sumRTC = 0;
  ret &= system_rtc_mem_read(RTC_BASE_USERVAR + (size >> 2), reinterpret_cast<uint8_t *>(&sumRTC), 4);

  if (!ret || (sumRTC != sumRAM))
  {
      # ifdef RTC_STRUCT_DEBUG
    addLog(LOG_LEVEL_ERROR, F("RTC  : Checksum error on reading RTC user var"));
      # endif // ifdef RTC_STRUCT_DEBUG
    memset(buffer, 0, size);
  }
  return ret;
  #endif 
}

