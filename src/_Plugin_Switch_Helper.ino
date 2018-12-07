//##############################################################################
//######## Shared functions for all switch input related plugins. ##############
//##############################################################################

#define PLUGIN_HELPER_DOUBLECLICK_MIN_INTERVAL 1000
#define PLUGIN_HELPER_DOUBLECLICK_MAX_INTERVAL 3000
#define PLUGIN_HELPER_LONGPRESS_MIN_INTERVAL   1000
#define PLUGIN_HELPER_LONGPRESS_MAX_INTERVAL   5000
#define PLUGIN_HELPER_DC_DISABLED              0
#define PLUGIN_HELPER_DC_LOW                   1
#define PLUGIN_HELPER_DC_HIGH                  2
#define PLUGIN_HELPER_DC_BOTH                  3
#define PLUGIN_HELPER_LONGPRESS_DISABLED       0
#define PLUGIN_HELPER_LONGPRESS_LOW            1
#define PLUGIN_HELPER_LONGPRESS_HIGH           2
#define PLUGIN_HELPER_LONGPRESS_BOTH           3

#define PLUGIN_HELPER_BOOT_STATE_ID            "p_hlp_boot"

#define PLUGIN_HELPER_DEBOUNCE_ID              "p_hlp_debounce"

#define PLUGIN_HELPER_DC_EVENT_ID              "p_hlp_dc"
#define PLUGIN_HELPER_DC_MAX_INT_ID            "p_hlp_dcmaxint"

#define PLUGIN_HELPER_LP_EVENT_ID              "p_hlp_lp"
#define PLUGIN_HELPER_LP_MIN_INT_ID            "p_hlp_lpminint"
#define PLUGIN_HELPER_LP_SAFE_ID               "p_hlp_sb"

void addAdvancedEventManagementSubHeader() {
  addFormSubHeader(F("Advanced event management"));
}

void addSendBootStateForm(byte taskIndex, byte settingsOffset) {
  addFormCheckBox(F("Send Boot state"), F(PLUGIN_HELPER_BOOT_STATE_ID),
                  Settings.TaskDevicePluginConfig[taskIndex][settingsOffset]);
}

void saveSendBootStateForm(byte taskIndex, byte settingsOffset) {
  Settings.TaskDevicePluginConfig[taskIndex][settingsOffset] =
      isFormItemChecked(F(PLUGIN_HELPER_BOOT_STATE_ID));
}

void addDebounceForm(byte taskIndex, byte settingsOffset) {
  addFormNumericBox(
      F("De-bounce (ms)"), F(PLUGIN_HELPER_DEBOUNCE_ID),
      round(Settings.TaskDevicePluginConfigFloat[taskIndex][settingsOffset]), 0,
      250);
}

void saveDebounceForm(byte taskIndex, byte settingsOffset) {
  Settings.TaskDevicePluginConfigFloat[taskIndex][settingsOffset] =
      getFormItemInt(F(PLUGIN_HELPER_DEBOUNCE_ID));
}

void setDoubleClickMinInterval(byte taskIndex) {
  if (Settings.TaskDevicePluginConfigFloat[taskIndex][1] < PLUGIN_HELPER_DOUBLECLICK_MIN_INTERVAL)
    Settings.TaskDevicePluginConfigFloat[taskIndex][1] = PLUGIN_HELPER_DOUBLECLICK_MIN_INTERVAL;
}

void addDoubleClickEventForm(byte taskIndex) {
  setDoubleClickMinInterval(taskIndex);

  byte choiceDC = Settings.TaskDevicePluginConfig[taskIndex][4];
  String buttonDC[4];
  buttonDC[0] = F("Disabled");
  buttonDC[1] = F("Active only on LOW (EVENT=3)");
  buttonDC[2] = F("Active only on HIGH (EVENT=3)");
  buttonDC[3] = F("Active on LOW & HIGH (EVENT=3)");
  int buttonDCValues[4] = {PLUGIN_HELPER_DC_DISABLED, PLUGIN_HELPER_DC_LOW,
                           PLUGIN_HELPER_DC_HIGH, PLUGIN_HELPER_DC_BOTH};

  addFormSelector(F("Doubleclick event"), F(PLUGIN_HELPER_DC_EVENT_ID), 4,
                  buttonDC, buttonDCValues, choiceDC);

  addFormNumericBox(F("Doubleclick max. interval (ms)"),
                    F(PLUGIN_HELPER_DC_MAX_INT_ID),
                    round(Settings.TaskDevicePluginConfigFloat[taskIndex][1]),
                    PLUGIN_HELPER_DOUBLECLICK_MIN_INTERVAL,
                    PLUGIN_HELPER_DOUBLECLICK_MAX_INTERVAL);
}

