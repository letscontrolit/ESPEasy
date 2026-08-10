#include "../Helpers/RTCSRAMStorage.h"
#include "../../../src/Globals/Settings.h"
#include "../../../src/Helpers/I2C_access.h"
#include "../../../ESPEasy_common.h"
#include "../../../src/Helpers/StringConverter.h"
#include "../../../src/DataTypes/TimeSource.h"
#include <RTClib.h>

#if FEATURE_RTC_SRAM_STORAGE

namespace ESPEasy {
namespace eeprom {

# if FEATURE_SRAM_STORAGE_DOUBLE
constexpr uint32_t sizeof_rtcsram_slot = sizeof(double);
# else // if FEATURE_SRAM_STORAGE_DOUBLE
constexpr uint32_t sizeof_rtcsram_slot = sizeof(float);
# endif // if FEATURE_SRAM_STORAGE_DOUBLE

/**
 * Check if the RTC is properly initialized and enabled.
 * Returns the external rtc type if all is OK
 */
uint8_t checkRTCSRAMEnabled() {
  const ExtTimeSource_e extRtcType = static_cast<ExtTimeSource_e>(Settings.ExternalTimeSource);

  if ((ExtTimeSource_e::DS1307 == extRtcType) ||
      (ExtTimeSource_e::DS3232 == extRtcType)) { // RTC with SRAM Configured?
    return Settings.ExternalTimeSource;
  }
  return 0;
}

/**
 * Get the RTC I2C Address
 */
uint8_t getRTCI2CAddress() {
  const ExtTimeSource_e type = static_cast<ExtTimeSource_e>(Settings.ExternalTimeSource);

  switch (type)
  {
    case ExtTimeSource_e::DS1307:
      return DS1307_ADDRESS;
    case ExtTimeSource_e::DS3232:
      return DS3231_ADDRESS;
    case ExtTimeSource_e::DS3231:
    case ExtTimeSource_e::PCF8523:
    case ExtTimeSource_e::PCF8563:
    case ExtTimeSource_e::None:
      return 0;
  }
  return 0;

}

/**
 * Switch to I2C Bus and multiplexer channel of External EEPROM
 */
uint8_t selectRTCSRAMI2CBus() {
  const ExtTimeSource_e type = static_cast<ExtTimeSource_e>(Settings.ExternalTimeSource);

  if (checkRTCSRAMEnabled() > 0) { // RTC Module Configured?
    # if FEATURE_I2C_MULTIPLE
    const uint8_t i2cBus = Settings.getI2CInterfaceRTC();
    # else // if FEATURE_I2C_MULTIPLE
    constexpr uint8_t i2cBus = 0;
    # endif // if FEATURE_I2C_MULTIPLE

    switch (type)
    {
      case ExtTimeSource_e::DS1307:
        I2CSelect_Max100kHz_ClockSpeed(i2cBus);
        break;
      case ExtTimeSource_e::DS3232:
        I2CSelectHighClockSpeed(i2cBus);
        break;
      case ExtTimeSource_e::DS3231:
      case ExtTimeSource_e::PCF8523:
      case ExtTimeSource_e::PCF8563:
      case ExtTimeSource_e::None:
        break;
    }

    if (0 == I2C_wakeup(getRTCI2CAddress())) {
      return Settings.ExternalTimeSource;
    }
  }
  return 0;
}

/**
 * RTC SRAM size in bytes
 */
uint32_t getRTCSRAMSize() {
  const ExtTimeSource_e type = static_cast<ExtTimeSource_e>(Settings.ExternalTimeSource);

  switch (type)
  {
    case ExtTimeSource_e::DS1307:
      return 56ul;
    case ExtTimeSource_e::DS3232:
      return 240ul;
    case ExtTimeSource_e::DS3231:
    case ExtTimeSource_e::PCF8523:
    case ExtTimeSource_e::PCF8563:
    case ExtTimeSource_e::None:
      return 0;
  }
  return 0;
}

/**
 * RTC SRAM _relative_ address for slot or 0xFFFF when error
 */
uint32_t getRTCSRAMAddressForSlot(uint32_t slot) {
  if (checkRTCSRAMEnabled() > 0) {
    const uint32_t rtcSramSize = getRTCSRAMSize();

    if ((rtcSramSize > 0) && (slot < getRTCSRAMMaxSlots())) {
      const uint32_t slotAddr = slot * sizeof_rtcsram_slot;

      if (slotAddr < rtcSramSize) {
        return slotAddr;
      }
    }
  }
  return std::numeric_limits<uint32_t>::max();
}

/**
 * EEPROM available number of slots, max use all available space minus some administrative bytes
 */
uint32_t getRTCSRAMMaxSlots() {
  if (checkRTCSRAMEnabled() > 0) {
    const uint32_t rtcSramSize = getRTCSRAMSize();

    if (rtcSramSize > 0) {
      const uint32_t slotMax = (uint32_t)(rtcSramSize / sizeof_rtcsram_slot);

      return slotMax;
    }
  }
  return 0;
}

/**
 * EEPROM write value to slot if the slot is valid
 */
bool writeRTCSRAMSlot(uint32_t                slot,
                      SRAM_STORAGE_FLOAT_TYPE data)
{
  const uint32_t addr = getRTCSRAMAddressForSlot(slot);

  if ((addr != std::numeric_limits<uint32_t>::max()) && (selectRTCSRAMI2CBus() > 0)) {
    const ExtTimeSource_e type = static_cast<ExtTimeSource_e>(Settings.ExternalTimeSource);
    uint8_t _b[sizeof_rtcsram_slot]{};

    switch (type)
    {
      case ExtTimeSource_e::DS1307:
      {
        RTC_DS1307 rtc;
        rtc.readnvram(_b, sizeof_rtcsram_slot, addr);
        const SRAM_STORAGE_FLOAT_TYPE oldData = *(SRAM_STORAGE_FLOAT_TYPE *)&_b[0];

        if (!essentiallyEqual(oldData, data)) {
          rtc.writenvram(addr, (uint8_t *)&data, sizeof_rtcsram_slot);
        }
        return true;
      }
      case ExtTimeSource_e::DS3232:
      {
        RTC_DS3231 rtc;
        rtc.readnvram(_b, sizeof_rtcsram_slot, addr);
        const SRAM_STORAGE_FLOAT_TYPE oldData = *(SRAM_STORAGE_FLOAT_TYPE *)&_b[0];

        if (!essentiallyEqual(oldData, data)) {
          rtc.writenvram(addr, (uint8_t *)&data, sizeof_rtcsram_slot);
        }
        return true;
      }
      case ExtTimeSource_e::DS3231:
      case ExtTimeSource_e::PCF8523:
      case ExtTimeSource_e::PCF8563:
      case ExtTimeSource_e::None:
        return false;
    }

  }
  return false;
}

/**
 * RTC SRAM read value from slot or 0 when invalid
 */
SRAM_STORAGE_FLOAT_TYPE readRTCSRAMSlot(uint32_t slot) {
  const uint32_t addr = getRTCSRAMAddressForSlot(slot);

  if ((addr != std::numeric_limits<uint32_t>::max()) && (selectRTCSRAMI2CBus() > 0)) {
    const ExtTimeSource_e type = static_cast<ExtTimeSource_e>(Settings.ExternalTimeSource);
    uint8_t _b[sizeof_rtcsram_slot]{};

    switch (type)
    {
      case ExtTimeSource_e::DS1307:
      {
        RTC_DS1307 rtc;
        rtc.readnvram(_b, sizeof_rtcsram_slot, addr);
        return *(SRAM_STORAGE_FLOAT_TYPE *)&_b[0];
      }
      case ExtTimeSource_e::DS3232:
      {
        RTC_DS3231 rtc;
        rtc.readnvram(_b, sizeof_rtcsram_slot, addr);
        return *(SRAM_STORAGE_FLOAT_TYPE *)&_b[0];
      }
      case ExtTimeSource_e::DS3231:
      case ExtTimeSource_e::PCF8523:
      case ExtTimeSource_e::PCF8563:
      case ExtTimeSource_e::None:
        break;
    }

  }
  # if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  return 0.0;
  # else // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  return 0.0f;
  # endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
}

} // namespace eeprom
} // namespace ESPEasy
#endif // if FEATURE_RTC_SRAM_STORAGE
