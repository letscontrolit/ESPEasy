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
  _debounceInterval_ms(P001_DEBOUNCE),
  _doubleClickMaxInterval_ms(P001_DC_MAX_INT),
  _longpressMinInterval_ms(P001_LP_MIN_INT),
  _doubleClickCounter(0),
  _safeButtonCounter(0),
  _gpioPin(CONFIG_PIN1),
  _dcMode(P001_DOUBLECLICK),
  _safeButton(P001_SAFE_BTN != 0),
  _longpressFired(false)
{
  _portStatus_key = createKey(PLUGIN_GPIO, _gpioPin);

  // Read current status or create empty if it does not exist
  portStatusStruct newStatus = globalMapPortStatus[_portStatus_key];

  // read and store current state to prevent switching at boot time
  newStatus.state  = GPIO_Read_Switch_State(event);
  newStatus.output = newStatus.state;

  // add this GPIO/port as a task
  if (newStatus.task < 3) {
    newStatus.task++;
  }

  // setPinState(PLUGIN_ID_001, _gpioPin, PIN_MODE_INPUT, switchstate[event->TaskIndex]);
  //  if it is in the device list we assume it's an input pin
  if (Settings.TaskDevicePin1PullUp[event->TaskIndex])
  {
    setInternalGPIOPullupMode(_gpioPin);
    newStatus.mode = PIN_MODE_INPUT_PULLUP;
  }
  else
  {
    pinMode(_gpioPin, INPUT);
    newStatus.mode = PIN_MODE_INPUT;
  }

  // if boot state must be send, inverse default state
  // this is done to force the trigger in PLUGIN_TEN_PER_SECOND
  if (P001_BOOTSTATE)
  {
    newStatus.state  = !newStatus.state;
    newStatus.output = !newStatus.output;
  }
  savePortStatus(_portStatus_key, newStatus);

  // set initial UserVar of the switch
  const float stateValue = Settings.TaskDevicePin1Inversed[event->TaskIndex] ? !newStatus.state : newStatus.state;

  UserVar.setFloat(event->TaskIndex, 0, stateValue);

  // store millis for debounce, doubleclick and long press
  const unsigned long cur_millis = millis();

  _debounceTimer    = cur_millis; // debounce timer
  _doubleClickTimer = cur_millis; // doubleclick timer
  _longpressTimer   = cur_millis; // longpress timer
}

P001_data_struct::~P001_data_struct()
{
  if (_portStatus_key != 0) {
    removeTaskFromPort(_portStatus_key);
  }
}

