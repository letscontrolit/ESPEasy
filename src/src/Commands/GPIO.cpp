#include "../Commands/GPIO.h"

#include "../../ESPEasy_common.h"


#include "../../ESPEasy-Globals.h"

#include "../Commands/Common.h"
#include "../DataStructs/PinMode.h"
#include "../ESPEasyCore/Controller.h"
#include "../ESPEasyCore/ESPEasyGPIO.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/ESPEasy_Scheduler.h"
#include "../Globals/GlobalMapPortStatus.h"
#include "../Helpers/Audio.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/PortStatus.h"
#include "../Helpers/Numerical.h"

#if FEATURE_GPIO_USE_ESP8266_WAVEFORM
# include <core_esp8266_waveform.h>
#endif 

// Forward declarations of functions used in this module
// Normally those would be declared in the .h file as private members
// But since these are not part of a class, forward declare them in the .cpp
//void createAndSetPortStatus_Mode_State(uint32_t key, uint8_t newMode, int8_t newState);
const __FlashStringHelper * getPluginIDAndPrefix(char selection, pluginID_t& pluginID, bool& success);
void logErrorGpioOffline(const __FlashStringHelper * prefix, int port);
void logErrorGpioOutOfRange(const __FlashStringHelper * prefix, int port, const char* Line = nullptr);
void logErrorGpioNotOutput(const __FlashStringHelper * prefix, int port);
void logErrorModeOutOfRange(const __FlashStringHelper * prefix, int port);
bool gpio_monitor_helper(int port, struct EventStruct *event, const char* Line);
bool gpio_unmonitor_helper(int port, struct EventStruct *event, const char* Line);
#ifdef USES_P009
bool mcpgpio_range_pattern_helper(struct EventStruct *event, const char* Line, bool isWritePattern);
#endif
#ifdef USES_P019
bool pcfgpio_range_pattern_helper(struct EventStruct *event, const char* Line, bool isWritePattern);
#endif
bool gpio_mode_range_helper(uint8_t pin, uint8_t pinMode, struct EventStruct *event, const char* Line);
#ifdef USES_P019
uint8_t getPcfAddress(uint8_t pin);
#endif
bool setGPIOMode(uint8_t pin, uint8_t mode);
#ifdef USES_P019
bool setPCFMode(uint8_t pin, uint8_t mode);
#endif
#ifdef USES_P009
bool setMCPMode(uint8_t pin, uint8_t mode);
bool mcpgpio_plugin_range_helper(uint8_t pin1, uint8_t pin2, uint16_t &result);
#endif
#ifdef USES_P019
bool pcfgpio_plugin_range_helper(uint8_t pin1, uint8_t pin2, uint16_t &result);
#endif


/*************************************************************************/

const __FlashStringHelper * Command_GPIO_Monitor(struct EventStruct *event, const char *Line)
{
  if (gpio_monitor_helper(event->Par2, event, Line)) {
    return return_command_success();
  }
  return return_command_failed();
}

const __FlashStringHelper * Command_GPIO_MonitorRange(struct EventStruct *event, const char *Line)
{
  bool success = true;

  for (uint8_t i = event->Par2; i <= event->Par3; i++) {
    success &= gpio_monitor_helper(i, event, Line);
  }
  return success ? return_command_success() : return_command_failed();
}

bool gpio_monitor_helper(int port, struct EventStruct *event, const char *Line)
{
  pluginID_t pluginID = INVALID_PLUGIN_ID;
  bool success = false;

  // parseString(Line, 2).charAt(0)='g':gpio; ='p':pcf; ='m':mcp
  const __FlashStringHelper * logPrefix = getPluginIDAndPrefix(parseString(Line, 2).charAt(0), pluginID, success);

  if (success && checkValidPortRange(pluginID, port))
  {
    const uint32_t key = createKey(pluginID, port); // WARNING: 'monitor' uses Par2 instead of Par1
    // if (!existPortStatus(key)) globalMapPortStatus[key].mode=PIN_MODE_OUTPUT;
    addMonitorToPort(key);

    int8_t state;

    // giig1967g: Comment next 3 lines to receive an EVENT just after calling the monitor command
    GPIO_Read(pluginID, port, state);
    globalMapPortStatus[key].state = state;

    if (state == -1) { globalMapPortStatus[key].mode = PIN_MODE_OFFLINE; }

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = concat(logPrefix,  F(" port #")); 
      log += port; 
      log += F(": added to monitor list.");
      addLogMove(LOG_LEVEL_INFO, log);
    }
    String dummy;
    SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, dummy, 0);

    return true;
  } else {
    logErrorGpioOutOfRange(logPrefix, port, Line);
    return false;
  }
}

const __FlashStringHelper * Command_GPIO_UnMonitor(struct EventStruct *event, const char *Line)
{
  if (gpio_unmonitor_helper(event->Par2, event, Line)) {
    return return_command_success();
  }
  else {
    return return_command_failed();
  }
}

const __FlashStringHelper * Command_GPIO_UnMonitorRange(struct EventStruct *event, const char *Line)
{
  bool success = true;

  for (uint8_t i = event->Par2; i <= event->Par3; i++) {
    success &= gpio_unmonitor_helper(i, event, Line);
  }
  return success ? return_command_success() : return_command_failed();
}

bool gpio_unmonitor_helper(int port, struct EventStruct *event, const char *Line)
{

  pluginID_t pluginID = INVALID_PLUGIN_ID;
  bool success = false;

  // parseString(Line, 2).charAt(0)='g':gpio; ='p':pcf; ='m':mcp
  const __FlashStringHelper * logPrefix = getPluginIDAndPrefix(parseString(Line, 2).charAt(0), pluginID, success);

  if (success && checkValidPortRange(pluginID, port))
  {
    const uint32_t key = createKey(pluginID, port); // WARNING: 'monitor' uses Par2 instead of Par1
    String dummy;
    SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, dummy, 0);

    removeMonitorFromPort(key);
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = concat(logPrefix, F(" port #"));
      log += port;
      log += F(": removed from monitor list.");
      addLogMove(LOG_LEVEL_INFO, log);
    }

    return true;
  } else {
    logErrorGpioOutOfRange(logPrefix, port, Line);
    return false;
  }
}

