#ifdef USES_P019
//#######################################################################################################
//#################################### Plugin 019: PCF8574 ##############################################
//#######################################################################################################

/**************************************************\
CONFIG
TaskDevicePluginConfig settings:
0: send boot state (true,false)
1:
2:
3:
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

#define PLUGIN_019
#define PLUGIN_ID_019         19
#define PLUGIN_NAME_019       "Switch input - PCF8574"
#define PLUGIN_VALUENAME1_019 "Switch"

boolean Plugin_019(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  //static int8_t switchstate[TASKS_MAX];

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_019;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
        Device[deviceCount].Ports = 8;
        Device[deviceCount].PullUpOption = false;
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
        string = F(PLUGIN_NAME_019);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_019));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        //@giig1967g: set current task value for taking actions after changes
        const uint32_t key = createKey(PLUGIN_ID_019,Settings.TaskDevicePort[event->TaskIndex]);
        if (existPortStatus(key)) {
          globalMapPortStatus[key].previousTask = event->TaskIndex;
        }

        addSendBootStateForm(event->TaskIndex, 0);

        addAdvancedEventManagementSubHeader();

        addDebounceForm(event->TaskIndex);
        addDoubleClickEventForm(event->TaskIndex);
        addLongPressEventForm(event->TaskIndex);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        saveSendBootStateForm(event->TaskIndex, 0);
        saveDebounceForm(event->TaskIndex);
        saveDoubleClickEventForm(event->TaskIndex);
        saveLongPressEventForm(event->TaskIndex);

        update_globalMapPortStatus_onSave(event->TaskIndex, PLUGIN_ID_019);
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        //apply INIT only if PIN is in range. Do not start INIT if pin not set in the device page.
        if (Settings.TaskDevicePort[event->TaskIndex] >= 0)
        {
          portStatusStruct newStatus;
          const uint32_t key = createKey(PLUGIN_ID_019,Settings.TaskDevicePort[event->TaskIndex]);
          //Read current status or create empty if it does not exist
          newStatus = globalMapPortStatus[key];

          // read and store current state to prevent switching at boot time
          // "state" could be -1, 0 or 1
          newStatus.updatePinState(Plugin_019_Read(Settings.TaskDevicePort[event->TaskIndex]));
          newStatus.output = newStatus.state;
          (newStatus.portStateError()) ? newStatus.mode = PIN_MODE_OFFLINE : newStatus.mode = PIN_MODE_INPUT; // @giig1967g: if it is in the device list we assume it's an input pin
          newStatus.task++; // add this GPIO/port as a task

          // @giig1967g-20181022: set initial UserVar of the switch
          if (newStatus.state != -1 && Settings.TaskDevicePin1Inversed[event->TaskIndex]) {
            UserVar[event->BaseVarIndex] = !newStatus.state;
          } else {
            UserVar[event->BaseVarIndex] = newStatus.state;
          }

          // if boot state must be send, inverse default state
          // this is done to force the trigger in PLUGIN_TEN_PER_SECOND
          if (hlp_getMustSendBootState(event->TaskIndex, 0))
            newStatus.updatePinState(!newStatus.state);
            // FIXME TD-er: Why not set the output state, like is done in P001?


          // @giig1967g-20181022: counter = 0
          hlp_setDoubleClickCounter(event->TaskIndex, 0);     //doubleclick counter
          hlp_setSafebuttonCounter(event->TaskIndex, 0); //safebutton counter

          // @giig1967g-20181022: used to track if LP has fired
          hlp_setLongPressFired(event->TaskIndex, false);

          // @giig1967g-20181022: store millis for debounce, doubleclick and long press
          hlp_setClicktimeDebounce(event->TaskIndex, millis()); //debounce timer
          hlp_setClicktimeDoubleClick(event->TaskIndex, millis()); //doubleclick timer
          hlp_setClicktimeLongpress(event->TaskIndex, millis()); //longpress timer

          setDoubleClickMinInterval(event->TaskIndex);
          setLongPressMinInterval(event->TaskIndex);

          //setPinState(PLUGIN_ID_019, Settings.TaskDevicePort[event->TaskIndex], PIN_MODE_INPUT, switchstate[event->TaskIndex]);
          savePortStatus(key,newStatus);

        }
        success = true;
        break;
      }

      case PLUGIN_UNCONDITIONAL_POLL:
        {
          // port monitoring, generates an event by rule command 'monitor,pcf,port#'
          for (auto it=globalMapPortStatus.begin(); it!=globalMapPortStatus.end(); ++it) {
            if (getPluginFromKey(it->first)==PLUGIN_ID_019 && it->second.mustPollGpioState()) {
              const uint16_t port = getPortFromKey(it->first);
              int8_t state = Plugin_019_Read(port);
              if (it->second.state != state) {
                if (it->second.mode == PIN_MODE_OFFLINE) it->second.mode=PIN_MODE_UNDEFINED; //changed from offline to online
                if (state == -1) it->second.mode=PIN_MODE_OFFLINE; //changed from online to offline
                if (!it->second.task) it->second.updatePinState(state); //do not update state if task flag=1 otherwise it will not be picked up by 10xSEC function
                if (it->second.monitor) {
                  String eventString = F("PCF#");
                  eventString += port;
                  eventString += '=';
                  eventString += state;
                  rulesProcessing(eventString);
                }
              }
            }
          }
          break;
        }

    case PLUGIN_TEN_PER_SECOND:
      {
        const int8_t state = Plugin_019_Read(Settings.TaskDevicePort[event->TaskIndex]);
        const bool gpio_state = state == 1;
        bool needToSendEvent = false;
        bool sendState = gpio_state;
        byte output_value = gpio_state;


        /**************************************************************************\
        20181022 - @giig1967g: new doubleclick logic is:
        if there is a 'state' change, check debounce period.
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
        const uint32_t key = createKey(PLUGIN_ID_019,Settings.TaskDevicePort[event->TaskIndex]);
        //WARNING operator [],creates an entry in map if key doesn't exist:
        portStatusStruct currentStatus = globalMapPortStatus[key];

        const bool gpio_state_changed = state != currentStatus.state;

        //Bug fixed: avoid 10xSEC in case of a non-fully configured device (no port defined yet)
        if (state != -1 && Settings.TaskDevicePort[event->TaskIndex]>=0) {

          //CASE 1: using SafeButton, so wait 1 more 100ms cycle to acknowledge the status change
          //QUESTION: MAYBE IT'S BETTER TO WAIT 2 CYCLES??
          if (round(hlp_getUseSafeButton(event->TaskIndex)) && gpio_state_changed && hlp_getSafebuttonCounter(event->TaskIndex) == 0)
          {
            addLog(LOG_LEVEL_DEBUG, F("PCF :SafeButton activated"));
            hlp_setSafebuttonCounter(event->TaskIndex, 1);
          }
          //CASE 2: not using SafeButton, or already waited 1 more 100ms cycle, so proceed.
          else if (gpio_state_changed)
          {
            // Reset SafeButton counter
            hlp_setSafebuttonCounter(event->TaskIndex, 0);
            hlp_resetLongPressTimer(event->TaskIndex);
            if (hlp_debounceTimoutPassed(event->TaskIndex)) //de-bounce check
            {
              hlp_processDoubleClick(event->TaskIndex, gpio_state);
              needToSendEvent = true;

              if (currentStatus.mode == PIN_MODE_OFFLINE || currentStatus.mode == PIN_MODE_UNDEFINED)
              {
                currentStatus.mode = PIN_MODE_INPUT; //changed from offline to online
              }
              currentStatus.updatePinState(state);
              sendState = currentStatus.getPinState();
              output_value = hlp_getOutputValue_updateSendState(event->TaskIndex, sendState);
              event->sensorType = SENSOR_TYPE_SWITCH;

              hlp_setClicktimeDebounce(event->TaskIndex, millis());
            }
            savePortStatus(key, currentStatus);
          }

          //CASE 3: status unchanged. Checking longpress:
          else if (hlp_LongPressEnabled_and_notFired(event->TaskIndex, gpio_state)) {

            /**************************************************************************\
            20181022 - @giig1967g: new longpress logic is:
            if there is no 'state' change, check if longpress interval reached
            When reached send longpress event.
            Returned Event value = state + 10
            So if state = 0 => EVENT longpress = 10
            if state = 1 => EVENT longpress = 11
            So we can trigger longpress for high or low contact

            In rules this can be checked:
            on Button#Switch=10 do //will fire if longpress when state = 0
            on Button#Switch=11 do //will fire if longpress when state = 1
            \**************************************************************************/
            // Reset SafeButton counter
            hlp_setSafebuttonCounter(event->TaskIndex, 0);

            if (hlp_LongPressIntervalReached(event->TaskIndex))
            {
              hlp_setLongPressFired(event->TaskIndex,  true); //fired = true
              needToSendEvent = true;
              sendState = gpio_state;
              output_value = hlp_getOutputValue_updateSendState(event->TaskIndex, sendState);
            }
          } else {
            // Reset SafeButton counter
            hlp_setSafebuttonCounter(event->TaskIndex, 0);
          }
        } else if (gpio_state_changed && state == -1) {
          //set UserVar and switchState = -1 and send EVENT to notify user
          UserVar[event->BaseVarIndex] = state;
          //switchstate[event->TaskIndex] = state;
          currentStatus.updatePinState(state);
          currentStatus.mode = PIN_MODE_OFFLINE;
          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("PCF  : Port=");
            log += Settings.TaskDevicePort[event->TaskIndex];
            log += F(" is offline (EVENT= -1)");
            addLog(LOG_LEVEL_INFO, log);
          }
          sendData(event);
          savePortStatus(key, currentStatus);
        }
        if (needToSendEvent) {
          UserVar[event->BaseVarIndex] = output_value;
          hlp_logState_and_Output(event->TaskIndex, gpio_state, sendState, output_value, PLUGIN_ID_019);
          sendData(event);

          //reset Userdata so it displays the correct gpio_state value in the web page
          UserVar[event->BaseVarIndex] = sendState ? 1 : 0;
        }
        success = true;
        break;
      }

      //giig1967g: Added EXIT function
      case PLUGIN_EXIT:
      {
        removeTaskFromPort(createKey(PLUGIN_ID_019,Settings.TaskDevicePort[event->TaskIndex]));
        break;
      }

    case PLUGIN_READ:
      {
        // We do not actually read the pin state as this is already done 10x/second
        // Instead we just send the last known state stored in Uservar
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("PCF  : Port= ");
          log += Settings.TaskDevicePort[event->TaskIndex];
          log += F(" State=");
          log += UserVar[event->BaseVarIndex];
          addLog(LOG_LEVEL_INFO, log);
        }
        success = true;
        break;
      }

    case PLUGIN_REQUEST:
      {
        //parseString(string, 1) = device
        //parseString(string, 2) = command
        //parseString(string, 3) = gpio number

        // returns pin value using syntax: [plugin#pcfgpio#pinstate#xx]
        if (string.length()>=16 && string.substring(0,16).equalsIgnoreCase(F("pcfgpio,pinstate")))
        {
          int par1;
          if (validIntFromString(parseString(string, 3), par1)) {
            string = Plugin_019_Read(par1);
          }
          success = true;
        }
        break;
      }

    case PLUGIN_WRITE:
      {
        String log = "";
        String command = parseString(string, 1);

        if (command == F("pcfgpio"))
        {
          success = true;
          if (event->Par1 > 0 && event->Par1 <= 128)
          {
            portStatusStruct tempStatus;
            const uint32_t key = createKey(PLUGIN_ID_019,event->Par1);
            // WARNING: operator [] creates an entry in the map if key does not exist
            // So the next command should be part of each command:
            tempStatus = globalMapPortStatus[key];

            int8_t currentState = Plugin_019_Read(event->Par1);

            if (currentState == -1) {
              tempStatus.mode=PIN_MODE_OFFLINE;
              tempStatus.updatePinState(-1);
              tempStatus.command=1; //set to 1 in order to display the status in the PinStatus page
              savePortStatus(key,tempStatus);
              log = String(F("PCF  : GPIO ")) + String(event->Par1) + String(F(" is offline (-1). Cannot set value."));
            } else if (event->Par2 == 2) { //INPUT
          	  // PCF8574 specific: only can read 0/low state, so we must send 1
          	  //setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_INPUT, 1);
              tempStatus.mode=PIN_MODE_INPUT;
              tempStatus.updatePinState(currentState);
              tempStatus.command=1; //set to 1 in order to display the status in the PinStatus page
              savePortStatus(key,tempStatus);
          	  Plugin_019_Write(event->Par1,1);
          	  log = String(F("PCF  : GPIO INPUT ")) + String(event->Par1) + String(F(" Set to 1"));
            } else { // OUTPUT
          	  //setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, event->Par2);
              tempStatus.mode=PIN_MODE_OUTPUT;
              tempStatus.updatePinState(event->Par2);
              tempStatus.command=1; //set to 1 in order to display the status in the PinStatus page
              savePortStatus(key,tempStatus);
              Plugin_019_Write(event->Par1, event->Par2);
          	  log = String(F("PCF  : GPIO OUTPUT ")) + String(event->Par1) + String(F(" Set to ")) + String(event->Par2);
            }
            addLog(LOG_LEVEL_INFO, log);
            SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);
            //SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_019, event->Par1, log, 0));
          }
        } else if (command == F("pcfgpiotoggle")) {
          success = true;
          if (event->Par1 > 0 && event->Par1 <= 128)
          {
            portStatusStruct tempStatus;
            const uint32_t key = createKey(PLUGIN_ID_019,event->Par1);
            // WARNING: operator [] creates an entry in the map if key does not exist
            // So the next command should be part of each command:
            tempStatus = globalMapPortStatus[key];
            int8_t currentState = Plugin_019_Read(event->Par1);
            bool needToSave = false;

            if (currentState == -1) {
              tempStatus.mode=PIN_MODE_OFFLINE;
              tempStatus.updatePinState(-1);
              tempStatus.command=1; //set to 1 in order to display the status in the PinStatus page
              savePortStatus(key,tempStatus);
              log = String(F("PCF  : GPIO ")) + String(event->Par1) + String(F(" is offline (-1). Cannot set value."));
              needToSave = true;
            } else if (tempStatus.mode == PIN_MODE_OUTPUT || tempStatus.mode == PIN_MODE_UNDEFINED) { //toggle only output pins
              tempStatus.updatePinState(!currentState); //toggle current state value
              tempStatus.mode = PIN_MODE_OUTPUT;
              tempStatus.command=1; //set to 1 in order to display the status in the PinStatus page
              savePortStatus(key,tempStatus);
              Plugin_019_Write(event->Par1, tempStatus.state);
              log = String(F("PCF  : Toggle GPIO ")) + String(event->Par1) + String(F(" Set to ")) + String(tempStatus.state);
              needToSave = true;
            }
            if (needToSave) {
              //setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, !currentState);
              addLog(LOG_LEVEL_INFO, log);
              //SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_019, event->Par1, log, 0));
              SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);
            }
          }
        } else if (command == F("pcfpulse")) {
          success = true;
          if (event->Par1 > 0 && event->Par1 <= 128)
          {
            portStatusStruct tempStatus;
            const uint32_t key = createKey(PLUGIN_ID_019,event->Par1);
            // WARNING: operator [] creates an entry in the map if key does not exist
            // So the next command should be part of each command:
            tempStatus = globalMapPortStatus[key];

            //setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, event->Par2);
            tempStatus.mode = PIN_MODE_OUTPUT;
            tempStatus.updatePinState(event->Par2);
            savePortStatus(key,tempStatus);
            Plugin_019_Write(event->Par1, event->Par2);
            delay(event->Par3);

            tempStatus.mode = PIN_MODE_OUTPUT;
            tempStatus.updatePinState(!event->Par2);
            tempStatus.command=1; //set to 1 in order to display the status in the PinStatus page
            savePortStatus(key,tempStatus);
            Plugin_019_Write(event->Par1, !event->Par2);
            //setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, !event->Par2);

            log = String(F("PCF  : GPIO ")) + String(event->Par1) + String(F(" Pulsed for ")) + String(event->Par3) + String(F(" mS"));
            addLog(LOG_LEVEL_INFO, log);
            //SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_019, event->Par1, log, 0));
            SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);
          }
        } else if (command == F("pcflongpulse")) {
          success = true;
          if (event->Par1 > 0 && event->Par1 <= 128)
          {
            portStatusStruct tempStatus;
            const uint32_t key = createKey(PLUGIN_ID_019,event->Par1);
            // WARNING: operator [] creates an entry in the map if key does not exist
            // So the next command should be part of each command:
            tempStatus = globalMapPortStatus[key];

            //setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, event->Par2);
            tempStatus.mode = PIN_MODE_OUTPUT;
            tempStatus.updatePinState(event->Par2);
            tempStatus.command=1; //set to 1 in order to display the status in the PinStatus page
            savePortStatus(key,tempStatus);
            Plugin_019_Write(event->Par1, event->Par2);
            setPluginTaskTimer(event->Par3 * 1000, PLUGIN_ID_019, event->TaskIndex, event->Par1, !event->Par2); //Calls PLUGIN_TIMER_IN
            log = String(F("PCF  : GPIO ")) + String(event->Par1) + String(F(" Pulse set for ")) + String(event->Par3) + String(F(" S"));
            addLog(LOG_LEVEL_INFO, log);
            //SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_019, event->Par1, log, 0));
            SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);
          }
        } else if (command == F("status")) {
          if (parseString(string, 2) == F("pcf"))
          {
            success = true;
            const uint32_t key = createKey(PLUGIN_ID_019,event->Par2); //WARNING: 'status' uses Par2 instead of Par1

            if (existPortStatus(key))  // has been set as output
              SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, dummyString, 0);
            else
            {
              int state = Plugin_019_Read(event->Par2); // report as input
              if (state != -1)
                SendStatusOnlyIfNeeded(event->Source, NO_SEARCH_PIN_STATE, key, dummyString, state);
            }
          }
        }  else if (command == F("monitor")) {
          if (parseString(string, 2) == F("pcf"))
          {
            success = true;
            const uint32_t key = createKey(PLUGIN_ID_019,event->Par2); //WARNING: 'monitor' uses Par2 instead of Par1

            addMonitorToPort(key);
            log = String(F("PCF  : PORT ")) + String(event->Par2) + String(F(" added to monitor list."));
            addLog(LOG_LEVEL_INFO, log);
            SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, dummyString, 0);
          }
        }  else if (command == F("unmonitor")) {
          if (parseString(string, 2) == F("pcf"))
          {
            success = true;
            const uint32_t key = createKey(PLUGIN_ID_019,event->Par2); //WARNING: 'monitor' uses Par2 instead of Par1

            removeMonitorFromPort(key);
            log = String(F("PCF  : PORT ")) + String(event->Par2) + String(F(" removed from monitor list."));
            addLog(LOG_LEVEL_INFO, log);
            SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, dummyString, 0);
          }
        }
        break;
      }

    case PLUGIN_TIMER_IN:
      {
        //setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, event->Par2);
        portStatusStruct tempStatus;
        // WARNING: operator [] creates an entry in the map if key does not exist
        const uint32_t key = createKey(PLUGIN_ID_019,event->Par1);
        tempStatus = globalMapPortStatus[key];

        tempStatus.updatePinState(event->Par2);
        tempStatus.mode = PIN_MODE_OUTPUT;
        savePortStatus(key,tempStatus);
        Plugin_019_Write(event->Par1, event->Par2);

        break;
      }
  }
  return success;
}


