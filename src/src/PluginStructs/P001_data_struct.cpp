#include "../PluginStructs/P001_data_struct.h"

#ifdef USES_P001


# include "../Helpers/_Plugin_Helper_webform.h"

# include "../ESPEasyCore/ESPEasyGPIO.h"

// TD-er: Needed to fix a mistake in earlier fixes.
uint8_t P001_data_struct::P001_getSwitchType(struct EventStruct *event)
{
  const uint8_t choice = P001_SWITCH_OR_DIMMER;

  if ((choice == 2) || // Old implementation for Dimmer
      (choice == PLUGIN_001_TYPE_DIMMER))
  {
    return PLUGIN_001_TYPE_DIMMER;
  }
  return PLUGIN_001_TYPE_SWITCH;
}

P001_data_struct::P001_data_struct(struct EventStruct *event) :
  _data(
    PLUGIN_GPIO,
    CONFIG_PIN1,
    P001_DEBOUNCE,
    P001_DC_MAX_INT,
    P001_LP_MIN_INT,
    P001_DOUBLECLICK,
    P001_LONGPRESS,
    P001_SAFE_BTN != 0,
    P001_BOOTSTATE,
    (P001_getSwitchType(event) == PLUGIN_001_TYPE_DIMMER) ? SWITCH_TYPE_DIMMER : P001_BUTTON_TYPE,
    P001_DIMMER_VALUE)
{
  uint8_t pinModeValue = PIN_MODE_INPUT;

  // setPinState(PLUGIN_ID_001, _data._pin, PIN_MODE_INPUT, switchstate[event->TaskIndex]);
  //  if it is in the device list we assume it's an input pin
  if (Settings.TaskDevicePin1PullUp[event->TaskIndex])
  {
    setInternalGPIOPullupMode(_data._pin);
    pinModeValue = PIN_MODE_INPUT_PULLUP;
  }
  else
  {
    pinMode(_data._pin, INPUT);
    pinModeValue = PIN_MODE_INPUT;
  }


  // read and store current state to prevent switching at boot time
  // "state" could be -1, 0 or 1
  const int8_t pinState = GPIO_Read_Switch_State(event);

  _data.init(
    event,
    pinState,
    pinModeValue);
}

