#include "../WebServer/EepromVarPage.h"

#if FEATURE_EEPROM_EXTERNAL || FEATURE_RTC_SRAM_STORAGE

# include "../WebServer/ESPEasy_WebServer.h"
# include "../WebServer/AccessControl.h"
# include "../WebServer/Markup.h"
# include "../WebServer/Markup_Forms.h"
# include "../WebServer/HTML_wrappers.h"

# include "../Globals/Settings.h"
# include "../Globals/ExtraTaskSettings.h"

# if FEATURE_EEPROM_EXTERNAL
#  include "../../ESPEasy/eeprom/Helpers/EEPROMExternal.h"
# endif // if FEATURE_EEPROM_EXTERNAL
# if FEATURE_RTC_SRAM_STORAGE
#  include "../../ESPEasy/eeprom/Helpers/RTCSRAMStorage.h"
# endif // if FEATURE_RTC_SRAM_STORAGE
# include "../Helpers/ESPEasy_Storage.h"

# include "../Helpers/StringConverter.h"

void handle_eepromvars() {
  if (!isLoggedIn()) { return; }

  if (!startStream_send_stdTemplate(MENU_INDEX_TOOLS)) { return; }

  # if FEATURE_EEPROM_EXTERNAL

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
      const ESPEASY_RULES_FLOAT_TYPE value = ESPEasy::eeprom::readEEPROMSlot(slot);

      if (slot % 50 == 0) { delay(0); }

      if (!isnan(value) && !essentiallyZero(value)) {
        ++count;
        html_TR_TD();
        addHtmlInt(slot);
        html_TD();
        #  if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
        addHtml(doubleToString(value, ESPEASY_DOUBLE_NR_DECIMALS, true));
        #  else // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
        addHtml(toString(value, ESPEASY_FLOAT_NR_DECIMALS, true));
        #  endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
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
    addHtml(F("External EEPROM not enabled.<BR>"));
  }
  # endif // if FEATURE_EEPROM_EXTERNAL
  # if FEATURE_RTC_SRAM_STORAGE

  if (ESPEasy::eeprom::checkRTCSRAMEnabled() > 0) {
    // the table header
    html_table_class_normal();
    html_TR();
    html_table_header(F("External RTC SRAM"),
                      300);
    html_table_header(toString(Settings.ExtTimeSource()),
                      500);
    html_table_header(F(""),
                      400);
    html_table_header(F(""));

    html_TR();

    // sub-table header
    html_table_header(F("Slot"),                         300);
    html_table_header(F("Value (only non-zero values)"), 500);
    html_table_header(F(""));
    html_table_header(F(""));

    const uint32_t maxSlots = ESPEasy::eeprom::getRTCSRAMMaxSlots();
    uint32_t count{};

    for (uint32_t slot = 0; slot < maxSlots; ++slot) {
      const ESPEASY_RULES_FLOAT_TYPE value = ESPEasy::eeprom::readRTCSRAMSlot(slot);

      if (slot % 50 == 0) { delay(0); }

      if (!isnan(value) && !essentiallyZero(value)) {
        ++count;
        html_TR_TD();
        addHtmlInt(slot);
        html_TD();
        #  if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
        addHtml(doubleToString(value, ESPEASY_DOUBLE_NR_DECIMALS, true));
        #  else // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
        addHtml(toString(value, ESPEASY_FLOAT_NR_DECIMALS, true));
        #  endif // if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
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
    addHtml(F("External RTC SRAM not available."));
  }
  # endif // if FEATURE_RTC_SRAM_STORAGE
  html_end_form();
  sendTail_stdtemplate();
}

#endif // if FEATURE_EEPROM_EXTERNAL || FEATURE_RTC_SRAM_STORAGE