const __FlashStringHelper * Command_GPIO_LongPulse(struct EventStruct *event, const char *Line)
{
  event->Par3 *= 1000;
  event->Par4 *= 1000;
  event->Par5 *= 1000;
  return Command_GPIO_LongPulse_Ms(event, Line);
}

const __FlashStringHelper * Command_GPIO_LongPulse_Ms(struct EventStruct *event, const char *Line)
{
  pluginID_t pluginID = INVALID_PLUGIN_ID;
  bool success = false;

  // Line[0]='l':longpulse; ='p':pcflongpulse; ='m':mcplongpulse
  const __FlashStringHelper * logPrefix = getPluginIDAndPrefix(Line[0], pluginID, success);

  if (success && checkValidPortRange(pluginID, event->Par1))
  {
    // Event parameters
    // Par1: pin nr
    // Par2: state
    // Par3: duration of  state
    // Par4: duration of !state
    // Par5: repeat count (only when Par4 > 0)
    //       -1 = repeat indefinately
    //        0 = only once
    //        N = Repeat N times
    Scheduler.clearGPIOTimer(pluginID, event->Par1);
    const uint32_t key = createKey(pluginID, event->Par1);
    createAndSetPortStatus_Mode_State(key, PIN_MODE_OUTPUT, event->Par2);

    #if FEATURE_GPIO_USE_ESP8266_WAVEFORM
    bool usingWaveForm = 
        event->Par3 > 0 && event->Par3 < 15000 &&
        event->Par4 > 0 && event->Par4 < 15000 &&
        event->Par5 != 0;
        
    if (usingWaveForm) {
      // Preferred is to use the ESP8266 waveform function.
      // Max time high or low is roughly 53 sec @ 80 MHz or half @160 MHz.
      // This is not available on ESP32.

      const uint8_t pin = event->Par1;
      uint32_t timeHighUS = event->Par3 * 1000;
      uint32_t timeLowUS  = event->Par4 * 1000;

      if (event->Par2 == 0) {
        std::swap(timeHighUS, timeLowUS);
      }
      uint32_t runTimeUS = 0;
      if (event->Par5 > 0) {
        // Must set slightly lower than expected duration as it will be rounded up.
        runTimeUS = event->Par5 * (timeHighUS + timeLowUS) - ((timeHighUS + timeLowUS) / 2);
      }

      pinMode(event->Par1, OUTPUT);
      usingWaveForm = startWaveform(
        pin, timeHighUS, timeLowUS, runTimeUS);
    }
    #else
    // waveform function not available on ESP32
    const bool usingWaveForm = false;
    #endif

    if (!usingWaveForm) {
      // Par1 = pinnr
      // Par2 = pin state
      // Par3 = timeHigh in msec
      // Par4 = timeLow in msec
      // Par5 = repeat count
      GPIO_Write(pluginID, event->Par1, event->Par2);
      if (event->Par4 > 0 && event->Par5 != 0) {
        // Compute repeat interval

        // Schedule switching pin to given state for repeat
        Scheduler.setGPIOTimer(
          event->Par3,   // msecFromNow
          pluginID,    
          event->Par1,   // Pin/port nr
          event->Par2,   // pin state
          event->Par3,   // repeat interval (high)
          event->Par5,   // repeat count
          event->Par4);  // alternate interval (low)
      } else {
        // Schedule switching pin back to original state
        Scheduler.setGPIOTimer(
          event->Par3,   // msecFromNow
          pluginID,    
          event->Par1,   // Pin/port nr
          !event->Par2,  // pin state
          0, // repeatInterva
          0); // repeatCount
      }
    }


    String log = logPrefix;
    log += F(" : port ");
    log += event->Par1;
    log += F(". Pulse H:");
    log += event->Par3;
    if (event->Par4 > 0 && event->Par5 != 0) {
      log += F(" L:");
      log += event->Par4;
      log += F(" #:");
      log += event->Par5;
    }
    log += F(" ms");
    addLog(LOG_LEVEL_INFO, log);
    SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);

    return return_command_success();
  } else {
    logErrorGpioOutOfRange(logPrefix, event->Par1, Line);
    return return_command_failed();
  }
}

const __FlashStringHelper * Command_GPIO_Status(struct EventStruct *event, const char *Line)
{
  bool sendStatusFlag;
  uint8_t pluginID = 0;

  switch (tolower(parseString(Line, 2).charAt(0)))
  {
    case 'g': // gpio
      pluginID       = PLUGIN_GPIO;
      sendStatusFlag = true;
      break;
#ifdef USES_P009
    case 'm': // mcp
      pluginID       = PLUGIN_MCP;
      sendStatusFlag = GPIO_MCP_Read(event->Par2) == -1;
      break;
#endif
#ifdef USES_P019
    case 'p': // pcf
      pluginID       = PLUGIN_PCF;
      sendStatusFlag = GPIO_PCF_Read(event->Par2) == -1;
      break;
#endif
    default:
      addLog(LOG_LEVEL_ERROR, F("Plugin not included in build"));
      return return_command_failed();
  }

  if (!checkValidPortRange(pluginID, event->Par2))
  {
    return return_command_failed();
  }
  const uint32_t key = createKey(pluginID, event->Par2); // WARNING: 'status' uses Par2 instead of Par1
  String dummy;
  SendStatusOnlyIfNeeded(event, sendStatusFlag, key, dummy, 0);
  return return_command_success();
}

const __FlashStringHelper * Command_GPIO_PWM(struct EventStruct *event, const char *Line)
{
  // Par1: GPIO
  // Par2: Duty Cycle
  // Par3: Fade duration
  // Par4: Frequency

  // For now, we only support the internal GPIO pins.
  const __FlashStringHelper * logPrefix = F("GPIO");
  uint32_t frequency = event->Par4;
  uint32_t key       = 0;

  if (set_Gpio_PWM(event->Par1, event->Par2, event->Par3, frequency, key)) {
    String log = F("PWM  : GPIO: ");
    log += event->Par1;
    log += F(" duty: ");
    log += event->Par2;

    if (event->Par3 != 0) {
      log += F(" Fade: ");
      log += event->Par3;
      log += F(" ms");
    }

    if (event->Par4 != 0) {
      log += F(" f: ");
      log += frequency;
      log += F(" Hz");
    }
    addLog(LOG_LEVEL_INFO, log);
    SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);

    // SendStatus(event, getPinStateJSON(SEARCH_PIN_STATE, pluginID, event->Par1, log, 0));

    return return_command_success();
  }
  logErrorGpioOutOfRange(logPrefix, event->Par1, Line);
  return return_command_failed();
}

