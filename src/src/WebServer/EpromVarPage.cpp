#include "../WebServer/EepromVarPage.h"


#include "../WebServer/ESPEasy_WebServer.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Forms.h"
#include "../WebServer/HTML_wrappers.h"

#include "../Globals/Settings.h"
#include "../Globals/ExtraTaskSettings.h"

#include "../../ESPEasy/eeprom/Helpers/EEPROMExternal.h"
#include "../Helpers/ESPEasy_Storage.h"

#include "../Helpers/StringConverter.h"

#if FEATURE_EEPROM_EXTERNAL

void handle_eepromvars() {
  if (!isLoggedIn()) { return; }

  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  if (ESPEasy::eeprom::checkEEPROMEnabled() > 0) {
    // the table header
    html_table_class_normal();
    html_TR();
    html_table_header(F("External EEPROM"),
                      300);
    html_table_header(ESPEasy::eeprom::getEEPROMName(static_cast<ESPEasy::eeprom::EEPROMExternal_Type_e>(Settings.EEPROMExternalType())),
                      500);
    html_table_header(ESPEasy::eeprom::isEEPROMExternalWriteProtected() ? F("Write-protected!") : F(""),
                      400);
    html_table_header(F(""));

    html_TR();

    // sub-table header
    html_table_header(F("Slot"),                         300);
    html_table_header(F("Value (only non-zero values)"), 500);
    html_table_header(F(""));
    html_table_header(F(""));

    const uint32_t maxSlots = ESPEasy::eeprom::getEEPROMMaxSlots();
    uint32_t count{};

    for (uint32_t slot = 0; slot < maxSlots; ++slot) {
      const float value = ESPEasy::eeprom::readEEPROMSlot(slot);

      if (slot % 50 == 0) { delay(0); }

      if (!isnan(value) && !essentiallyZero(value)) {
        ++count;
        html_TR_TD();
        addHtmlInt(slot);
        html_TD();
        # if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
        addHtml(doubleToString(value));
        # else // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
        addHtml(toString(value));
        # endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
        html_TD(2);
      }
    }

    addTableSeparator(F("Summary"), 3, 2);

    html_TR_TD();
    addHtml(F("Slots occupied: "));

    if (0 == count) {
      addHtml(F("none"));
    } else {
      addHtmlInt(count);
    }
    addHtml(F("<TD colspan=\"2\">"));
    addHtml(F("Slots available: "));
    addHtmlInt(maxSlots - count);

    if (count > 0) {
      addHtml(F(" of "));
      addHtmlInt(maxSlots);
    }
    html_TD();

    html_end_table();
  } else {
    addHtml(F("External EEPROM not enabled."));
  }
  html_end_form();
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

#endif // if FEATURE_EEPROM_EXTERNAL
