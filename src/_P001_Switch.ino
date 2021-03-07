#include "_Plugin_Helper.h"

#ifdef USES_P001

#include "src/DataStructs/PinMode.h"
#include "src/Commands/GPIO.h"
#include "src/ESPEasyCore/Controller.h"
#include "src/ESPEasyCore/ESPEasyGPIO.h"
#include "src/Helpers/Audio.h"
#include "src/Helpers/PortStatus.h"
#include "src/Helpers/Scheduler.h"

// #######################################################################################################
// #################################### Plugin 001: Input Switch #########################################
// #######################################################################################################

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
#define PLUGIN_VALUENAME1_001 "State"

// Make sure the initial default is a switch (value 0)
#define PLUGIN_001_TYPE_SWITCH                   0
#define PLUGIN_001_TYPE_DIMMER                   3 // Due to some changes in previous versions, do not use 2.
#define PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH     0
#define PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW   1
#define PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH  2
#define PLUGIN_001_DOUBLECLICK_MIN_INTERVAL      1000
#define PLUGIN_001_DOUBLECLICK_MAX_INTERVAL      3000
#define PLUGIN_001_LONGPRESS_MIN_INTERVAL        500
#define PLUGIN_001_LONGPRESS_MAX_INTERVAL        5000
#define PLUGIN_001_DC_DISABLED                   0
#define PLUGIN_001_DC_LOW                        1
#define PLUGIN_001_DC_HIGH                       2
#define PLUGIN_001_DC_BOTH                       3
#define PLUGIN_001_LONGPRESS_DISABLED            0
#define PLUGIN_001_LONGPRESS_LOW                 1
#define PLUGIN_001_LONGPRESS_HIGH                2
#define PLUGIN_001_LONGPRESS_BOTH                3