const __FlashStringHelper * Command_GPIO_Tone(struct EventStruct *event, const char *Line)
{
  // play a tone on pin par1, with frequency par2 and duration in msec par3.
  unsigned long duration   = event->Par3;
  bool mustScheduleToneOff = false;

  if (duration > 50) {
    duration            = 0;
    mustScheduleToneOff = true;
  }

  if (tone_espEasy(event->Par1, event->Par2, duration)) {
    if (mustScheduleToneOff) {
      // For now, we only support the internal GPIO pins.
      uint8_t pluginID = PLUGIN_GPIO;
      Scheduler.setGPIOTimer(event->Par3, pluginID, event->Par1, 0);
    }
    return return_command_success();
  }
  return return_command_failed();
}

const __FlashStringHelper * Command_GPIO_RTTTL(struct EventStruct *event, const char *Line)
{
  #if FEATURE_RTTTL

  // FIXME: Absolutely no error checking in play_rtttl, until then keep it only in testing
  // play a tune via a RTTTL string, look at https://www.letscontrolit.com/forum/viewtopic.php?f=4&t=343&hilit=speaker&start=10 for
  // more info.

  String melody = parseStringToEndKeepCase(Line, 2);
  melody.replace('-', '#');

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = concat(F("RTTTL : pin: "),  event->Par1);
    log += F(" melody: ");
    log += melody;
    addLogMove(LOG_LEVEL_INFO, log);
  }

  if (play_rtttl(event->Par1, melody.c_str())) {
    return return_command_success();
  }
  #else // if FEATURE_RTTTL
  addLog(LOG_LEVEL_ERROR, F("RTTTL : command not included in build"));
  #endif // if FEATURE_RTTTL
  return return_command_failed();
}

const __FlashStringHelper * Command_GPIO_Pulse(struct EventStruct *event, const char *Line)
{
  const __FlashStringHelper * logPrefix = F("");
  bool   success  = false;
  uint8_t   pluginID = INVALID_PLUGIN_ID;

  switch (tolower(Line[0]))
  {
    case 'p':                        // pulse or pcfpulse

      if (tolower(Line[1]) == 'u') { // pulse
        pluginID  = PLUGIN_GPIO;
        logPrefix = F("GPIO");
        success   = true;
      } else if (tolower(Line[1]) == 'c') { // pcfpulse
        pluginID  = PLUGIN_PCF;
        logPrefix = F("PCF");
        success   = true;
      }
      break;
    case 'm': // mcp
      pluginID  = PLUGIN_MCP;
      logPrefix = F("MCP");
      success   = true;
      break;
  }

  if (success && checkValidPortRange(pluginID, event->Par1))
  {
    const uint32_t key = createKey(pluginID, event->Par1);

    createAndSetPortStatus_Mode_State(key, PIN_MODE_OUTPUT, event->Par2);
    GPIO_Write(pluginID, event->Par1, event->Par2);

    delay(event->Par3);

    createAndSetPortStatus_Mode_State(key, PIN_MODE_OUTPUT, !event->Par2);
    GPIO_Write(pluginID, event->Par1, !event->Par2);

    String log;
    log += logPrefix;
    log += concat(F(" : port "),  event->Par1);
    log += concat(F(". Pulse set for "),  event->Par3);
    log += F(" ms");
    addLog(LOG_LEVEL_INFO, log);
    SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);

    return return_command_success();
  } else {
    logErrorGpioOutOfRange(logPrefix, event->Par1, Line);
    return return_command_failed();
  }
}

const __FlashStringHelper * Command_GPIO_Toggle(struct EventStruct *event, const char *Line)
{
  pluginID_t pluginID = INVALID_PLUGIN_ID;
  bool success = false;

  // Line[0]='g':gpiotoggle; ='p':pcfgpiotoggle; ='m':mcpgpiotoggle
  const __FlashStringHelper * logPrefix = getPluginIDAndPrefix(Line[0], pluginID, success);

  if (success && checkValidPortRange(pluginID, event->Par1))
  {
    const uint32_t key = createKey(pluginID, event->Par1);

    // WARNING: operator [] creates an entry in the map if key does not exist
    // So the next command should be part of each command:
    uint8_t   mode;
    int8_t state;

    auto it = globalMapPortStatus.find(key);

    if (it != globalMapPortStatus.end()) {
      mode  = it->second.mode;
      state = it->second.state;
    } else {
      GPIO_Read(pluginID, event->Par1, state);
      mode = (state == -1) ? PIN_MODE_OFFLINE : PIN_MODE_OUTPUT;
    }

    switch (mode) {
      case PIN_MODE_OUTPUT:
      case PIN_MODE_UNDEFINED:
      {
        createAndSetPortStatus_Mode_State(key, PIN_MODE_OUTPUT, !state);
        GPIO_Write(pluginID, event->Par1, !state);

        String log = logPrefix;
        log += F(" toggle: port#");
        log += event->Par1;
        log += F(": set to ");
        log += static_cast<int>(!state);
        addLog(LOG_LEVEL_ERROR, log);
        SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);

        return return_command_success();
        break;
      }
      case PIN_MODE_OFFLINE:
        logErrorGpioOffline(logPrefix, event->Par1);
        return return_command_failed();
      default:
        logErrorGpioNotOutput(logPrefix, event->Par1);
        return return_command_failed();
    }
  } else {
    logErrorGpioOutOfRange(logPrefix, event->Par1, Line);
    return return_command_failed();
  }
}

