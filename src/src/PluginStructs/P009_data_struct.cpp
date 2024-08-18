#include "../PluginStructs/P009_data_struct.h"

#ifdef USES_P009

# include "../Helpers/_Plugin_Helper_webform.h"

# include "../ESPEasyCore/ESPEasyGPIO.h"


P009_data_struct::P009_data_struct(struct EventStruct *event) :
  _data(
    PLUGIN_MCP,
    CONFIG_PORT,
    P009_DEBOUNCE,
    P009_DC_MAX_INT,
    P009_LP_MIN_INT,
    P009_DOUBLECLICK,
    P009_SAFE_BTN != 0,
    P009_BOOTSTATE)
# if FEATURE_I2C_DEVICE_CHECK
  , _address(0x20 + ((_data._pin - 1) / 16)) // unit = (_data._pin - 1) / 16
# endif // if FEATURE_I2C_DEVICE_CHECK
{
  // Turn on Pullup resistor
  setMCPInputAndPullupMode(_data._pin, true);

  // read and store current state to prevent switching at boot time
  // "state" could be -1, 0 or 1
  const int8_t state = GPIO_MCP_Read(_data._pin);

  _data.init(
    event,
    state,
    (state == -1) ?  PIN_MODE_OFFLINE : PIN_MODE_INPUT_PULLUP);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO,
           concat(F("MCP INIT="), state));
  }
}

