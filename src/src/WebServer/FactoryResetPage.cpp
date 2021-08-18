#include "../WebServer/FactoryResetPage.h"


#ifdef WEBSERVER_FACTORY_RESET

#include "../WebServer/WebServer.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/JSON.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"
#include "../WebServer/Markup_Forms.h"

#include "../DataTypes/DeviceModel.h"

#include "../Globals/ResetFactoryDefaultPref.h"

#include "../Helpers/ESPEasy_FactoryDefault.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Hardware.h"

// ********************************************************************************
// Web Interface Factory Reset
// ********************************************************************************
void handle_factoryreset() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_factoryreset"));
  #endif

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);
  html_add_form();
  html_table_class_normal();
  html_TR();
  addFormHeader(F("Factory Reset"));

#ifndef LIMIT_BUILD_SIZE
  if (web_server.hasArg(F("fdm"))) {
    DeviceModel model = static_cast<DeviceModel>(getFormItemInt(F("fdm")));

    if (modelMatchingFlashSize(model)) {
      setFactoryDefault(model);
    }
  }


  if (web_server.hasArg(F("savepref"))) {
    // User choose a pre-defined config and wants to save it as the new default.
    ResetFactoryDefaultPreference.keepUnitName(isFormItemChecked(F("kun")));
    ResetFactoryDefaultPreference.keepWiFi(isFormItemChecked(F("kw")));
    ResetFactoryDefaultPreference.keepNetwork(isFormItemChecked(F("knet")));
    ResetFactoryDefaultPreference.keepNTP(isFormItemChecked(F("kntp")));
    ResetFactoryDefaultPreference.keepLogSettings(isFormItemChecked(F("klog")));
    applyFactoryDefaultPref();
    addHtmlError(SaveSettings());
  }
#endif

  if (web_server.hasArg(F("performfactoryreset"))) {
    // User confirmed to really perform the reset.
    applyFactoryDefaultPref();

    // No need to call SaveSettings(); ResetFactory() will save the new settings.
    ResetFactory();
  } else {
    #ifndef LIMIT_BUILD_SIZE
    // Nothing chosen yet, show options.
    addTableSeparator(F("Settings to keep"), 2, 3);

    addRowLabel(F("Keep Unit/Name"));
    addCheckBox(F("kun"), ResetFactoryDefaultPreference.keepUnitName());

    addRowLabel(F("Keep WiFi config"));
    addCheckBox(F("kw"), ResetFactoryDefaultPreference.keepWiFi());

    addRowLabel(F("Keep Network config"));
    addCheckBox(F("knet"), ResetFactoryDefaultPreference.keepNetwork());

    addRowLabel(F("Keep NTP/DST config"));
    addCheckBox(F("kntp"), ResetFactoryDefaultPreference.keepNTP());

    addRowLabel(F("Keep log config"));
    addCheckBox(F("klog"), ResetFactoryDefaultPreference.keepLogSettings());

    addTableSeparator(F("Pre-defined configurations"), 2, 3);
    addRowLabel(F("Pre-defined config"));
    addPreDefinedConfigSelector();


    html_TR_TD();
    html_TD();
    addSubmitButton(F("Save Preferences"), F("savepref"));
    #endif

    html_TR_TD_height(30);

    addTableSeparator(F("Immediate full reset"), 2, 3);
    addRowLabel(F("Erase settings files"));
    addSubmitButton(F("Factory Reset"), F("performfactoryreset"), F("red"));
  }

  html_end_table();
  html_end_form();
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

// ********************************************************************************
// Create pre-defined config selector
// ********************************************************************************
void addPreDefinedConfigSelector() {
  DeviceModel active_model = ResetFactoryDefaultPreference.getDeviceModel();

  addSelector_Head_reloadOnChange(F("fdm"));

  for (uint8_t x = 0; x < static_cast<uint8_t>(DeviceModel::DeviceModel_MAX); ++x) {
    DeviceModel model = static_cast<DeviceModel>(x);
    if (modelMatchingFlashSize(model)) {
      addSelector_Item(
        getDeviceModelString(model),
        x,
        model == active_model);
    }
  }
  addSelector_Foot();
}

#ifdef WEBSERVER_NEW_UI
void handle_factoryreset_json() {
  if (!isLoggedIn()) { return; }
  TXBuffer.startJsonStream();
  addHtml('{');
#ifndef LIMIT_BUILD_SIZE
  if (web_server.hasArg(F("fdm"))) {
    DeviceModel model = static_cast<DeviceModel>(getFormItemInt(F("fdm")));

    if (modelMatchingFlashSize(model)) {
      setFactoryDefault(model);
    }
  }

  if (web_server.hasArg(F("kun"))) {
    ResetFactoryDefaultPreference.keepUnitName(isFormItemChecked(F("kun")));
  }

  if (web_server.hasArg(F("kw"))) {
    ResetFactoryDefaultPreference.keepWiFi(isFormItemChecked(F("kw")));
  }

  if (web_server.hasArg(F("knet"))) {
    ResetFactoryDefaultPreference.keepNetwork(isFormItemChecked(F("knet")));
  }

  if (web_server.hasArg(F("kntp"))) {
    ResetFactoryDefaultPreference.keepNTP(isFormItemChecked(F("kntp")));
  }

  if (web_server.hasArg(F("klog"))) {
    ResetFactoryDefaultPreference.keepLogSettings(isFormItemChecked(F("klog")));
  }
#endif
  String error;
  bool   performReset = false;
  bool   savePref     = false;

  if (web_server.hasArg(F("savepref"))) {
    // User choose a pre-defined config and wants to save it as the new default.
    savePref = true;
  }

  if (web_server.hasArg(F("performfactoryreset"))) {
    // User confirmed to really perform the reset.
    performReset = true;
    savePref     = true;
  } else {
    error = F("no reset");
  }

  if (savePref) {
    applyFactoryDefaultPref();
    error = SaveSettings();
  }

  if (error.isEmpty()) {
    error = F("ok");
  }

  stream_last_json_object_value(F("status"), error);
  addHtml('}');
  TXBuffer.endStream();

  if (performReset) {
    ResetFactory();
  }
}

#endif // WEBSERVER_NEW_UI

#endif // ifdef WEBSERVER_FACTORY_RESET
