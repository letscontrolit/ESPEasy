#include "../PluginStructs/P001_data_struct.h"

#ifdef USES_P001

# include "../Helpers/_Plugin_Helper_webform.h"

# include "../ESPEasyCore/ESPEasyGPIO.h"

// TD-er: Needed to fix a mistake in earlier fixes.
uint8_t P001_data_struct::P001_getSwitchType(struct EventStruct *event)
{
  const uint8_t choice = PCONFIG(0);

  if ((choice == 2) || // Old implementation for Dimmer
      (choice == PLUGIN_001_TYPE_DIMMER))
  {
    return PLUGIN_001_TYPE_DIMMER;
  }
  return PLUGIN_001_TYPE_SWITCH;
}

void P001_data_struct::init(struct EventStruct *event)
{
  portStatusStruct newStatus;
  const uint32_t   key = createKey(PLUGIN_GPIO, CONFIG_PIN1);

  // Read current status or create empty if it does not exist
  newStatus = globalMapPortStatus[key];

  // read and store current state to prevent switching at boot time
  newStatus.state                                          = GPIO_Read_Switch_State(event);
  newStatus.output                                         = newStatus.state;
  (newStatus.task < 3) ? newStatus.task++ : newStatus.task = 3; // add this GPIO/port as a task

  // setPinState(PLUGIN_ID_001, CONFIG_PIN1, PIN_MODE_INPUT, switchstate[event->TaskIndex]);
  //  if it is in the device list we assume it's an input pin
  if (Settings.TaskDevicePin1PullUp[event->TaskIndex])
  {
    setInternalGPIOPullupMode(CONFIG_PIN1);
    newStatus.mode = PIN_MODE_INPUT_PULLUP;
  }
  else
  {
    pinMode(CONFIG_PIN1, INPUT);
    newStatus.mode = PIN_MODE_INPUT;
  }

  // if boot state must be send, inverse default state
  // this is done to force the trigger in PLUGIN_TEN_PER_SECOND
  if (P001_BOOTSTATE)
  {
    newStatus.state  = !newStatus.state;
    newStatus.output = !newStatus.output;
  }
  savePortStatus(key, newStatus);

  // set initial UserVar of the switch
  if (Settings.TaskDevicePin1Inversed[event->TaskIndex])
  {
    UserVar.setFloat(event->TaskIndex, 0, !newStatus.state);
  }
  else
  {
    UserVar.setFloat(event->TaskIndex, 0, newStatus.state);
  }

  // counters = 0
  _doubleClickCounter      = 0; // doubleclick counter
  _safeButtonCounter = 0; // safebutton counter

  // used to track if LP has fired
  _longpressFired = 0;

  {
    // store millis for debounce, doubleclick and long press
    const unsigned long cur_millis = millis();
    _debounceTimer = cur_millis; // debounce timer
    _doubleClickTimer = cur_millis; // doubleclick timer
    _longpressTimer = cur_millis; // longpress timer
  }

  // set minimum value for doubleclick MIN interval speed
  if (P001_DC_MAX_INT < SWITCH_DOUBLECLICK_MIN_INTERVAL)
  {
    P001_DC_MAX_INT = SWITCH_DOUBLECLICK_MIN_INTERVAL;
  }

  // set minimum value for longpress MIN interval speed
  if (P001_LP_MIN_INT < SWITCH_LONGPRESS_MIN_INTERVAL)
  {
    P001_LP_MIN_INT = SWITCH_LONGPRESS_MIN_INTERVAL;
  }
}

