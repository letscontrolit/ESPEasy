#include "../Helpers/_Plugin_Helper_webform.h"

#include "../Globals/GlobalMapPortStatus.h"

#include "../Helpers/PortStatus.h"

#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Forms.h"


#define SWITCH_DOUBLECLICK_MAX_INTERVAL      3000
#define SWITCH_LONGPRESS_MAX_INTERVAL        5000
#define SWITCH_DC_DISABLED                   0
#define SWITCH_DC_LOW                        1
#define SWITCH_DC_HIGH                       2
#define SWITCH_DC_BOTH                       3
#define SWITCH_LONGPRESS_DISABLED            0
#define SWITCH_LONGPRESS_LOW                 1
#define SWITCH_LONGPRESS_HIGH                2
#define SWITCH_LONGPRESS_BOTH                3


void SwitchWebformLoad(
  const int16_t& bootState,
  const float  & debounce_ms,
  const int16_t& doubleClickEvent,
  float        & doubleClickMaxInterval,
  const int16_t& longPressEvent,
  float        & longPressMinInterval_ms,
  const float  & useSafeButton)
{
  addFormCheckBox(F("Send Boot state"), F("sw_boot"),
                  bootState);

  addFormSubHeader(F("Advanced event management"));

  addFormNumericBox(F("De-bounce (ms)"), F("sw_debounce"), lround(debounce_ms), 0, 250);

  // set minimum value for doubleclick MIN max speed
  if (doubleClickMaxInterval < SWITCH_DOUBLECLICK_MIN_INTERVAL) {
    doubleClickMaxInterval = SWITCH_DOUBLECLICK_MIN_INTERVAL;
  }

  {
    uint8_t choiceDC                       = doubleClickEvent;
    const __FlashStringHelper *buttonDC[4] = {
      F("Disabled"),
      F("Active only on LOW (EVENT=3)"),
      F("Active only on HIGH (EVENT=3)"),
      F("Active on LOW & HIGH (EVENT=3)")
    };
    int buttonDCValues[4] = { SWITCH_DC_DISABLED, SWITCH_DC_LOW, SWITCH_DC_HIGH, SWITCH_DC_BOTH };

    addFormSelector(F("Doubleclick event"), F("sw_dc"), 4, buttonDC, buttonDCValues, choiceDC);
  }

  addFormNumericBox(F("Doubleclick max. interval (ms)"),
                    F("sw_dcmaxinterval"),
                    lround(doubleClickMaxInterval),
                    SWITCH_DOUBLECLICK_MIN_INTERVAL,
                    SWITCH_DOUBLECLICK_MAX_INTERVAL);

  // set minimum value for longpress MIN max speed
  if (longPressMinInterval_ms < SWITCH_LONGPRESS_MIN_INTERVAL) {
    longPressMinInterval_ms = SWITCH_LONGPRESS_MIN_INTERVAL;
  }

  {
    uint8_t choiceLP                       = longPressEvent;
    const __FlashStringHelper *buttonLP[4] = {
      F("Disabled"),
      F("Active only on LOW (EVENT= 10 [NORMAL] or 11 [INVERSED])"),
      F("Active only on HIGH (EVENT= 11 [NORMAL] or 10 [INVERSED])"),
      F("Active on LOW & HIGH (EVENT= 10 or 11)")
    };
    int buttonLPValues[4] =
    { SWITCH_LONGPRESS_DISABLED, SWITCH_LONGPRESS_LOW, SWITCH_LONGPRESS_HIGH, SWITCH_LONGPRESS_BOTH };
    addFormSelector(F("Longpress event"), F("sw_lp"), 4, buttonLP, buttonLPValues, choiceLP);
  }

  addFormNumericBox(F("Longpress min. interval (ms)"),
                    F("sw_lpmininterval"),
                    lround(longPressMinInterval_ms),
                    SWITCH_LONGPRESS_MIN_INTERVAL,
                    SWITCH_LONGPRESS_MAX_INTERVAL);

  addFormCheckBox(F("Use Safe Button (slower)"), F("sw_sb"), lround(useSafeButton));

  // TO-DO: add Extra-Long Press event
  // addFormCheckBox(F("Extra-Longpress event (20 & 21)"), F("sw_elp"), PCONFIG_LONG(1));
  // addFormNumericBox(F("Extra-Longpress min. interval (ms)"), F("sw_elpmininterval"), PCONFIG_LONG(2), 500, 2000);
}

void SwitchWebformSave(
  taskIndex_t TaskIndex,
  pluginID_t  pluginID,
  int16_t   & bootState,
  float     & debounce_ms,
  int16_t   & doubleClickEvent,
  float     & doubleClickMaxInterval,
  int16_t   & longPressEvent,
  float     & longPressMinInterval_ms,
  float     & useSafeButton)
{
  bootState = isFormItemChecked(F("sw_boot"));

  debounce_ms = getFormItemInt(F("sw_debounce"));

  doubleClickEvent       = getFormItemInt(F("sw_dc"));
  doubleClickMaxInterval = getFormItemInt(F("sw_dcmaxinterval"));

  longPressEvent          = getFormItemInt(F("sw_lp"));
  longPressMinInterval_ms = getFormItemInt(F("sw_lpmininterval"));

  useSafeButton = isFormItemChecked(F("sw_sb"));

  // TO-DO: add Extra-Long Press event
  // PCONFIG_LONG(1) = isFormItemChecked(F("sw_elp"));
  // PCONFIG_LONG(2) = getFormItemInt(F("sw_elpmininterval"));

  // check if a task has been edited and remove 'task' bit from the previous pin
  for (std::map<uint32_t, portStatusStruct>::iterator it = globalMapPortStatus.begin(); it != globalMapPortStatus.end(); ++it) {
    if ((it->second.previousTask == TaskIndex) && (getPluginFromKey(it->first) == pluginID)) {
      globalMapPortStatus[it->first].previousTask = -1;
      removeTaskFromPort(it->first);
      break;
    }
  }
}
