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
3:

TaskDevicePluginConfigLong settings:
0: clickTime debounce ms
1: clickTime doubleclick ms
2: clickTime longpress ms
3:
\**************************************************/

#define PLUGIN_019
#define PLUGIN_ID_019         19
#define PLUGIN_NAME_019       "Switch input - PCF8574"
#define PLUGIN_VALUENAME1_019 "Switch"
#define PLUGIN_019_DOUBLECLICK_MIN_INTERVAL 1000
#define PLUGIN_019_DOUBLECLICK_MAX_INTERVAL 3000
#define PLUGIN_019_LONGPRESS_MIN_INTERVAL 1000
#define PLUGIN_019_LONGPRESS_MAX_INTERVAL 5000
#define PLUGIN_019_DC_DISABLED 0
#define PLUGIN_019_DC_LOW 1
#define PLUGIN_019_DC_HIGH 2
#define PLUGIN_019_DC_BOTH 3
#define PLUGIN_019_LONGPRESS_DISABLED 0
#define PLUGIN_019_LONGPRESS_LOW 1
#define PLUGIN_019_LONGPRESS_HIGH 2
#define PLUGIN_019_LONGPRESS_BOTH 3

boolean Plugin_019(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static int8_t switchstate[TASKS_MAX];

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
        addFormCheckBox(F("Send Boot state"), F("p019_boot"), Settings.TaskDevicePluginConfig[event->TaskIndex][0]);

        //@giig1967-20181022
        addFormSubHeader(F("Advanced event management"));

        addFormNumericBox(F("De-bounce (ms)"), F("p019_debounce"), round(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0]), 0, 250);

        //set minimum value for doubleclick MIN max speed
        if (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] < PLUGIN_019_DOUBLECLICK_MIN_INTERVAL)
          Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] = PLUGIN_019_DOUBLECLICK_MIN_INTERVAL;

        byte choiceDC = Settings.TaskDevicePluginConfig[event->TaskIndex][4];
        String buttonDC[4];
        buttonDC[0] = F("Disabled");
        buttonDC[1] = F("Active only on LOW (EVENT=3)");
        buttonDC[2] = F("Active only on HIGH (EVENT=3)");
        buttonDC[3] = F("Active on LOW & HIGH (EVENT=3)");
        int buttonDCValues[4] = {PLUGIN_019_DC_DISABLED, PLUGIN_019_DC_LOW, PLUGIN_019_DC_HIGH,PLUGIN_019_DC_BOTH};
        addFormSelector(F("Doubleclick event"), F("p019_dc"), 4, buttonDC, buttonDCValues, choiceDC);

        addFormNumericBox(F("Doubleclick max. interval (ms)"), F("p019_dcmaxinterval"), round(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1]), PLUGIN_019_DOUBLECLICK_MIN_INTERVAL, PLUGIN_019_DOUBLECLICK_MAX_INTERVAL);

        //set minimum value for longpress MIN max speed
        if (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] < PLUGIN_019_LONGPRESS_MIN_INTERVAL)
          Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] = PLUGIN_019_LONGPRESS_MIN_INTERVAL;

        byte choiceLP = Settings.TaskDevicePluginConfig[event->TaskIndex][5];
        String buttonLP[4];
        buttonLP[0] = F("Disabled");
        buttonLP[1] = F("Active only on LOW (EVENT= 10 [NORMAL] or 11 [INVERSED])");
        buttonLP[2] = F("Active only on HIGH (EVENT= 11 [NORMAL] or 10 [INVERSED])");
        buttonLP[3] = F("Active on LOW & HIGH (EVENT= 10 or 11)");
        int buttonLPValues[4] = {PLUGIN_019_LONGPRESS_DISABLED, PLUGIN_019_LONGPRESS_LOW, PLUGIN_019_LONGPRESS_HIGH,PLUGIN_019_LONGPRESS_BOTH};
        addFormSelector(F("Longpress event"), F("p019_lp"), 4, buttonLP, buttonLPValues, choiceLP);

        addFormNumericBox(F("Longpress min. interval (ms)"), F("p019_lpmininterval"), round(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2]), PLUGIN_019_LONGPRESS_MIN_INTERVAL, PLUGIN_019_LONGPRESS_MAX_INTERVAL);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = isFormItemChecked(F("p019_boot"));

        //@giig1967-20181022
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0] = getFormItemInt(F("p019_debounce"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][4] = getFormItemInt(F("p019_dc"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] = getFormItemInt(F("p019_dcmaxinterval"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][5] = getFormItemInt(F("p019_lp"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] = getFormItemInt(F("p019_lpmininterval"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        // read and store current state to prevent switching at boot time
        // "state" could be -1, 0 or 1
        switchstate[event->TaskIndex] = Plugin_019_Read(Settings.TaskDevicePort[event->TaskIndex]);

        // if boot state must be send, inverse default state
        // this is done to force the trigger in PLUGIN_TEN_PER_SECOND
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0])
          switchstate[event->TaskIndex] = !switchstate[event->TaskIndex];

        // @giig1967g-20181022: set initial UserVar of the switch
        if (switchstate[event->TaskIndex] != -1 && Settings.TaskDevicePin1Inversed[event->TaskIndex]) {
          UserVar[event->BaseVarIndex] = !switchstate[event->TaskIndex];
        } else {
          UserVar[event->BaseVarIndex] = switchstate[event->TaskIndex];
        }

        // @giig1967g-20181022: doubleclick counter = 0
        Settings.TaskDevicePluginConfig[event->TaskIndex][7]=0;

        // @giig1967g-20181022: used to track if LP has fired
        Settings.TaskDevicePluginConfig[event->TaskIndex][6]=false;

        // @giig1967g-20181022: store millis for debounce, doubleclick and long press
        Settings.TaskDevicePluginConfigLong[event->TaskIndex][0]=millis(); //debounce timer
        Settings.TaskDevicePluginConfigLong[event->TaskIndex][1]=millis(); //doubleclick timer
        Settings.TaskDevicePluginConfigLong[event->TaskIndex][2]=millis(); //longpress timer

        // @giig1967g-20181022: set minimum value for doubleclick MIN max speed
        if (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] < PLUGIN_019_DOUBLECLICK_MIN_INTERVAL)
          Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] = PLUGIN_019_DOUBLECLICK_MIN_INTERVAL;

        // @giig1967g-20181022: set minimum value for longpress MIN max speed
        if (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] < PLUGIN_019_LONGPRESS_MIN_INTERVAL)
          Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] = PLUGIN_019_LONGPRESS_MIN_INTERVAL;

        // @giig1967g-20181022: if it is in the device list we assume it's an input pin
        setPinState(PLUGIN_ID_019, Settings.TaskDevicePort[event->TaskIndex], PIN_MODE_INPUT, switchstate[event->TaskIndex]);

        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:

      {
        const int8_t state = Plugin_019_Read(Settings.TaskDevicePort[event->TaskIndex]);
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
        if (state != -1) {
          if (state != switchstate[event->TaskIndex]) {
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

              switchstate[event->TaskIndex] = state;

              byte output_value;
              boolean sendState = switchstate[event->TaskIndex];

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
                String log = F("PCF  : Port=");
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
                String log = F("PCF  : LongPress: Port= ");
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
          }
        } else if (state != switchstate[event->TaskIndex]) {
          //set UserVar and switchState = -1 and send EVENT to notify user
          UserVar[event->BaseVarIndex] = state;
          switchstate[event->TaskIndex] = state;
          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("PCF  : Port=");
            log += Settings.TaskDevicePort[event->TaskIndex];
            log += F(" is offline (EVENT= -1)");
            addLog(LOG_LEVEL_INFO, log);
          }
          sendData(event);
        }
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        // We do not actually read the pin state as this is already done 10x/second
        // Instead we just send the last known state stored in Uservar
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("PCF  : State ");
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
          if (event->Par2 == 2) { //INPUT
        	  // PCF8574 specific: only can read 0/low state, so we must send 1
        	  setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_INPUT, 1);
        	  Plugin_019_Write(event->Par1,1);
        	  log = String(F("PCF  : GPIO ")) + String(event->Par1) + String(F(" Set to 1"));
          }
          else { // OUTPUT
        	  setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, event->Par2);
        	  Plugin_019_Write(event->Par1, event->Par2);
        	  log = String(F("PCF  : GPIO ")) + String(event->Par1) + String(F(" Set to ")) + String(event->Par2);
          }
          addLog(LOG_LEVEL_INFO, log);
          SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_019, event->Par1, log, 0));
        }

        if (command == F("pcfgpiotoggle"))
        {
          success = true;
          byte mode;
          uint16_t currentState;

          if (hasPinState(PLUGIN_ID_019,event->Par1)) {
            getPinState(PLUGIN_ID_019, event->Par1, &mode, &currentState);
          } else {
            currentState = Plugin_019_Read(event->Par1);
            mode = PIN_MODE_OUTPUT;
          }

          if (mode != PIN_MODE_INPUT) {
            setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, !currentState);
            Plugin_019_Write(event->Par1, !currentState);
            log = String(F("PCF  : Toggle GPIO ")) + String(event->Par1) + String(F(" Set to ")) + String(!currentState);
            addLog(LOG_LEVEL_INFO, log);
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_019, event->Par1, log, 0));
          }
        }

        if (command == F("pcfpulse"))
        {
          success = true;
          setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, event->Par2);
          Plugin_019_Write(event->Par1, event->Par2);
          delay(event->Par3);
          Plugin_019_Write(event->Par1, !event->Par2);
          setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, !event->Par2);
          log = String(F("PCF  : GPIO ")) + String(event->Par1) + String(F(" Pulsed for ")) + String(event->Par3) + String(F(" mS"));
          addLog(LOG_LEVEL_INFO, log);
          SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_019, event->Par1, log, 0));
        }

        if (command == F("pcflongpulse"))
        {
          success = true;
          setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, event->Par2);
          Plugin_019_Write(event->Par1, event->Par2);
          setPluginTaskTimer(event->Par3 * 1000, PLUGIN_ID_019, event->TaskIndex, event->Par1, !event->Par2);
          log = String(F("PCF  : GPIO ")) + String(event->Par1) + String(F(" Pulse set for ")) + String(event->Par3) + String(F(" S"));
          addLog(LOG_LEVEL_INFO, log);
          SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_019, event->Par1, log, 0));
        }

        if (command == F("status"))
        {
          if (parseString(string, 2) == F("pcf"))
          {
            success = true;
            String status = "";
            if (hasPinState(PLUGIN_ID_019, event->Par2))  // has been set as output
              status = getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_019, event->Par2, dummyString, 0);
            else
            {
              int state = Plugin_019_Read(event->Par2); // report as input
              if (state != -1)
                status = getPinStateJSON(NO_SEARCH_PIN_STATE, PLUGIN_ID_019, event->Par2, dummyString, state);
            }
            SendStatus(event->Source, status);
          }
        }

        break;
      }

    case PLUGIN_TIMER_IN:
      {
        Plugin_019_Write(event->Par1, event->Par2);
        setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, event->Par2);
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


//********************************************************************************
// PCF8574 write
//********************************************************************************
boolean Plugin_019_Write(byte Par1, byte Par2)
{
  byte unit = (Par1 - 1) / 8;
  byte port = Par1 - (unit * 8);
  uint8_t address = 0x20 + unit;
  if (unit > 7) address += 0x10;

  //generate bitmask
  int i = 0;
  byte portmask = 0;
  byte mode = 0;
  uint16_t value = 0;
  unit *= 8; // calculate first pin
  unit += 1;
  for(i =0;i<8;i++){
	  mode =0;
	  if(!getPinState(PLUGIN_ID_019, unit, &mode, &value) || mode == PIN_MODE_INPUT || (mode == PIN_MODE_OUTPUT && value == 1))
		  portmask |= (1 << i);
	  unit++;
  }

  if (Par2 == 1)
    portmask |= (1 << (port - 1));
  else
    portmask &= ~(1 << (port - 1));

  Wire.beginTransmission(address);
  Wire.write(portmask);
  Wire.endTransmission();

  return true;
}
#endif // USES_P019
