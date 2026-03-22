#include "../Helpers/_Plugin_Helper_GPIO.h"

#include "../ESPEasyCore/ESPEasyGPIO.h"
#include "../Globals/GlobalMapPortStatus.h"
#include "../Globals/RuntimeData.h"
#include "../Globals/Settings.h"
#include "../Helpers/PortStatus.h"


GPIO_plugin_helper_data_t::GPIO_plugin_helper_data_t(
  pluginID_t pluginNumber,
  uint16_t   pin,
  uint32_t   debounceInterval_ms,
  uint32_t   doubleClickMaxInterval_ms,
  uint32_t   longpressMinInterval_ms,
  uint8_t    dcMode,
  uint8_t    longpressEvent,
  bool       safeButton,
  bool       sendBootState,
  uint8_t    switchType,
  uint8_t    dimmerValue) :
  _portStatus_key(createKey(pluginNumber, pin)),
  _debounceInterval_ms(debounceInterval_ms),
  _doubleClickMaxInterval_ms(doubleClickMaxInterval_ms),
  _longpressMinInterval_ms(longpressMinInterval_ms),
  _doubleClickCounter(0),
  _safeButtonCounter(0),
  _pin(pin),
  _pluginNumber(pluginNumber),
  _dcMode(dcMode),
  _longpressEvent(longpressEvent),
  _switchType(switchType),
  _dimmerValue(dimmerValue),
  _safeButton(safeButton),
  _sendBootState(sendBootState),
  _longpressFired(false)
{
  // store millis for debounce, doubleclick and long press
  const unsigned long cur_millis = millis();

  _debounceTimer    = cur_millis; // debounce timer
  _doubleClickTimer = cur_millis; // doubleclick timer
  _longpressTimer   = cur_millis; // longpress timer
}

GPIO_plugin_helper_data_t::~GPIO_plugin_helper_data_t()
{
  if (_portStatus_key != 0) {
    removeTaskFromPort(_portStatus_key);
  }
}

bool GPIO_plugin_helper_data_t::init(
  struct EventStruct *event,
  int8_t              pinState,
  uint8_t             pinModeValue)
{
  // Read current status or create empty if it does not exist
  portStatusStruct newStatus = globalMapPortStatus[_portStatus_key];

  newStatus.state  = pinState;
  newStatus.output = newStatus.state; // FIXME TD-er: Is this correct to set output value based on pinState?
  newStatus.mode   = pinModeValue;

  // @giig1967g: if it is in the device list we assume it's an input pin
  // add this GPIO/port as a task
  if (newStatus.task < 3) {
    newStatus.task++;
  }

  // if boot pinState must be send, inverse default pinState
  // this is done to force the trigger in PLUGIN_TEN_PER_SECOND
  if (_sendBootState) {
    newStatus.state = !newStatus.state;

    if (_pluginNumber == PLUGIN_GPIO) {
      // FIXME TD-er: This was only done in P001, should this be done for PCF/MCP too?
      // Or maybe this is wrong to do for internal GPIO?
      newStatus.output = !newStatus.output;
    }
  }

  // setPinState(PLUGIN_ID_009, _mcpPin, PIN_MODE_INPUT, switchstate[event->TaskIndex]);
  savePortStatus(_portStatus_key, newStatus);

  // @giig1967g-20181022: set initial UserVar of the switch
  if ((newStatus.state != -1) && Settings.TaskDevicePin1Inversed[event->TaskIndex]) {
    UserVar.setFloat(event->TaskIndex, 0, !newStatus.state);
  } else {
    UserVar.setFloat(event->TaskIndex, 0, newStatus.state);
  }

  return true;
}