void P001_data_struct::tenPerSecond(struct EventStruct *event)
{
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

  if (validGpio(CONFIG_PIN1))
  {
    const uint32_t key = createKey(PLUGIN_GPIO, CONFIG_PIN1);

    // WARNING operator [],creates an entry in map if key doesn't exist:
    portStatusStruct currentStatus = globalMapPortStatus[key];

    const int8_t state = GPIO_Read_Switch_State(CONFIG_PIN1, currentStatus.mode);

    //        if (currentStatus.mode != PIN_MODE_OUTPUT )
    //        {
    // CASE 1: using SafeButton, so wait 1 more 100ms cycle to acknowledge the status change
    // QUESTION: MAYBE IT'S BETTER TO WAIT 2 CYCLES??
    if (lround(P001_SAFE_BTN) && (state != currentStatus.state) && (_safeButtonCounter == 0))
    {
# ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, F("SW  : 1st click"));
# endif // ifndef BUILD_NO_DEBUG
      _safeButtonCounter = 1;
    }

    // CASE 2: not using SafeButton, or already waited 1 more 100ms cycle, so proceed.
    else if ((state != currentStatus.state) || currentStatus.forceEvent)
    {
      // Reset SafeButton counter
      _safeButtonCounter = 0;

      // reset timer for long press
      _longpressTimer = millis();
      _longpressFired = 0;

      const unsigned long debounceTime = timePassedSince(_debounceTimer);

      if (debounceTime >= (unsigned long)lround(P001_DEBOUNCE)) // de-bounce check
      {
        const unsigned long deltaDC = timePassedSince(_doubleClickTimer);

        if ((deltaDC >= (unsigned long)lround(P001_DC_MAX_INT)) ||
            (_doubleClickCounter == 3))
        {
          // reset timer for doubleclick
          _doubleClickCounter = 0;
          _doubleClickTimer = millis();
        }

        // just to simplify the reading of the code
        const int16_t DC      = P001_DOUBLECLICK;
        const int16_t COUNTER = _doubleClickCounter;

        // check settings for doubleclick according to the settings
        if (COUNTER == 0) {
          if ((DC == 3) || ((DC == 1) && (state == 0)) || ((DC == 2) && (state == 1))) {
            _doubleClickCounter++;
          }
        } else {
          _doubleClickCounter++;
        }

        currentStatus.state = state;
        const boolean currentOutputState = currentStatus.output;
        boolean new_outputState          = currentOutputState;

        switch (PCONFIG(2))
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
          boolean sendState = new_outputState;

          if (Settings.TaskDevicePin1Inversed[event->TaskIndex])
          {
            sendState = !sendState;
          }

          if ((_doubleClickCounter == 3) && (P001_DOUBLECLICK > 0))
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
              output_value = PCONFIG(1);

              // Only set type to being dimmer when setting a value else it is "switched off".
              event->sensorType = Sensor_VType::SENSOR_TYPE_DIMMER;
            }
          }
          UserVar.setFloat(event->TaskIndex, 0, output_value);

# ifndef BUILD_NO_DEBUG


        if (loglevelActiveFor(LOG_LEVEL_INFO))
        {
            addLogMove(LOG_LEVEL_INFO,
                        strformat(F("SW  : GPIO=%d State=%d%s"),
                                CONFIG_PIN1,
                                state ? 1 : 0,
                                concat(output_value == 3 ? F(" Doubleclick=") : F(" Output value="),
                                        static_cast<int>(output_value))
                                    .c_str()));
        }
           
# endif // ifndef BUILD_NO_DEBUG

          // send task event
          sendData(event);

          // send monitor event
          if (currentStatus.monitor)
          {
            sendMonitorEvent(monitorEventString, CONFIG_PIN1, output_value);
          }

          // reset Userdata so it displays the correct state value in the web page
          UserVar.setFloat(event->TaskIndex, 0, sendState ? 1 : 0);
        }
        _debounceTimer = millis();
      }

      // Reset forceEvent
      currentStatus.forceEvent = 0;

      savePortStatus(key, currentStatus);
    }

    // just to simplify the reading of the code
# define LP P001_LONGPRESS
# define FIRED _longpressFired

    // CASE 3: status unchanged. Checking longpress:
    // Check if LP is enabled and if LP has not fired yet
    else if (!FIRED && ((LP == 3) || ((LP == 1) && (state == 0)) || ((LP == 2) && (state == 1))))
    {
# undef LP
# undef FIRED

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

      if (deltaLP >= (unsigned long)lround(P001_LP_MIN_INT))
      {
        uint8_t output_value;
        bool    needToSendEvent = false;

        _longpressFired = 1;

        switch (PCONFIG(2))
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
          boolean sendState = state;

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
                addLogMove(LOG_LEVEL_INFO,
                            strformat(F("SW  : LongPress: GPIO= %d State=%d Output value=%d"),
                                    CONFIG_PIN1,
                                    state ? 1 : 0,
                                    output_value));
            }
# endif // ifndef BUILD_NO_DEBUG

          // send task event
          sendData(event);

          // send monitor event
          if (currentStatus.monitor)
          {
            sendMonitorEvent(monitorEventString, CONFIG_PIN1, output_value);
          }

          // reset Userdata so it displays the correct state value in the web page
          UserVar.setFloat(event->TaskIndex, 0, sendState ? 1 : 0);
        }
        savePortStatus(key, currentStatus);
      }
    }
    else
    {
      if (_safeButtonCounter == 1)
      { // Safe Button detected. Send EVENT value = 4
        const uint8_t SAFE_BUTTON_EVENT = 4;

        // Reset SafeButton counter
        _safeButtonCounter = 0;

        // Create EVENT with value = 4 for SafeButton false positive detection
        const int tempUserVar = lround(UserVar[event->BaseVarIndex]);
        UserVar.setFloat(event->TaskIndex, 0, SAFE_BUTTON_EVENT);

# ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO))
        {
          addLogMove(LOG_LEVEL_INFO,
                     strformat(F("SW  : SafeButton: false positive detected. GPIO= %d State=%d"), CONFIG_PIN1, tempUserVar));
        }
# endif // ifndef BUILD_NO_DEBUG

        // send task event: DO NOT SEND TASK EVENT
        // sendData(event);
        // send monitor event
        if (currentStatus.monitor)
        {
          sendMonitorEvent(monitorEventString, CONFIG_PIN1, SAFE_BUTTON_EVENT);
        }

        // reset Userdata so it displays the correct state value in the web page
        UserVar.setFloat(event->TaskIndex, 0, tempUserVar);
      }
    }

    // OUTPUT PIN

    /*
            }

                    else if ((state != currentStatus.state) || currentStatus.forceEvent) {
                      // Reset forceEvent
                      currentStatus.forceEvent = 0;
                      currentStatus.state = state;
                      UserVar.setFloat(event->TaskIndex, 0, state);

                      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                        String log = F("SW  : GPIO=");
                        log += CONFIG_PIN1;
                        log += F(" State=");
                        log += state ? '1' : '0';
                        addLog(LOG_LEVEL_INFO, log);
                      }
                      sendData(event);
                      savePortStatus(key, currentStatus);

                    }
     */
  }
}

#endif // ifdef USES_P001
