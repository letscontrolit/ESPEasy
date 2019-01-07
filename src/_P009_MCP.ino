#ifdef USES_P009
//#######################################################################################################
//#################################### Plugin 009: MCP23017 input #######################################
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

#define PLUGIN_009
#define PLUGIN_ID_009         9
#define PLUGIN_NAME_009       "Switch input - MCP23017"
#define PLUGIN_VALUENAME1_009 "Switch"
#define PLUGIN_009_DOUBLECLICK_MIN_INTERVAL 1000
#define PLUGIN_009_DOUBLECLICK_MAX_INTERVAL 3000
#define PLUGIN_009_LONGPRESS_MIN_INTERVAL 1000
#define PLUGIN_009_LONGPRESS_MAX_INTERVAL 5000
#define PLUGIN_009_DC_DISABLED 0
#define PLUGIN_009_DC_LOW 1
#define PLUGIN_009_DC_HIGH 2
#define PLUGIN_009_DC_BOTH 3
#define PLUGIN_009_LONGPRESS_DISABLED 0
#define PLUGIN_009_LONGPRESS_LOW 1
#define PLUGIN_009_LONGPRESS_HIGH 2
#define PLUGIN_009_LONGPRESS_BOTH 3

boolean Plugin_009(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  //static int8_t switchstate[TASKS_MAX];

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_009;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
        Device[deviceCount].Ports = 16;
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
        string = F(PLUGIN_NAME_009);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_009));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        //@giig1967g: set current task value for taking actions after changes
        const uint32_t key = createKey(PLUGIN_ID_009,Settings.TaskDevicePort[event->TaskIndex]);
        if (existPortStatus(key)) {
          globalMapPortStatus[key].previousTask = event->TaskIndex;
        }

        addFormCheckBox(F("Send Boot state") ,F("p009_boot"), Settings.TaskDevicePluginConfig[event->TaskIndex][0]);

        //@giig1967-20181022
        addFormSubHeader(F("Advanced event management"));

        addFormNumericBox(F("De-bounce (ms)"), F("p009_debounce"), round(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0]), 0, 250);

        //set minimum value for doubleclick MIN max speed
        if (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] < PLUGIN_009_DOUBLECLICK_MIN_INTERVAL)
          Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] = PLUGIN_009_DOUBLECLICK_MIN_INTERVAL;

        byte choiceDC = Settings.TaskDevicePluginConfig[event->TaskIndex][4];
        String buttonDC[4];
        buttonDC[0] = F("Disabled");
        buttonDC[1] = F("Active only on LOW (EVENT=3)");
        buttonDC[2] = F("Active only on HIGH (EVENT=3)");
        buttonDC[3] = F("Active on LOW & HIGH (EVENT=3)");
        int buttonDCValues[4] = {PLUGIN_009_DC_DISABLED, PLUGIN_009_DC_LOW, PLUGIN_009_DC_HIGH,PLUGIN_009_DC_BOTH};
        addFormSelector(F("Doubleclick event"), F("p009_dc"), 4, buttonDC, buttonDCValues, choiceDC);

        addFormNumericBox(F("Doubleclick max. interval (ms)"), F("p009_dcmaxinterval"), round(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1]), PLUGIN_009_DOUBLECLICK_MIN_INTERVAL, PLUGIN_009_DOUBLECLICK_MAX_INTERVAL);

        //set minimum value for longpress MIN max speed
        if (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] < PLUGIN_009_LONGPRESS_MIN_INTERVAL)
          Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] = PLUGIN_009_LONGPRESS_MIN_INTERVAL;

        byte choiceLP = Settings.TaskDevicePluginConfig[event->TaskIndex][5];
        String buttonLP[4];
        buttonLP[0] = F("Disabled");
        buttonLP[1] = F("Active only on LOW (EVENT= 10 [NORMAL] or 11 [INVERSED])");
        buttonLP[2] = F("Active only on HIGH (EVENT= 11 [NORMAL] or 10 [INVERSED])");
        buttonLP[3] = F("Active on LOW & HIGH (EVENT= 10 or 11)");

        int buttonLPValues[4] = {PLUGIN_009_LONGPRESS_DISABLED, PLUGIN_009_LONGPRESS_LOW, PLUGIN_009_LONGPRESS_HIGH,PLUGIN_009_LONGPRESS_BOTH};
        addFormSelector(F("Longpress event"), F("p009_lp"), 4, buttonLP, buttonLPValues, choiceLP);

        addFormNumericBox(F("Longpress min. interval (ms)"), F("p009_lpmininterval"), round(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2]), PLUGIN_009_LONGPRESS_MIN_INTERVAL, PLUGIN_009_LONGPRESS_MAX_INTERVAL);

        addFormCheckBox(F("Use Safe Button (slower)"), F("p009_sb"), round(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][3]));

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = isFormItemChecked(F("p009_boot"));

        //@giig1967-20181022
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0] = getFormItemInt(F("p009_debounce"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][4] = getFormItemInt(F("p009_dc"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] = getFormItemInt(F("p009_dcmaxinterval"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][5] = getFormItemInt(F("p009_lp"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] = getFormItemInt(F("p009_lpmininterval"));

        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][3] = isFormItemChecked(F("p009_sb"));

        //check if a task has been edited and remove task flag from the previous pin
        for (std::map<uint32_t,portStatusStruct>::iterator it=globalMapPortStatus.begin(); it!=globalMapPortStatus.end(); ++it) {
          if (it->second.previousTask == event->TaskIndex && getPluginFromKey(it->first)==PLUGIN_ID_009) {
            globalMapPortStatus[it->first].previousTask = -1;
            removeTaskFromPort(it->first);
            break;
          }
        }
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        // Turn on Pullup resistor
        Plugin_009_Config(Settings.TaskDevicePort[event->TaskIndex], 1);

        //apply INIT only if PIN is in range. Do not start INIT if pin not set in the device page.
        if (Settings.TaskDevicePort[event->TaskIndex] >= 0)
        {
          portStatusStruct newStatus;
          const uint32_t key = createKey(PLUGIN_ID_009,Settings.TaskDevicePort[event->TaskIndex]);
          //Read current status or create empty if it does not exist
          newStatus = globalMapPortStatus[key];

          // read and store current state to prevent switching at boot time
          // "state" could be -1, 0 or 1
          newStatus.state = Plugin_009_Read(Settings.TaskDevicePort[event->TaskIndex]);
          newStatus.output = newStatus.state;
          (newStatus.state == -1) ? newStatus.mode = PIN_MODE_OFFLINE : newStatus.mode = PIN_MODE_INPUT_PULLUP; // @giig1967g: if it is in the device list we assume it's an input pin
          newStatus.task++; // add this GPIO/port as a task

          // @giig1967g-20181022: set initial UserVar of the switch
          if (newStatus.state != -1 && Settings.TaskDevicePin1Inversed[event->TaskIndex]) {
            UserVar[event->BaseVarIndex] = !newStatus.state;
          } else {
            UserVar[event->BaseVarIndex] = newStatus.state;
          }

          // if boot state must be send, inverse default state
          // this is done to force the trigger in PLUGIN_TEN_PER_SECOND
          if (Settings.TaskDevicePluginConfig[event->TaskIndex][0])
            newStatus.state = !newStatus.state;

          // @giig1967g-20181022: doubleclick counter = 0
          Settings.TaskDevicePluginConfig[event->TaskIndex][7]=0;     //doubleclick counter
          Settings.TaskDevicePluginConfigLong[event->TaskIndex][3]=0; //safebutton counter

          // @giig1967g-20181022: used to track if LP has fired
          Settings.TaskDevicePluginConfig[event->TaskIndex][6]=false;

          // @giig1967g-20181022: store millis for debounce, doubleclick and long press
          Settings.TaskDevicePluginConfigLong[event->TaskIndex][0]=millis(); //debounce timer
          Settings.TaskDevicePluginConfigLong[event->TaskIndex][1]=millis(); //doubleclick timer
          Settings.TaskDevicePluginConfigLong[event->TaskIndex][2]=millis(); //longpress timer

          // @giig1967g-20181022: set minimum value for doubleclick MIN max speed
          if (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] < PLUGIN_009_DOUBLECLICK_MIN_INTERVAL)
            Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] = PLUGIN_009_DOUBLECLICK_MIN_INTERVAL;

          // @giig1967g-20181022: set minimum value for longpress MIN max speed
          if (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] < PLUGIN_009_LONGPRESS_MIN_INTERVAL)
            Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] = PLUGIN_009_LONGPRESS_MIN_INTERVAL;

          //setPinState(PLUGIN_ID_009, Settings.TaskDevicePort[event->TaskIndex], PIN_MODE_INPUT, switchstate[event->TaskIndex]);
          savePortStatus(key,newStatus);
        }
        success = true;
        break;
      }
