#ifdef WEBSERVER_FACTORY_RESET

#include "src/Globals/ResetFactoryDefaultPref.h"

// ********************************************************************************
// Web Interface Factory Reset
// ********************************************************************************
void handle_factoryreset(void) {
  checkRAM(F("handle_factoryreset"));

  if (!isLoggedIn(void)) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream(void);
  sendHeadandTail_stdtemplate(_HEAD);
  html_add_form(void);
  html_table_class_normal(void);
  html_TR(void);
  addFormHeader(F("Factory Reset"));

  if (WebServer.hasArg("fdm")) {
    DeviceModel model = static_cast<DeviceModel>(getFormItemInt("fdm"));

    if (modelMatchingFlashSize(model)) {
      setFactoryDefault(model);
    }
  }

  if (WebServer.hasArg(F("savepref"))) {
    // User choose a pre-defined config and wants to save it as the new default.
    ResetFactoryDefaultPreference.keepUnitName(isFormItemChecked("kun"));
    ResetFactoryDefaultPreference.keepWiFi(isFormItemChecked("kw"));
    ResetFactoryDefaultPreference.keepNetwork(isFormItemChecked("knet"));
    ResetFactoryDefaultPreference.keepNTP(isFormItemChecked("kntp"));
    ResetFactoryDefaultPreference.keepLogSettings(isFormItemChecked("klog"));
    applyFactoryDefaultPref(void);
    addHtmlError(SaveSettings(void));
  }

  if (WebServer.hasArg(F("performfactoryreset"))) {
    // User confirmed to really perform the reset.
    applyFactoryDefaultPref(void);

    // No need to call SaveSettings(void); ResetFactory(void) will save the new settings.
    ResetFactory(void);
  } else {
    // Nothing chosen yet, show options.
    addTableSeparator(F("Settings to keep"), 2, 3);

    addRowLabel(F("Keep Unit/Name"));
    addCheckBox("kun", ResetFactoryDefaultPreference.keepUnitName(void));

    addRowLabel(F("Keep WiFi config"));
    addCheckBox("kw", ResetFactoryDefaultPreference.keepWiFi(void));

    addRowLabel(F("Keep Network config"));
    addCheckBox("knet", ResetFactoryDefaultPreference.keepNetwork(void));

    addRowLabel(F("Keep NTP/DST config"));
    addCheckBox("kntp", ResetFactoryDefaultPreference.keepNTP(void));

    addRowLabel(F("Keep log config"));
    addCheckBox("klog", ResetFactoryDefaultPreference.keepLogSettings(void));

    addTableSeparator(F("Pre-defined configurations"), 2, 3);
    addRowLabel(F("Pre-defined config"));
    addPreDefinedConfigSelector(void);


    html_TR_TD(void);
    html_TD(void);
    addSubmitButton(F("Save Preferences"), F("savepref"));


    html_TR_TD_height(30);

    addTableSeparator(F("Immediate full reset"), 2, 3);
    addRowLabel(F("Erase settings files"));
    addSubmitButton(F("Factory Reset"), F("performfactoryreset"), F("red"));
  }

  html_end_table(void);
  html_end_form(void);
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream(void);
}

// ********************************************************************************
// Create pre-defined config selector
// ********************************************************************************
void addPreDefinedConfigSelector(void) {
  DeviceModel active_model = ResetFactoryDefaultPreference.getDeviceModel(void);

  addSelector_Head("fdm", true);

  for (byte x = 0; x < DeviceModel_MAX; ++x) {
    DeviceModel model = static_cast<DeviceModel>(x);
    addSelector_Item(
      getDeviceModelString(model),
      x,
      model == active_model,
      !modelMatchingFlashSize(model),
      ""
      );
  }
  addSelector_Foot(void);
}

#ifdef WEBSERVER_NEW_UI
void handle_factoryreset_json(void) {
  if (!isLoggedIn(void)) { return; }
  TXBuffer.startJsonStream(void);
  TXBuffer += "{";

  if (WebServer.hasArg("fdm")) {
    DeviceModel model = static_cast<DeviceModel>(getFormItemInt("fdm"));

    if (modelMatchingFlashSize(model)) {
      setFactoryDefault(model);
    }
  }

  if (WebServer.hasArg("kun")) {
    ResetFactoryDefaultPreference.keepUnitName(isFormItemChecked("kun"));
  }

  if (WebServer.hasArg("kw")) {
    ResetFactoryDefaultPreference.keepWiFi(isFormItemChecked("kw"));
  }

  if (WebServer.hasArg("knet")) {
    ResetFactoryDefaultPreference.keepNetwork(isFormItemChecked("knet"));
  }

  if (WebServer.hasArg("kntp")) {
    ResetFactoryDefaultPreference.keepNTP(isFormItemChecked("kntp"));
  }

  if (WebServer.hasArg("klog")) {
    ResetFactoryDefaultPreference.keepLogSettings(isFormItemChecked("klog"));
  }
  String error;
  bool performReset = false;
  bool savePref = false;
  if (WebServer.hasArg(F("savepref"))) {
    // User choose a pre-defined config and wants to save it as the new default.
    savePref = true;
  }

  if (WebServer.hasArg(F("performfactoryreset"))) {
    // User confirmed to really perform the reset.
    performReset = true;
    savePref = true;
  } else {
    error = F("no reset");
  }
  if (savePref) {
    applyFactoryDefaultPref(void);
    error = SaveSettings(void);
  }

  if (error.length(void) == 0) {
    error = F("ok");
  }

  stream_last_json_object_value(F("status"), error);
  TXBuffer += "}";
  TXBuffer.endStream(void);

  if (performReset) {
    ResetFactory(void);
  }
}

#endif // WEBSERVER_NEW_UI

#endif // ifdef WEBSERVER_FACTORY_RESET
