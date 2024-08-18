#include "../PluginStructs/P009_data_struct.h"

#ifdef USES_P009

# include "../Helpers/_Plugin_Helper_webform.h"

# include "../ESPEasyCore/ESPEasyGPIO.h"


P009_data_struct::P009_data_struct(struct EventStruct *event) :
  _debounceInterval_ms(P009_DEBOUNCE),
  _doubleClickMaxInterval_ms(P009_DC_MAX_INT),
  _longpressMinInterval_ms(P009_LP_MIN_INT),
  _doubleClickCounter(0),
  _safeButtonCounter(0),
  _mcpPin(CONFIG_PORT),
  _dcMode(P009_DOUBLECLICK),
# if FEATURE_I2C_DEVICE_CHECK
  _address(0x20 + ((_mcpPin - 1) / 16)), // unit = (_mcpPin - 1) / 16
# endif // if FEATURE_I2C_DEVICE_CHECK
  _safeButton(P009_SAFE_BTN != 0),
  _longpressFired(false)
{
  // Turn on Pullup resistor
  setMCPInputAndPullupMode(_mcpPin, true);

  _portStatus_key = createKey(PLUGIN_MCP, _mcpPin);

  // Read current status or create empty if it does not exist
  portStatusStruct newStatus = globalMapPortStatus[_portStatus_key];

  // read and store current state to prevent switching at boot time
  // "state" could be -1, 0 or 1
  newStatus.state = GPIO_MCP_Read(_mcpPin);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO,
           concat(F("MCP INIT="), newStatus.state));
  }
  newStatus.output = newStatus.state;
  newStatus.mode   = (newStatus.state == -1) ?  PIN_MODE_OFFLINE : PIN_MODE_INPUT_PULLUP;

  // @giig1967g: if it is in the device list we assume it's an input pin
  // add this GPIO/port as a task
  if (newStatus.task < 3) {
    newStatus.task++;
  }


  // @giig1967g-20181022: set initial UserVar of the switch
  if ((newStatus.state != -1) && Settings.TaskDevicePin1Inversed[event->TaskIndex]) {
    UserVar.setFloat(event->TaskIndex, 0, !newStatus.state);
  } else {
    UserVar.setFloat(event->TaskIndex, 0, newStatus.state);
  }

  // if boot state must be send, inverse default state
  // this is done to force the trigger in PLUGIN_TEN_PER_SECOND
  if (P009_BOOTSTATE) {
    newStatus.state = !newStatus.state;
  }

  // setPinState(PLUGIN_ID_009, _mcpPin, PIN_MODE_INPUT, switchstate[event->TaskIndex]);
  savePortStatus(_portStatus_key, newStatus);

  // store millis for debounce, doubleclick and long press
  const unsigned long cur_millis = millis();

  _debounceTimer    = cur_millis; // debounce timer
  _doubleClickTimer = cur_millis; // doubleclick timer
  _longpressTimer   = cur_millis; // longpress timer
}

P009_data_struct::~P009_data_struct()
{
  if (_portStatus_key != 0) {
    removeTaskFromPort(_portStatus_key);
  }
}

