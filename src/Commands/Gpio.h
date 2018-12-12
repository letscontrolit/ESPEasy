#ifndef COMMAND_GPIO_H
#define COMMAND_GPIO_H


#include "ESPEasy-GPIO.h"

#if defined(ESP8266)
Servo servo1;
Servo servo2;
#endif /* if defined(ESP8266) */


// Forward declarations
void     analogWriteESP(int pin,
                        int value,
                        unsigned int frequency);
String   Command_longPulse(struct EventStruct *event,
                           const char         *Line,
                           bool                time_in_msec);
String return_command_failed_invalid_GPIO(byte pinNumber);

// **************************************************************************/
// Command "gpio"
// GPIO,<gpio>,<state>
// State == 2 is input mode with pull up.
// **************************************************************************/
String Command_GPIO(struct EventStruct *event, const char *Line)
{
  if (!checkValidGpioPin(event->Par1)) return return_command_failed_invalid_GPIO(event->Par1);
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
  if (tempStatus.monitor) tempStatus.forceMonitorEvent=1; //set to 1 in order to force an EVENT in case monitor is requested
  savePortStatus(key, tempStatus);

  String log = String(F("GPIO : ")) + String(event->Par1) + String(F(" Set to ")) + String(event->Par2);
  addLog(LOG_LEVEL_INFO, log);
  SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);

  // SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_000, event->Par1, log, 0));
  return return_command_success();
}

// **************************************************************************/
// Command "gpiotoggle"
// gpiotoggle,<gpio>
// Toggles only the output when it is set to output mode.
// **************************************************************************/
String Command_GPIOtoggle(struct EventStruct *event, const char *Line)
{
  if (!checkValidGpioPin(event->Par1)) return return_command_failed_invalid_GPIO(event->Par1);
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
    if (tempStatus.monitor) tempStatus.forceMonitorEvent=1; //set to 1 in order to force an EVENT in case monitor is requested

    pinMode(event->Par1, OUTPUT);
    digitalWrite(event->Par1, tempStatus.state);

    // setPinState(PLUGIN_ID_000, event->Par1, PIN_MODE_OUTPUT, !currentState);
    savePortStatus(key, tempStatus);
    String log = String(F("SW   : Toggle GPIO ")) + String(event->Par1) + String(F(" Set to ")) + String(tempStatus.state);
    addLog(LOG_LEVEL_INFO, log);
    SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);

    // SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_000, event->Par1, log, 0));
  }
  return return_command_success();
}

// **************************************************************************/
// Command "pwm"
// PWM,<GPIO>,<duty>
// PWM,<GPIO>,<duty>,<duration>
// PWM,<GPIO>,<duty>,<duration>,<frequency>
// Sets pin to output and mode to PWM (disable reading for example)
// **************************************************************************/
String Command_PWM(struct EventStruct *event, const char *Line)
{
  if (!checkValidGpioPin(event->Par1)) return return_command_failed_invalid_GPIO(event->Par1);
  portStatusStruct tempStatus;
  const uint32_t   key = createInternalGpioKey(event->Par1);

  // WARNING: operator [] creates an entry in the map if key does not exist
  // So the next command should be part of each command:
  tempStatus = globalMapPortStatus[key];

  unsigned int frequency = 1000;
  if (event->Par4 != 0)
    frequency = event->Par4;

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
      analogWriteESP(event->Par1, new_value, frequency);
      delay(1);
    }
  }
  analogWriteESP(event->Par1, event->Par2, frequency);

  // setPinState(PLUGIN_ID_000, event->Par1, PIN_MODE_PWM, event->Par2);
  tempStatus.mode    = PIN_MODE_PWM;
  tempStatus.state   = event->Par2;
  tempStatus.output  = event->Par2;
  tempStatus.command = 1; // set to 1 in order to display the status in the PinStatus page

  savePortStatus(key, tempStatus);
  String log = F("GPIO : ");
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
  return return_command_success();
}