void saveDoubleClickEventForm(byte taskIndex) {
  Settings.TaskDevicePluginConfig[taskIndex][4] =
      getFormItemInt(F(PLUGIN_HELPER_DC_EVENT_ID));
  Settings.TaskDevicePluginConfigFloat[taskIndex][1] =
      getFormItemInt(F(PLUGIN_HELPER_DC_MAX_INT_ID));
}


void setLongPressMinInterval(byte taskIndex) {
  if (Settings.TaskDevicePluginConfigFloat[taskIndex][2] < PLUGIN_HELPER_LONGPRESS_MIN_INTERVAL)
    Settings.TaskDevicePluginConfigFloat[taskIndex][2] = PLUGIN_HELPER_LONGPRESS_MIN_INTERVAL;
}

void addLongPressEventForm(byte taskIndex) {
  setLongPressMinInterval(taskIndex);

  byte choiceLP = Settings.TaskDevicePluginConfig[taskIndex][5];
  String buttonLP[4];
  buttonLP[0] = F("Disabled");
  buttonLP[1] = F("Active only on LOW (EVENT= 10 [NORMAL] or 11 [INVERSED])");
  buttonLP[2] = F("Active only on HIGH (EVENT= 11 [NORMAL] or 10 [INVERSED])");
  buttonLP[3] = F("Active on LOW & HIGH (EVENT= 10 or 11)");
  int buttonLPValues[4] = {
      PLUGIN_HELPER_LONGPRESS_DISABLED, PLUGIN_HELPER_LONGPRESS_LOW,
      PLUGIN_HELPER_LONGPRESS_HIGH, PLUGIN_HELPER_LONGPRESS_BOTH};
  addFormSelector(F("Longpress event"), F(PLUGIN_HELPER_LP_EVENT_ID), 4,
                  buttonLP, buttonLPValues, choiceLP);

  addFormNumericBox(F("Longpress min. interval (ms)"),
                    F(PLUGIN_HELPER_LP_MIN_INT_ID),
                    round(Settings.TaskDevicePluginConfigFloat[taskIndex][2]),
                    PLUGIN_HELPER_LONGPRESS_MIN_INTERVAL,
                    PLUGIN_HELPER_LONGPRESS_MAX_INTERVAL);

  addFormCheckBox(F("Use Safe Button (slower)"), F(PLUGIN_HELPER_LP_SAFE_ID),
                  round(Settings.TaskDevicePluginConfigFloat[taskIndex][3]));

  // TO-DO: add Extra-Long Press event
  // addFormCheckBox(F("Extra-Longpress event (20 & 21)"), F("p001_elp"),
  //                 Settings.TaskDevicePluginConfigLong[taskIndex][1]);
  // addFormNumericBox(F("Extra-Longpress min. interval (ms)"),
  //                 F("p001_elpmininterval"),
  //                 Settings.TaskDevicePluginConfigLong[taskIndex][2], 500, 2000);
}

void saveLongPressEventForm(byte taskIndex) {
  Settings.TaskDevicePluginConfig[taskIndex][5] =
      getFormItemInt(F(PLUGIN_HELPER_LP_EVENT_ID));
  Settings.TaskDevicePluginConfigFloat[taskIndex][2] =
      getFormItemInt(F(PLUGIN_HELPER_LP_MIN_INT_ID));

  Settings.TaskDevicePluginConfigFloat[taskIndex][3] =
      isFormItemChecked(F(PLUGIN_HELPER_LP_SAFE_ID));

  // TO-DO: add Extra-Long Press event
  // Settings.TaskDevicePluginConfigLong[taskIndex][1] =
  //     isFormItemChecked(F("p001_elp"));
  // Settings.TaskDevicePluginConfigLong[taskIndex][2] =
  //     getFormItemInt(F("p001_elpmininterval"));
}
