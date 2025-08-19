#include "../Helpers/EEPROMExternal.h"
#include "../Globals/Settings.h"
#include "../Helpers/I2C_access.h"
#include "../../ESPEasy_common.h"

#if FEATURE_EEPROM_EXTERNAL

AT24CX *EEPROMExternal = nullptr;

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

uint32_t getEEPROMSize(EEPROMExternal_Type_e type) {
  switch (type) {
    case EEPROMExternal_Type_e::AT24C256:
      return 32768;
    case EEPROMExternal_Type_e::AT24C512:
      return 65536;
    # if EEPROM_SUPPORT_AT24C1024
    case EEPROMExternal_Type_e::AT24C1024:
      return 131072;
    # endif // if EEPROM_SUPPORT_AT24C1024
    # if EEPROM_SUPPORT_AT24C2048
    case EEPROMExternal_Type_e::AT24C2048:
      return 262144;
    # endif // if EEPROM_SUPPORT_AT24C2048
    case EEPROMExternal_Type_e::AT24C32:
      return 4096;
    case EEPROMExternal_Type_e::AT24C64:
      return 8192;
    case EEPROMExternal_Type_e::AT24C128:
      return 16384;
  }
  return 0;
}

uint32_t getEEPROMSize(EEPROMExternal_Type_e type,
                       uint8_t             & pageSize) {
  pageSize = 0;

  switch (type) {
    case EEPROMExternal_Type_e::AT24C256:
      pageSize = 64;
    case EEPROMExternal_Type_e::AT24C512:
      pageSize = 128;
    # if EEPROM_SUPPORT_AT24C1024
    case EEPROMExternal_Type_e::AT24C1024:
      pageSize = 128;
    # endif // if EEPROM_SUPPORT_AT24C1024
    # if EEPROM_SUPPORT_AT24C2048
    case EEPROMExternal_Type_e::AT24C2048:
      pageSize = 128;
    # endif // if EEPROM_SUPPORT_AT24C2048
    case EEPROMExternal_Type_e::AT24C32:
      pageSize = 32;
    case EEPROMExternal_Type_e::AT24C64:
      pageSize = 32;
    case EEPROMExternal_Type_e::AT24C128:
      pageSize = 64;
  }
  return getEEPROMSize(type);
}

const __FlashStringHelper* getEEPROMName(EEPROMExternal_Type_e type) {
  # ifndef BUILD_NO_DEBUG

  switch (type) {
    case EEPROMExternal_Type_e::AT24C256:
      return F("AT24C256/MB85RC256");
    case EEPROMExternal_Type_e::AT24C512:
      return F("AT24C512/MB85RC512");
    #  if EEPROM_SUPPORT_AT24C1024
    case EEPROMExternal_Type_e::AT24C1024:
      return F("AT24C1024/MB85RC1M");
    #  endif // if EEPROM_SUPPORT_AT24C1024
    #  if EEPROM_SUPPORT_AT24C2048
    case EEPROMExternal_Type_e::AT24C2048:
      return F("AT24C2048");
    #  endif // if EEPROM_SUPPORT_AT24C2048
    case EEPROMExternal_Type_e::AT24C32:
      return F("AT24C32");
    case EEPROMExternal_Type_e::AT24C64:
      return F("AT24C64/MB85RC64");
    case EEPROMExternal_Type_e::AT24C128:
      return F("AT24C128/MB85RC128");
  }
  return F("");
  # else // ifndef BUILD_NO_DEBUG
  return F("EEPROM/FRAM");
  # endif // ifndef BUILD_NO_DEBUG
}

#endif // if FEATURE_EEPROM_EXTERNAL
