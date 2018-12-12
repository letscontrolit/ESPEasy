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


//#######################################################################################################
// PLUGIN_WEBFORM_LOAD  &  PLUGIN_WEBFORM_SAVE
//#######################################################################################################

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

void addDebounceForm(byte taskIndex) {
  addFormNumericBox(F("De-bounce (ms)"), F(PLUGIN_HELPER_DEBOUNCE_ID),
                    round(hlp_getDebounceInterval(taskIndex)), 0, 250);
}

void saveDebounceForm(byte taskIndex) {
  hlp_setDebounceInterval(taskIndex,
                          getFormItemInt(F(PLUGIN_HELPER_DEBOUNCE_ID)));
}

void setDoubleClickMinInterval(byte taskIndex) {
  if (hlp_getDoubleClickInterval(taskIndex) <
      PLUGIN_HELPER_DOUBLECLICK_MIN_INTERVAL)
    hlp_setDoubleClickInterval(taskIndex,
                               PLUGIN_HELPER_DOUBLECLICK_MIN_INTERVAL);
}

void addDoubleClickEventForm(byte taskIndex) {
  setDoubleClickMinInterval(taskIndex);

  byte choiceDC = hlp_getUseDoubleClick(taskIndex);
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
                    round(hlp_getDoubleClickInterval(taskIndex)),
                    PLUGIN_HELPER_DOUBLECLICK_MIN_INTERVAL,
                    PLUGIN_HELPER_DOUBLECLICK_MAX_INTERVAL);
}

void saveDoubleClickEventForm(byte taskIndex) {
  hlp_setUseDoubleClick(taskIndex, getFormItemInt(F(PLUGIN_HELPER_DC_EVENT_ID)));
  hlp_setDoubleClickInterval(taskIndex, getFormItemInt(F(PLUGIN_HELPER_DC_MAX_INT_ID)));
}


void setLongPressMinInterval(byte taskIndex) {
  if (hlp_getLongPressInterval(taskIndex) < PLUGIN_HELPER_LONGPRESS_MIN_INTERVAL)
    hlp_setLongPressInterval(taskIndex, PLUGIN_HELPER_LONGPRESS_MIN_INTERVAL);
}

void addLongPressEventForm(byte taskIndex) {
  setLongPressMinInterval(taskIndex);

  byte choiceLP = hlp_getUseLongPress(taskIndex);
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
                    round(hlp_getLongPressInterval(taskIndex)),
                    PLUGIN_HELPER_LONGPRESS_MIN_INTERVAL,
                    PLUGIN_HELPER_LONGPRESS_MAX_INTERVAL);

  addFormCheckBox(F("Use Safe Button (slower)"), F(PLUGIN_HELPER_LP_SAFE_ID),
                  round(hlp_getUseSafeButton(taskIndex)));

  // TO-DO: add Extra-Long Press event
  // addFormCheckBox(F("Extra-Longpress event (20 & 21)"), F("p001_elp"),
  //                 hlp_getClicktimeDoubleClick(taskIndex));
  // addFormNumericBox(F("Extra-Longpress min. interval (ms)"),
  //                 F("p001_elpmininterval"),
  //                 hlp_getClicktimeLongpress(taskIndex), 500, 2000);
}

void saveLongPressEventForm(byte taskIndex) {
  hlp_setUseLongPress(taskIndex, getFormItemInt(F(PLUGIN_HELPER_LP_EVENT_ID)));
  hlp_setLongPressInterval(taskIndex,
                           getFormItemInt(F(PLUGIN_HELPER_LP_MIN_INT_ID)));

  hlp_setUseSafeButton(taskIndex,
                       isFormItemChecked(F(PLUGIN_HELPER_LP_SAFE_ID)));

  // TO-DO: add Extra-Long Press event
  // hlp_setClicktimeDoubleClick(taskIndex, isFormItemChecked(F("p001_elp")));
  // hlp_setClicktimeLongpress(taskIndex,
  // getFormItemInt(F("p001_elpmininterval")));
}

// check if a task has been edited and remove 'task' bit from the previous pin
void update_globalMapPortStatus_onSave(byte taskIndex, byte pluginID) {
  for (auto it = globalMapPortStatus.begin(); it != globalMapPortStatus.end();
       ++it) {
    if (it->second.previousTask == taskIndex &&
        getPluginFromKey(it->first) == pluginID) {
      globalMapPortStatus[it->first].previousTask = -1;
      removeTaskFromPort(it->first);
      break;
    }
  }
}

//#######################################################################################################
// PLUGIN_TEN_PER_SECOND actions
//#######################################################################################################