const __FlashStringHelper * Command_GPIO(struct EventStruct *event, const char *Line)
{
  pluginID_t pluginID = INVALID_PLUGIN_ID;
  bool success = false;

  // Line[0]='g':gpio; ='p':pcfgpio; ='m':mcpgpio
  const __FlashStringHelper * logPrefix = getPluginIDAndPrefix(Line[0], pluginID, success);

  if (success && checkValidPortRange(pluginID, event->Par1))
  {
    int8_t state = 0;
    uint8_t   mode;

    if (event->Par2 == 2) { // INPUT
      mode = PIN_MODE_INPUT_PULLUP;

      switch (pluginID) {
        case PLUGIN_GPIO:
          setInternalGPIOPullupMode(event->Par1);
          state = GPIO_Read_Switch_State(event->Par1, PIN_MODE_INPUT_PULLUP);
          break;
#ifdef USES_P009
        case PLUGIN_MCP:
          setMCPInputAndPullupMode(event->Par1, true);
          GPIO_Read(PLUGIN_MCP, event->Par1, state);
          break;
#endif
#ifdef USES_P019
        case PLUGIN_PCF:
          // PCF8574 specific: only can read 0/low state, so we must send 1
          state = 1;
          break;
#endif
        default:
          addLog(LOG_LEVEL_ERROR, F("Plugin not included in build"));
          return return_command_failed();
      }
    } else { // OUTPUT
      mode  = PIN_MODE_OUTPUT;
      state = (event->Par2 == 0) ? 0 : 1;
    }

    const uint32_t key = createKey(pluginID, event->Par1);

    if (globalMapPortStatus[key].mode != PIN_MODE_OFFLINE)
    {
      int8_t currentState;
      GPIO_Read(pluginID, event->Par1, currentState);

      if (currentState == -1) {
        mode  = PIN_MODE_OFFLINE;
        state = -1;
      }

      createAndSetPortStatus_Mode_State(key, mode, state);

      if ((mode == PIN_MODE_OUTPUT) || (pluginID == PLUGIN_PCF)) { GPIO_Write(pluginID, event->Par1, state, mode); }

      String log = logPrefix;
      log += F(" : port#");
      log += event->Par1;
      log += F(": set to ");
      log += state;
      addLog(LOG_LEVEL_INFO, log);
      SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);
      return return_command_success();
    } else {
      logErrorGpioOffline(logPrefix, event->Par1);
      return return_command_failed();
    }
  } else {
    logErrorGpioOutOfRange(logPrefix, event->Par1, Line);
    return return_command_failed();
  }
}

void logErrorGpio(const __FlashStringHelper * prefix, int port, const __FlashStringHelper * description)
{
  if (port >= 0) {
    addLog(LOG_LEVEL_ERROR, concat(prefix, F(" : port#")) + String(port) + description);
  }
}

void logErrorModeOutOfRange(const __FlashStringHelper * prefix, int port)
{
  logErrorGpio(prefix, port, F(" mode selection is incorrect. Valid values are: 0, 1 or 2."));
}

void logErrorGpioOffline(const __FlashStringHelper * prefix, int port)
{
  logErrorGpio(prefix, port, F(" is offline."));
}

void logErrorGpioOutOfRange(const __FlashStringHelper * prefix, int port, const char *Line)
{
  logErrorGpio(prefix, port, F(" is out of range"));
  # ifndef BUILD_NO_DEBUG
  if (port >= 0) {
    if (Line != nullptr) {
      addLog(LOG_LEVEL_DEBUG, Line);
    }
  }
  #endif
}

void logErrorGpioNotOutput(const __FlashStringHelper * prefix, int port)
{
  logErrorGpio(prefix, port, F(" is not an output port"));
}

void createAndSetPortStatus_Mode_State(uint32_t key, uint8_t newMode, int8_t newState)
{
  // WARNING: operator [] creates an entry in the map if key does not exist

  #ifdef ESP32
  switch (newMode) {
    case PIN_MODE_PWM:
    case PIN_MODE_SERVO:
      break;
    default:
      checkAndClearPWM(key);
      break;
  }
  #endif


  // If it doesn't exist, it is now created.
  globalMapPortStatus[key].mode = newMode;
  auto it = globalMapPortStatus.find(key);

  if (it != globalMapPortStatus.end()) {
    // Should always be true, as it would be created if it didn't exist.
    it->second.command = 1; // set to 1 in order to display the status in the PinStatus page

    // only force events if state has changed
    if (it->second.state != newState) {
      it->second.state        = newState;
      it->second.output       = newState;
      it->second.forceEvent   = 1;
      it->second.forceMonitor = 1;
    }
  }
}

const __FlashStringHelper * getPluginIDAndPrefix(char selection, pluginID_t& pluginID, bool& success)
{
  success = true;
  switch (tolower(selection))
  {
    case 'g': // gpio
    case 'l': // longpulse (gpio)
      pluginID  = PLUGIN_GPIO;
      return F("GPIO");
#ifdef USES_P009
    case 'm': // mcp & mcplongpulse
      pluginID  = PLUGIN_MCP;
      return F("MCP");
#endif
#ifdef USES_P019
    case 'p': // pcf & pcflongpulse
      pluginID  = PLUGIN_PCF;
      return F("PCF");
#endif
    default:
      break;
  }
  success = false;
  return F("Plugin not included in build");
}

struct range_pattern_helper_data {
  range_pattern_helper_data() {
    // Make sure the pointer is always set.
    logPrefix = F("GPIO");
  }


  const __FlashStringHelper *  logPrefix;
  uint32_t write = 0;
  uint32_t mask  = 0;

  uint8_t firstPin     = 0;
  uint8_t lastPin      = 0;
  uint8_t numBytes     = 0;
  uint8_t deltaStart   = 0;
  uint8_t numBits      = 0;
  uint8_t firstAddress = 0;
  uint8_t firstBank    = 0;
  uint8_t initVal      = 0;
  bool isMask       = false;
  bool valid        = false;
};


range_pattern_helper_data range_helper_shared(pluginID_t plugin, uint8_t pin1, uint8_t pin2)
{
  range_pattern_helper_data data;

  switch (plugin) {
    case PLUGIN_PCF:
      data.logPrefix = F("PCF");
      break;
    case PLUGIN_MCP:
      data.logPrefix = F("MCP");
      break;
  }

  if ((pin2 < pin1) ||
      !checkValidPortRange(plugin, pin1) ||
      !checkValidPortRange(plugin, pin2) ||
      ((pin2 - pin1 + 1) > 16)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      addLogMove(LOG_LEVEL_ERROR, concat(data.logPrefix, F(": pin numbers out of range.")));
    }
    return data;
  }

  data.firstPin   = ((pin1 - 1) & 0xF8) + 1;
  data.lastPin    = ((pin2 - 1) & 0xF8) + 8;
  data.numBytes   = (data.lastPin - data.firstPin + 1) / 8;
  data.deltaStart = pin1 - data.firstPin;
  data.numBits    = pin2 - pin1 + 1;

  if (plugin == PLUGIN_MCP) {
    data.firstAddress = ((pin1 - 1) / 16) + 0x20;
    data.firstBank    = (((data.firstPin - 1) / 8) + 2) % 2;
    data.initVal      = 2 * data.firstAddress + data.firstBank;
  }

  data.valid = true;
  return data;
}