//********************************************************************************
// PCF8574 read
//********************************************************************************
//@giig1967g-20181023: changed to int8_t
int8_t Plugin_019_Read(byte Par1)
{
  int8_t state = -1;
  byte unit = (Par1 - 1) / 8;
  byte port = Par1 - (unit * 8);
  uint8_t address = 0x20 + unit;
  if (unit > 7) address += 0x10;

  // get the current pin status
  Wire.requestFrom(address, (uint8_t)0x1);
  if (Wire.available())
  {
    state = ((Wire.read() & _BV(port - 1)) >> (port - 1));
  }
  return state;
}

uint8_t Plugin_019_ReadAllPins(uint8_t address)
{
  uint8_t rawState = 0;

  Wire.requestFrom(address, (uint8_t)0x1);
  if (Wire.available())
  {
    rawState =Wire.read();
  }
  return rawState;
}

//********************************************************************************
// PCF8574 write
//********************************************************************************
boolean Plugin_019_Write(byte Par1, byte Par2)
{
  uint8_t unit = (Par1 - 1) / 8;
  uint8_t port = Par1 - (unit * 8);
  uint8_t address = 0x20 + unit;
  if (unit > 7) address += 0x10;

  //generate bitmask
  int i = 0;
  uint8_t portmask = 255;
  unit = unit * 8 + 1; // calculate first pin

  uint32_t key;

  for(i=0; i<8; i++){
    key = createKey(PLUGIN_ID_019,unit+i);

    if (existPortStatus(key) && globalMapPortStatus[key].mode == PIN_MODE_OUTPUT && globalMapPortStatus[key].state == 0)
      portmask &= ~(1 << i); //set port i = 0
  }

  key = createKey(PLUGIN_ID_019,Par1);

  if (Par2 == 1)
    portmask |= (1 << (port-1));
  else
    portmask &= ~(1 << (port-1));

  Wire.beginTransmission(address);
  Wire.write(portmask);
  Wire.endTransmission();

  return true;
}
#endif // USES_P019