// Reset timer for long press
void hlp_resetLongPressTimer(byte taskIndex) {
  hlp_setClicktimeLongpress(taskIndex, millis());
  hlp_setLongPressFired(taskIndex,  false);
}


bool hlp_debounceTimoutPassed(byte taskIndex) {
  const unsigned long debounceTime = timePassedSince(hlp_getClicktimeDebounce(taskIndex));
  return (debounceTime >= (unsigned long)lround(hlp_getDebounceInterval(taskIndex)));
}

void hlp_processDoubleClick(byte taskIndex, bool gpio_state) {
  const unsigned long deltaDC = timePassedSince(hlp_getClicktimeDoubleClick(taskIndex));
  if ((deltaDC >= (unsigned long)lround(Settings.TaskDevicePluginConfigFloat[taskIndex][1])) ||
       hlp_getDoubleClickCounter(taskIndex) == 3)
  {
    //reset timer for doubleclick
    // FIXME TD-er: Temporary counters and states should not be in settings.
    hlp_setDoubleClickCounter(taskIndex, 0);
    hlp_setClicktimeDoubleClick(taskIndex, millis());
  }

  //just to simplify the reading of the code
  const int16_t COUNTER = Settings.TaskDevicePluginConfig[taskIndex][7];
  const int16_t DC = hlp_getUseDoubleClick(taskIndex);

  //check settings for doubleclick according to the settings
  if ( COUNTER != 0 ||
       ( COUNTER == 0 &&
        (DC == 3 ||
        (DC == 1 && !gpio_state) ||
        (DC == 2 && gpio_state))) )
  {
    Settings.TaskDevicePluginConfig[taskIndex][7]++;
  }
}

bool hlp_LongPressIntervalReached(byte taskIndex) {
  const unsigned long deltaLP = timePassedSince(hlp_getClicktimeLongpress(taskIndex));
  return deltaLP >= (unsigned long)lround(hlp_getLongPressInterval(taskIndex));
}

byte hlp_getOutputValue_updateSendState(byte taskIndex, bool& sendState) {
  byte output_value;
  if (Settings.TaskDevicePin1Inversed[taskIndex])
    sendState = !sendState;

  if (hlp_getLongPressFired(taskIndex)) {
    output_value = sendState ? 11 : 10;
  }
  else if (hlp_getDoubleClickCounter(taskIndex) == 3 && hlp_getUseDoubleClick(taskIndex) > 0)
  {
    output_value = 3; //double click
  } else {
    output_value = sendState ? 1 : 0; //single click
  }
  return output_value;
}

void hlp_logState_and_Output(byte taskIndex, bool gpio_state, bool sendState, byte output_value, byte pluginId) {
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(64);
    switch (pluginId) {
      case  0: log += F("SW "); break;
      case  9: log += F("MCP"); break;
      case 19: log += F("PCF"); break;
      default: log += F("???"); break;
    }
    log += F("  : Port=");
    (pluginId==0) ? log += Settings.TaskDevicePin1[taskIndex] : log += Settings.TaskDevicePort[taskIndex];
    log += F(" pinState=");
    log += gpio_state ? '1' : '0';
    log += F(" sendState=");
    log += sendState ? '1' : '0';
    switch (output_value) {
      case 3: log += F(" Doubleclick="); break;
      case 10:
      case 11: log += F(" LongPress="); break;

      default:
       log += F(" Output value=");
      break;
    }
    log += output_value;
    addLog(LOG_LEVEL_INFO, log);
  }
}

bool hlp_LongPressEnabled_and_notFired(byte taskIndex, bool gpio_state) {
  //just to simplify the reading of the code
  const int16_t LP = hlp_getUseLongPress(taskIndex);
  const int16_t FIRED = hlp_getLongPressFired(taskIndex);
  //Check if LP is enabled and if LP has not fired yet
  if (FIRED) return false;
  return (
    LP == 3 ||
    (LP == 1 && !gpio_state) ||
    (LP == 2 && gpio_state) );
}

//#######################################################################################################
// Settings for switch plugins
//#######################################################################################################

/**************************************************\
TaskDevicePluginConfig settings:
0 or 3: send boot state (true,false)
1:
2:
3:
4: use doubleclick (0,1,2,3)
5: use longpress (0,1,2,3)
6: LP fired (true,false)
7: doubleclick counter (=0,1,2,3)
\**************************************************/
int16_t hlp_getMustSendBootState(byte taskIndex, byte settingsOffset) {
  return Settings.TaskDevicePluginConfig[taskIndex][settingsOffset];
}

