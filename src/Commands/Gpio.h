#ifndef COMMAND_GPIO_H
#define COMMAND_GPIO_H

#if defined(ESP8266)
Servo servo1;
Servo servo2;
#endif /* if defined(ESP8266) */

#define PLUGIN_ID_000    0

// Forward declarations
uint32_t createInternalGpioKey(uint16_t portNumber);
bool     read_GPIO_state(struct EventStruct *event);
bool     read_GPIO_state(byte pinNumber,
                         byte pinMode);
void     analogWriteESP(int pin,
                        int value);
String   Command_longPulse(struct EventStruct *event,
                           const char         *Line,
                           bool                time_in_msec);


String Command_GPIO(struct EventStruct *event, const char *Line)
{
  if ((event->Par1 >= 0) && (event->Par1 <= PIN_D_MAX))
  {
    portStatusStruct tempStatus;
    const uint32_t   key = createInternalGpioKey(event->Par1);

    // WARNING: operator [] creates an entry in the map if key does not exist
    // So the next command should be part of each command:
    tempStatus = globalMapPortStatus[key];

    if (event->Par2 == 2) // input PIN
    {
      // setPinState(PLUGIN_ID_000, event->Par1, PIN_MODE_INPUT, 0);
      pinMode(event->Par1, INPUT_PULLUP);
      tempStatus.mode   = PIN_MODE_INPUT_PULLUP;
      tempStatus.state  = read_GPIO_state(event->Par1, tempStatus.mode);
      tempStatus.output = tempStatus.state;
    } else {
      // setPinState(PLUGIN_ID_000, event->Par1, PIN_MODE_OUTPUT, event->Par2);
      pinMode(event->Par1, OUTPUT);
      digitalWrite(event->Par1, event->Par2);
      tempStatus.mode   = PIN_MODE_OUTPUT;
      tempStatus.state  = event->Par2;
      tempStatus.output = event->Par2;
    }
    tempStatus.command = 1; // set to 1 in order to display the status in the PinStatus page
    savePortStatus(key, tempStatus);

    String log = String(F("SW   : GPIO ")) + String(event->Par1) + String(F(" Set to ")) + String(event->Par2);
    addLog(LOG_LEVEL_INFO, log);
    SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);

    // SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_000, event->Par1, log, 0));
  }
  return return_command_success();
}

String Command_GPIOtoggle(struct EventStruct *event, const char *Line)
{
  if ((event->Par1 >= 0) && (event->Par1 <= PIN_D_MAX))
  {
    portStatusStruct tempStatus;
    const uint32_t   key = createInternalGpioKey(event->Par1);

    // WARNING: operator [] creates an entry in the map if key does not exist
    // So the next command should be part of each command:
    tempStatus = globalMapPortStatus[key];

    if ((tempStatus.mode == PIN_MODE_OUTPUT) || (tempStatus.mode == PIN_MODE_UNDEFINED)) { // toggle only output pins
      tempStatus.state   = !(read_GPIO_state(event->Par1, tempStatus.mode));               // toggle current state value
      tempStatus.output  = tempStatus.state;
      tempStatus.mode    = PIN_MODE_OUTPUT;
      tempStatus.command = 1;                                                              // set to 1 in order to display the status in the
                                                                                           // PinStatus page

      pinMode(event->Par1, OUTPUT);
      digitalWrite(event->Par1, tempStatus.state);

      // setPinState(PLUGIN_ID_000, event->Par1, PIN_MODE_OUTPUT, !currentState);
      savePortStatus(key, tempStatus);
      String log = String(F("SW   : Toggle GPIO ")) + String(event->Par1) + String(F(" Set to ")) + String(tempStatus.state);
      addLog(LOG_LEVEL_INFO, log);
      SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);

      // SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_000, event->Par1, log, 0));
    }
  }
  return return_command_success();
}

