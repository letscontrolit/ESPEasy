#include "../Commands/EEPROMExternal.h"
#include "../../ESPEasy/eeprom/Helpers/EEPROMExternal.h"

#include "../../ESPEasy_common.h"

#include "../Commands/Common.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../Helpers/Misc.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/StringConverter.h"

#if FEATURE_EEPROM_EXTERNAL

// Command: WriteEE,<slot>,<value>  : set a slot value. 0 is 'erased'
// Command: WriteEE,erase,erase     : reset all slots to 0
const __FlashStringHelper* Command_writeEE(struct EventStruct *event, const char *Line)
{
  uint32_t slot{};
  float    value{};

  if (validUIntFromString(parseString(Line, 2), slot) && validFloatFromString(parseString(Line, 3), value)) {
    return return_command_boolean_result_flashstr(ESPEasy::eeprom::writeEEPROMSlot(slot, value));
  } else if (equals(parseString(Line, 2), F("erase")) && equals(parseString(Line, 3), F("erase")))  {
    for (uint32_t slot = 0; slot < ESPEasy::eeprom::getEEPROMMaxSlots(); ++slot) {
      ESPEasy::eeprom::writeEEPROMSlot(slot, 0.0f);
    }
    addLog(LOG_LEVEL_INFO, F("EEPROM: All slot-values erased."));
    return return_command_success_flashstr();
  } else if (equals(parseString(Line, 2), F("check")) && equals(parseString(Line, 3), F("wp")))  {
    addLog(LOG_LEVEL_INFO, F("EEPROM: Check write-protect."));
    ESPEasy::eeprom::checkEEPROMExternalWriteProtected(true);
    
    if (ESPEasy::eeprom::isEEPROMExternalWriteProtected()) {
      addLog(LOG_LEVEL_INFO, concat(F("EEPROM: Write-protected! Status: "), static_cast<uint8_t>(ESPEasy::eeprom::checkEEPROMExternalWriteProtected())));
    }
    return return_command_success_flashstr();
  }
  return return_command_failed_flashstr();
}

#endif // if FEATURE_EEPROM_EXTERNAL