void P009_data_struct::tenPerSecond(struct EventStruct *event)
{
# if FEATURE_I2C_DEVICE_CHECK

  if (!I2C_deviceCheck(_address, event->TaskIndex, 10, PLUGIN_I2C_GET_ADDRESS)) { // Generate stats
    return;                                                                       // Will return the default false for success
  }
# endif // if FEATURE_I2C_DEVICE_CHECK

  const int8_t state = GPIO_MCP_Read(_mcpPin);

  // Avoid 10xSEC in case of a non-fully configured device (no port defined yet)
  if (state == -1) {
    return;
  }

  const __FlashStringHelper *monitorEventString = F("MCP");

  /**************************************************************************\
     20181022 - @giig1967g: new doubleclick logic is:
     if there is a 'state' change, check debounce period.
     Then if doubleclick interval exceeded, reset _doubleClickCounter to 0
     _doubleClickCounter contains the current status for doubleclick:
     0: start counting
     1: 1st click
     2: 2nd click
     3: 3rd click = doubleclick event if inside interval (calculated as: '3rd click time' minus '1st click time')
     Returned EVENT value is = 3 always for doubleclick
     In rules this can be checked:
     on Button#State=3 do //will fire if doubleclick
  \**************************************************************************/

  // WARNING operator [],creates an entry in map if key doesn't exist:
  portStatusStruct currentStatus = globalMapPortStatus[_portStatus_key];

  // CASE 1: using SafeButton, so wait 1 more 100ms cycle to acknowledge the status change
  if (_safeButton && (state != currentStatus.state) && (_safeButtonCounter == 0))
  {
# ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_DEBUG, F("MCP :SafeButton 1st click."));
# endif // ifndef BUILD_NO_DEBUG
    _safeButtonCounter = 1;

    return;
  }

  // CASE 2: not using SafeButton, or already waited 1 more 100ms cycle, so proceed.
  if ((state != currentStatus.state) || currentStatus.forceEvent)
  {
    // Reset SafeButton counter
    _safeButtonCounter = 0;

    // @giig1967g20181022: reset timer for long press
    _longpressTimer = millis();
    _longpressFired = false;

    const unsigned long debounceTime = timePassedSince(_debounceTimer);

    if (debounceTime >= _debounceInterval_ms) // de-bounce check
    {
      const unsigned long deltaDC = timePassedSince(_doubleClickTimer);

      if ((deltaDC >= _doubleClickMaxInterval_ms) ||
          (_doubleClickCounter == 3))
      {
        // reset timer for doubleclick
        _doubleClickCounter = 0;
        _doubleClickTimer   = millis();
      }

      // check settings for doubleclick according to the settings
      if (_doubleClickCounter == 0) {
        if ((_dcMode == SWITCH_DC_BOTH) ||                   // "Active on LOW & HIGH (EVENT=3)"
            ((_dcMode == SWITCH_DC_LOW) && (state == 0)) ||  // "Active only on LOW (EVENT=3)"
            ((_dcMode == SWITCH_DC_HIGH) && (state == 1))) { // "Active only on HIGH (EVENT=3)"
          _doubleClickCounter++;
        }
      } else {
        _doubleClickCounter++;
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

      if ((_doubleClickCounter == 3) && (_dcMode > 0))
      {
        output_value = 3;                 // double click
      } else {
        output_value = sendState ? 1 : 0; // single click
      }

      UserVar.setFloat(event->TaskIndex, 0, output_value);

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = strformat(F("MCP  : Port=%d State=%d"), _mcpPin, state);
        log += output_value == 3 ? F(" Doubleclick=") : F(" Output value=");
        log += output_value;
        addLogMove(LOG_LEVEL_INFO, log);
      }

      // send task event
      sendData(event);

      // send monitor event
      if (currentStatus.monitor) { sendMonitorEvent(monitorEventString, _mcpPin, output_value); }

      // Reset forceEvent
      currentStatus.forceEvent = 0;

      savePortStatus(_portStatus_key, currentStatus);
    }
    savePortStatus(_portStatus_key, currentStatus);
    return;
  }

  // just to simplify the reading of the code
  const int16_t LP = P009_LONGPRESS;

  // CASE 3: status unchanged. Checking longpress:
  // Check if LP is enabled and if LP has not fired yet
  if (!_longpressFired) {
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
      _safeButtonCounter = 0;

      const unsigned long deltaLP = timePassedSince(_longpressTimer);

      if (deltaLP >= _longpressMinInterval_ms)
      {
        uint8_t output_value;
        _longpressFired = true; // fired = true

        bool sendState = state;

        if (Settings.TaskDevicePin1Inversed[event->TaskIndex]) {
          sendState = !sendState;
        }

        output_value = (sendState ? 1 : 0) + 10;

        UserVar.setFloat(event->TaskIndex, 0, output_value);

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO,
                 strformat(F("MCP  : LongPress: Port=%d State=%d Output value=%d"), _mcpPin, state ? 1 : 0, output_value));
        }

        // send task event
        sendData(event);

        // send monitor event
        if (currentStatus.monitor) { sendMonitorEvent(monitorEventString, _mcpPin, output_value); }

        // reset Userdata so it displays the correct state value in the web page
        UserVar.setFloat(event->TaskIndex, 0, sendState ? 1 : 0);
      }
      return;
    }
  }

  if (_safeButtonCounter == 1)
  {
    // Safe Button detected. Send EVENT value = 4
    constexpr uint8_t SAFE_BUTTON_EVENT = 4;

    // Reset SafeButton counter
    _safeButtonCounter = 0;

    // Create EVENT with value = 4 for SafeButton false positive detection
    const int tempUserVar = lround(UserVar[event->BaseVarIndex]);
    UserVar.setFloat(event->TaskIndex, 0, SAFE_BUTTON_EVENT);

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO,
             strformat(F("MCP : SafeButton: false positive detected. GPIO= %d State=%d"), _mcpPin, tempUserVar));
    }

    // send task event: DO NOT SEND TASK EVENT
    // sendData(event);
    // send monitor event
    if (currentStatus.monitor) { sendMonitorEvent(monitorEventString, _mcpPin, 4); }

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
             strformat(F("MCP  : Port=%d is offline (EVENT= -1)"), _mcpPin));
    }

    // send task event
    sendData(event);

    // send monitor event
    if (currentStatus.monitor) { sendMonitorEvent(monitorEventString, _mcpPin, -1); }

    savePortStatus(_portStatus_key, currentStatus);
  }
}

#endif // ifdef USES_P009