void P009_data_struct::tenPerSecond(struct EventStruct *event)
{
# if FEATURE_I2C_DEVICE_CHECK

  if (!I2C_deviceCheck(_address, event->TaskIndex, 10, PLUGIN_I2C_GET_ADDRESS)) { // Generate stats
    return;                                                                       // Will return the default false for success
  }
# endif // if FEATURE_I2C_DEVICE_CHECK

  const int8_t state = GPIO_MCP_Read(_data._pin);

  // Avoid 10xSEC in case of a non-fully configured device (no port defined yet)
  if (state == -1) {
    return;
  }

  const __FlashStringHelper *monitorEventString = F("MCP");

  /**************************************************************************\
     20181022 - @giig1967g: new doubleclick logic is:
     if there is a 'state' change, check debounce period.
     Then if doubleclick interval exceeded, reset _data._doubleClickCounter to 0
     _data._doubleClickCounter contains the current status for doubleclick:
     0: start counting
     1: 1st click
     2: 2nd click
     3: 3rd click = doubleclick event if inside interval (calculated as: '3rd click time' minus '1st click time')
     Returned EVENT value is = 3 always for doubleclick
     In rules this can be checked:
     on Button#State=3 do //will fire if doubleclick
  \**************************************************************************/

  // WARNING operator [],creates an entry in map if key doesn't exist:
  portStatusStruct currentStatus = globalMapPortStatus[_data._portStatus_key];

  // CASE 1: using SafeButton, so wait 1 more 100ms cycle to acknowledge the status change
  if (_data._safeButton && (state != currentStatus.state) && (_data._safeButtonCounter == 0))
  {
# ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_DEBUG, F("MCP :SafeButton 1st click."));
# endif // ifndef BUILD_NO_DEBUG
    _data._safeButtonCounter = 1;

    return;
  }

  // CASE 2: not using SafeButton, or already waited 1 more 100ms cycle, so proceed.
  if ((state != currentStatus.state) || currentStatus.forceEvent)
  {
    // Reset SafeButton counter
    _data._safeButtonCounter = 0;

    // @giig1967g20181022: reset timer for long press
    _data._longpressTimer = millis();
    _data._longpressFired = false;

    const unsigned long debounceTime = timePassedSince(_data._debounceTimer);

    if (debounceTime >= _data._debounceInterval_ms) // de-bounce check
    {
      const unsigned long deltaDC = timePassedSince(_data._doubleClickTimer);

      if ((deltaDC >= _data._doubleClickMaxInterval_ms) ||
          (_data._doubleClickCounter == 3))
      {
        // reset timer for doubleclick
        _data._doubleClickCounter = 0;
        _data._doubleClickTimer   = millis();
      }

      // check settings for doubleclick according to the settings
      if (_data._doubleClickCounter == 0) {
        if ((_data._dcMode == SWITCH_DC_BOTH) ||                   // "Active on LOW & HIGH (EVENT=3)"
            ((_data._dcMode == SWITCH_DC_LOW) && (state == 0)) ||  // "Active only on LOW (EVENT=3)"
            ((_data._dcMode == SWITCH_DC_HIGH) && (state == 1))) { // "Active only on HIGH (EVENT=3)"
          _data._doubleClickCounter++;
        }
      } else {
        _data._doubleClickCounter++;
      }

      // switchstate[event->TaskIndex] = state;
      if ((currentStatus.mode == PIN_MODE_OFFLINE) ||
          (currentStatus.mode == PIN_MODE_UNDEFINED)) { currentStatus.mode = PIN_MODE_INPUT_PULLUP; // changed from offline to online
      }
      currentStatus.state = state;

      uint8_t output_value;

      // bool sendState = switchstate[event->TaskIndex];
      bool sendState = currentStatus.state;

      if (Settings.TaskDevicePin1Inversed[event->TaskIndex]) {
        sendState = !sendState;
      }

      if ((_data._doubleClickCounter == 3) && (_data._dcMode > 0))
      {
        output_value = 3;                 // double click
      } else {
        output_value = sendState ? 1 : 0; // single click
      }

      UserVar.setFloat(event->TaskIndex, 0, output_value);

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = strformat(F("MCP  : Port=%d State=%d"), _data._pin, state);
        log += output_value == 3 ? F(" Doubleclick=") : F(" Output value=");
        log += output_value;
        addLogMove(LOG_LEVEL_INFO, log);
      }

      // send task event
      sendData(event);

      // send monitor event
      if (currentStatus.monitor) { sendMonitorEvent(monitorEventString, _data._pin, output_value); }

      // Reset forceEvent
      currentStatus.forceEvent = 0;

      savePortStatus(_data._portStatus_key, currentStatus);
    }
    savePortStatus(_data._portStatus_key, currentStatus);
    return;
  }

  // just to simplify the reading of the code
  const int16_t LP = P009_LONGPRESS;

  // CASE 3: status unchanged. Checking longpress:
  // Check if LP is enabled and if LP has not fired yet
  if (!_data._longpressFired) {
    if ((LP == SWITCH_LONGPRESS_BOTH) ||                  // "Active on LOW & HIGH (EVENT= 10 or 11)"
        ((LP == SWITCH_LONGPRESS_LOW) && (state == 0)) || // "Active only on LOW (EVENT= 10 [NORMAL] or 11 [INVERSED])"
        ((LP == SWITCH_LONGPRESS_HIGH) && (state == 1)))  // "Active only on HIGH (EVENT= 11 [NORMAL] or 10 [INVERSED])"
    {
      /**************************************************************************\
         20181022 - @giig1967g: new longpress logic is:
         if there is no 'state' change, check if longpress interval reached
         When reached send longpress event.
         Returned Event value = state + 10
         So if state = 0 => EVENT longpress = 10
         if state = 1 => EVENT longpress = 11
         So we can trigger longpress for high or low contact
         In rules this can be checked:
         on Button#State=10 do //will fire if longpress when state = 0
         on Button#State=11 do //will fire if longpress when state = 1
      \**************************************************************************/

      // Reset SafeButton counter
      _data._safeButtonCounter = 0;

      const unsigned long deltaLP = timePassedSince(_data._longpressTimer);

      if (deltaLP >= _data._longpressMinInterval_ms)
      {
        uint8_t output_value;
        _data._longpressFired = true; // fired = true

        bool sendState = state;

        if (Settings.TaskDevicePin1Inversed[event->TaskIndex]) {
          sendState = !sendState;
        }

        output_value = (sendState ? 1 : 0) + 10;

        UserVar.setFloat(event->TaskIndex, 0, output_value);

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO,
                 strformat(F("MCP  : LongPress: Port=%d State=%d Output value=%d"), _data._pin, state ? 1 : 0, output_value));
        }

        // send task event
        sendData(event);

        // send monitor event
        if (currentStatus.monitor) { sendMonitorEvent(monitorEventString, _data._pin, output_value); }

        // reset Userdata so it displays the correct state value in the web page
        UserVar.setFloat(event->TaskIndex, 0, sendState ? 1 : 0);
      }
      return;
    }
  }

  if (_data._safeButtonCounter == 1)
  {
    // Safe Button detected. Send EVENT value = 4
    constexpr uint8_t SAFE_BUTTON_EVENT = 4;

    // Reset SafeButton counter
    _data._safeButtonCounter = 0;

    // Create EVENT with value = 4 for SafeButton false positive detection
    const int tempUserVar = lround(UserVar[event->BaseVarIndex]);
    UserVar.setFloat(event->TaskIndex, 0, SAFE_BUTTON_EVENT);

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO,
             strformat(F("MCP : SafeButton: false positive detected. GPIO= %d State=%d"), _data._pin, tempUserVar));
    }

    // send task event: DO NOT SEND TASK EVENT
    // sendData(event);
    // send monitor event
    if (currentStatus.monitor) { sendMonitorEvent(monitorEventString, _data._pin, 4); }

    // reset Userdata so it displays the correct state value in the web page
    UserVar.setFloat(event->TaskIndex, 0, tempUserVar);
    return;
  }

  if ((state != currentStatus.state) && (state == -1)) {
    // set UserVar and switchState = -1 and send EVENT to notify user
    UserVar.setFloat(event->TaskIndex, 0, state);
    currentStatus.mode = PIN_MODE_OFFLINE;

    // switchstate[event->TaskIndex] = state;
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO,
             strformat(F("MCP  : Port=%d is offline (EVENT= -1)"), _data._pin));
    }

    // send task event
    sendData(event);

    // send monitor event
    if (currentStatus.monitor) { sendMonitorEvent(monitorEventString, _data._pin, -1); }

    savePortStatus(_data._portStatus_key, currentStatus);
  }
}

#endif // ifdef USES_P009