/*
      case PLUGIN_UNCONDITIONAL_POLL:
        {
          // port monitoring, generates an event by rule command 'monitor,pcf,port#'
          for (std::map<uint32_t,portStatusStruct>::iterator it=globalMapPortStatus.begin(); it!=globalMapPortStatus.end(); ++it) {
            if ((it->second.monitor || it->second.command || it->second.init) && getPluginFromKey(it->first)==PLUGIN_ID_009) {
              const uint16_t port = getPortFromKey(it->first);
              int8_t state = Plugin_009_Read(port);
              if (it->second.state != state || it->second.forceMonitor) {
                if (it->second.mode == PIN_MODE_OFFLINE) it->second.mode=PIN_MODE_UNDEFINED; //changed from offline to online
                if (state == -1) it->second.mode=PIN_MODE_OFFLINE; //changed from online to offline
                if (!it->second.task) it->second.state = state; //do not update state if task flag=1 otherwise it will not be picked up by 10xSEC function
                if (it->second.monitor) {
                  it->second.forceMonitor=0; //reset flag
                  String eventString = F("MCP#");
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
*/
      case PLUGIN_MONITOR:
        {
          // port monitoring, generates an event by rule command 'monitor,gpio,port#'
          const uint32_t key = createKey(PLUGIN_ID_009,event->Par1);
          const portStatusStruct currentStatus = globalMapPortStatus[key];

          //if (currentStatus.monitor || currentStatus.command || currentStatus.init) {
            byte state = Plugin_009_Read(event->Par1);
            if (currentStatus.state != state || currentStatus.forceMonitor) {
              if (!currentStatus.task) globalMapPortStatus[key].state = state; //do not update state if task flag=1 otherwise it will not be picked up by 10xSEC function
              if (currentStatus.monitor) {
                globalMapPortStatus[key].forceMonitor=0; //reset flag
                String eventString = F("MCP#");
                eventString += event->Par1;
                eventString += '=';
                eventString += state;
                rulesProcessing(eventString);
              }
            }
          //}

          break;
        }

    case PLUGIN_TEN_PER_SECOND:
      {
        const int8_t state = Plugin_009_Read(Settings.TaskDevicePort[event->TaskIndex]);
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
        portStatusStruct currentStatus;
        const uint32_t key = createKey(PLUGIN_ID_009,Settings.TaskDevicePort[event->TaskIndex]);
        //WARNING operator [],creates an entry in map if key doesn't exist:
        currentStatus = globalMapPortStatus[key];

        //Bug fixed: avoid 10xSEC in case of a non-fully configured device (no port defined yet)
        if (state != -1 && Settings.TaskDevicePort[event->TaskIndex]>=0) {

          //CASE 1: using SafeButton, so wait 1 more 100ms cycle to acknowledge the status change
          if (round(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][3]) && state != currentStatus.state && Settings.TaskDevicePluginConfigLong[event->TaskIndex][3]==0)
          {
            addLog(LOG_LEVEL_DEBUG,F("MCP :SafeButton 1st click."))
            Settings.TaskDevicePluginConfigLong[event->TaskIndex][3] = 1;
          }
          //CASE 2: not using SafeButton, or already waited 1 more 100ms cycle, so proceed.
          else if (state != currentStatus.state || currentStatus.forceEvent)
          {
            //Reset forceEvent
            currentStatus.forceEvent = 0;

            // Reset SafeButton counter
            Settings.TaskDevicePluginConfigLong[event->TaskIndex][3] = 0;

            //@giig1967g20181022: reset timer for long press
            Settings.TaskDevicePluginConfigLong[event->TaskIndex][2]=millis();
            Settings.TaskDevicePluginConfig[event->TaskIndex][6] = false;

            const unsigned long debounceTime = timePassedSince(Settings.TaskDevicePluginConfigLong[event->TaskIndex][0]);
            if (debounceTime >= (unsigned long)lround(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0])) //de-bounce check
            {
              const unsigned long deltaDC = timePassedSince(Settings.TaskDevicePluginConfigLong[event->TaskIndex][1]);
              if ((deltaDC >= (unsigned long)lround(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1])) ||
                   Settings.TaskDevicePluginConfig[event->TaskIndex][7]==3)
              {
                //reset timer for doubleclick
                Settings.TaskDevicePluginConfig[event->TaskIndex][7]=0;
                Settings.TaskDevicePluginConfigLong[event->TaskIndex][1]=millis();
              }

//just to simplify the reading of the code
#define COUNTER Settings.TaskDevicePluginConfig[event->TaskIndex][7]
#define DC Settings.TaskDevicePluginConfig[event->TaskIndex][4]

              //check settings for doubleclick according to the settings
              if ( COUNTER!=0 || ( COUNTER==0 && (DC==3 || (DC==1 && state==0) || (DC==2 && state==1))) )
                Settings.TaskDevicePluginConfig[event->TaskIndex][7]++;
#undef DC
#undef COUNTER

              //switchstate[event->TaskIndex] = state;
              if (currentStatus.mode == PIN_MODE_OFFLINE || currentStatus.mode == PIN_MODE_UNDEFINED) currentStatus.mode = PIN_MODE_INPUT_PULLUP; //changed from offline to online
              currentStatus.state = state;

              byte output_value;
              //boolean sendState = switchstate[event->TaskIndex];
              boolean sendState = currentStatus.state;

              if (Settings.TaskDevicePin1Inversed[event->TaskIndex])
                sendState = !sendState;

              if (Settings.TaskDevicePluginConfig[event->TaskIndex][7]==3 && Settings.TaskDevicePluginConfig[event->TaskIndex][4]>0)
              {
                output_value = 3; //double click
              } else {
                output_value = sendState ? 1 : 0; //single click
              }

              UserVar[event->BaseVarIndex] = output_value;

              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                String log = F("MCP  : Port=");
                log += Settings.TaskDevicePort[event->TaskIndex];
                log += F(" State=");
                log += state;
                log += output_value==3 ? F(" Doubleclick=") : F(" Output value=");
                log += output_value;
                addLog(LOG_LEVEL_INFO, log);
              }
              event->sensorType = SENSOR_TYPE_SWITCH;
              sendData(event);

              //reset Userdata so it displays the correct state value in the web page
              UserVar[event->BaseVarIndex] = sendState ? 1 : 0;

              Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] = millis();
            }
            savePortStatus(key,currentStatus);
          }