// **************************************************************************/
// Command "pulse"
// Pulse,<GPIO>,<state>,<duration>
// Sens a "short" (< 1000 msec) pulse.
// Command is blocking for the duration of the pulse
// **************************************************************************/
String Command_Pulse(struct EventStruct *event, const char *Line)
{
  if (!checkValidGpioPin(event->Par1)) return return_command_failed_invalid_GPIO(event->Par1);
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

  String log = String(F("GPIO : ")) + String(event->Par1) + String(F(" Pulsed for ")) + String(event->Par3) + String(F(" mS"));
  addLog(LOG_LEVEL_INFO, log);
  SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);

  // SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_000, event->Par1, log, 0));
  return return_command_success();
}


// **************************************************************************/
// Command "LongPulse"
// LongPulse,<GPIO>,<state>,<duration>
// Sens a "long" (typ. > 50 msec) pulse.
// Command is NOT blocking for the duration of the pulse (duration in seconds)
// **************************************************************************/
String Command_longPulse_seconds(struct EventStruct *event, const char *Line)
{
  return Command_longPulse(event, Line, false);
}

// **************************************************************************/
// Command "LongPulse_mS"
// LongPulse_mS,<GPIO>,<state>,<duration>
// Sens a "long" (typ. > 50 msec) pulse.
// Command is NOT blocking for the duration of the pulse (duration in msec)
// **************************************************************************/
String Command_longPulse_msec(struct EventStruct *event, const char *Line)
{
  return Command_longPulse(event, Line, true);
}

String Command_longPulse(struct EventStruct *event, const char *Line, bool time_in_msec)
{
  if (!checkValidGpioPin(event->Par1)) return return_command_failed_invalid_GPIO(event->Par1);
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
  setGPIOTimer(timer, event->Par1, inversePinStateValue);
  String log = String(F("GPIO : ")) + String(event->Par1) +
               String(F(" Pulse set for ")) + String(event->Par3) + String(time_in_msec ? F(" msec") : F(" sec"));
  addLog(LOG_LEVEL_INFO, log);
  SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);

  // SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_000, event->Par1, log, 0));
  return return_command_success();
}

// **************************************************************************/
// Command "servo"
// Servo,<servo>,<GPIO>,<position>
// **************************************************************************/
String Command_servo(struct EventStruct *event, const char *Line)
{
  // GPIO number is stored inside event->Par2 instead of event->Par1 as in all the other commands
  // So needs to reload the tempPortStruct.
  if (!checkValidGpioPin(event->Par1)) return return_command_failed_invalid_GPIO(event->Par1);
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
    String log = String(F("GPIO : ")) + String(event->Par2) + String(F(" Servo set to ")) + String(event->Par3);
    addLog(LOG_LEVEL_INFO, log);
    SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);

    // SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_000, event->Par2, log, 0));
    return return_command_success();
  }
  return return_command_failed();
}

// **************************************************************************/
// Command "status_gpio"
// status_gpio,<GPIO>
// Get the status of the given GPIO pin.
// **************************************************************************/
String Command_status_gpio(struct EventStruct *event, const char *Line)
{
  if (!checkValidGpioPin(event->Par1)) return return_command_failed_invalid_GPIO(event->Par1);
  const uint32_t key = createInternalGpioKey(event->Par1);
  SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, dummyString, 0);

  // SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_000, event->Par2, dummyString, 0));
  return return_command_success();
}

// **************************************************************************/
// Command "monitor_gpio"
// monitor_gpio,<GPIO>
// Make sure the given GPIO pin is tracked in the pin status structure.
// **************************************************************************/
String Command_monitor_gpio(struct EventStruct *event, const char *Line)
{
  if (!checkValidGpioPin(event->Par1)) return return_command_failed_invalid_GPIO(event->Par1);
  const uint32_t key = createInternalGpioKey(event->Par1);

  addMonitorToPort(key);
  //giig1967g: Comment next line to receive an EVENT just after calling the monitor command
  globalMapPortStatus[key].state = read_GPIO_state(event->Par1, globalMapPortStatus[key].mode); //set initial value to avoid an event just after calling the command

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

// **************************************************************************/
// Command "unmonitor_gpio"
// unmonitor_gpio,<GPIO>
// Remove the given GPIO pin from the pin status structure.
// **************************************************************************/
String Command_unmonitor_gpio(struct EventStruct *event, const char *Line)
{
  if (!checkValidGpioPin(event->Par1)) return return_command_failed_invalid_GPIO(event->Par1);
  const uint32_t key = createInternalGpioKey(event->Par1);

  SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, dummyString, 0);
  removeMonitorFromPort(key);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(36);
    log  = String(F("GPIO "));
    log += event->Par1;
    log += F(" removed from monitor list.");
    addLog(LOG_LEVEL_INFO, log);
  }
  return return_command_success();
}