range_pattern_helper_data range_pattern_helper_shared(pluginID_t plugin, struct EventStruct *event, const char *Line, bool isWritePattern)
{
  range_pattern_helper_data data = range_helper_shared(plugin, event->Par1, event->Par2);

  if (!data.valid) {
    return data;
  }

  if (isWritePattern) {
    data.logPrefix = F("GPIOPattern");
  } else {
    data.logPrefix = F("GPIORange");
  }
  data.valid  = false;
  data.isMask = !parseString(Line, 5).isEmpty();

  if (data.isMask) {
    data.mask  = event->Par4 & ((1 << (data.numBytes * 8)) - 1);
    data.mask &= ((1 << data.numBits) - 1);
    data.mask  = data.mask << data.deltaStart;
  } else {
    data.mask = (1 << data.numBits) - 1;
    data.mask = data.mask << (data.deltaStart);
  }

  if (isWritePattern) {                                         // write pattern is present
    data.write  = event->Par3 & ((1 << (data.numBytes * 8)) - 1); // limit number of bytes
    data.write &= ((1 << data.numBits) - 1);                    // limit to number of bits
    data.write  = data.write << data.deltaStart;                // shift to start from starting pin
  } else {                                                      // write pattern not present
    if (event->Par3 == 0) {
      data.write = 0;
    } else if (event->Par3 == 1) {
      data.write = (1 << data.numBits) - 1;
      data.write = data.write << data.deltaStart;
    } else {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        addLogMove(LOG_LEVEL_ERROR, concat(data.logPrefix,  F(": write value must be 0 or 1.")));
      }
      return data;
    }
  }


  data.valid = true;
  return data;
}

/******************************************************************************
** Par1=starting pin
** Par2=ending pin (must be higher of starting pin; and maximum 16 pin per command)
** Par3=write pattern: it's a write pattern. Write 0 or 1.
**                     Example: use decimal number 15 (in binary is 00001111) to set to 1 pin 1,2,3 and 4 and to set to 0 pins 5,6,7,8
**                     if number of bit lower than number of pins, then padded with 0;
**                     if number of bit higher than number of pins, then it's truncated.
** Par4=mask (optional): if not present assume to operate in all pins; if present is used as a mask (1=update, 0=do not update).
**            if number of bit lower than number of pins, then padded with 0;
**            if number of bit higher than number of pins, then it's truncated.
**
**  examples:
**  mcpgpioPattern,1,8,255
**     write pattern = '1101' that will be padded as: '0000001101'
**     mask not present, assume mask = '1111111111'
**  mcpgpioPattern,3,12,13
**     write pattern = '1101' that will be padded as: '0000001101'
**     mask not present, assume mask = '1111111111'
**  mcpgpioPattern,3,12,525
**     write pattern = 525 = '100001101'
**     mask not present, assume mask = '1111111111'
**  mcpgpioPattern,3,12,525,973
**     write pattern = 525 = '100001101'
**     mask = 973 = '1111001101'
**     write pattern after mask = '1000xx11x1' where x indicates that the pin will not be changed
******************************************************************************/
#ifdef USES_P009
const __FlashStringHelper * Command_GPIO_McpGPIOPattern(struct EventStruct *event, const char *Line)
{
  return mcpgpio_range_pattern_helper(event, Line, true) ? return_command_success() : return_command_failed();
}
#endif

#ifdef USES_P019
const __FlashStringHelper * Command_GPIO_PcfGPIOPattern(struct EventStruct *event, const char *Line)
{
  return pcfgpio_range_pattern_helper(event, Line, true) ? return_command_success() : return_command_failed();
}
#endif

/******************************************************************************
** Par1=starting pin
** Par2=ending pin (must be higher of starting pin; and maximum 16 pin per command)
** Par3=write value: if 0 (or 1) then assume 0 (or 1) for all the pins in the range;
** Par4=mask (optional): if not present assume to operate in all pins; if present is used as a mask (1=update, 0=do not update).
**            if number of bit lower than number of pins, then padded with 0;
**            if number of bit higher than number of pins, then it's truncated.
**
**  examples:
**  mcpgpioRange,1,8,1: set pins 1 to 8 to 1
**  mcpgpioRange,3,12,1: set pins 3 to 12 to 1
**  mcpgpioRange,5,17,0: set pins 5 to 17 to 0
**  mcpgpioRange,3,12,1,525
**     mask = '0100001101'
**     write pattern after mask = 'x1xxxx11x1' where x indicates that the pin will not be changed
**  mcpgpioRange,3,12,1,973
**     mask = 973 = '1111001101'
**     write pattern after mask = '1111xx11x1' where x indicates that the pin will not be changed
******************************************************************************/
#ifdef USES_P009
const __FlashStringHelper * Command_GPIO_McpGPIORange(struct EventStruct *event, const char *Line)
{
  return mcpgpio_range_pattern_helper(event, Line, false) ? return_command_success() : return_command_failed();
}
#endif

#ifdef USES_P019
const __FlashStringHelper * Command_GPIO_PcfGPIORange(struct EventStruct *event, const char *Line)
{
  return pcfgpio_range_pattern_helper(event, Line, false) ? return_command_success() : return_command_failed();
}
#endif