String Command_PWM(struct EventStruct *event, const char *Line)
{
  if ((event->Par1 >= 0) && (event->Par1 <= PIN_D_MAX))
  {
    portStatusStruct tempStatus;
    const uint32_t   key = createInternalGpioKey(event->Par1);

    // WARNING: operator [] creates an entry in the map if key does not exist
    // So the next command should be part of each command:
    tempStatus = globalMapPortStatus[key];

    #if defined(ESP8266)
    pinMode(event->Par1, OUTPUT);
    #endif /* if defined(ESP8266) */

    if (event->Par3 != 0)
    {
      const byte prev_mode  = tempStatus.mode;
      uint16_t   prev_value = tempStatus.state;

      // getPinState(PLUGIN_ID_000, event->Par1, &prev_mode, &prev_value);
      if (prev_mode != PIN_MODE_PWM) {
        prev_value = 0;
      }

      int32_t step_value = ((event->Par2 - prev_value) << 12) / event->Par3;
      int32_t curr_value = prev_value << 12;

      int i = event->Par3;

      while (i--) {
        curr_value += step_value;
        int16_t new_value;
        new_value = (uint16_t)(curr_value >> 12);
        analogWriteESP(event->Par1, new_value);
        delay(1);
      }
    }
    analogWriteESP(event->Par1, event->Par2);

    // setPinState(PLUGIN_ID_000, event->Par1, PIN_MODE_PWM, event->Par2);
    tempStatus.mode    = PIN_MODE_PWM;
    tempStatus.state   = event->Par2;
    tempStatus.output  = event->Par2;
    tempStatus.command = 1; // set to 1 in order to display the status in the PinStatus page

    savePortStatus(key, tempStatus);
    String log = F("SW   : GPIO ");
    log += event->Par1;
    log += F(" Set PWM to ");
    log += event->Par2;

    if (event->Par3 != 0) {
      log += F(" duration ");
      log += event->Par3;
    }
    addLog(LOG_LEVEL_INFO, log);
    SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);

    // SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_000, event->Par1, log, 0));
  }
  return return_command_success();
}

String Command_Pulse(struct EventStruct *event, const char *Line)
{
  if ((event->Par1 >= 0) && (event->Par1 <= PIN_D_MAX))
  {
    portStatusStruct tempStatus;
    const uint32_t   key = createInternalGpioKey(event->Par1);

    // WARNING: operator [] creates an entry in the map if key does not exist
    // So the next command should be part of each command:
    tempStatus = globalMapPortStatus[key];

    pinMode(event->Par1, OUTPUT);
    digitalWrite(event->Par1, event->Par2);
    delay(event->Par3);
    digitalWrite(event->Par1, !event->Par2);

    // setPinState(PLUGIN_ID_000, event->Par1, PIN_MODE_OUTPUT, event->Par2);
    tempStatus.mode    = PIN_MODE_OUTPUT;
    tempStatus.state   = event->Par2;
    tempStatus.output  = event->Par2;
    tempStatus.command = 1; // set to 1 in order to display the status in the PinStatus page
    savePortStatus(key, tempStatus);

    String log = String(F("SW   : GPIO ")) + String(event->Par1) + String(F(" Pulsed for ")) + String(event->Par3) + String(F(" mS"));
    addLog(LOG_LEVEL_INFO, log);
    SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);

    // SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_000, event->Par1, log, 0));
  }
  return return_command_success();
}

String Command_longPulse_seconds(struct EventStruct *event, const char *Line)
{
  return Command_longPulse(event, Line, false);
}

String Command_longPulse_msec(struct EventStruct *event, const char *Line)
{
  return Command_longPulse(event, Line, true);
}

String Command_longPulse(struct EventStruct *event, const char *Line, bool time_in_msec)
{
  if ((event->Par1 >= 0) && (event->Par1 <= PIN_D_MAX))
  {
    portStatusStruct tempStatus;
    const uint32_t   key = createInternalGpioKey(event->Par1);

    // WARNING: operator [] creates an entry in the map if key does not exist
    // So the next command should be part of each command:
    tempStatus = globalMapPortStatus[key];

    const bool pinStateHigh             = event->Par2 != 0;
    const uint16_t pinStateValue        = pinStateHigh ? 1 : 0;
    const uint16_t inversePinStateValue = pinStateHigh ? 0 : 1;
    pinMode(event->Par1, OUTPUT);
    digitalWrite(event->Par1, pinStateValue);

    // setPinState(PLUGIN_ID_000, event->Par1, PIN_MODE_OUTPUT, pinStateValue);
    tempStatus.mode    = PIN_MODE_OUTPUT;
    tempStatus.state   = event->Par2;
    tempStatus.output  = event->Par2;
    tempStatus.command = 1; // set to 1 in order to display the status in the PinStatus page
    savePortStatus(key, tempStatus);

    // FIXME TD-er. No longer part of Plugin, so must use other scheduler mechanism to set GPIO back
    unsigned long timer = time_in_msec ? event->Par3 : event->Par3 * 1000;

    // Create a future system timer call to set the GPIO pin back to its normal value.
    setPluginTaskTimer(timer, PLUGIN_ID_000, event->TaskIndex, event->Par1, inversePinStateValue);
    String log = String(F("SW   : GPIO ")) + String(event->Par1) +
                 String(F(" Pulse set for ")) + String(event->Par3) + String(time_in_msec ? F(" msec") : F(" sec"));
    addLog(LOG_LEVEL_INFO, log);
    SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);

    // SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_000, event->Par1, log, 0));
  }
  return return_command_success();
}

