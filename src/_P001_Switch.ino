#ifdef USES_P001
//#######################################################################################################
//#################################### Plugin 001: Input Switch #########################################
//#######################################################################################################

/**************************************************\
CONFIG
TaskDevicePluginConfig settings:
0: button type (switch or dimmer)
1: dim value
2: button option (normal, push high, push low)
3: send boot state (true,false)
4: use doubleclick (0,1,2,3)
5: use longpress (0,1,2,3)
6: LP fired (true,false)
7: doubleclick counter (=0,1,2,3)

TaskDevicePluginConfigFloat settings:
0: debounce interval ms
1: doubleclick interval ms
2: longpress interval ms
3: use safebutton (=0,1)

TaskDevicePluginConfigLong settings:
0: clickTime debounce ms
1: clickTime doubleclick ms
2: clickTime longpress ms
3: safebutton counter (=0,1)
\**************************************************/

#define PLUGIN_001
#define PLUGIN_ID_001         1
#define PLUGIN_NAME_001       "Switch input - Switch"
#define PLUGIN_VALUENAME1_001 "Switch"
// Make sure the initial default is a switch (value 0)
#define PLUGIN_001_TYPE_SWITCH 0
#define PLUGIN_001_TYPE_DIMMER 3 // Due to some changes in previous versions, do not use 2.
#define PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH 0
#define PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW 1
#define PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH 2


// Needed for the GPIO handling.
#include "Commands/Gpio.h"
// As soon as we allow switch plugin to be used on port expander, we may need to move those functions.
// For now, the Switch plugin only supports the internal GPIO pins.

