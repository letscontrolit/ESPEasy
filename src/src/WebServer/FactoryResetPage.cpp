#include "../WebServer/FactoryResetPage.h"


#ifdef WEBSERVER_FACTORY_RESET

#include "../WebServer/WebServer.h"
#include "../WebServer/HTML_wrappers.h"
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
  checkRAM(F("handle_factoryreset"));

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);
  html_add_form();
  html_table_class_normal();
  html_TR();
  addFormHeader(F("Factory Reset"));

  if (web_server.hasArg("fdm")) {
    DeviceModel model = static_cast<DeviceModel>(getFormItemInt("fdm"));

    if (modelMatchingFlashSize(model)) {
      setFactoryDefault(model);
    }
  }

  if (web_server.hasArg(F("savepref"))) {
    // User choose a pre-defined config and wants to save it as the new default.
    ResetFactoryDefaultPreference.keepUnitName(isFormItemChecked("kun"));
    ResetFactoryDefaultPreference.keepWiFi(isFormItemChecked("kw"));
    ResetFactoryDefaultPreference.keepNetwork(isFormItemChecked("knet"));
    ResetFactoryDefaultPreference.keepNTP(isFormItemChecked("kntp"));
    ResetFactoryDefaultPreference.keepLogSettings(isFormItemChecked("klog"));
    applyFactoryDefaultPref();
    addHtmlError(SaveSettings());
  }

  if (web_server.hasArg(F("performfactoryreset"))) {
    // User confirmed to really perform the reset.
    applyFactoryDefaultPref();

    // No need to call SaveSettings(); ResetFactory() will save the new settings.
    ResetFactory();
  } else {
    // Nothing chosen yet, show options.
    addTableSeparator(F("Settings to keep"), 2, 3);

    addRowLabel(F("Keep Unit/Name"));
    addCheckBox("kun", ResetFactoryDefaultPreference.keepUnitName());

    addRowLabel(F("Keep WiFi config"));
    addCheckBox("kw", ResetFactoryDefaultPreference.keepWiFi());

    addRowLabel(F("Keep Network config"));
    addCheckBox("knet", ResetFactoryDefaultPreference.keepNetwork());

    addRowLabel(F("Keep NTP/DST config"));
    addCheckBox("kntp", ResetFactoryDefaultPreference.keepNTP());

    addRowLabel(F("Keep log config"));
    addCheckBox("klog", ResetFactoryDefaultPreference.keepLogSettings());

    addTableSeparator(F("Pre-defined configurations"), 2, 3);
    addRowLabel(F("Pre-defined config"));
    addPreDefinedConfigSelector();


    html_TR_TD();
    html_TD();
    addSubmitButton(F("Save Preferences"), F("savepref"));


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

  addSelector_Head_reloadOnChange("fdm");

  for (byte x = 0; x < DeviceModel_MAX; ++x) {
    DeviceModel model = static_cast<DeviceModel>(x);
    if (modelMatchingFlashSize(model)) {
      addSelector_Item(
        getDeviceModelString(model),
        x,
        model == active_model,
        false,
        ""
        );
    }
  }
  addSelector_Foot();
}

#ifdef WEBSERVER_NEW_UI
void handle_factoryreset_json() {
  if (!isLoggedIn()) { return; }
  TXBuffer.startJsonStream();
  addHtml("{");

  if (web_server.hasArg("fdm")) {
    DeviceModel model = static_cast<DeviceModel>(getFormItemInt("fdm"));

    if (modelMatchingFlashSize(model)) {
      setFactoryDefault(model);
    }
  }

  if (web_server.hasArg("kun")) {
    ResetFactoryDefaultPreference.keepUnitName(isFormItemChecked("kun"));
  }

  if (web_server.hasArg("kw")) {
    ResetFactoryDefaultPreference.keepWiFi(isFormItemChecked("kw"));
  }

  if (web_server.hasArg("knet")) {
    ResetFactoryDefaultPreference.keepNetwork(isFormItemChecked("knet"));
  }

  if (web_server.hasArg("kntp")) {
    ResetFactoryDefaultPreference.keepNTP(isFormItemChecked("kntp"));
  }

  if (web_server.hasArg("klog")) {
    ResetFactoryDefaultPreference.keepLogSettings(isFormItemChecked("klog"));
  }
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

  if (error.length() == 0) {
    error = F("ok");
  }

  stream_last_json_object_value(F("status"), error);
  addHtml("}");
  TXBuffer.endStream();

  if (performReset) {
    ResetFactory();
  }
}

#endif // WEBSERVER_NEW_UI

#endif // ifdef WEBSERVER_FACTORY_RESET