String Command_servo(struct EventStruct *event, const char *Line)
{
  // GPIO number is stored inside event->Par2 instead of event->Par1 as in all the other commands
  // So needs to reload the tempPortStruct.
  if ((event->Par1 >= 0) && (event->Par1 <= 2)) {
    portStatusStruct tempStatus;
    const uint32_t   key = createInternalGpioKey(event->Par2); // WARNING: 'servo' uses Par2 instead of Par1
    // WARNING: operator [] creates an entry in the map if key does not exist
    // So the next command should be part of each command:
    tempStatus = globalMapPortStatus[key];

    switch (event->Par1)
    {
    case 1:

      // IRAM: doing servo stuff uses 740 bytes IRAM. (doesnt matter how many instances)
        #if defined(ESP8266)

      // SPECIAL CASE TO ALLOW SERVO TO BE DETATTCHED AND SAVE POWER.
      if (event->Par3 >= 9000) {
        servo1.detach();
      } else {
        servo1.attach(event->Par2);
        servo1.write(event->Par3);
      }
        #endif /* if defined(ESP8266) */
      break;
    case 2:
        #if defined(ESP8266)

      if (event->Par3 >= 9000) {
        servo2.detach();
      } else {
        servo2.attach(event->Par2);
        servo2.write(event->Par3);
      }
        #endif /* if defined(ESP8266) */
      break;
    }

    // setPinState(PLUGIN_ID_000, event->Par2, PIN_MODE_SERVO, event->Par3);
    tempStatus.mode    = PIN_MODE_SERVO;
    tempStatus.state   = event->Par3;
    tempStatus.output  = event->Par3;
    tempStatus.command = 1; // set to 1 in order to display the status in the PinStatus page
    savePortStatus(key, tempStatus);
    String log = String(F("SW   : GPIO ")) + String(event->Par2) + String(F(" Servo set to ")) + String(event->Par3);
    addLog(LOG_LEVEL_INFO, log);
    SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);

    // SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_000, event->Par2, log, 0));
  }
  return return_command_success();
}

String Command_status_gpio(struct EventStruct *event, const char *Line)
{
  const uint32_t key = createInternalGpioKey(event->Par1);

  SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, dummyString, 0);

  // SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_000, event->Par2, dummyString, 0));
  return return_command_success();
}

String Command_monitor_gpio(struct EventStruct *event, const char *Line)
{
  const uint32_t key = createInternalGpioKey(event->Par1);

  addMonitorToPort(key);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(32);
    log  = String(F("GPIO "));
    log += event->Par1;
    log += F(" added to monitor list.");
    addLog(LOG_LEVEL_INFO, log);
  }
  SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, dummyString, 0);
  return return_command_success();
}

String Command_unmonitor_gpio(struct EventStruct *event, const char *Line)
{
  const uint32_t key = createInternalGpioKey(event->Par1);

  removeMonitorFromPort(key);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(36);
    log  = String(F("GPIO "));
    log += event->Par1;
    log += F(" removed from monitor list.");
    addLog(LOG_LEVEL_INFO, log);
  }
  SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, dummyString, 0);
  return return_command_success();
}

String Command_inputswitchstate(struct EventStruct *event, const char *Line)
{
  portStatusStruct tempStatus;
  const uint32_t   key = createInternalGpioKey(Settings.TaskDevicePin1[event->Par1]);

  // WARNING: operator [] creates an entry in the map if key does not exist
  // So the next command should be part of each command:
  tempStatus = globalMapPortStatus[key];

  UserVar[event->Par1 * VARS_PER_TASK] = event->Par2;
  tempStatus.output                    = event->Par2;
  tempStatus.command                   = 1;
  savePortStatus(key, tempStatus);
  return return_command_success();
}

