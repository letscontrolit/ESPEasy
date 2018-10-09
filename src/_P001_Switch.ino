#ifdef USES_P001
//#######################################################################################################
//#################################### Plugin 001: Input Switch #########################################
//#######################################################################################################

#define PLUGIN_001
#define PLUGIN_ID_001         1
#define PLUGIN_NAME_001       "Switch input - Switch"
#define PLUGIN_VALUENAME1_001 "Switch"
#if defined(ESP8266)
  Servo servo1;
  Servo servo2;
#endif
#define GPIO_MAX 17
// Make sure the initial default is a switch (value 0)
#define PLUGIN_001_TYPE_SWITCH 0
#define PLUGIN_001_TYPE_DIMMER 3 // Due to some changes in previous versions, do not use 2.
#define PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH 0
#define PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW 1
#define PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH 2
#define PLUGIN_001_DOUBLECLICK_MIN_MAX_SPEED 500
#define PLUGIN_001_DOUBLECLICK_MAX_MAX_SPEED 2000
#define PLUGIN_001_LONGPRESS_MIN_MIN_SPEED 1000
#define PLUGIN_001_LONGPRESS_MAX_MIN_SPEED 5000


unsigned int Plugin_001_clickCounter[TASKS_MAX];
unsigned long Plugin_001_clickTimeDC[TASKS_MAX];
unsigned long Plugin_001_clickTimePrevious[TASKS_MAX];

boolean  Plugin_001_firedLP[TASKS_MAX];
unsigned long Plugin_001_clickTimeLP[TASKS_MAX];

boolean Plugin_001_read_switch_state(struct EventStruct *event) {
  return digitalRead(Settings.TaskDevicePin1[event->TaskIndex]) == HIGH;
}

