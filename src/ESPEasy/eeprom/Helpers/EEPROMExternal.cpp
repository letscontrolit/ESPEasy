#include "../Helpers/EEPROMExternal.h"
#if FEATURE_EEPROM_EXTERNAL

# include "../../../ESPEasy_common.h"
# include "../../../src/DataStructs/TimingStats.h"
# include "../../../src/Globals/Settings.h"
# include "../../../src/Helpers/I2C_access.h"
# include "../../../src/Helpers/StringConverter.h"


namespace ESPEasy {
namespace eeprom {
AT24Cxxx *EEPROMExternal                                 = nullptr;
EEPROMExternal_WriteProtect_e EEPROMExternalWriteProtect = EEPROMExternal_WriteProtect_e::Undefined;
bool EEPROMParamsOkState{};
LongTermTimer EEPROMParamsOkTimer;

constexpr uint32_t sizeof_eeprom_slot = sizeof(double);

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

    if (ESPEasy::eeprom::selectEEPROMI2CBusAndMultiplexer()) { // Switch to I2C Bus and multiplexer channel of External EEPROM
      // We have an I2C device at this address, let's assume it's an EEPROM...
      uint8_t pageSize        = 0;
      uint8_t delay           = 0;
      const size_t eepromSize = ESPEasy::eeprom::getEEPROMSize(eepromType, pageSize, delay);
      ESPEasy::eeprom::EEPROMExternal = new (std::nothrow) AT24Cxxx(eepromAddress, Wire, delay, eepromSize, pageSize);

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
  const uint16_t eepromVersionParam{};
  EEPROMExternal->get(EEPROM_PARAMS_VERSION_ADDRESS, eepromVersionParam);

  EEPROMParamsOkTimer.setNow();
  EEPROMParamsOkState = false;

  if (EEPROM_PARAMS_CURRENT_VERSION == eepromVersionParam) {
    EEPROMParamsOkState = true;
  }

  return EEPROMParamsOkState;
}

/**
 * Update the stored parameters in EEPROM
 * - Version
 */
void updateEEPROMExternalParameters() {
  const uint16_t eepromVersionParam{};

  EEPROMExternal->get(EEPROM_PARAMS_VERSION_ADDRESS, eepromVersionParam);

  if (EEPROM_PARAMS_CURRENT_VERSION != eepromVersionParam) {
    EEPROMExternal->put(EEPROM_PARAMS_VERSION_ADDRESS, (uint16_t)EEPROM_PARAMS_CURRENT_VERSION);
  }
}

/**
 * Check if the EEPROM is properly initialized and enabled.
 * Returns the I2C address if all is OK
 */
uint8_t checkEEPROMEnabled() {
  const uint8_t eepromAddress = Settings.EEPROMExternalI2CAddress();

  if ((nullptr != EEPROMExternal) && eepromAddress) { // EEPROM Configured?
    return eepromAddress;
  }
  return (uint8_t)0;
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
bool    isEEPROMExternalWriteProtected() { return EEPROMExternal_WriteProtect_e::ReadWrite != checkEEPROMExternalWriteProtected(); }

/**
 * Switch to I2C Bus and multiplexer channel of External EEPROM
 */
uint8_t selectEEPROMI2CBusAndMultiplexer() {
  const uint8_t eepromAddress = Settings.EEPROMExternalI2CAddress();

  if (eepromAddress) { // EEPROM Configured?
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
  return (uint8_t)0;
}

/**
 * EEPROM size in bytes
 */
size_t getEEPROMSize(EEPROMExternal_Type_e type) {
  switch (type)
  {
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
  return 0ul;
}

/**
 * EEPROM pagesize in bytes
 */
size_t getEEPROMSize(EEPROMExternal_Type_e type,
                     uint8_t             & pageSize,
                     uint8_t             & delay) {
  pageSize = (uint8_t)0;
  delay    = (uint8_t)0; // ms

  switch (type)
  {
    case EEPROMExternal_Type_e::AT24C256:
    case EEPROMExternal_Type_e::AT24C128:
      delay = (uint8_t)6; // fall through
    case EEPROMExternal_Type_e::MB85RC256:
    case EEPROMExternal_Type_e::MB85RC128:
      pageSize = (uint8_t)64;
      break;
    # if EEPROM_SUPPORT_AT24C1024
    case EEPROMExternal_Type_e::AT24C1024:
      delay = (uint8_t)6; // fall through
    case EEPROMExternal_Type_e::MB85RC1M:
    # endif // if EEPROM_SUPPORT_AT24C1024
    # if EEPROM_SUPPORT_AT24C2048
    case EEPROMExternal_Type_e::AT24C2048:
      delay = (uint8_t)6; // fall through
    case EEPROMExternal_Type_e::MB85RC2M:
    # endif // if EEPROM_SUPPORT_AT24C2048
    case EEPROMExternal_Type_e::AT24C512:
      delay = (uint8_t)6; // fall through
    case EEPROMExternal_Type_e::MB85RC512:
      pageSize = (uint8_t)128;
      break;
    case EEPROMExternal_Type_e::AT24C32:
    case EEPROMExternal_Type_e::AT24C64:
      delay = (uint8_t)10; // fall through
    case EEPROMExternal_Type_e::MB85RC32:
    case EEPROMExternal_Type_e::MB85RC64:
      pageSize = (uint8_t)32;
      break;
  }
  return getEEPROMSize(type);
}

/**
 * EEPROM/FRAM name
 */
const __FlashStringHelper* getEEPROMName(EEPROMExternal_Type_e type) {
  switch (type)
  {
    case EEPROMExternal_Type_e::AT24C256:
      return F("AT24C256");
    case EEPROMExternal_Type_e::AT24C512:
      return F("AT24C512");
    # if EEPROM_SUPPORT_AT24C1024
    case EEPROMExternal_Type_e::AT24C1024:
      return F("AT24C1024");
    # endif // if EEPROM_SUPPORT_AT24C1024
    # if EEPROM_SUPPORT_AT24C2048
    case EEPROMExternal_Type_e::AT24C2048:
      return F("AT24C2048");
    # endif // if EEPROM_SUPPORT_AT24C2048
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
    # if EEPROM_SUPPORT_AT24C1024
    case EEPROMExternal_Type_e::MB85RC1M:
      return F("MB85RC1M");
    # endif // if EEPROM_SUPPORT_AT24C1024
    # if EEPROM_SUPPORT_AT24C2048
    case EEPROMExternal_Type_e::MB85RC2M:
      return F("MB85RC2M");
    # endif // if EEPROM_SUPPORT_AT24C2048
    case EEPROMExternal_Type_e::MB85RC32:
      return F("MB85RC32");
    case EEPROMExternal_Type_e::MB85RC64:
      return F("MB85RC64");
    case EEPROMExternal_Type_e::MB85RC128:
      return F("MB85RC128");
  }
  return F("");
}

/**
 * EEPROM address for slot or 0xFFFF when error
 */
uint32_t getEEPROMAddressForSlot(uint32_t slot) {
  if (checkEEPROMEnabled()) {
    const uint32_t eepromSize = getEEPROMSize(static_cast<EEPROMExternal_Type_e>(Settings.EEPROMExternalType()));

    if (eepromSize && (slot < getEEPROMMaxSlots())) {
      const uint32_t slotAddr = EEPROM_CUSTOM_START_OFFSET + (slot * sizeof_eeprom_slot);

      if (slotAddr < eepromSize) {
        return slotAddr;
      }
    }
  }
  return std::numeric_limits<uint32_t>::max();
}

/**
 * EEPROM available number of slots, max use all available space minus some administrative bytes
 */
uint32_t getEEPROMMaxSlots() {
  if (checkEEPROMEnabled()) {
    const uint32_t eepromSize = getEEPROMSize(static_cast<EEPROMExternal_Type_e>(Settings.EEPROMExternalType()));

    if (eepromSize) {
      const uint32_t slotMax = (unsigned long)(((eepromSize - EEPROM_CUSTOM_START_OFFSET) / EEPROM_CUSTOM_DIVISOR) / sizeof_eeprom_slot);

      return slotMax;
    }
  }
  return 0ul;
}

/**
 * EEPROM write value to slot if the slot is valid
 */
bool writeEEPROMSlot(uint32_t                 slot,
                     ESPEASY_RULES_FLOAT_TYPE data)
{
  const uint32_t addr = getEEPROMAddressForSlot(slot);

  if ((addr != std::numeric_limits<uint32_t>::max()) && !isEEPROMExternalWriteProtected()) {
    double oldData{};
    {
      START_TIMER;
      EEPROMExternal->get(addr, oldData);
      STOP_TIMER(READ_EEPROM_SLOT);
    }

    if (!essentiallyEqual(oldData, data)) {
      START_TIMER;
      const double _wrdata = data;
      EEPROMExternal->put(addr, _wrdata); // Always write double size!
      STOP_TIMER(WRITE_EEPROM_SLOT);
    }
    return true;
  }
  return false;
}

/**
 * EEPROM read value from slot or 0 when invalid
 */
ESPEASY_RULES_FLOAT_TYPE readEEPROMSlot(uint32_t slot) {
  const uint32_t addr = getEEPROMAddressForSlot(slot);
  double res{};

  if (addr != std::numeric_limits<uint32_t>::max()) {
    START_TIMER;
    EEPROMExternal->get(addr, res);
    STOP_TIMER(READ_EEPROM_SLOT);
  }
  return res;
}

} // namespace eeprom
} // namespace ESPEasy
#endif // if FEATURE_EEPROM_EXTERNAL