boolean Plugin_001(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  //static byte switchstate[TASKS_MAX];
  //static byte outputstate[TASKS_MAX];
  //static int8_t PinMonitor[GPIO_MAX];
  //static int8_t PinMonitorState[GPIO_MAX];

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_001;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = true;
        Device[deviceCount].InverseLogicOption = true;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_001);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_001));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        // FIXME TD-er: This plugin is handling too much.
        // - switch/dimmer input
        // - PWM output
        // - switch output (relays)
        // - servo output
        // - sending pulses
        // - playing tunes
        event->String1 = formatGpioName_bidirectional("");
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        //@giig1967g: set current task value for taking actions after changes in the task gpio
        const uint32_t key = createInternalGpioKey(Settings.TaskDevicePin1[event->TaskIndex]);
        if (existPortStatus(key)) {
          globalMapPortStatus[key].previousTask = event->TaskIndex;
        }

        String options[2];
        options[0] = F("Switch");
        options[1] = F("Dimmer");
        int optionValues[2] = { PLUGIN_001_TYPE_SWITCH, PLUGIN_001_TYPE_DIMMER };
        const byte switchtype = P001_getSwitchType(event);
        addFormSelector(F("Switch Type"), F("p001_type"), 2, options, optionValues, switchtype);

        if (switchtype == PLUGIN_001_TYPE_DIMMER)
        {
          addFormNumericBox(F("Dim value"), F("p001_dimvalue"), Settings.TaskDevicePluginConfig[event->TaskIndex][1], 0, 255);
        }

        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        String buttonOptions[3];
        buttonOptions[0] = F("Normal Switch");
        buttonOptions[1] = F("Push Button Active Low");
        buttonOptions[2] = F("Push Button Active High");
        int buttonOptionValues[3] = {PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH, PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW, PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH};
        addFormSelector(F("Switch Button Type"), F("p001_button"), 3, buttonOptions, buttonOptionValues, choice);

        addSendBootStateForm(event->TaskIndex, 3);

        addAdvancedEventManagementSubHeader();

        addDebounceForm(event->TaskIndex);
        addDoubleClickEventForm(event->TaskIndex);
        addLongPressEventForm(event->TaskIndex);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("p001_type"));
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == PLUGIN_001_TYPE_DIMMER)
        {
          Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("p001_dimvalue"));
        }

        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("p001_button"));

        saveSendBootStateForm(event->TaskIndex, 3);
        saveDebounceForm(event->TaskIndex);
        saveDoubleClickEventForm(event->TaskIndex);
        saveLongPressEventForm(event->TaskIndex);

        update_globalMapPortStatus_onSave(event->TaskIndex, PLUGIN_ID_000);
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        //apply INIT only if PORT is in range. Do not start INIT if port not set in the device page.
        if (checkValidGpioPin(Settings.TaskDevicePin1[event->TaskIndex]))
        {
          const uint32_t key = createInternalGpioKey(Settings.TaskDevicePin1[event->TaskIndex]);
          //Read current status or create empty if it does not exist
          portStatusStruct newStatus = globalMapPortStatus[key];

          // read and store current state to prevent switching at boot time
          newStatus.updatePinState(read_GPIO_state(event));
          newStatus.output = newStatus.state;
          newStatus.task++; // add this GPIO/port as a task

          //setPinState(PLUGIN_ID_001, Settings.TaskDevicePin1[event->TaskIndex], PIN_MODE_INPUT, switchstate[event->TaskIndex]);
          //  if it is in the device list we assume it's an input pin
          if (Settings.TaskDevicePin1PullUp[event->TaskIndex]) {
            pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT_PULLUP);
            newStatus.mode = PIN_MODE_INPUT_PULLUP;
          } else {
            pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT);
            newStatus.mode = PIN_MODE_INPUT;
          }
          // if boot state must be send, inverse default state
          // this is done to force the trigger in PLUGIN_TEN_PER_SECOND
          if (hlp_getMustSendBootState(event->TaskIndex, 3))
          {
            newStatus.updatePinState(!newStatus.state);
            newStatus.output = !newStatus.output;
          }

          // set initial UserVar of the switch
          if (Settings.TaskDevicePin1Inversed[event->TaskIndex]){
            UserVar[event->BaseVarIndex] = !newStatus.state;
          } else {
            UserVar[event->BaseVarIndex] = newStatus.state;
          }

          // counters = 0
          hlp_setDoubleClickCounter(event->TaskIndex, 0);     //doubleclick counter
          hlp_setSafebuttonCounter(event->TaskIndex, 0); //safebutton counter

          //used to track if LP has fired
          hlp_setLongPressFired(event->TaskIndex, false);

          //store millis for debounce, doubleclick and long press
          unsigned long currentTime = millis();
          hlp_setClicktimeDebounce(event->TaskIndex, currentTime); //debounce timer
          hlp_setClicktimeDoubleClick(event->TaskIndex, currentTime); //doubleclick timer
          hlp_setClicktimeLongpress(event->TaskIndex, currentTime); //longpress timer

          setDoubleClickMinInterval(event->TaskIndex);
          setLongPressMinInterval(event->TaskIndex);

          savePortStatus(key,newStatus);
        }
        success = true;
        break;
      }

    case PLUGIN_REQUEST:
      {
        //String device = parseString(string, 1);
        //String command = parseString(string, 2);
        //String strPar1 = parseString(string, 3);

        // returns pin value using syntax: [plugin#gpio#pinstate#xx]
        if (string.length()>=13 && string.substring(0,13).equalsIgnoreCase(F("gpio,pinstate")))
        {
          int par1;
            if (validIntFromString(parseString(string, 3), par1)) {
            string = digitalRead(par1);
          }
          success = true;
        }
        break;
      }

    case PLUGIN_UNCONDITIONAL_POLL:
      {
        // port monitoring, generates an event by rule command 'monitor_gpio,port#'
        for (auto it=globalMapPortStatus.begin(); it!=globalMapPortStatus.end(); ++it) {
          if (it->second.mustPollGpioState() && getPluginFromKey(it->first)==PLUGIN_ID_000 ) {
            const uint16_t port = getPortFromKey(it->first);
            bool gpio_state = read_GPIO_state(port, it->second.mode);
            if (it->second.getPinState() != gpio_state) {
              if (!it->second.task) it->second.updatePinState(gpio_state); //do not update state if task flag=1 otherwise it will not be picked up by 10xSEC function
              if (it->second.monitor) {
                String eventString = F("GPIO#");
                eventString += port;
                eventString += '=';
                eventString += static_cast<int>(gpio_state);
                rulesProcessing(eventString);
              }
            }
          }
        }
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        const bool gpio_state = read_GPIO_state(event);
        bool needToSendEvent = false;
        bool sendState = gpio_state;
        byte output_value = gpio_state;


        /**************************************************************************\
        20181009 - @giig1967g: new doubleclick logic is:
        if there is a 'gpio_state' change, check debounce period.
        Then if doubleclick interval exceeded, reset Settings.TaskDevicePluginConfig[event->TaskIndex][7] to 0
        Settings.TaskDevicePluginConfig[event->TaskIndex][7] contains the current status for doubleclick:
        0: start counting
        1: 1st click
        2: 2nd click
        3: 3rd click = doubleclick event if inside interval (calculated as: '3rd click time' minus '1st click time')

        Returned EVENT value is = 3 always for doubleclick
        In rules this can be checked:
        on Button#Switch=3 do //will fire if doubleclick
        \**************************************************************************/

        //long difftimer1 = 0;
        //long difftimer2 = 0;
        //long timerstats = millis();

        //Bug fixed: avoid 10xSEC in case of a non-fully configured device (no GPIO defined yet)
        if (checkValidGpioPin(Settings.TaskDevicePin1[event->TaskIndex])) {
          const uint32_t key = createInternalGpioKey(Settings.TaskDevicePin1[event->TaskIndex]);
          //WARNING operator [],creates an entry in map if key doesn't exist:
          portStatusStruct currentStatus = globalMapPortStatus[key];

          //CASE 1: using SafeButton, so wait 1 more 100ms cycle to acknowledge the status change
          //QUESTION: MAYBE IT'S BETTER TO WAIT 2 CYCLES??
          const bool gpio_state_changed = gpio_state != currentStatus.getPinState();
          if (round(hlp_getUseSafeButton(event->TaskIndex)) && gpio_state_changed && hlp_getSafebuttonCounter(event->TaskIndex) == 0)
          {
            addLog(LOG_LEVEL_DEBUG, F("SW  :SafeButton activated"));
            hlp_setSafebuttonCounter(event->TaskIndex, 1);
          }
          //CASE 2: not using SafeButton, or already waited 1 more 100ms cycle, so proceed.
          else if (gpio_state_changed)
          {
            // FIXME TD-er: Temporary counters and states should not be in settings.
            // Reset SafeButton counter
            hlp_setSafebuttonCounter(event->TaskIndex, 0);
            hlp_resetLongPressTimer(event->TaskIndex);
            if (hlp_debounceTimoutPassed(event->TaskIndex)) //de-bounce check
            {
              hlp_processDoubleClick(event->TaskIndex, gpio_state);

              currentStatus.updatePinState(gpio_state);
              const bool currentOutputState = currentStatus.output == 1;
              bool new_outputState = currentOutputState;
              switch(Settings.TaskDevicePluginConfig[event->TaskIndex][2])
              {
                case PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH:
                    new_outputState = gpio_state;
                  break;
                case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW:
                  if (!gpio_state)
                    new_outputState = !currentOutputState;
                  break;
                case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH:
                  if (gpio_state)
                    new_outputState = !currentOutputState;
                  break;
              }

              // send if output needs to be changed
              if (currentOutputState != new_outputState)
              {
                needToSendEvent = true;
                currentStatus.output = new_outputState;
                sendState = new_outputState;
                output_value = hlp_getOutputValue_updateSendState(event->TaskIndex, sendState);
                event->sensorType = SENSOR_TYPE_SWITCH;
                if (P001_getSwitchType(event) == PLUGIN_001_TYPE_DIMMER) {
                  if (sendState) {
                    output_value = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
                    // Only set type to being dimmer when setting a value else it is "switched off".
                    event->sensorType = SENSOR_TYPE_DIMMER;
                  }
                }
              }
              hlp_setClicktimeDebounce(event->TaskIndex, millis());
            }
            savePortStatus(key, currentStatus);
          }

          //CASE 3: status unchanged. Checking longpress:
          else if (hlp_LongPressEnabled_and_notFired(event->TaskIndex, gpio_state)) {

            /**************************************************************************\
            20181009 - @giig1967g: new longpress logic is:
            if there is no 'gpio_state' change, check if longpress interval reached
            When reached send longpress event.
            Returned Event value = gpio_state + 10
            So if gpio_state = 0 => EVENT longpress = 10
            if gpio_state = 1 => EVENT longpress = 11
            So we can trigger longpress for high or low contact

            In rules this can be checked:
            on Button#Switch=10 do //will fire if longpress when gpio_state = 0
            on Button#Switch=11 do //will fire if longpress when gpio_state = 1
            \**************************************************************************/
            // Reset SafeButton counter
            hlp_setSafebuttonCounter(event->TaskIndex, 0);

            if (hlp_LongPressIntervalReached(event->TaskIndex))
            {
              hlp_setLongPressFired(event->TaskIndex,  true);

              needToSendEvent = false;
              switch(Settings.TaskDevicePluginConfig[event->TaskIndex][2])
              {
                case PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH:
                    needToSendEvent = true;
                  break;
                case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW:
                  if (!gpio_state)
                    needToSendEvent = true;
                  break;
                case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH:
                  if (gpio_state)
                    needToSendEvent = true;
                  break;
              }

              if (needToSendEvent) {
                sendState = gpio_state;
                output_value = hlp_getOutputValue_updateSendState(event->TaskIndex, sendState);
              }
              savePortStatus(key, currentStatus);
            }
          } else {
            // Reset SafeButton counter
            hlp_setSafebuttonCounter(event->TaskIndex, 0);
          }
        }
        if (needToSendEvent) {
          UserVar[event->BaseVarIndex] = output_value;
          hlp_logState_and_Output(event->TaskIndex, gpio_state, sendState, output_value, PLUGIN_ID_000);
          sendData(event);

          //reset Userdata so it displays the correct gpio_state value in the web page
          UserVar[event->BaseVarIndex] = sendState ? 1 : 0;
        }
        success = true;
        break;
      }

    case PLUGIN_EXIT:
    {
      removeTaskFromPort(createInternalGpioKey(Settings.TaskDevicePin1[event->TaskIndex]));
      break;
    }

    case PLUGIN_READ:
      {
        // We do not actually read the pin state as this is already done 10x/second
        // Instead we just send the last known state stored in Uservar
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("SW   : State ");
          log += UserVar[event->BaseVarIndex];
          addLog(LOG_LEVEL_INFO, log);
        }
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String command = parseString(string, 1);
        if (command == F("inputswitchstate")) {
          if (checkValidGpioPin(event->Par1)) {
            portStatusStruct tempStatus;
            const uint32_t   key = createInternalGpioKey(Settings.TaskDevicePin1[event->Par1]);

            // WARNING: operator [] creates an entry in the map if key does not exist
            // So the next command should be part of each command:
            tempStatus = globalMapPortStatus[key];

            UserVar[event->Par1 * VARS_PER_TASK] = event->Par2;
            tempStatus.output                    = event->Par2;
            tempStatus.command                   = 1;
            savePortStatus(key, tempStatus);
            success = true;
          }
        }
        break;
      }

    case PLUGIN_TIMER_IN:
      {
        digitalWrite(event->Par1, event->Par2);
        //setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, event->Par2);
        portStatusStruct tempStatus;
        // WARNING: operator [] creates an entry in the map if key does not exist
        const uint32_t key = createInternalGpioKey(event->Par1);
        tempStatus = globalMapPortStatus[key];

        tempStatus.updatePinState(event->Par2);
        tempStatus.mode = PIN_MODE_OUTPUT;
        savePortStatus(key,tempStatus);
        break;
      }
  }
  return success;
}


// TD-er: Needed to fix a mistake in earlier fixes.
byte P001_getSwitchType(struct EventStruct *event) {
  byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
  switch (choice) {
    case 2: // Old implementation for Dimmer
    case PLUGIN_001_TYPE_DIMMER:
      choice = PLUGIN_001_TYPE_DIMMER;
      break;
    case 1: // Old implementation for switch
    case PLUGIN_001_TYPE_SWITCH:
    default:
      choice = PLUGIN_001_TYPE_SWITCH;
      break;
  }
  return choice;
}



#endif // USES_P001