void P001_data_struct::tenPerSecond(struct EventStruct *event)
{
  if (_gpioPin == -1) { return; }

  /**************************************************************************\
     20181009 - @giig1967g: new doubleclick logic is:
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

  // long difftimer1 = 0;
  // long difftimer2 = 0;
  // long timerstats = millis();

  // Bug fixed: avoid 10xSEC in case of a non-fully configured device (no GPIO defined yet)
  const __FlashStringHelper *monitorEventString = F("GPIO");

  // WARNING operator [],creates an entry in map if _portStatus_key doesn't exist:
  portStatusStruct currentStatus = globalMapPortStatus[_portStatus_key];

  const int8_t state = GPIO_Read_Switch_State(_gpioPin, currentStatus.mode);

  //        if (currentStatus.mode != PIN_MODE_OUTPUT )
  //        {
  // CASE 1: using SafeButton, so wait 1 more 100ms cycle to acknowledge the status change
  // QUESTION: MAYBE IT'S BETTER TO WAIT 2 CYCLES??
  if (_safeButton && (state != currentStatus.state) && (_safeButtonCounter == 0))
  {
# ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_DEBUG, F("SW  : 1st click"));
# endif // ifndef BUILD_NO_DEBUG
    _safeButtonCounter = 1;

    return;
  }

  // CASE 2: not using SafeButton, or already waited 1 more 100ms cycle, so proceed.
  if ((state != currentStatus.state) || currentStatus.forceEvent)
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
        if ((_dcMode == SWITCH_DC_BOTH) ||                   // "Active on LOW & HIGH (EVENT=3)"
            ((_dcMode == SWITCH_DC_LOW) && (state == 0)) ||  // "Active only on LOW (EVENT=3)"
            ((_dcMode == SWITCH_DC_HIGH) && (state == 1))) { // "Active only on HIGH (EVENT=3)"
          _doubleClickCounter++;
        }
      } else {
        _doubleClickCounter++;
      }

      currentStatus.state = state;
      const bool currentOutputState = currentStatus.output;
      bool new_outputState          = currentOutputState;

      switch (P001_BUTTON_TYPE)
      {
        case PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH:
          new_outputState = state;
          break;
        case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW:

          if (!state)
          {
            new_outputState = !currentOutputState;
          }
          break;
        case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH:

          if (state)
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
          output_value = 3; // double click
        }
        else
        {
          output_value = sendState ? 1 : 0; // single click
        }
        event->sensorType = Sensor_VType::SENSOR_TYPE_SWITCH;

        if (P001_getSwitchType(event) == PLUGIN_001_TYPE_DIMMER)
        {
          if (sendState)
          {
            output_value = P001_DIMMER_VALUE;

            // Only set type to being dimmer when setting a value else it is "switched off".
            event->sensorType = Sensor_VType::SENSOR_TYPE_DIMMER;
          }
        }
        UserVar.setFloat(event->TaskIndex, 0, output_value);

# ifndef BUILD_NO_DEBUG


        if (loglevelActiveFor(LOG_LEVEL_INFO))
        {
          // Need to split this log creation or else uncrustify would fail
          const String trailingStr = concat(output_value == 3 ? F(" Doubleclick=") : F(" Output value="),
                                            static_cast<int>(output_value));
          String log = strformat(F("SW  : GPIO=%d State=%d%s"),
                                 _gpioPin,
                                 state ? 1 : 0,
                                 trailingStr.c_str());
          addLogMove(LOG_LEVEL_INFO, log);
        }

# endif // ifndef BUILD_NO_DEBUG

        // send task event
        sendData(event);

        // send monitor event
        if (currentStatus.monitor)
        {
          sendMonitorEvent(monitorEventString, _gpioPin, output_value);
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

  // just to simplify the reading of the code
  const int16_t LP = P001_LONGPRESS;

  // CASE 3: status unchanged. Checking longpress:
  // Check if LP is enabled and if LP has not fired yet
  if (!_longpressFired) {
    if ((LP == SWITCH_LONGPRESS_BOTH) ||                  // "Active on LOW & HIGH (EVENT= 10 or 11)"
        ((LP == SWITCH_LONGPRESS_LOW) && (state == 0)) || // "Active only on LOW (EVENT= 10 [NORMAL] or 11 [INVERSED])"
        ((LP == SWITCH_LONGPRESS_HIGH) && (state == 1)))  // "Active only on HIGH (EVENT= 11 [NORMAL] or 10 [INVERSED])"
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

        switch (P001_BUTTON_TYPE)
        {
          case PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH:
            needToSendEvent = true;
            break;
          case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW:

            if (!state)
            {
              needToSendEvent = true;
            }
            break;
          case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH:

            if (state)
            {
              needToSendEvent = true;
            }
            break;
        }

        if (needToSendEvent)
        {
          bool sendState = state;

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
            String log = strformat(F("SW  : LongPress: GPIO= %d State=%d Output value=%d"),
                                   _gpioPin,
                                   state ? 1 : 0,
                                   output_value);
            addLogMove(LOG_LEVEL_INFO, log);
          }
# endif // ifndef BUILD_NO_DEBUG

          // send task event
          sendData(event);

          // send monitor event
          if (currentStatus.monitor)
          {
            sendMonitorEvent(monitorEventString, _gpioPin, output_value);
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

# ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO))
    {
      addLogMove(LOG_LEVEL_INFO,
                 strformat(F("SW  : SafeButton: false positive detected. GPIO= %d State=%d"), _gpioPin, tempUserVar));
    }
# endif // ifndef BUILD_NO_DEBUG

    // send task event: DO NOT SEND TASK EVENT
    // sendData(event);
    // send monitor event
    if (currentStatus.monitor)
    {
      sendMonitorEvent(monitorEventString, _gpioPin, SAFE_BUTTON_EVENT);
    }

    // reset Userdata so it displays the correct state value in the web page
    UserVar.setFloat(event->TaskIndex, 0, tempUserVar);
  }
}

#endif // ifdef USES_P001