boolean Plugin_001(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  // static byte switchstate[TASKS_MAX];
  // static byte outputstate[TASKS_MAX];
  // static int8_t PinMonitor[GPIO_MAX];
  // static int8_t PinMonitorState[GPIO_MAX];

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_001;
      Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SWITCH;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = true;
      Device[deviceCount].InverseLogicOption = true;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].TimerOptional      = true;
      Device[deviceCount].GlobalSyncOption   = true;
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
      // FIXME TD-er: Split functionality of this plugin into 2 new ones:
      // - switch/dimmer input
      // - switch output (relays)
      event->String1 = formatGpioName_bidirectional("");
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      // @giig1967g: set current task value for taking actions after changes in the task gpio
      const uint32_t key = createKey(PLUGIN_ID_001, CONFIG_PIN1);

      auto it = globalMapPortStatus.find(key);
      if (it != globalMapPortStatus.end()) {
        it->second.previousTask = event->TaskIndex;
      }

      {
        String options[2];
        options[0] = F("Switch");
        options[1] = F("Dimmer");
        int optionValues[2]   = { PLUGIN_001_TYPE_SWITCH, PLUGIN_001_TYPE_DIMMER };
        const byte switchtype = P001_getSwitchType(event);
        addFormSelector(F("Switch Type"), F("p001_type"), 2, options, optionValues, switchtype);

        if (switchtype == PLUGIN_001_TYPE_DIMMER)
        {
          addFormNumericBox(F("Dim value"), F("p001_dimvalue"), PCONFIG(1), 0, 255);
        }
      }

      {
        byte   choice = PCONFIG(2);
        String buttonOptions[3];
        buttonOptions[0] = F("Normal Switch");
        buttonOptions[1] = F("Push Button Active Low");
        buttonOptions[2] = F("Push Button Active High");
        int buttonOptionValues[3] =
        { PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH, PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW, PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH };
        addFormSelector(F("Switch Button Type"), F("p001_button"), 3, buttonOptions, buttonOptionValues, choice);
      }

      addFormCheckBox(F("Send Boot state"), F("p001_boot"),
                      PCONFIG(3));

      addFormSubHeader(F("Advanced event management"));

      addFormNumericBox(F("De-bounce (ms)"), F("p001_debounce"), round(PCONFIG_FLOAT(0)), 0, 250);

      // set minimum value for doubleclick MIN max speed
      if (PCONFIG_FLOAT(1) < PLUGIN_001_DOUBLECLICK_MIN_INTERVAL) {
        PCONFIG_FLOAT(1) = PLUGIN_001_DOUBLECLICK_MIN_INTERVAL;
      }

      {
        byte   choiceDC = PCONFIG(4);
        String buttonDC[4];
        buttonDC[0] = F("Disabled");
        buttonDC[1] = F("Active only on LOW (EVENT=3)");
        buttonDC[2] = F("Active only on HIGH (EVENT=3)");
        buttonDC[3] = F("Active on LOW & HIGH (EVENT=3)");
        int buttonDCValues[4] = { PLUGIN_001_DC_DISABLED, PLUGIN_001_DC_LOW, PLUGIN_001_DC_HIGH, PLUGIN_001_DC_BOTH };

        addFormSelector(F("Doubleclick event"), F("p001_dc"), 4, buttonDC, buttonDCValues, choiceDC);
      }

      addFormNumericBox(F("Doubleclick max. interval (ms)"),
                        F("p001_dcmaxinterval"),
                        round(PCONFIG_FLOAT(1)),
                        PLUGIN_001_DOUBLECLICK_MIN_INTERVAL,
                        PLUGIN_001_DOUBLECLICK_MAX_INTERVAL);

      // set minimum value for longpress MIN max speed
      if (PCONFIG_FLOAT(2) < PLUGIN_001_LONGPRESS_MIN_INTERVAL) {
        PCONFIG_FLOAT(2) = PLUGIN_001_LONGPRESS_MIN_INTERVAL;
      }

      {
        byte   choiceLP = PCONFIG(5);
        String buttonLP[4];
        buttonLP[0] = F("Disabled");
        buttonLP[1] = F("Active only on LOW (EVENT= 10 [NORMAL] or 11 [INVERSED])");
        buttonLP[2] = F("Active only on HIGH (EVENT= 11 [NORMAL] or 10 [INVERSED])");
        buttonLP[3] = F("Active on LOW & HIGH (EVENT= 10 or 11)");
        int buttonLPValues[4] =
        { PLUGIN_001_LONGPRESS_DISABLED, PLUGIN_001_LONGPRESS_LOW, PLUGIN_001_LONGPRESS_HIGH, PLUGIN_001_LONGPRESS_BOTH };
        addFormSelector(F("Longpress event"), F("p001_lp"), 4, buttonLP, buttonLPValues, choiceLP);
      }

      addFormNumericBox(F("Longpress min. interval (ms)"),
                        F("p001_lpmininterval"),
                        round(PCONFIG_FLOAT(2)),
                        PLUGIN_001_LONGPRESS_MIN_INTERVAL,
                        PLUGIN_001_LONGPRESS_MAX_INTERVAL);

      addFormCheckBox(F("Use Safe Button (slower)"), F("p001_sb"), round(PCONFIG_FLOAT(3)));

      // TO-DO: add Extra-Long Press event
      // addFormCheckBox(F("Extra-Longpress event (20 & 21)"), F("p001_elp"), PCONFIG_LONG(1));
      // addFormNumericBox(F("Extra-Longpress min. interval (ms)"), F("p001_elpmininterval"), PCONFIG_LONG(2), 500, 2000);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("p001_type"));

      if (PCONFIG(0) == PLUGIN_001_TYPE_DIMMER)
      {
        PCONFIG(1) = getFormItemInt(F("p001_dimvalue"));
      }

      PCONFIG(2) = getFormItemInt(F("p001_button"));

      PCONFIG(3) = isFormItemChecked(F("p001_boot"));

      PCONFIG_FLOAT(0) = getFormItemInt(F("p001_debounce"));

      PCONFIG(4)       = getFormItemInt(F("p001_dc"));
      PCONFIG_FLOAT(1) = getFormItemInt(F("p001_dcmaxinterval"));

      PCONFIG(5)       = getFormItemInt(F("p001_lp"));
      PCONFIG_FLOAT(2) = getFormItemInt(F("p001_lpmininterval"));

      PCONFIG_FLOAT(3) = isFormItemChecked(F("p001_sb"));

      // TO-DO: add Extra-Long Press event
      // PCONFIG_LONG(1) = isFormItemChecked(F("p001_elp"));
      // PCONFIG_LONG(2) = getFormItemInt(F("p001_elpmininterval"));

      // check if a task has been edited and remove 'task' bit from the previous pin
      for (std::map<uint32_t, portStatusStruct>::iterator it = globalMapPortStatus.begin(); it != globalMapPortStatus.end(); ++it) {
        if ((it->second.previousTask == event->TaskIndex) && (getPluginFromKey(it->first) == PLUGIN_ID_001)) {
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
      // apply INIT only if PORT is in range. Do not start INIT if port not set in the device page.
      if ((CONFIG_PIN1 >= 0) && (CONFIG_PIN1 <= PIN_D_MAX))
      {
        portStatusStruct newStatus;
        const uint32_t   key = createKey(PLUGIN_ID_001, CONFIG_PIN1);

        // Read current status or create empty if it does not exist
        newStatus = globalMapPortStatus[key];

        // read and store current state to prevent switching at boot time
        newStatus.state                                          = GPIO_Read_Switch_State(event);
        newStatus.output                                         = newStatus.state;
        (newStatus.task < 3) ? newStatus.task++ : newStatus.task = 3; // add this GPIO/port as a task

        // setPinState(PLUGIN_ID_001, CONFIG_PIN1, PIN_MODE_INPUT, switchstate[event->TaskIndex]);
        //  if it is in the device list we assume it's an input pin
        if (Settings.TaskDevicePin1PullUp[event->TaskIndex]) {
          setInternalGPIOPullupMode(CONFIG_PIN1);
          newStatus.mode = PIN_MODE_INPUT_PULLUP;
        } else {
          pinMode(CONFIG_PIN1, INPUT);
          newStatus.mode = PIN_MODE_INPUT;
        }

        // if boot state must be send, inverse default state
        // this is done to force the trigger in PLUGIN_TEN_PER_SECOND
        if (PCONFIG(3))
        {
          newStatus.state  = !newStatus.state;
          newStatus.output = !newStatus.output;
        }
        savePortStatus(key, newStatus);

        // set initial UserVar of the switch
        if (Settings.TaskDevicePin1Inversed[event->TaskIndex]) {
          UserVar[event->BaseVarIndex] = !newStatus.state;
        } else {
          UserVar[event->BaseVarIndex] = newStatus.state;
        }

        // counters = 0
        PCONFIG(7)      = 0; // doubleclick counter
        PCONFIG_LONG(3) = 0; // safebutton counter

        // used to track if LP has fired
        PCONFIG(6) = false;

        // store millis for debounce, doubleclick and long press
        PCONFIG_LONG(0) = millis(); // debounce timer
        PCONFIG_LONG(1) = millis(); // doubleclick timer
        PCONFIG_LONG(2) = millis(); // longpress timer

        // set minimum value for doubleclick MIN interval speed
        if (PCONFIG_FLOAT(1) < PLUGIN_001_DOUBLECLICK_MIN_INTERVAL) {
          PCONFIG_FLOAT(1) = PLUGIN_001_DOUBLECLICK_MIN_INTERVAL;
        }

        // set minimum value for longpress MIN interval speed
        if (PCONFIG_FLOAT(2) < PLUGIN_001_LONGPRESS_MIN_INTERVAL) {
          PCONFIG_FLOAT(2) = PLUGIN_001_LONGPRESS_MIN_INTERVAL;
        }
      }
      success = true;
      break;
    }
/*
    case PLUGIN_REQUEST:
    {
      // String device = parseString(string, 1);
      // String command = parseString(string, 2);
      // String strPar1 = parseString(string, 3);

      // returns pin value using syntax: [plugin#gpio#pinstate#xx]
      if ((string.length() >= 13) && string.substring(0, 13).equalsIgnoreCase(F("gpio,pinstate")))
      {
        int par1;

        if (validIntFromString(parseString(string, 3), par1)) {
          string = digitalRead(par1);
        }
        success = true;
      }
      break;
    }
*/
    /*
          case PLUGIN_UNCONDITIONAL_POLL:
            {
              // port monitoring, generates an event by rule command 'monitor,gpio,port#'
              for (std::map<uint32_t,portStatusStruct>::iterator it=globalMapPortStatus.begin(); it!=globalMapPortStatus.end(); ++it) {
                if ((it->second.monitor || it->second.command || it->second.init) && getPluginFromKey(it->first)==PLUGIN_ID_001) {
                  const uint16_t port = getPortFromKey(it->first);
                  byte state = Plugin_001_read_switch_state(port, it->second.mode);
                  if (it->second.state != state || it->second.forceMonitor) {
                    if (!it->second.task) it->second.state = state; //do not update state if task flag=1 otherwise it will not be picked up
                       by 10xSEC function
                    if (it->second.monitor) {
                      it->second.forceMonitor=0; //reset flag
                      String eventString = F("GPIO#");
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
/*
    case PLUGIN_MONITOR:
    {
      // port monitoring, generates an event by rule command 'monitor,gpio,port#'
      const uint32_t key                   = createKey(PLUGIN_ID_001, event->Par1);
      const portStatusStruct currentStatus = globalMapPortStatus[key];

      // if (currentStatus.monitor || currentStatus.command || currentStatus.init) {
      byte state = GPIO_Read_Switch_State(event->Par1, currentStatus.mode);

      if ((currentStatus.state != state) || (currentStatus.forceMonitor && currentStatus.monitor)) {
        if (!currentStatus.task) globalMapPortStatus[key].state = state; //do not update state if task flag=1 otherwise it will not be picked up by 10xSEC function
        if (currentStatus.monitor) {
          String eventString = F("GPIO#");
          eventString += event->Par1;
          eventString += '=';
          eventString += state;
          rulesProcessing(eventString);
        }
      }
      globalMapPortStatus[key].forceMonitor = 0; // reset flag

      // }

      break;
    }
*/
    case PLUGIN_TEN_PER_SECOND:
    {
      /**************************************************************************\
         20181009 - @giig1967g: new doubleclick logic is:
         if there is a 'state' change, check debounce period.
         Then if doubleclick interval exceeded, reset PCONFIG(7) to 0
         PCONFIG(7) contains the current status for doubleclick:
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
      const String monitorEventString = F("GPIO");

      if ((CONFIG_PIN1 >= 0) && (CONFIG_PIN1 <= PIN_D_MAX))
      {
        const uint32_t   key = createKey(PLUGIN_ID_001, CONFIG_PIN1);
        // WARNING operator [],creates an entry in map if key doesn't exist:
        portStatusStruct currentStatus = globalMapPortStatus[key];

        const int8_t state = GPIO_Read_Switch_State(CONFIG_PIN1,currentStatus.mode);

//        if (currentStatus.mode != PIN_MODE_OUTPUT )
//        {
          // CASE 1: using SafeButton, so wait 1 more 100ms cycle to acknowledge the status change
          // QUESTION: MAYBE IT'S BETTER TO WAIT 2 CYCLES??
          if (round(PCONFIG_FLOAT(3)) && (state != currentStatus.state) && (PCONFIG_LONG(3) == 0))
          {
  #ifndef BUILD_NO_DEBUG
            addLog(LOG_LEVEL_DEBUG, F("SW  : 1st click"));
  #endif // ifndef BUILD_NO_DEBUG
            PCONFIG_LONG(3) = 1;
          }

          // CASE 2: not using SafeButton, or already waited 1 more 100ms cycle, so proceed.
          else if ((state != currentStatus.state) || currentStatus.forceEvent)
          {
            // Reset SafeButton counter
            PCONFIG_LONG(3) = 0;

            // reset timer for long press
            PCONFIG_LONG(2) = millis();
            PCONFIG(6)      = false;

            const unsigned long debounceTime = timePassedSince(PCONFIG_LONG(0));

            if (debounceTime >= (unsigned long)lround(PCONFIG_FLOAT(0))) // de-bounce check
            {
              const unsigned long deltaDC = timePassedSince(PCONFIG_LONG(1));

              if ((deltaDC >= (unsigned long)lround(PCONFIG_FLOAT(1))) ||
                  (PCONFIG(7) == 3))
              {
                // reset timer for doubleclick
                PCONFIG(7) = 0;
                PCONFIG_LONG(1) = millis();
              }

              // just to simplify the reading of the code
    #define COUNTER PCONFIG(7)
    #define DC PCONFIG(4)

              // check settings for doubleclick according to the settings
              if ((COUNTER != 0) || ((COUNTER == 0) && ((DC == 3) || ((DC == 1) && (state == 0)) || ((DC == 2) && (state == 1))))) {
                PCONFIG(7)++;
              }
    #undef DC
    #undef COUNTER

              currentStatus.state = state;
              const boolean currentOutputState = currentStatus.output;
              boolean new_outputState          = currentOutputState;

              switch (PCONFIG(2))
              {
                case PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH:
                  new_outputState = state;
                  break;
                case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW:

                  if (!state) {
                    new_outputState = !currentOutputState;
                  }
                  break;
                case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH:

                  if (state) {
                    new_outputState = !currentOutputState;
                  }
                  break;
              }

              // send if output needs to be changed
              if (currentOutputState != new_outputState || currentStatus.forceEvent)
              {
                byte output_value;
                currentStatus.output = new_outputState;
                boolean sendState = new_outputState;

                if (Settings.TaskDevicePin1Inversed[event->TaskIndex]) {
                  sendState = !sendState;
                }

                if ((PCONFIG(7) == 3) && (PCONFIG(4) > 0))
                {
                  output_value = 3;                 // double click
                } else {
                  output_value = sendState ? 1 : 0; // single click
                }
                event->sensorType = Sensor_VType::SENSOR_TYPE_SWITCH;

                if (P001_getSwitchType(event) == PLUGIN_001_TYPE_DIMMER) {
                  if (sendState) {
                    output_value = PCONFIG(1);

                    // Only set type to being dimmer when setting a value else it is "switched off".
                    event->sensorType = Sensor_VType::SENSOR_TYPE_DIMMER;
                  }
                }
                UserVar[event->BaseVarIndex] = output_value;
                
                #ifndef BUILD_NO_DEBUG
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  String log = F("SW  : GPIO=");
                  log += CONFIG_PIN1;
                  log += F(" State=");
                  log += state ? '1' : '0';
                  log += output_value == 3 ? F(" Doubleclick=") : F(" Output value=");
                  log += output_value;
                  addLog(LOG_LEVEL_INFO, log);
                }
                #endif
                // send task event
                sendData(event);
                // send monitor event
                if (currentStatus.monitor) sendMonitorEvent(monitorEventString.c_str(), CONFIG_PIN1, output_value);

                // reset Userdata so it displays the correct state value in the web page
                UserVar[event->BaseVarIndex] = sendState ? 1 : 0;
              }
              PCONFIG_LONG(0) = millis();
            }
            // Reset forceEvent
            currentStatus.forceEvent = 0;

            savePortStatus(key, currentStatus);
          }

          // just to simplify the reading of the code
    #define LP PCONFIG(5)
    #define FIRED PCONFIG(6)

          // CASE 3: status unchanged. Checking longpress:
          // Check if LP is enabled and if LP has not fired yet
          else if (!FIRED && ((LP == 3) || ((LP == 1) && (state == 0)) || ((LP == 2) && (state == 1)))) {
    #undef LP
    #undef FIRED

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
            PCONFIG_LONG(3) = 0;

            const unsigned long deltaLP = timePassedSince(PCONFIG_LONG(2));

            if (deltaLP >= (unsigned long)lround(PCONFIG_FLOAT(2)))
            {
              byte output_value;
              byte needToSendEvent = false;

              PCONFIG(6) = true;

              switch (PCONFIG(2))
              {
                case PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH:
                  needToSendEvent = true;
                  break;
                case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW:

                  if (!state) {
                    needToSendEvent = true;
                  }
                  break;
                case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH:

                  if (state) {
                    needToSendEvent = true;
                  }
                  break;
              }

              if (needToSendEvent) {
                boolean sendState = state;

                if (Settings.TaskDevicePin1Inversed[event->TaskIndex]) {
                  sendState = !sendState;
                }
                output_value = sendState ? 11 : 10;

                // output_value = output_value + 10;

                UserVar[event->BaseVarIndex] = output_value;

                #ifndef BUILD_NO_DEBUG
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  String log = F("SW  : LongPress: GPIO= ");
                  log += CONFIG_PIN1;
                  log += F(" State=");
                  log += state ? '1' : '0';
                  log += F(" Output value=");
                  log += output_value;
                  addLog(LOG_LEVEL_INFO, log);
                }
                #endif
                // send task event
                sendData(event);
                // send monitor event
                if (currentStatus.monitor) sendMonitorEvent(monitorEventString.c_str(), CONFIG_PIN1, output_value);

                // reset Userdata so it displays the correct state value in the web page
                UserVar[event->BaseVarIndex] = sendState ? 1 : 0;
              }
              savePortStatus(key, currentStatus);
            }
          } else {
            if (PCONFIG_LONG(3) == 1) { // Safe Button detected. Send EVENT value = 4
              const byte SAFE_BUTTON_EVENT = 4;
              // Reset SafeButton counter
              PCONFIG_LONG(3) = 0;

              // Create EVENT with value = 4 for SafeButton false positive detection
              const int tempUserVar = round(UserVar[event->BaseVarIndex]);
              UserVar[event->BaseVarIndex] = SAFE_BUTTON_EVENT;

              #ifndef BUILD_NO_DEBUG
              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                String log = F("SW  : SafeButton: false positive detected. GPIO= ");
                log += CONFIG_PIN1;
                log += F(" State=");
                log += tempUserVar;
                addLog(LOG_LEVEL_INFO, log);
              }
              #endif
              // send task event
              sendData(event);
              // send monitor event
              if (currentStatus.monitor) sendMonitorEvent(monitorEventString.c_str(), CONFIG_PIN1, SAFE_BUTTON_EVENT);

              // reset Userdata so it displays the correct state value in the web page
              UserVar[event->BaseVarIndex] = tempUserVar;
            }
          }

          //OUTPUT PIN
/*
        }

        else if ((state != currentStatus.state) || currentStatus.forceEvent) {
          // Reset forceEvent
          currentStatus.forceEvent = 0;
          currentStatus.state = state;
          UserVar[event->BaseVarIndex] = state;

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
      success = true;
      break;
    }

    case PLUGIN_EXIT:
    {
      removeTaskFromPort(createKey(PLUGIN_ID_001, CONFIG_PIN1));
      break;
    }

    case PLUGIN_READ:
    {
      // We do not actually read the pin state as this is already done 10x/second
      // Instead we just send the last known state stored in Uservar
      #ifndef BUILD_NO_DEBUG
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("SW   : State ");
        log += UserVar[event->BaseVarIndex];
        addLog(LOG_LEVEL_INFO, log);
      }
      #endif
      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      String log;
      String command = parseString(string, 1);

      // WARNING: don't read "globalMapPortStatus[key]" here, as it will create a new entry if key does not exist


      if (command == F("inputswitchstate")) {
        success = true;
        //@giig1967g deprecated since 2019-11-26
        log = String(F("inputswitchstate is deprecated")) + string;
        addLog(LOG_LEVEL_ERROR, log);

/*        portStatusStruct tempStatus;
        const uint32_t key = createKey(PLUGIN_ID_001, Settings.TaskDevicePin1[event->Par1]);

        // WARNING: operator [] creates an entry in the map if key does not exist
        // So the next command should be part of each command:
        tempStatus = globalMapPortStatus[key];

        UserVar[event->Par1 * VARS_PER_TASK] = event->Par2;
        tempStatus.output                    = event->Par2;
        tempStatus.command                   = 1;
        savePortStatus(key, tempStatus); */
      }
      break;
    }

    case PLUGIN_TIMER_IN:
    {
      digitalWrite(event->Par1, event->Par2);

      // setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, event->Par2);
      portStatusStruct tempStatus;

      // WARNING: operator [] creates an entry in the map if key does not exist
      const uint32_t key = createKey(PLUGIN_ID_001, event->Par1);
      tempStatus = globalMapPortStatus[key];

      tempStatus.state = event->Par2;
      tempStatus.mode  = PIN_MODE_OUTPUT;
//sp    (tempStatus.monitor) ? tempStatus.forceMonitor = 1 : tempStatus.forceMonitor = 0;
      tempStatus.forceMonitor = 1; //added to send event for longpulse command
      savePortStatus(key, tempStatus);
      break;
    }
  }
  return success;
}

// TD-er: Needed to fix a mistake in earlier fixes.
byte P001_getSwitchType(struct EventStruct *event) {
  byte choice = PCONFIG(0);

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
