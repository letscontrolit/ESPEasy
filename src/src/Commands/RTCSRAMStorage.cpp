#include "../Commands/RTCSRAMStorage.h"
#if FEATURE_RTC_SRAM_STORAGE
# include "../../ESPEasy/eeprom/Helpers/RTCSRAMStorage.h"

# include "../../ESPEasy_common.h"

# include "../Commands/Common.h"

# include "../DataStructs/ESPEasy_EventStruct.h"

# include "../Helpers/ESPEasy_time_calc.h"
# include "../Helpers/Misc.h"
# include "../Helpers/Numerical.h"
# include "../Helpers/StringConverter.h"

// Command: WriteRTC,<slot>,<value>  : set a slot value. 0 is 'erased'
// Command: WriteRTC,erase,erase     : reset all slots to 0
const __FlashStringHelper* Command_writeRTC(struct EventStruct *event, const char *Line)
{
  uint32_t slot{};

  # if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  double value{};
  bool   validValue = validDoubleFromString(parseString(Line, 3), value);
  # else // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
  float value{};
  bool  validValue = validFloatFromString(parseString(Line, 3), value);
  # endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE

  if (validUIntFromString(parseString(Line, 2), slot) && validValue) {
    return return_command_boolean_result_flashstr(ESPEasy::eeprom::writeRTCSRAMSlot(slot, value));
  } else if (equals(parseString(Line, 2), F("erase")) && equals(parseString(Line, 3), F("erase"))) {
    uint32_t start = millis();

    for (uint32_t slot = 0; slot < ESPEasy::eeprom::getRTCSRAMMaxSlots(); ++slot) {
      # if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      ESPEasy::eeprom::writeRTCSRAMSlot(slot, 0.0);
      # else // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      ESPEasy::eeprom::writeRTCSRAMSlot(slot, 0.0f);
      # endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE

      if ((slot % 50 == 0) || (timePassedSince(start) > 50)) {
        delay(0);
        start = millis();
      }
    }
    addLog(LOG_LEVEL_INFO, F("RTC SRAM: All slot-values erased."));
    return return_command_success_flashstr();
  }
  return return_command_failed_flashstr();
}

#endif // if FEATURE_EEPROM_EXTERNAL