void GPIO_plugin_helper_data_t::tenPerSecond(
  struct EventStruct        *event,
  const __FlashStringHelper *monitorEventString,
  int8_t                     pinState)
{
  // Avoid 10xSEC in case of a non-fully configured device (no port defined yet)
  if (pinState == -1) {
    // FIXME TD-er: This will cause the last check at the end of this function not to be executed.
    // Is this correct?
    return;
  }

  /**************************************************************************\
     20181022 - @giig1967g: new doubleclick logic is:
     if there is a 'pinState' change, check debounce period.
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
  // QUESTION: MAYBE IT'S BETTER TO WAIT 2 CYCLES??
  if (_safeButton && (pinState != currentStatus.state) && (_safeButtonCounter == 0))
  {
#ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_DEBUG, concat(monitorEventString, F(" : SafeButton 1st click")));
#endif // ifndef BUILD_NO_DEBUG
    _safeButtonCounter = 1;

    return;
  }

  // CASE 2: not using SafeButton, or already waited 1 more 100ms cycle, so proceed.
  if ((pinState != currentStatus.state) || currentStatus.forceEvent)
  {
    // Reset SafeButton counter
    _safeButtonCounter = 0;

    // reset timer for long press
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
        if ((_dcMode == SWITCH_DC_BOTH) ||                      // "Active on LOW & HIGH (EVENT=3)"
            ((_dcMode == SWITCH_DC_LOW) && (pinState == 0)) ||  // "Active only on LOW (EVENT=3)"
            ((_dcMode == SWITCH_DC_HIGH) && (pinState == 1))) { // "Active only on HIGH (EVENT=3)"
          _doubleClickCounter++;
        }
      } else {
        _doubleClickCounter++;
      }

      if ((currentStatus.mode == PIN_MODE_OFFLINE) ||
          (currentStatus.mode == PIN_MODE_UNDEFINED))
      {
        currentStatus.mode = PIN_MODE_INPUT_PULLUP; // changed from offline to online
      }

      currentStatus.state = pinState;
      const bool currentOutputState = currentStatus.output;
      bool new_outputState          = currentOutputState;

      switch (_switchType)
      {
        case SWITCH_TYPE_NORMAL_SWITCH:
          new_outputState = pinState;
          break;
        case SWITCH_TYPE_PUSH_ACTIVE_LOW:

          if (!pinState)
          {
            new_outputState = !currentOutputState;
          }
          break;
        case SWITCH_TYPE_PUSH_ACTIVE_HIGH:

          if (pinState)
          {
            new_outputState = !currentOutputState;
          }
          break;
      }

      // send if output needs to be changed
      if ((currentOutputState != new_outputState) || currentStatus.forceEvent)
      {
        uint8_t output_value;
        currentStatus.output = new_outputState;
        bool sendState = new_outputState;

        if (Settings.TaskDevicePin1Inversed[event->TaskIndex])
        {
          sendState = !sendState;
        }

        if ((_doubleClickCounter == 3) && (_dcMode > 0))
        {
          output_value = 3;                 // double click
        } else {
          output_value = sendState ? 1 : 0; // single click
        }
        event->sensorType = Sensor_VType::SENSOR_TYPE_SWITCH;

        if (_switchType == SWITCH_TYPE_DIMMER)
        {
          if (sendState)
          {
            output_value = _dimmerValue;

            // Only set type to being dimmer when setting a value else it is "switched off".
            event->sensorType = Sensor_VType::SENSOR_TYPE_DIMMER;
          }
        }
        UserVar.setFloat(event->TaskIndex, 0, output_value);

#ifndef BUILD_NO_DEBUG


        if (loglevelActiveFor(LOG_LEVEL_INFO))
        {
          // Need to split this log creation or else uncrustify would fail
          String log = monitorEventString;
          log += strformat(F("  : Port=%d State=%d"), _pin, pinState);
          log += output_value == 3 ? F(" Doubleclick=") : F(" Output value=");
          log += output_value;
          addLogMove(LOG_LEVEL_INFO, log);
        }

#endif // ifndef BUILD_NO_DEBUG

        // send task event
        sendData(event);

        // send monitor event
        if (currentStatus.monitor)
        {
          sendMonitorEvent(monitorEventString, _pin, output_value);
        }

        // reset Userdata so it displays the correct state value in the web page
        UserVar.setFloat(event->TaskIndex, 0, sendState ? 1 : 0);
      }
      _debounceTimer = millis();
    }

    // Reset forceEvent
    currentStatus.forceEvent = 0;

    savePortStatus(_portStatus_key, currentStatus);
    return;
  }

  // CASE 3: status unchanged. Checking longpress:
  // Check if LP is enabled and if LP has not fired yet
  if (!_longpressFired) {
    if ((_longpressEvent == SWITCH_LONGPRESS_BOTH) ||                     // "Active on LOW & HIGH (EVENT= 10 or 11)"
        ((_longpressEvent == SWITCH_LONGPRESS_LOW) && (pinState == 0)) || // "Active only on LOW (EVENT= 10 [NORMAL] or 11 [INVERSED])"
        ((_longpressEvent == SWITCH_LONGPRESS_HIGH) && (pinState == 1)))  // "Active only on HIGH (EVENT= 11 [NORMAL] or 10 [INVERSED])"
    {
      /**************************************************************************\
         20181009 - @giig1967g: new longpress logic is:
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
        bool    needToSendEvent = false;

        _longpressFired = true;

        switch (_switchType)
        {
          case SWITCH_TYPE_NORMAL_SWITCH:
            needToSendEvent = true;
            break;
          case SWITCH_TYPE_PUSH_ACTIVE_LOW:

            if (!pinState)
            {
              needToSendEvent = true;
            }
            break;
          case SWITCH_TYPE_PUSH_ACTIVE_HIGH:

            if (pinState)
            {
              needToSendEvent = true;
            }
            break;
        }

        if (needToSendEvent)
        {
          bool sendState = pinState;

          if (Settings.TaskDevicePin1Inversed[event->TaskIndex])
          {
            sendState = !sendState;
          }
          output_value = sendState ? 11 : 10;

          // output_value = output_value + 10;
          UserVar.setFloat(event->TaskIndex, 0, output_value);

#ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_INFO))
          {
            // Need to split this log creation or else uncrustify would fail
            String log = monitorEventString;
            log += strformat(
              F(" : LongPress: Port=%d State=%d Output value=%d"),
              _pin,
              pinState ? 1 : 0,
              output_value);
            addLogMove(LOG_LEVEL_INFO, log);
          }
#endif // ifndef BUILD_NO_DEBUG

          // send task event
          sendData(event);

          // send monitor event
          if (currentStatus.monitor)
          {
            sendMonitorEvent(monitorEventString, _pin, output_value);
          }

          // reset Userdata so it displays the correct state value in the web page
          UserVar.setFloat(event->TaskIndex, 0, sendState ? 1 : 0);
        }
        savePortStatus(_portStatus_key, currentStatus);
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

#ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO))
    {
      addLogMove(LOG_LEVEL_INFO,
                 concat(monitorEventString,
                        strformat(
                          F(" : SafeButton: false positive detected. Port=%d State=%d"),
                          _pin,
                          tempUserVar)));
    }
#endif // ifndef BUILD_NO_DEBUG

    // send task event: DO NOT SEND TASK EVENT
    // sendData(event);
    // send monitor event
    if (currentStatus.monitor)
    {
      sendMonitorEvent(monitorEventString, _pin, SAFE_BUTTON_EVENT);
    }

    // reset Userdata so it displays the correct state value in the web page
    UserVar.setFloat(event->TaskIndex, 0, tempUserVar);
    return;
  }

  if ((pinState != currentStatus.state) && (pinState == -1)) {
    // set UserVar and switchState = -1 and send EVENT to notify user
    UserVar.setFloat(event->TaskIndex, 0, pinState);
    currentStatus.mode = PIN_MODE_OFFLINE;

    // switchstate[event->TaskIndex] = pinState;
#ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO,
             concat(monitorEventString,
                    strformat(
                      F(" : Port=%d is offline (EVENT= -1)"),
                      _pin)));
    }
#endif // ifndef BUILD_NO_DEBUG

    // send task event
    sendData(event);

    // send monitor event
    if (currentStatus.monitor)
    {
      sendMonitorEvent(monitorEventString, _pin, -1);
    }

    savePortStatus(_portStatus_key, currentStatus);
  }
}