void hlp_setMustSendBootState(byte taskIndex, byte settingsOffset, int16_t value) {
   Settings.TaskDevicePluginConfig[taskIndex][settingsOffset] = value;
}


int16_t hlp_getUseDoubleClick(byte taskIndex) {
  return Settings.TaskDevicePluginConfig[taskIndex][4];
}

void hlp_setUseDoubleClick(byte taskIndex, int16_t value) {
  Settings.TaskDevicePluginConfig[taskIndex][4] = value;
}

int16_t hlp_getUseLongPress(byte taskIndex) {
  return Settings.TaskDevicePluginConfig[taskIndex][5];
}

void hlp_setUseLongPress(byte taskIndex, int16_t value) {
  Settings.TaskDevicePluginConfig[taskIndex][5] = value;
}

int16_t hlp_getLongPressFired(byte taskIndex) {
  return Settings.TaskDevicePluginConfig[taskIndex][6];
}

// FIXME TD-er: Mainly used to store temporary values.
void hlp_setLongPressFired(byte taskIndex, int16_t value) {
  Settings.TaskDevicePluginConfig[taskIndex][6] = value;
}

int16_t hlp_getDoubleClickCounter(byte taskIndex) {
  return Settings.TaskDevicePluginConfig[taskIndex][7];
}

// FIXME TD-er: Mainly used to store temporary values.
void hlp_setDoubleClickCounter(byte taskIndex, int16_t value) {
   Settings.TaskDevicePluginConfig[taskIndex][7] = value;
}


/**************************************************\
TaskDevicePluginConfigFloat settings:
0: debounce interval ms
1: doubleclick interval ms
2: longpress interval ms
3: use safebutton (=0,1)
\**************************************************/
float hlp_getDebounceInterval(byte taskIndex) {
  return Settings.TaskDevicePluginConfigFloat[taskIndex][0];
}

void hlp_setDebounceInterval(byte taskIndex, float value) {
  Settings.TaskDevicePluginConfigFloat[taskIndex][0] = value;
}

float hlp_getDoubleClickInterval(byte taskIndex) {
  return Settings.TaskDevicePluginConfigFloat[taskIndex][1];
}

void hlp_setDoubleClickInterval(byte taskIndex, float value) {
  Settings.TaskDevicePluginConfigFloat[taskIndex][1] = value;
}

float hlp_getLongPressInterval(byte taskIndex) {
  return Settings.TaskDevicePluginConfigFloat[taskIndex][2];
}

void hlp_setLongPressInterval(byte taskIndex, float value) {
  Settings.TaskDevicePluginConfigFloat[taskIndex][2] = value;
}

float hlp_getUseSafeButton(byte taskIndex) {
  return Settings.TaskDevicePluginConfigFloat[taskIndex][3];
}

void hlp_setUseSafeButton(byte taskIndex, float value) {
  Settings.TaskDevicePluginConfigFloat[taskIndex][3] = value;
}

/**************************************************\
TaskDevicePluginConfigLong settings:
0: clickTime debounce ms
1: clickTime doubleclick ms
2: clickTime longpress ms
3: safebutton counter (=0,1)
\**************************************************/
long hlp_getClicktimeDebounce(byte taskIndex) {
  return Settings.TaskDevicePluginConfigLong[taskIndex][0];
}

// FIXME TD-er: Mainly used to store temporary values.
void hlp_setClicktimeDebounce(byte taskIndex, long value) {
  Settings.TaskDevicePluginConfigLong[taskIndex][0] = value;
}

long hlp_getClicktimeDoubleClick(byte taskIndex) {
  return Settings.TaskDevicePluginConfigLong[taskIndex][1];
}

// FIXME TD-er: Mainly used to store temporary values.
void hlp_setClicktimeDoubleClick(byte taskIndex, long value) {
  Settings.TaskDevicePluginConfigLong[taskIndex][1] = value;
}

long hlp_getClicktimeLongpress(byte taskIndex) {
  return Settings.TaskDevicePluginConfigLong[taskIndex][2];
}

// FIXME TD-er: Mainly used to store temporary values.
void hlp_setClicktimeLongpress(byte taskIndex, long value) {
  Settings.TaskDevicePluginConfigLong[taskIndex][2] = value;
}

long hlp_getSafebuttonCounter(byte taskIndex) {
  return Settings.TaskDevicePluginConfigLong[taskIndex][3];
}

// FIXME TD-er: Mainly used to store temporary values.
void hlp_setSafebuttonCounter(byte taskIndex, long value) {
  Settings.TaskDevicePluginConfigLong[taskIndex][3] = value;
}