boolean Plugin_001(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static boolean switchstate[TASKS_MAX];
  static boolean outputstate[TASKS_MAX];
  static int8_t PinMonitor[GPIO_MAX];
  static int8_t PinMonitorState[GPIO_MAX];

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

    case PLUGIN_WEBFORM_LOAD:
      {
        String options[2];
        options[0] = F("Switch");
        options[1] = F("Dimmer");
        int optionValues[2] = { PLUGIN_001_TYPE_SWITCH, PLUGIN_001_TYPE_DIMMER };
        const byte switchtype = P001_getSwitchType(event);
        addFormSelector(F("Switch Type"), F("plugin_001_type"), 2, options, optionValues, switchtype);

        if (switchtype == PLUGIN_001_TYPE_DIMMER)
        {
          char tmpString[128];
          sprintf_P(tmpString, PSTR("<TR><TD>Dim value:<TD><input type='text' name='plugin_001_dimvalue' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
          addHtml(tmpString);
        }

        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        String buttonOptions[3];
        buttonOptions[0] = F("Normal Switch");
        buttonOptions[1] = F("Push Button Active Low");
        buttonOptions[2] = F("Push Button Active High");
        int buttonOptionValues[3] = {PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH, PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW, PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH};
        addFormSelector(F("Switch Button Type"), F("plugin_001_button"), 3, buttonOptions, buttonOptionValues, choice);

        addFormCheckBox(F("Send Boot state"),F("plugin_001_boot"),
        		Settings.TaskDevicePluginConfig[event->TaskIndex][3]);

        addFormSubHeader(F("Advanced event management"));

        addFormNumericBox(F("De-bounce (ms)"), F("plugin_001_debounce"), Settings.TaskDevicePluginConfig[event->TaskIndex][4], 0, 250);

        //set minimum value for doubleclick MIN max speed
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][6] < PLUGIN_001_DOUBLECLICK_MIN_MAX_SPEED)
          Settings.TaskDevicePluginConfig[event->TaskIndex][6] = PLUGIN_001_DOUBLECLICK_MIN_MAX_SPEED;

        addFormCheckBox(F("Doubleclick event (3)"), F("plugin_001_dc"), Settings.TaskDevicePluginConfig[event->TaskIndex][5]);
        addFormNumericBox(F("Doubleclick max. interval (ms)"), F("plugin_001_dcmaxinterval"), Settings.TaskDevicePluginConfig[event->TaskIndex][6], PLUGIN_001_DOUBLECLICK_MIN_MAX_SPEED, PLUGIN_001_DOUBLECLICK_MAX_MAX_SPEED);

        //set minimum value for longpress MIN max speed
        if (Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] < PLUGIN_001_LONGPRESS_MIN_MIN_SPEED)
          Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] = PLUGIN_001_LONGPRESS_MIN_MIN_SPEED;

        addFormCheckBox(F("Longpress event (10 & 11)"), F("plugin_001_lp"), Settings.TaskDevicePluginConfig[event->TaskIndex][7]);
        addFormNumericBox(F("Longpress min. interval (ms)"), F("plugin_001_lpmininterval"), Settings.TaskDevicePluginConfigLong[event->TaskIndex][0], PLUGIN_001_LONGPRESS_MIN_MIN_SPEED, PLUGIN_001_LONGPRESS_MAX_MIN_SPEED);

        //TO-DO: add Extra-Long Press event
        //addFormCheckBox(F("Extra-Longpress event (20 & 21)"), F("plugin_001_elp"), Settings.TaskDevicePluginConfigLong[event->TaskIndex][1]);
        //addFormNumericBox(F("Extra-Longpress min. interval (ms)"), F("plugin_001_elpmininterval"), Settings.TaskDevicePluginConfigLong[event->TaskIndex][2], 500, 2000);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("plugin_001_type"));
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == PLUGIN_001_TYPE_DIMMER)
        {
          Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("plugin_001_dimvalue"));
        }

        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("plugin_001_button"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][3] = isFormItemChecked(F("plugin_001_boot"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][4] = getFormItemInt(F("plugin_001_debounce"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][5] = isFormItemChecked(F("plugin_001_dc"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][6] = getFormItemInt(F("plugin_001_dcmaxinterval"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][7] = isFormItemChecked(F("plugin_001_lp"));
        Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] = getFormItemInt(F("plugin_001_lpmininterval"));

        //TO-DO: add Extra-Long Press event
        //Settings.TaskDevicePluginConfigLong[event->TaskIndex][1] = isFormItemChecked(F("plugin_001_elp"));
        //Settings.TaskDevicePluginConfigLong[event->TaskIndex][2] = getFormItemInt(F("plugin_001_elpmininterval"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        for (byte x=0; x < GPIO_MAX; x++){
           PinMonitor[x] = 0;
           PinMonitorState[x] = 0;
          }

        if (Settings.TaskDevicePin1PullUp[event->TaskIndex])
          pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT_PULLUP);
        else
          pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT);

        setPinState(PLUGIN_ID_001, Settings.TaskDevicePin1[event->TaskIndex], PIN_MODE_INPUT, 0);

        switchstate[event->TaskIndex] = Plugin_001_read_switch_state(event);
        outputstate[event->TaskIndex] = switchstate[event->TaskIndex];

        // if boot state must be send, inverse default state
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][3])
        {
          switchstate[event->TaskIndex] = !switchstate[event->TaskIndex];
          outputstate[event->TaskIndex] = !outputstate[event->TaskIndex];
        }

        // counter = 0
        Plugin_001_clickCounter[event->TaskIndex]=0;
        Plugin_001_firedLP[event->TaskIndex]=false;

        //store millis for debounce, doubleclick and long press
        Plugin_001_clickTimeDC[event->TaskIndex]=millis();
        Plugin_001_clickTimePrevious[event->TaskIndex]=millis();
        Plugin_001_clickTimeLP[event->TaskIndex]=millis();

        //set minimum value for doubleclick MIN max speed
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][6] < PLUGIN_001_DOUBLECLICK_MIN_MAX_SPEED)
          Settings.TaskDevicePluginConfig[event->TaskIndex][6] = PLUGIN_001_DOUBLECLICK_MIN_MAX_SPEED;

        //set minimum value for longpress MIN max speed
        if (Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] < PLUGIN_001_LONGPRESS_MIN_MIN_SPEED)
          Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] = PLUGIN_001_LONGPRESS_MIN_MIN_SPEED;

        success = true;
        break;
      }

    case PLUGIN_REQUEST:
      {
        String device = parseString(string, 1);
        String command = parseString(string, 2);
        String strPar1 = parseString(string, 3);
        if (device == F("gpio") && command == F("pinstate"))
        {
          int par1;
          if (validIntFromString(strPar1, par1)) {
            string = digitalRead(par1);
          }
          success = true;
        }
        break;
      }

    case PLUGIN_UNCONDITIONAL_POLL:
      {
        // port monitoring, on request by rule command
        for (byte x=0; x < GPIO_MAX; x++)
           if (PinMonitor[x] != 0){
             byte state = digitalRead(x);
             if (PinMonitorState[x] != state){
               String eventString = F("GPIO#");
               eventString += x;
               eventString += F("=");
               eventString += state;
               rulesProcessing(eventString);
               PinMonitorState[x] = state;
             }
           }
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        const boolean state = Plugin_001_read_switch_state(event);

        /**************************************************************************\
        20181009 - @giig1967g: new doubleclick logic is:
        if there is a 'state' change, check debounce period.
        Then if doubleclick interval exceeded, reset Plugin_001_clickCounter to 0
        Plugin_001_clickCounter contains the current status for doubleclick:
        0: start counting
        1: 1st click
        2: 2nd click
        3: 3rd click = doubleclick event if inside interval (calculated as: '3rd click time' minus '1st click time')

        Returned EVENT value is = 3 always for doubleclick
        In rules this can be checked:
        on Button#Switch=3 do //will fire if doubleclick
        \**************************************************************************/

        if (state != switchstate[event->TaskIndex])
        {
          //reset timer for long press
          Plugin_001_clickTimeLP[event->TaskIndex]=millis();
          Plugin_001_firedLP[event->TaskIndex] = false;

          const unsigned long debounceTime = timePassedSince(Plugin_001_clickTimePrevious[event->TaskIndex]);
          if (debounceTime >= (unsigned long)Settings.TaskDevicePluginConfig[event->TaskIndex][4]) //de-bounce check
          {
            const unsigned long deltaDC = timePassedSince(Plugin_001_clickTimeDC[event->TaskIndex]);
            if (deltaDC >= (unsigned long)Settings.TaskDevicePluginConfig[event->TaskIndex][6])
            {
              //reset timer for doubleclick
              Plugin_001_clickCounter[event->TaskIndex]=0;
              Plugin_001_clickTimeDC[event->TaskIndex]=millis();
            }
            Plugin_001_clickCounter[event->TaskIndex]++;

            switchstate[event->TaskIndex] = state;
            const boolean currentOutputState = outputstate[event->TaskIndex];
            boolean new_outputState = currentOutputState;
            switch(Settings.TaskDevicePluginConfig[event->TaskIndex][2])
            {
              case PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH:
                  new_outputState = state;
                break;
              case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW:
                if (!state)
                  new_outputState = !currentOutputState;
                break;
              case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH:
                if (state)
                  new_outputState = !currentOutputState;
                break;
            }

            // send if output needs to be changed
            if (currentOutputState != new_outputState)
            {
              byte output_value;
              outputstate[event->TaskIndex] = new_outputState;
              boolean sendState = new_outputState;
              if (Settings.TaskDevicePin1Inversed[event->TaskIndex])
                sendState = !sendState;

              if (Plugin_001_clickCounter[event->TaskIndex]==3 && Settings.TaskDevicePluginConfig[event->TaskIndex][5])
              {
                output_value = 3; //double click
              } else {
                output_value = sendState ? 1 : 0; //single click
              }
              event->sensorType = SENSOR_TYPE_SWITCH;
              if (P001_getSwitchType(event) == PLUGIN_001_TYPE_DIMMER) {
                if (sendState) {
                  output_value = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
                  // Only set type to being dimmer when setting a value else it is "switched off".
                  event->sensorType = SENSOR_TYPE_DIMMER;
                }
              }
              UserVar[event->BaseVarIndex] = output_value;
              String log = F("SW   : Switch state ");
              log += state ? F("1") : F("0");
              log += F(" Output value ");
              log += output_value;
              addLog(LOG_LEVEL_INFO, log);
              sendData(event);
            }
            Plugin_001_clickTimePrevious[event->TaskIndex] = millis();
          }
        }
        //check if LP is enabled and if LP has not fired yet
        else if (Settings.TaskDevicePluginConfig[event->TaskIndex][7] && !Plugin_001_firedLP[event->TaskIndex]) {
          /**************************************************************************\
          20181009 - @giig1967g: new longpress logic is:
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

          //TO DO:
          //SE PUSH BUTTON HIGH HA SENSO SOLO 11
          //SE PUSH BUTTON LOW HA SENSO SOLO 10
          const unsigned long deltaLP = timePassedSince(Plugin_001_clickTimeLP[event->TaskIndex]);
          if (deltaLP >= (unsigned long)Settings.TaskDevicePluginConfigLong[event->TaskIndex][0])
          {
            byte output_value;
            byte needToSendEvent = false;

            Plugin_001_firedLP[event->TaskIndex] = true;

            switch(Settings.TaskDevicePluginConfig[event->TaskIndex][2])
            {
              case PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH:
                  needToSendEvent = true;
                break;
              case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW:
                if (!state)
                  needToSendEvent = true;
                break;
              case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH:
                if (state)
                  needToSendEvent = true;
                break;
            }

            if (needToSendEvent) {
              boolean sendState = state;
              if (Settings.TaskDevicePin1Inversed[event->TaskIndex])
                sendState = !sendState;
              output_value = sendState ? 1 : 0;
              output_value = output_value + 10;

              UserVar[event->BaseVarIndex] = output_value;
              String log = F("SW   : LongPress: Switch state ");
              log += state ? F("1") : F("0");
              log += F(" Output value ");
              log += output_value;
              addLog(LOG_LEVEL_INFO, log);
              sendData(event);
            }
          }
        }

        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        // We do not actually read the pin state as this is already done 10x/second
        // Instead we just send the last known state stored in Uservar
        String log = F("SW   : State ");
        log += UserVar[event->BaseVarIndex];
        addLog(LOG_LEVEL_INFO, log);
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String log = "";
        String command = parseString(string, 1);

        if (command == F("gpio"))
        {
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= PIN_D_MAX)
          {
            if (event->Par2 == 2) {
              pinMode(event->Par1, INPUT);
              setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_INPUT, 0);
            } else {
              pinMode(event->Par1, OUTPUT);
              digitalWrite(event->Par1, event->Par2);
              setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, event->Par2);
            }
            log = String(F("SW   : GPIO ")) + String(event->Par1) + String(F(" Set to ")) + String(event->Par2);
            addLog(LOG_LEVEL_INFO, log);
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par1, log, 0));
          }
        }

        if (command == F("gpiotoggle"))
        {
          success = true;
          byte mode;
          uint16_t currentState;

          getPinState(PLUGIN_ID_001, event->Par1, &mode, &currentState);
          if (mode == PIN_MODE_OUTPUT || mode == PIN_MODE_UNDEFINED) { //toggle only output pins
            digitalWrite(event->Par1, !currentState);
            setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, !currentState);
            log = String(F("SW   : Toggle GPIO ")) + String(event->Par1) + String(F(" Set to ")) + String(!currentState);
            addLog(LOG_LEVEL_INFO, log);
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par1, log, 0));
          }
        }

        if (command == F("pwm"))
        {
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= PIN_D_MAX)
          {
            #if defined(ESP8266)
              pinMode(event->Par1, OUTPUT);
            #endif
            if(event->Par3 != 0)
            {
              byte prev_mode;
              uint16_t prev_value;
              getPinState(PLUGIN_ID_001, event->Par1, &prev_mode, &prev_value);
              if(prev_mode != PIN_MODE_PWM)
                prev_value = 0;

              int32_t step_value = ((event->Par2 - prev_value) << 12) / event->Par3;
              int32_t curr_value = prev_value << 12;

              int i = event->Par3;
              while(i--){
                curr_value += step_value;
                int16_t new_value;
                new_value = (uint16_t)(curr_value >> 12);
                #if defined(ESP8266)
                  analogWrite(event->Par1, new_value);
                #endif
                #if defined(ESP32)
                  analogWriteESP32(event->Par1, new_value);
                #endif
                delay(1);
              }
            }

            #if defined(ESP8266)
              analogWrite(event->Par1, event->Par2);
            #endif
            #if defined(ESP32)
              analogWriteESP32(event->Par1, event->Par2);
            #endif
            setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_PWM, event->Par2);
            log = String(F("SW   : GPIO ")) + String(event->Par1) + String(F(" Set PWM to ")) + String(event->Par2);
            addLog(LOG_LEVEL_INFO, log);
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par1, log, 0));
          }
        }

        if (command == F("pulse"))
        {
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= PIN_D_MAX)
          {
            pinMode(event->Par1, OUTPUT);
            digitalWrite(event->Par1, event->Par2);
            delay(event->Par3);
            digitalWrite(event->Par1, !event->Par2);
            setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, event->Par2);
            log = String(F("SW   : GPIO ")) + String(event->Par1) + String(F(" Pulsed for ")) + String(event->Par3) + String(F(" mS"));
            addLog(LOG_LEVEL_INFO, log);
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par1, log, 0));
          }
        }

        if ((command == F("longpulse")) || (command == F("longpulse_ms")))
        {
          boolean time_in_msec = command == F("longpulse_ms");
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= PIN_D_MAX)
          {
            const bool pinStateHigh = event->Par2 != 0;
            const uint16_t pinStateValue = pinStateHigh ? 1 : 0;
            const uint16_t inversePinStateValue = pinStateHigh ? 0 : 1;
            pinMode(event->Par1, OUTPUT);
            digitalWrite(event->Par1, pinStateValue);
            setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, pinStateValue);
            unsigned long timer = time_in_msec ? event->Par3 : event->Par3 * 1000;
            // Create a future system timer call to set the GPIO pin back to its normal value.
            setPluginTaskTimer(timer, PLUGIN_ID_001, event->TaskIndex, event->Par1, inversePinStateValue);
            log = String(F("SW   : GPIO ")) + String(event->Par1) +
                  String(F(" Pulse set for ")) + String(event->Par3) + String(time_in_msec ? F(" msec") : F(" sec"));
            addLog(LOG_LEVEL_INFO, log);
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par1, log, 0));
          }
        }

        if (command == F("servo"))
        {
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= 2)
            switch (event->Par1)
            {
              case 1:

                //IRAM: doing servo stuff uses 740 bytes IRAM. (doesnt matter how many instances)
                #if defined(ESP8266)
                  servo1.attach(event->Par2);
                  servo1.write(event->Par3);
                #endif
                break;
              case 2:
                #if defined(ESP8266)
                  servo2.attach(event->Par2);
                  servo2.write(event->Par3);
                #endif
                break;
            }
          setPinState(PLUGIN_ID_001, event->Par2, PIN_MODE_SERVO, event->Par3);
          log = String(F("SW   : GPIO ")) + String(event->Par2) + String(F(" Servo set to ")) + String(event->Par3);
          addLog(LOG_LEVEL_INFO, log);
          SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par2, log, 0));
        }

        if (command == F("status"))
        {
          if (parseString(string, 2) == F("gpio"))
          {
            success = true;
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par2, dummyString, 0));
          }
        }

        if (command == F("monitor"))
        {
          if (parseString(string, 2) == F("gpio"))
          {
            PinMonitor[event->Par2] = 1;
            success = true;
          }
        }

        if (command == F("inputswitchstate"))
        {
          success = true;
          UserVar[event->Par1 * VARS_PER_TASK] = event->Par2;
          outputstate[event->Par1] = event->Par2;
        }

        // FIXME: Absolutely no error checking in play_rtttl, until then keep it only in testing
        //play a tune via a RTTTL string, look at https://www.letscontrolit.com/forum/viewtopic.php?f=4&t=343&hilit=speaker&start=10 for more info.
        if (command == F("rtttl"))
        {
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= 16)
          {
            pinMode(event->Par1, OUTPUT);
            // char sng[1024] ="";
            String tmpString=string;
            tmpString.replace('-', '#');
            // tmpString.toCharArray(sng, 1024);
            play_rtttl(event->Par1, tmpString.c_str());
            setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, event->Par2);
            log = String(F("SW   : ")) + string;
            addLog(LOG_LEVEL_INFO, log);
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par1, log, 0));
          }
        }

        //play a tone on pin par1, with frequency par2 and duration par3.
        if (command == F("tone"))
        {
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= PIN_D_MAX)
          {
            pinMode(event->Par1, OUTPUT);
            tone_espEasy(event->Par1, event->Par2, event->Par3);
            setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, event->Par2);
            log = String(F("SW   : ")) + string;
            addLog(LOG_LEVEL_INFO, log);
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par1, log, 0));
          }
        }

        break;
      }

    case PLUGIN_TIMER_IN:
      {
        digitalWrite(event->Par1, event->Par2);
        setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, event->Par2);
        break;
      }
  }
  return success;
}


#if defined(ESP32)
void analogWriteESP32(int pin, int value)
{
  // find existing channel if this pin has been used before
  int8_t ledChannel = -1;
  for(byte x = 0; x < 16; x++)
    if (ledChannelPin[x] == pin)
      ledChannel = x;

  if(ledChannel == -1) // no channel set for this pin
    {
      for(byte x = 0; x < 16; x++) // find free channel
        if (ledChannelPin[x] == -1)
          {
            int freq = 5000;
            ledChannelPin[x] = pin;  // store pin nr
            ledcSetup(x, freq, 10);  // setup channel
            ledcAttachPin(pin, x);   // attach to this pin
            ledChannel = x;
            break;
          }
    }
  ledcWrite(ledChannel, value);
}
#endif

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