//just to simplify the reading of the code
#define LP Settings.TaskDevicePluginConfig[event->TaskIndex][5]
#define FIRED Settings.TaskDevicePluginConfig[event->TaskIndex][6]

          //check if LP is enabled and if LP has not fired yet
          else if (!FIRED && (LP==3 ||(LP==1 && state==0)||(LP==2 && state==1) ) ) {

#undef LP
#undef FIRED

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
            Settings.TaskDevicePluginConfigLong[event->TaskIndex][3] = 0;

            const unsigned long deltaLP = timePassedSince(Settings.TaskDevicePluginConfigLong[event->TaskIndex][2]);
            if (deltaLP >= (unsigned long)lround(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2]))
            {
              byte output_value;
              Settings.TaskDevicePluginConfig[event->TaskIndex][6] = true; //fired = true

              boolean sendState = state;
              if (Settings.TaskDevicePin1Inversed[event->TaskIndex])
                sendState = !sendState;

              output_value = sendState ? 1 : 0;
              output_value = output_value + 10;

              UserVar[event->BaseVarIndex] = output_value;
              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                String log = F("MCP  : LongPress: Port=");
                log += Settings.TaskDevicePort[event->TaskIndex];
                log += F(" State=");
                log += state ? '1' : '0';
                log += F(" Output value=");
                log += output_value;
                addLog(LOG_LEVEL_INFO, log);
              }
              sendData(event);

              //reset Userdata so it displays the correct state value in the web page
              UserVar[event->BaseVarIndex] = sendState ? 1 : 0;
            }
          } else {
            if (Settings.TaskDevicePluginConfigLong[event->TaskIndex][3]==1) { //Safe Button detected. Send EVENT value = 4
              // Reset SafeButton counter
              Settings.TaskDevicePluginConfigLong[event->TaskIndex][3] = 0;

               //Create EVENT with value = 4 for SafeButton false positive detection
              const int tempUserVar = round(UserVar[event->BaseVarIndex]);
              UserVar[event->BaseVarIndex] = 4;
              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                String log = F("MCP : SafeButton: false positive detected. GPIO= ");
                log += Settings.TaskDevicePin1[event->TaskIndex];
                log += F(" State=");
                log += tempUserVar;
                addLog(LOG_LEVEL_INFO, log);
              }
              sendData(event);

              //reset Userdata so it displays the correct state value in the web page
              UserVar[event->BaseVarIndex] = tempUserVar;
            }
          }
        } else if (state != currentStatus.state && state == -1) {
          //set UserVar and switchState = -1 and send EVENT to notify user
          UserVar[event->BaseVarIndex] = state;
          currentStatus.mode = PIN_MODE_OFFLINE;
          //switchstate[event->TaskIndex] = state;
          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("MCP  : Port=");
            log += Settings.TaskDevicePort[event->TaskIndex];
            log += F(" is offline (EVENT= -1)");
            addLog(LOG_LEVEL_INFO, log);
          }
          sendData(event);
          savePortStatus(key,currentStatus);
        }
        success = true;
        break;
      }

      //giig1967g: Added EXIT function
      case PLUGIN_EXIT:
      {
        removeTaskFromPort(createKey(PLUGIN_ID_009,Settings.TaskDevicePort[event->TaskIndex]));
        break;
      }

    case PLUGIN_READ:
      {
        // We do not actually read the pin state as this is already done 10x/second
        // Instead we just send the last known state stored in Uservar
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("MCP   : Port=");
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

        // returns pin value using syntax: [plugin#mcpgpio#pinstate#xx]
        if (string.length()>=16 && string.substring(0,16).equalsIgnoreCase(F("mcpgpio,pinstate")))
        {
          int par1;
          if (validIntFromString(parseString(string, 3), par1)) {
            string = Plugin_009_Read(par1);
          }
          success = true;
        }
        break;
      }

    case PLUGIN_WRITE:
      {
        String log = "";
        String command = parseString(string, 1);

        if (command == F("mcpgpio"))
        {
          success = true;
          if (event->Par1 > 0 && event->Par1 <= 128)
          {
            portStatusStruct tempStatus;
            const uint32_t key = createKey(PLUGIN_ID_009,event->Par1);
            // WARNING: operator [] creates an entry in the map if key does not exist
            // So the next command should be part of each command:
            tempStatus = globalMapPortStatus[key];

            int8_t currentState = Plugin_009_Read(event->Par1);

            if (currentState == -1) {
              tempStatus.mode=PIN_MODE_OFFLINE;
              tempStatus.state=-1;
              log = String(F("MCP  : GPIO ")) + String(event->Par1) + String(F(" is offline (-1). Cannot set value."));
            } else if (event->Par2 == 2) { //INPUT
          	  // PCF8574 specific: only can read 0/low state, so we must send 1
          	  //setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_INPUT, 1);
              tempStatus.mode=PIN_MODE_INPUT_PULLUP;
              tempStatus.state = currentState;
          	  Plugin_009_Write(event->Par1,1);
          	  log = String(F("MCP  : GPIO INPUT ")) + String(event->Par1) + String(F(" Set to 1"));
            } else { // OUTPUT
          	  //setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, event->Par2);
          	  Plugin_009_Write(event->Par1, event->Par2);
              tempStatus.mode=PIN_MODE_OUTPUT;
              tempStatus.state=event->Par2;
          	  log = String(F("MCP  : GPIO OUTPUT ")) + String(event->Par1) + String(F(" Set to ")) + String(event->Par2);
            }
            tempStatus.command=1; //set to 1 in order to display the status in the PinStatus page
            tempStatus.forceEvent=1;
            if (tempStatus.monitor) tempStatus.forceMonitor=1; //set to 1 in order to force an EVENT in case monitor is requested

            savePortStatus(key,tempStatus);
            addLog(LOG_LEVEL_INFO, log);
            SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);
            //SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_019, event->Par1, log, 0));
          }
        }  else if (command == F("mcpgpiotoggle")) {
          success = true;
          if (event->Par1 > 0 && event->Par1 <= 128)
          {
            portStatusStruct tempStatus;
            const uint32_t key = createKey(PLUGIN_ID_009,event->Par1);
            // WARNING: operator [] creates an entry in the map if key does not exist
            // So the next command should be part of each command:
            tempStatus = globalMapPortStatus[key];
            int8_t currentState = Plugin_009_Read(event->Par1);
            bool needToSave = false;

            if (currentState == -1) {
              tempStatus.mode=PIN_MODE_OFFLINE;
              tempStatus.state=-1;
              log = String(F("MCP  : GPIO ")) + String(event->Par1) + String(F(" is offline (-1). Cannot set value."));
              needToSave = true;
            } else if (tempStatus.mode == PIN_MODE_OUTPUT || tempStatus.mode == PIN_MODE_UNDEFINED) { //toggle only output pins
              tempStatus.state = !currentState; //toggle current state value
              tempStatus.mode = PIN_MODE_OUTPUT;
              Plugin_009_Write(event->Par1, tempStatus.state);
              log = String(F("MCP  : Toggle GPIO ")) + String(event->Par1) + String(F(" Set to ")) + String(tempStatus.state);
              needToSave = true;
            }
            if (needToSave) {
              tempStatus.command=1; //set to 1 in order to display the status in the PinStatus page
              tempStatus.forceEvent=1;
              if (tempStatus.monitor) tempStatus.forceMonitor=1; //set to 1 in order to force an EVENT in case monitor is requested

              //setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, !currentState);
              savePortStatus(key,tempStatus);
              addLog(LOG_LEVEL_INFO, log);
              //SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_019, event->Par1, log, 0));
              SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);
            }
          }
        }  else if (command == F("mcppulse")) {
          success = true;
          if (event->Par1 > 0 && event->Par1 <= 128)
          {
            portStatusStruct tempStatus;
            const uint32_t key = createKey(PLUGIN_ID_009,event->Par1);
            // WARNING: operator [] creates an entry in the map if key does not exist
            // So the next command should be part of each command:
            tempStatus = globalMapPortStatus[key];

            //setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, event->Par2);
            Plugin_009_Write(event->Par1, event->Par2);
            delay(event->Par3);
            Plugin_009_Write(event->Par1, !event->Par2);
            //setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, !event->Par2);

            tempStatus.mode = PIN_MODE_OUTPUT;
            tempStatus.state = event->Par2;
            tempStatus.command=1; //set to 1 in order to display the status in the PinStatus page
            savePortStatus(key,tempStatus);

            log = String(F("MCP  : GPIO ")) + String(event->Par1) + String(F(" Pulsed for ")) + String(event->Par3) + String(F(" mS"));
            addLog(LOG_LEVEL_INFO, log);
            //SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_019, event->Par1, log, 0));
            SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);
          }
        }  else if (command == F("mcplongpulse")) {
          success = true;
          if (event->Par1 > 0 && event->Par1 <= 128)
          {
            portStatusStruct tempStatus;
            const uint32_t key = createKey(PLUGIN_ID_009,event->Par1);
            // WARNING: operator [] creates an entry in the map if key does not exist
            // So the next command should be part of each command:
            tempStatus = globalMapPortStatus[key];

            //setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, event->Par2);
            Plugin_009_Write(event->Par1, event->Par2);

            tempStatus.mode = PIN_MODE_OUTPUT;
            tempStatus.state = event->Par2;
            tempStatus.command=1; //set to 1 in order to display the status in the PinStatus page
            savePortStatus(key,tempStatus);

            setPluginTaskTimer(event->Par3 * 1000, PLUGIN_ID_009, event->TaskIndex, event->Par1, !event->Par2);
            log = String(F("MCP  : GPIO ")) + String(event->Par1) + String(F(" Pulse set for ")) + String(event->Par3) + String(F(" S"));
            addLog(LOG_LEVEL_INFO, log);
            //SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_019, event->Par1, log, 0));
            SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);
          }
        }  else if (command == F("status")) {
          if (parseString(string, 2) == F("mcp"))
          {
            success = true;
            const uint32_t key = createKey(PLUGIN_ID_009,event->Par2); //WARNING: 'status' uses Par2 instead of Par1

            if (existPortStatus(key))
              SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, dummyString, 0);
            else
           {
             int state = Plugin_009_Read(event->Par2); // report as input
             if (state != -1)
               SendStatusOnlyIfNeeded(event->Source, NO_SEARCH_PIN_STATE, key, dummyString, state);
             }
           }
        }  else if (command == F("monitor")) {
          if (parseString(string, 2) == F("mcp"))
          {
            success = true;
            const uint32_t key = createKey(PLUGIN_ID_009,event->Par2); //WARNING: 'monitor' uses Par2 instead of Par1

            addMonitorToPort(key);
            //giig1967g: Comment next line to receive an EVENT just after calling the monitor command
            globalMapPortStatus[key].state = Plugin_009_Read(event->Par2); //set initial value to avoid an event just after calling the command

            log = String(F("MCP  : GPIO ")) + String(event->Par2) + String(F(" added to monitor list."));
            addLog(LOG_LEVEL_INFO, log);
            SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, dummyString, 0);
          }
        }  else if (command == F("unmonitor")) {
          if (parseString(string, 2) == F("mcp"))
          {
            success = true;
            const uint32_t key = createKey(PLUGIN_ID_009,event->Par2); //WARNING: 'monitor' uses Par2 instead of Par1
            SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, dummyString, 0);

            removeMonitorFromPort(key);
            log = String(F("MCP  : GPIO ")) + String(event->Par2) + String(F(" removed from monitor list."));
            addLog(LOG_LEVEL_INFO, log);
          }
        }
        break;
      }

    case PLUGIN_TIMER_IN:
      {
        Plugin_009_Write(event->Par1, event->Par2);
        //setPinState(PLUGIN_ID_009, event->Par1, PIN_MODE_OUTPUT, event->Par2);
        portStatusStruct tempStatus;
        // WARNING: operator [] creates an entry in the map if key does not exist
        const uint32_t key = createKey(PLUGIN_ID_009,event->Par1);
        tempStatus = globalMapPortStatus[key];

        tempStatus.state = event->Par2;
        tempStatus.mode = PIN_MODE_OUTPUT;
        savePortStatus(key,tempStatus);
        break;
      }
  }
  return success;
}