void P001_data_struct::tenPerSecond(struct EventStruct *event)
{
  if (_data._pin == -1) { return; }

  /**************************************************************************\
     20181009 - @giig1967g: new doubleclick logic is:
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

  // long difftimer1 = 0;
  // long difftimer2 = 0;
  // long timerstats = millis();

  // Bug fixed: avoid 10xSEC in case of a non-fully configured device (no GPIO defined yet)
  const __FlashStringHelper *monitorEventString = F("GPIO");

  // WARNING operator [],creates an entry in map if _data._portStatus_key doesn't exist:
  portStatusStruct currentStatus = globalMapPortStatus[_data._portStatus_key];

  const int8_t pinState = GPIO_Read_Switch_State(_data._pin, currentStatus.mode);

  //        if (currentStatus.mode != PIN_MODE_OUTPUT )
  //        {
  // CASE 1: using SafeButton, so wait 1 more 100ms cycle to acknowledge the status change
  // QUESTION: MAYBE IT'S BETTER TO WAIT 2 CYCLES??
  if (_data._safeButton && (pinState != currentStatus.state) && (_data._safeButtonCounter == 0))
  {
# ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_DEBUG, F("SW  : 1st click"));
# endif // ifndef BUILD_NO_DEBUG
    _data._safeButtonCounter = 1;

    return;
  }

  // CASE 2: not using SafeButton, or already waited 1 more 100ms cycle, so proceed.
  if ((pinState != currentStatus.state) || currentStatus.forceEvent)
  {
    // Reset SafeButton counter
    _data._safeButtonCounter = 0;

    // reset timer for long press
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
            ((_data._dcMode == SWITCH_DC_LOW) && (pinState == 0)) ||  // "Active only on LOW (EVENT=3)"
            ((_data._dcMode == SWITCH_DC_HIGH) && (pinState == 1))) { // "Active only on HIGH (EVENT=3)"
          _data._doubleClickCounter++;
        }
      } else {
        _data._doubleClickCounter++;
      }

      currentStatus.state = pinState;
      const bool currentOutputState = currentStatus.output;
      bool new_outputState          = currentOutputState;

      switch (_data._switchType)
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

        if ((_data._doubleClickCounter == 3) && (_data._dcMode > 0))
        {
          output_value = 3; // double click
        }
        else
        {
          output_value = sendState ? 1 : 0; // single click
        }
        event->sensorType = Sensor_VType::SENSOR_TYPE_SWITCH;

        if (_data._switchType == SWITCH_TYPE_DIMMER)
        {
          if (sendState)
          {
            output_value = _data._dimmerValue;

            // Only set type to being dimmer when setting a value else it is "switched off".
            event->sensorType = Sensor_VType::SENSOR_TYPE_DIMMER;
          }
        }
        UserVar.setFloat(event->TaskIndex, 0, output_value);

# ifndef BUILD_NO_DEBUG


        if (loglevelActiveFor(LOG_LEVEL_INFO))
        {
          // Need to split this log creation or else uncrustify would fail
          String log = monitorEventString;
          log += strformat(F("  : Port=%d State=%d"), _data._pin, pinState);
          log += output_value == 3 ? F(" Doubleclick=") : F(" Output value=");
          log += output_value;
          addLogMove(LOG_LEVEL_INFO, log);
        }

# endif // ifndef BUILD_NO_DEBUG

        // send task event
        sendData(event);

        // send monitor event
        if (currentStatus.monitor)
        {
          sendMonitorEvent(monitorEventString, _data._pin, output_value);
        }

        // reset Userdata so it displays the correct state value in the web page
        UserVar.setFloat(event->TaskIndex, 0, sendState ? 1 : 0);
      }
      _data._debounceTimer = millis();
    }

    // Reset forceEvent
    currentStatus.forceEvent = 0;

    savePortStatus(_data._portStatus_key, currentStatus);
    return;
  }

  // CASE 3: status unchanged. Checking longpress:
  // Check if LP is enabled and if LP has not fired yet
  if (!_data._longpressFired) {
    if ((_data._longpressEvent == SWITCH_LONGPRESS_BOTH) ||                  // "Active on LOW & HIGH (EVENT= 10 or 11)"
        ((_data._longpressEvent == SWITCH_LONGPRESS_LOW) && (pinState == 0)) || // "Active only on LOW (EVENT= 10 [NORMAL] or 11 [INVERSED])"
        ((_data._longpressEvent == SWITCH_LONGPRESS_HIGH) && (pinState == 1)))  // "Active only on HIGH (EVENT= 11 [NORMAL] or 10 [INVERSED])"
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
      _data._safeButtonCounter = 0;

      const unsigned long deltaLP = timePassedSince(_data._longpressTimer);

      if (deltaLP >= _data._longpressMinInterval_ms)
      {
        uint8_t output_value;
        bool    needToSendEvent = false;

        _data._longpressFired = true;

        switch (_data._switchType)
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

# ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_INFO))
          {
            // Need to split this log creation or else uncrustify would fail
            String log = monitorEventString;
            log += strformat(
              F(" : LongPress: Port=%d State=%d Output value=%d"),
              _data._pin,
              pinState ? 1 : 0,
              output_value);
            addLogMove(LOG_LEVEL_INFO, log);
          }
# endif // ifndef BUILD_NO_DEBUG

          // send task event
          sendData(event);

          // send monitor event
          if (currentStatus.monitor)
          {
            sendMonitorEvent(monitorEventString, _data._pin, output_value);
          }

          // reset Userdata so it displays the correct state value in the web page
          UserVar.setFloat(event->TaskIndex, 0, sendState ? 1 : 0);
        }
        savePortStatus(_data._portStatus_key, currentStatus);
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

# ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO))
    {
      addLogMove(LOG_LEVEL_INFO,
                 concat(monitorEventString,
                        strformat(
                          F(" : SafeButton: false positive detected. Port=%d State=%d"),
                          _data._pin,
                          tempUserVar)));
    }
# endif // ifndef BUILD_NO_DEBUG

    // send task event: DO NOT SEND TASK EVENT
    // sendData(event);
    // send monitor event
    if (currentStatus.monitor)
    {
      sendMonitorEvent(monitorEventString, _data._pin, SAFE_BUTTON_EVENT);
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
# ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO,
             concat(monitorEventString,
                    strformat(
                        F(" : Port=%d is offline (EVENT= -1)"), 
                        _data._pin)));
    }
# endif // ifndef BUILD_NO_DEBUG

    // send task event
    sendData(event);

    // send monitor event
    if (currentStatus.monitor)
    {
      sendMonitorEvent(monitorEventString, _data._pin, -1);
    }

    savePortStatus(_data._portStatus_key, currentStatus);
  }
}

#endif // ifdef USES_P001