// **************************************************************************/
// Command "rtttl"
// play a melody on pin par1
// **************************************************************************/
String Command_rtttl(struct EventStruct *event, const char *Line)
{
  // FIXME: Absolutely no error checking in play_rtttl, until then keep it only in testing
  // play a tune via a RTTTL string, look at https://www.letscontrolit.com/forum/viewtopic.php?f=4&t=343&hilit=speaker&start=10 for more
  // info.
  if (!checkValidGpioPin(event->Par1)) return return_command_failed_invalid_GPIO(event->Par1);
  String tmpString = Line;
  int colonPos = tmpString.indexOf(':');
  if (colonPos < 0) {
    addLog(LOG_LEVEL_ERROR, F("RTTTL : Invalid formatted command"));
    return return_command_failed();
  }
/*
  String command = tmpString.substring(0, colonPos);
  tmpString = tmpString.substring(colonPos);
*/
  tmpString.replace('-', '#');


  portStatusStruct tempStatus;
  const uint32_t   key = createInternalGpioKey(event->Par1);

  // WARNING: operator [] creates an entry in the map if key does not exist
  // So the next command should be part of each command:
  tempStatus = globalMapPortStatus[key];

  pinMode(event->Par1, OUTPUT);
  play_rtttl(event->Par1, tmpString.c_str());

  // setPinState(PLUGIN_ID_000, event->Par1, PIN_MODE_OUTPUT, event->Par2);
  tempStatus.mode    = PIN_MODE_OUTPUT;
  tempStatus.state   = event->Par2;
  tempStatus.output  = event->Par2;
  tempStatus.command = 1; // set to 1 in order to display the status in the PinStatus page
  savePortStatus(key, tempStatus);
  SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, tmpString, 0);
  return return_command_success();
}

// **************************************************************************/
// Command "tone"
// play a tone on pin par1, with frequency par2 and duration par3.
// **************************************************************************/
String Command_tone(struct EventStruct *event, const char *Line)
{
  if (!checkValidGpioPin(event->Par1)) return return_command_failed_invalid_GPIO(event->Par1);
  portStatusStruct tempStatus;
  const uint32_t   key = createInternalGpioKey(event->Par1);

  // WARNING: operator [] creates an entry in the map if key does not exist
  // So the next command should be part of each command:
  tempStatus = globalMapPortStatus[key];

  pinMode(event->Par1, OUTPUT);
  unsigned int frequency = event->Par2;
  unsigned long duration = event->Par3;
  if (duration < 50) {
    tone_espEasy(event->Par1, frequency, duration);
  } else {
    // Do not wait for a very long time using delays.
    analogWriteESP(event->Par1, 100, frequency);
    // Create a future system timer call to set the GPIO pin back to its normal value.
    setGPIOTimer(duration, event->Par1, 0);
  }

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
  return return_command_success();
}

// **************************************************************************/
// Helper functions
// **************************************************************************/

String return_command_failed_invalid_GPIO(byte pinNumber) {
  if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
    String log;
    log  = F("GPIO : Invalid GPIO pin given: ");
    log += pinNumber;
    addLog(LOG_LEVEL_ERROR, log);
  }
  return return_command_failed();
}

#if defined(ESP32)
void analogWriteESP(int pin, int value, unsigned int frequency) {
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
}

#else /* if defined(ESP32) */

void analogWriteESP(int pin, int value, unsigned int frequency) {
  analogWriteFreq(frequency);
  analogWrite(pin, value);
}
#endif /* if defined(ESP32) */



#endif // COMMAND_GPIO_H