//********************************************************************************
// MCP23017 read
//********************************************************************************
int8_t Plugin_009_Read(byte Par1)
{
  int8_t state = -1;
  byte unit = (Par1 - 1) / 16;
  byte port = Par1 - (unit * 16);
  uint8_t address = 0x20 + unit;
  byte IOBankValueReg = 0x12;
  if (port > 8)
  {
    port = port - 8;
    IOBankValueReg++;
  }
  // get the current pin status
  Wire.beginTransmission(address);
  Wire.write(IOBankValueReg); // IO data register
  Wire.endTransmission();
  Wire.requestFrom(address, (uint8_t)0x1);
  if (Wire.available())
  {
    state = ((Wire.read() & _BV(port - 1)) >> (port - 1));
  }
  return state;
}


//********************************************************************************
// MCP23017 write
//********************************************************************************
boolean Plugin_009_Write(byte Par1, byte Par2)
{
  boolean success = false;
  byte portvalue = 0;
  byte unit = (Par1 - 1) / 16;
  byte port = Par1 - (unit * 16);
  uint8_t address = 0x20 + unit;
  byte IOBankConfigReg = 0;
  byte IOBankValueReg = 0x12;
  if (port > 8)
  {
    port = port - 8;
    IOBankConfigReg++;
    IOBankValueReg++;
  }
  // turn this port into output, first read current config
  Wire.beginTransmission(address);
  Wire.write(IOBankConfigReg); // IO config register
  Wire.endTransmission();
  Wire.requestFrom(address, (uint8_t)0x1);
  if (Wire.available())
  {
    portvalue = Wire.read();
    portvalue &= ~(1 << (port - 1)); // change pin from (default) input to output

    // write new IO config
    Wire.beginTransmission(address);
    Wire.write(IOBankConfigReg); // IO config register
    Wire.write(portvalue);
    Wire.endTransmission();
  }
  // get the current pin status
  Wire.beginTransmission(address);
  Wire.write(IOBankValueReg); // IO data register
  Wire.endTransmission();
  Wire.requestFrom(address, (uint8_t)0x1);
  if (Wire.available())
  {
    portvalue = Wire.read();
    if (Par2 == 1)
      portvalue |= (1 << (port - 1));
    else
      portvalue &= ~(1 << (port - 1));

    // write back new data
    Wire.beginTransmission(address);
    Wire.write(IOBankValueReg);
    Wire.write(portvalue);
    Wire.endTransmission();
    success = true;
  }
  return(success);
}


//********************************************************************************
// MCP23017 config
//********************************************************************************
void Plugin_009_Config(byte Par1, byte Par2)
{
  // boolean success = false;
  byte portvalue = 0;
  byte unit = (Par1 - 1) / 16;
  byte port = Par1 - (unit * 16);
  uint8_t address = 0x20 + unit;
  byte IOBankConfigReg = 0xC;
  if (port > 8)
  {
    port = port - 8;
    IOBankConfigReg++;
  }
  // turn this port pullup on
  Wire.beginTransmission(address);
  Wire.write(IOBankConfigReg);
  Wire.endTransmission();
  Wire.requestFrom(address, (uint8_t)0x1);
  if (Wire.available())
  {
    portvalue = Wire.read();
    if (Par2 == 1)
      portvalue |= (1 << (port - 1));
    else
      portvalue &= ~(1 << (port - 1));

    // write new IO config
    Wire.beginTransmission(address);
    Wire.write(IOBankConfigReg); // IO config register
    Wire.write(portvalue);
    Wire.endTransmission();
  }
}
#endif // USES_P009
