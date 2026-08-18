#include "../Commands/EEPROMExternal.h"
#if FEATURE_EEPROM_EXTERNAL
# include "../../ESPEasy/eeprom/Helpers/EEPROMExternal.h"

# include "../../ESPEasy_common.h"

# include "../Commands/Common.h"

# include "../DataStructs/ESPEasy_EventStruct.h"

# include "../Helpers/Misc.h"
# include "../Helpers/Numerical.h"
# include "../Helpers/StringConverter.h"

// Command: WriteEE,<slot>,<value>  : set a slot value. 0 is 'erased'
// Command: WriteEE,erase,erase     : reset all slots to 0
// Command: WriteEE,check,wp        : check if external EEPROM is write-protected
const __FlashStringHelper* Command_writeEE(struct EventStruct *event, const char *Line)
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
    return return_command_boolean_result_flashstr(ESPEasy::eeprom::writeEEPROMSlot(slot, value));
  } else if (equals(parseString(Line, 2), F("erase")) && equals(parseString(Line, 3), F("erase")))  {
    for (uint32_t slot = 0; slot < ESPEasy::eeprom::getEEPROMMaxSlots(); ++slot) {
      # if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      ESPEasy::eeprom::writeEEPROMSlot(slot, 0.0);
      # else // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
      ESPEasy::eeprom::writeEEPROMSlot(slot, 0.0f);
      # endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE

      if (slot % 50 == 0) { delay(0); }
    }
    addLog(LOG_LEVEL_INFO, F("EEPROM: All slot-values erased."));
    return return_command_success_flashstr();
  } else if (equals(parseString(Line, 2), F("check")) && equals(parseString(Line, 3), F("wp")))  {
    addLog(LOG_LEVEL_INFO, F("EEPROM: Check write-protect."));
    ESPEasy::eeprom::checkEEPROMExternalWriteProtected(true);

    if (ESPEasy::eeprom::isEEPROMExternalWriteProtected()) {
      addLog(LOG_LEVEL_INFO,
             concat(F("EEPROM: Write-protected! Status: "), static_cast<uint8_t>(ESPEasy::eeprom::checkEEPROMExternalWriteProtected())));
    }
    return return_command_success_flashstr();
  }
  return return_command_failed_flashstr();
}

#endif // if FEATURE_EEPROM_EXTERNAL