#ifdef USES_P009
bool mcpgpio_range_pattern_helper(struct EventStruct *event, const char *Line, bool isWritePattern)
{
  range_pattern_helper_data data = range_pattern_helper_shared(PLUGIN_MCP, event, Line, isWritePattern);

  if (!data.valid) {
    return false;
  }

  bool   onLine = false;
  for (uint8_t i = 0; i < data.numBytes; i++) {
    uint8_t readValue;
    const uint8_t    currentVal            = data.initVal + i;
    const uint8_t    currentAddress        = static_cast<int>(currentVal / 2);
    uint8_t          currentMask           = (data.mask  >> (8 * i)) & 0xFF;
    const uint8_t    currentInvertedMask   = 0xFF - currentMask;
    const uint8_t    currentWrite          = (data.write >> (8 * i)) & 0xFF;
    const uint8_t    currentGPIORegister   = ((currentVal % 2) == 0) ? MCP23017_GPIOA : MCP23017_GPIOB;
    const uint8_t    currentIOModeRegister = ((currentVal % 2) == 0) ? MCP23017_IODIRA : MCP23017_IODIRB;
    uint8_t    writeGPIOValue        = 0;

    if (GPIO_MCP_ReadRegister(currentAddress, currentIOModeRegister, &readValue)) {
      // set type to output only for the pins of the mask
      uint8_t writeModeValue = (readValue & currentInvertedMask);
      GPIO_MCP_WriteRegister(currentAddress, currentIOModeRegister, writeModeValue);
      GPIO_MCP_ReadRegister(currentAddress, currentGPIORegister, &readValue);

      // write to port
      writeGPIOValue = (readValue & currentInvertedMask) | (currentWrite & data.mask);
      GPIO_MCP_WriteRegister(currentAddress, currentGPIORegister, writeGPIOValue);

      onLine = true;
    } else {
      onLine = false;
    }

    const uint8_t   mode = (onLine) ? PIN_MODE_OUTPUT : PIN_MODE_OFFLINE;
    for (uint8_t j = 0; currentMask != 0 && j < 8; j++) {
      if (currentMask & 1) {  // only for the pins in the mask
        uint8_t currentPin    = data.firstPin + j + 8 * i;
        const uint32_t key = createKey(PLUGIN_MCP, currentPin);

        // state = onLine ? ((writeGPIOValue & uint8_t(pow(2,j))) >> j) : -1;
        const int8_t state = onLine ? ((writeGPIOValue & (1 << j)) >> j) : -1;

        createAndSetPortStatus_Mode_State(key, mode, state);
        String log;
        log += data.logPrefix;
        log += concat(F(": port#"), static_cast<int>(currentPin));
        log += concat(F(": set to "), static_cast<int>(state));
        addLog(LOG_LEVEL_INFO, log);
        SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);
      }
      currentMask >>= 1; // Shift the mask 1 position
    }
  }
  return onLine;
}
#endif

#ifdef USES_P019
uint8_t getPcfAddress(uint8_t pin)
{
  uint8_t retValue = static_cast<int>((pin - 1) / 8) + 0x20;

  if (retValue > 0x27) { retValue += 0x10; }
  return retValue;
}

bool pcfgpio_range_pattern_helper(struct EventStruct *event, const char *Line, bool isWritePattern)
{
  range_pattern_helper_data data = range_pattern_helper_shared(PLUGIN_PCF, event, Line, isWritePattern);

  if (!data.valid) {
    return false;
  }

  bool   onLine = false;
  String log;

  for (uint8_t i = 0; i < data.numBytes; i++) {
    uint8_t readValue;
    uint8_t    currentAddress = getPcfAddress(event->Par1 + 8 * i);

    uint8_t currentMask         = (data.mask  >> (8 * i)) & 0xFF;
    uint8_t currentInvertedMask = 0xFF - currentMask;
    uint8_t currentWrite        = (data.write >> (8 * i)) & 0xFF;
    uint8_t writeGPIOValue      = 255;

    onLine = GPIO_PCF_ReadAllPins(currentAddress, &readValue);

    if (onLine) { writeGPIOValue = (readValue & currentInvertedMask) | (currentWrite & data.mask); }

    uint8_t   mode = (onLine) ? PIN_MODE_OUTPUT : PIN_MODE_OFFLINE;
    int8_t state;

    for (uint8_t j = 0; j < 8; j++) {
      uint8_t currentPin    = data.firstPin + j + 8 * i;
      const uint32_t key = createKey(PLUGIN_PCF, currentPin);

      if (currentMask & 1) {  // only for the pins in the mask
        state = onLine ? ((writeGPIOValue & (1 << j)) >> j) : -1;

        createAndSetPortStatus_Mode_State(key, mode, state);
        log = concat(data.logPrefix, F(": port#")) + String(currentPin) + String(F(": set to ")) + String(state);
        addLog(LOG_LEVEL_INFO, log);
        SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);
      } else {
        // set to 1 the INPUT pins and the PIN that have not been initialized yet.
        if (!existPortStatus(key) ||
            (existPortStatus(key) &&
             ((globalMapPortStatus[key].mode == PIN_MODE_INPUT) || (globalMapPortStatus[key].mode == PIN_MODE_INPUT_PULLUP)))) {
          readValue |= (1 << j); // set port j = 1
        }
      }
      currentMask >>= 1; // Shift the mask 1 position
    }

    if (onLine) {
      writeGPIOValue = (readValue & currentInvertedMask) | (currentWrite & data.mask);

      // write to port
      GPIO_PCF_WriteAllPins(currentAddress, writeGPIOValue);
    }
  }
  return onLine;
}
#endif

bool setGPIOMode(uint8_t pin, uint8_t mode)
{
  if (!checkValidPortRange(PLUGIN_GPIO, pin)) {
    return false;
  }
  switch (mode) {
    case PIN_MODE_OUTPUT:
      pinMode(pin, OUTPUT);
      break;
    case PIN_MODE_INPUT_PULLUP:
      setInternalGPIOPullupMode(pin);
      break;
    case PIN_MODE_INPUT:
      pinMode(pin, INPUT);
      break;
  }
  return true;
}

#ifdef USES_P009
bool setMCPMode(uint8_t pin, uint8_t mode)
{
  if (checkValidPortRange(PLUGIN_MCP, pin)) {
    switch (mode) {
      case PIN_MODE_OUTPUT:
        setMCPOutputMode(pin);
        break;
      case PIN_MODE_INPUT_PULLUP:
        setMCPInputAndPullupMode(pin, true);
        break;
      case PIN_MODE_INPUT:
        setMCPInputAndPullupMode(pin, false);
        break;
    }
    return true;
  } else {
    return false;
  }
}
#endif

