#include "../WebServer/EepromVarPage.h"


#include "../WebServer/ESPEasy_WebServer.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Forms.h"
#include "../WebServer/HTML_wrappers.h"

#include "../Globals/Settings.h"
#include "../Globals/ExtraTaskSettings.h"

#include "../Helpers/EEPROMExternal.h"
#include "../Helpers/ESPEasy_Storage.h"

#include "../Helpers/StringConverter.h"

// #if FEATURE_STRING_VARIABLES
// # include "../Helpers/StringParser.h"
// #endif // if

#if FEATURE_EEPROM_EXTERNAL

void handle_eepromvars() {
  if (!isLoggedIn()) { return; }

  const bool showTasks = getFormItemInt(F("tasks"), 0) != 0;

  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  if (checkEEPROMEnabled() > 0) {
    // the table header
    html_table_class_normal();
    html_TR();
    html_table_header(F("External EEPROM"),                                                             300);
    html_table_header(getEEPROMName(static_cast<EEPROMExternal_Type_e>(Settings.EEPROMExternalType())), 500);
    html_table_header(isEEPROMExternalWriteProtected() ? F("Write-protected!") : F(""),                 400);
    html_table_header(F(""));

    if (showTasks) {
      html_TR();

      // sub-table header
      html_table_header(F("Task"),    300);
      html_table_header(F("Value"),   500);
      html_table_header(F("Content"), 400);
      html_table_header(F(""));

      for (taskIndex_t tsk = 0; tsk < TASKS_MAX; ++tsk) {
        LoadTaskSettings(tsk);
        html_TR_TD();
        addHtmlInt(tsk + 1);
        addHtml(' ');
        addHtml(getTaskDeviceName(tsk));

        for (taskVarIndex_t var = 0; var < VARS_PER_TASK; ++var) {
          if (var != 0) {
            html_TR_TD();
          }
          html_TD();
          addHtmlInt(var + 1);
          addHtml(' ');
          addHtml(getTaskValueName(tsk, var));
          html_TD();
          const uint32_t addr  = getEEPROMAddressForTaskValue(tsk, var);
          const float    value = EEPROMExternal->readFloat(addr);
          const uint32_t data  = EEPROMExternal->readLong(addr);

          if (isnan(value) || (addr == std::numeric_limits<uint32_t>::max())) {
            addHtml('-');
          } else {
            addHtml(strformat(F("%s (0x%04x)"), floatToString(value, ExtraTaskSettings.TaskDeviceValueDecimals[var]), data));
          }
        }
        delay(0);
      }
    }
    html_TR();

    // sub-table header
    html_table_header(F("Slot"),                         300);
    html_table_header(F("Value (only non-zero values)"), 500);
    html_table_header(F(""));
    html_table_header(F(""));

    const uint32_t maxSlots = getEEPROMMaxSlots();
    uint32_t count{};

    for (uint32_t slot = 0; slot < maxSlots; ++slot) {
      const float value = readEEPROMSlot(slot);

      if (slot % 50 == 0) { delay(0); }

      if (!isnan(value) && !essentiallyZero(value)) {
        ++count;
        html_TR_TD();
        addHtmlInt(slot);
        html_TD();
        addHtml(toString(value));
        html_TD(2);
      }
    }

    // TODO: List String values in EEPROM
    // TODO: List C016 Cache entries in EEPROM

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
