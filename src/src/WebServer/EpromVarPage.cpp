#include "../WebServer/EepromVarPage.h"


#include "../WebServer/ESPEasy_WebServer.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Forms.h"
#include "../WebServer/HTML_wrappers.h"

#include "../Globals/Settings.h"

#include "../Helpers/EEPROMExternal.h"

#include "../Helpers/StringConverter.h"

// #if FEATURE_STRING_VARIABLES
// # include "../Helpers/StringParser.h"
// #endif // if

#if FEATURE_EEPROM_EXTERNAL

void handle_eepromvars() {
  if (!isLoggedIn()) { return; }
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  if ((nullptr != EEPROMExternal) && (Settings.EEPROMExternalI2CAddress() > 0)) {
    // the table header
    html_table_class_normal();
    html_TR();
    html_table_header(F("External EEPROM"),                                                             300);
    html_table_header(getEEPROMName(static_cast<EEPROMExternal_Type_e>(Settings.EEPROMExternalType())), 400);
    html_table_header(F(""));
    html_TR();
    html_table_header(F("Slot"),                         300);
    html_table_header(F("Value (only non-zero values)"), 400);
    html_table_header(F(""));

    const uint32_t maxSlots = getEEPROMMaxSlots();
    uint32_t count{};

    for (uint32_t slot = 0; slot < maxSlots; ++slot) {
      const float value = readEEPROMSlot(slot);

      if (slot % 50 == 0) { delay(0); }

      if (!essentiallyZero(value)) {
        ++count;
        html_TR_TD();
        addHtmlInt(slot);
        html_TD();
        addHtml(toString(value));
        html_TD();
      }
    }

    // TODO: List String values in EEPROM

    addTableSeparator(F("Summary"), 3, 2);

    html_TR_TD();
    addHtml(F("Slots occupied: "));

    if (0 == count) {
      addHtml(F("none"));
    } else {
      addHtmlInt(count);
    }
    html_TD();
    addHtml(F("Slots available: "));
    addHtmlInt(maxSlots - count);

    if (count > 0) {
      addHtml(F(" of "));
      addHtmlInt(maxSlots);
    }
    html_TD();

    // TODO: List String summary in EEPROM

    html_end_table();
  } else {
    addHtml(F("External EEPROM not enabled."));
  }
  html_end_form();
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

#endif // if FEATURE_EEPROM_EXTERNAL