#ifdef USES_P019
bool setPCFMode(uint8_t pin, uint8_t mode)
{
  if (checkValidPortRange(PLUGIN_PCF, pin)) {
    switch (mode) {
      case PIN_MODE_OUTPUT:
        // do nothing
        break;
      case PIN_MODE_INPUT_PULLUP:
      case PIN_MODE_INPUT:
        setPCFInputMode(pin);
        break;
    }
    return true;
  } else {
    return false;
  }
}
#endif

/***********************************************
 * event->Par1: PIN to be set
 * event->Par2: MODE to be set:
 *             0 = OUTPUT
 *             1 = INPUT PULLUP or INPUT PULLDOWN (only for GPIO16)
 *             2 = INPUT
 **********************************************/
const __FlashStringHelper * Command_GPIO_Mode(struct EventStruct *event, const char *Line)
{
  if (gpio_mode_range_helper(event->Par1, event->Par2, event, Line)) {
    return return_command_success();
  }
  return return_command_failed();
}

const __FlashStringHelper * Command_GPIO_ModeRange(struct EventStruct *event, const char *Line)
{
  bool success = true;

  for (uint8_t i = event->Par1; i <= event->Par2; i++) {
    success &= gpio_mode_range_helper(i, event->Par3, event, Line);
  }
  return success ? return_command_success() : return_command_failed();
}

bool gpio_mode_range_helper(uint8_t pin, uint8_t pinMode, struct EventStruct *event, const char *Line)
{
  pluginID_t pluginID = INVALID_PLUGIN_ID;
  bool success = false;

  // Line[0]='g':gpio; ='p':pcfgpio; ='m':mcpgpio
  const __FlashStringHelper * logPrefix = getPluginIDAndPrefix(Line[0], pluginID, success);
  const __FlashStringHelper * logPostfix = F(""); // = new char;

  if (success && checkValidPortRange(pluginID, pin))
  {
    // int8_t state=0;
    uint8_t mode = 255;

    // bool setSuccess=false;

    switch (pinMode) {
      case 0:
        mode       = PIN_MODE_OUTPUT;
        logPostfix = F("OUTPUT");
        break;
      case 1:
        mode       = PIN_MODE_INPUT_PULLUP;
        logPostfix = F("INPUT PULLUP");
        break;
      case 2:
        mode       = PIN_MODE_INPUT;
        logPostfix = F("INPUT");
        break;
    }

    if (mode < 255) {
      switch (pluginID) {
        case PLUGIN_GPIO:
          /* setSuccess = */ setGPIOMode(pin, mode);
          break;
#ifdef USES_P019
        case PLUGIN_PCF:
          // set pin = 1 when INPUT
          /* setSuccess = */ setPCFMode(pin, mode);
          break;
#endif
#ifdef USES_P009
        case PLUGIN_MCP:
          /* setSuccess = */ setMCPMode(pin, mode);
          break;
#endif
        default:
          addLog(LOG_LEVEL_ERROR, F("Plugin not included in build"));
          return false;
      }

      const uint32_t key = createKey(pluginID, pin);

      if (globalMapPortStatus[key].mode != PIN_MODE_OFFLINE)
      {
        int8_t currentState;
        GPIO_Read(pluginID, pin, currentState);

        // state = currentState;

        if (currentState == -1) {
          mode = PIN_MODE_OFFLINE;

          // state = -1;
        }

        createAndSetPortStatus_Mode_State(key, mode, currentState);

        const String log = concat(logPrefix, F(" : port#")) + String(pin) + String(F(": MODE set to ")) + logPostfix + String(F(". Value = ")) +
                     String(currentState);
        addLog(LOG_LEVEL_INFO, log);
        SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);
        return true;
      } else {
        logErrorGpioOffline(logPrefix, pin);
        return false;
      }
    }
    logErrorModeOutOfRange(logPrefix, pin);
    return false;
  }
  logErrorGpioOutOfRange(logPrefix, pin, Line);
  return false;
}