String Command_rtttl(struct EventStruct *event, const char *Line)
{
  // FIXME: Absolutely no error checking in play_rtttl, until then keep it only in testing
  // play a tune via a RTTTL string, look at https://www.letscontrolit.com/forum/viewtopic.php?f=4&t=343&hilit=speaker&start=10 for more
  // info.
  if ((event->Par1 >= 0) && (event->Par1 <= PIN_D_MAX))
  {
    // FIXME TD-er  For now disabled, so we can see where the notes parameters are.

    /*
       portStatusStruct tempStatus;
       const uint32_t   key = createInternalGpioKey(event->Par1);

       // WARNING: operator [] creates an entry in the map if key does not exist
       // So the next command should be part of each command:
       tempStatus = globalMapPortStatus[key];

       pinMode(event->Par1, OUTPUT);

       // char sng[1024] ="";
       String tmpString = string;
       tmpString.replace('-', '#');

       // tmpString.toCharArray(sng, 1024);
       play_rtttl(event->Par1, tmpString.c_str());

       // setPinState(PLUGIN_ID_000, event->Par1, PIN_MODE_OUTPUT, event->Par2);
       tempStatus.mode    = PIN_MODE_OUTPUT;
       tempStatus.state   = event->Par2;
       tempStatus.output  = event->Par2;
       tempStatus.command = 1; // set to 1 in order to display the status in the PinStatus page
       savePortStatus(key, tempStatus);
       String log = String(F("SW   : ")) + string;
       addLog(LOG_LEVEL_INFO, log);
       SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);

       // SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_000, event->Par1, log, 0));
     */
  }
  return return_command_success();
}

// **************************************************************************/
// Command "tone"
// play a tone on pin par1, with frequency par2 and duration par3.
// **************************************************************************/
String Command_tone(struct EventStruct *event, const char *Line)
{
  if ((event->Par1 >= 0) && (event->Par1 <= PIN_D_MAX))
  {
    portStatusStruct tempStatus;
    const uint32_t   key = createInternalGpioKey(event->Par1);

    // WARNING: operator [] creates an entry in the map if key does not exist
    // So the next command should be part of each command:
    tempStatus = globalMapPortStatus[key];

    pinMode(event->Par1, OUTPUT);
    tone_espEasy(event->Par1, event->Par2, event->Par3);

    // setPinState(PLUGIN_ID_000, event->Par1, PIN_MODE_OUTPUT, event->Par2);
    tempStatus.mode    = PIN_MODE_OUTPUT;
    tempStatus.state   = event->Par2;
    tempStatus.output  = event->Par2;
    tempStatus.command = 1;            // set to 1 in order to display the status in the PinStatus page
    savePortStatus(key, tempStatus);
    String log = String(F("Tone : ")); // + string;
    addLog(LOG_LEVEL_INFO, log);
    SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);

    // SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_000, event->Par1, log, 0));
  }

  return return_command_success();
}

// **************************************************************************/
// Helper functions
// **************************************************************************/
uint32_t createInternalGpioKey(uint16_t portNumber) {
  return createKey(PLUGIN_ID_000, portNumber);
}

bool read_GPIO_state(struct EventStruct *event) {
  byte pinNumber     = Settings.TaskDevicePin1[event->TaskIndex];
  const uint32_t key = createInternalGpioKey(pinNumber);

  if (existPortStatus(key)) {
    return read_GPIO_state(pinNumber, globalMapPortStatus[key].mode);
  }
  return false;
}

bool read_GPIO_state(byte pinNumber, byte pinMode) {
  bool canRead = false;

  switch (pinMode)
  {
  case PIN_MODE_UNDEFINED:
  case PIN_MODE_INPUT:
  case PIN_MODE_INPUT_PULLUP:
  case PIN_MODE_OUTPUT:
    canRead = true;
    break;
  case PIN_MODE_PWM:
    break;
  case PIN_MODE_SERVO:
    break;
  case PIN_MODE_OFFLINE:
    break;
  default:
    break;
  }

  if (!canRead) { return false; }

  // Do not read from the pin while mode is set to PWM or servo.
  // See https://github.com/letscontrolit/ESPEasy/issues/2117#issuecomment-443516794
  return digitalRead(pinNumber) == HIGH;
}

void analogWriteESP(int pin, int value)
{
  #if defined(ESP32)

  // find existing channel if this pin has been used before
  int8_t ledChannel = -1;

  for (byte x = 0; x < 16; x++) {
    if (ledChannelPin[x] == pin) {
      ledChannel = x;
    }
  }

  if (ledChannel == -1)             // no channel set for this pin
  {
    for (byte x = 0; x < 16; x++) { // find free channel
      if (ledChannelPin[x] == -1)
      {
        int freq = 5000;
        ledChannelPin[x] = pin; // store pin nr
        ledcSetup(x, freq, 10); // setup channel
        ledcAttachPin(pin, x);  // attach to this pin
        ledChannel = x;
        break;
      }
    }
  }
  ledcWrite(ledChannel, value);
  #else /* if defined(ESP32) */
  analogWrite(pin, value);
  #endif /* if defined(ESP32) */
}

#endif // COMMAND_GPIO_H