bool getGPIOPinStateValues(String& str) {
  // parseString(string, 1) = device (gpio,mcpgpio,pcfgpio) that can be shortened to g, m or p
  // parseString(string, 2) = command (pinstate,pinrange)
  // parseString(string, 3) = gpio 1st number or a range separated by '-'
  bool   success = false;
  const String device     = parseString(str, 1);
  const String command    = parseString(str, 2);
  const String gpio_descr = parseString(str, 3);

  if ((command.length() >= 8) && command.equalsIgnoreCase(F("pinstate")) && (device.length() > 0)) {
    #ifndef BUILD_NO_DEBUG
    String logPrefix;
    #endif
    // returns pin value using syntax: [plugin#xxxxxxx#pinstate#x]
    int par1;
    const bool validArgument = validIntFromString(gpio_descr, par1);
    #if FEATURE_PINSTATE_EXTENDED
    pluginID_t pluginID = INVALID_PLUGIN_ID;
    #endif // if FEATURE_PINSTATE_EXTENDED

    if (validArgument) {
      switch (device[0]) {
        case 'g':
        {
          #if FEATURE_PINSTATE_EXTENDED
          pluginID  = PLUGIN_GPIO;
          #endif // if FEATURE_PINSTATE_EXTENDED
          str       = digitalRead(par1);
          #ifndef BUILD_NO_DEBUG
          logPrefix = F("GPIO");
          #endif
          success   = true;
          break;
        }

#ifdef USES_P009
        case 'm':
          #if FEATURE_PINSTATE_EXTENDED
          pluginID  = PLUGIN_MCP;
          #endif // if FEATURE_PINSTATE_EXTENDED
          str       = GPIO_MCP_Read(par1);
          #ifndef BUILD_NO_DEBUG
          logPrefix = F("MCP");
          #endif
          success   = true;
          break;
#endif

#ifdef USES_P019
        case 'p':
          #if FEATURE_PINSTATE_EXTENDED
          pluginID  = PLUGIN_PCF;
          #endif // if FEATURE_PINSTATE_EXTENDED
          str       = GPIO_PCF_Read(par1);
          #ifndef BUILD_NO_DEBUG
          logPrefix = F("PCF");
          #endif
          success   = true;
          break;
#endif
        default:
        {
          #if FEATURE_PINSTATE_EXTENDED
          unsigned int plugin = INVALID_PLUGIN_ID;
          if (validUIntFromString(device, plugin) && (plugin != INVALID_PLUGIN_ID)) { // Valid plugin ID?
            pluginID  = plugin;
            #ifndef BUILD_NO_DEBUG
            logPrefix = F("P");
            if (pluginID < 100) { logPrefix += '0'; }
            if (pluginID < 10)  { logPrefix += '0'; }
            logPrefix += pluginID;
            #endif
          } else 
          #endif // if FEATURE_PINSTATE_EXTENDED
          {
            addLog(LOG_LEVEL_ERROR, F("Plugin not included in build"));
            return false;
          }
        }
      }
      #if FEATURE_PINSTATE_EXTENDED
      if (pluginID != INVALID_PLUGIN_ID) {
        const uint32_t key       = createKey(pluginID, par1);
        const auto it            = globalMapPortStatus.find(key);
        const bool notGpioMcpPcf = ((pluginID != PLUGIN_GPIO) && (pluginID != PLUGIN_MCP) && (pluginID != PLUGIN_PCF));

        if (it != globalMapPortStatus.end() && ((it->second.mode == PIN_MODE_PWM) || (it->second.mode == PIN_MODE_SERVO) || notGpioMcpPcf)) {
          // For GPIO/MCP/PCF PWM or SERVO mode get the last set duty cycle or for other plugins get the PWM/SERVO or pinstate
          str     = it->second.getValue();
          success = true;
        }
      }
      #endif // if FEATURE_PINSTATE_EXTENDED
    }

    if (success) {
      #ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, logPrefix + F(" PLUGIN PINSTATE pin =") + String(par1) + F("; value=") + str);
      #endif // ifndef BUILD_NO_DEBUG
    } else {
      addLog(LOG_LEVEL_ERROR, F(" PLUGIN PINSTATE. Syntax error. Pin parameter is not numeric"));
    }
  } else if ((command.length() >= 8) && command.equalsIgnoreCase(F("pinrange"))) {
    // returns pin value using syntax: [plugin#xxxxxxx#pinrange#x-y]
    int  par1, par2;
    bool successPar = false;
    int  dashpos    = gpio_descr.indexOf('-');

    if (dashpos != -1) {
      // Found an extra '-' in the 4th param, will split.
      successPar  = validIntFromString(gpio_descr.substring(dashpos + 1), par2);
      successPar &= validIntFromString(gpio_descr.substring(0, dashpos), par1);
    }

    if (successPar) {
      const __FlashStringHelper * logPrefix = F("");

      switch (device[0]) {
#ifdef USES_P009
        case 'm':
        {
          uint16_t tempValue = 0;
          logPrefix = F("MCP");
          success   = mcpgpio_plugin_range_helper(par1, par2, tempValue);
          str       = String(tempValue);
          break;
        }
#endif

#ifdef USES_P019
        case 'p':
        {
          uint16_t tempValue = 0;
          logPrefix = F("PCF");
          success   = pcfgpio_plugin_range_helper(par1, par2, tempValue);
          str       = String(tempValue);
          break;
        }
          #endif
        default:
          addLog(LOG_LEVEL_ERROR, F("PLUGIN PINSTATE. Plugin not included in build"));
          return false;

      }

      if (success) {
        #ifndef BUILD_NO_DEBUG
        addLogMove(LOG_LEVEL_DEBUG,
               concat(logPrefix, F(" PLUGIN RANGE pin start=")) + String(par1) + 
               concat(F("; pin end="), par2) + 
               concat(F("; value="), str));
        #endif // ifndef BUILD_NO_DEBUG
      } else {
        addLogMove(LOG_LEVEL_ERROR,
               concat(logPrefix,  F(" IS OFFLINE. PLUGIN RANGE pin start=")) + String(par1) + 
               concat(F("; pin end="), par2) +
               concat(F("; value="), str));
      }
    } else {
      addLog(LOG_LEVEL_ERROR, F(" PLUGIN PINRANGE. Syntax error. Pin parameters are not numeric."));
    }
  } else {
    addLog(LOG_LEVEL_ERROR, F("Syntax error. Invalid command. Valid commands are 'pinstate' and 'pinrange'."));
  }

  if (!success) {
    str = '0';
  }
  return success;
}

#ifdef USES_P009
bool mcpgpio_plugin_range_helper(uint8_t pin1, uint8_t pin2, uint16_t& result)
{
  const range_pattern_helper_data data = range_helper_shared(PLUGIN_MCP, pin1, pin2);

  if (!data.valid) {
    return false;
  }

  //  data.logPrefix += F("PluginRead");

  String log;
  bool success = false;
  uint32_t tempResult = 0;

  for (uint8_t i = 0; i < data.numBytes; i++) {
    uint8_t readValue                 = 0;
    const uint8_t currentVal          = data.initVal + i;
    const uint8_t currentAddress      = static_cast<int>(currentVal / 2);
    const uint8_t currentGPIORegister = ((currentVal % 2) == 0) ? MCP23017_GPIOA : MCP23017_GPIOB;

    const bool onLine = GPIO_MCP_ReadRegister(currentAddress, currentGPIORegister, &readValue);

    if (onLine) {
      success = true; // One valid address
      tempResult += (static_cast<uint32_t>(readValue) << (8 * i)); 
    }
  }

  tempResult  = tempResult >> data.deltaStart;
  tempResult &= ((1 << data.numBits) - 1);
  result      = uint16_t(tempResult);

  return success;
}
#endif

#ifdef USES_P019
bool pcfgpio_plugin_range_helper(uint8_t pin1, uint8_t pin2, uint16_t& result)
{
  const range_pattern_helper_data data = range_helper_shared(PLUGIN_PCF, pin1, pin2);

  if (!data.valid) {
    return false;
  }

  //  data.logPrefix += F("PluginRead");

  String log;

  bool success = false;
  uint32_t tempResult = 0;

  for (uint8_t i = 0; i < data.numBytes; i++) {
    uint8_t readValue         = 0;
    const uint8_t currentAddress = getPcfAddress(pin1 + 8 * i);

    const bool onLine = GPIO_PCF_ReadAllPins(currentAddress, &readValue);

    if (onLine) { 
      success = true; // One valid address
      tempResult += (static_cast<uint32_t>(readValue) << (8 * i)); 
    }
  }

  tempResult  = tempResult >> data.deltaStart;
  tempResult &= ((1 << data.numBits) - 1);
  result      = uint16_t(tempResult);

  return success;
}
#endif