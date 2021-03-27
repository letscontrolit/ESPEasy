#include "../Commands/GPIO.h"


#include "../../ESPEasy-Globals.h"
#include "../../ESPEasy_common.h"
#include "../../ESPEasy_fdwdecl.h"
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


//predeclaration of functions used in this module
void createAndSetPortStatus_Mode_State(uint32_t key, byte newMode, int8_t newState);
bool getPluginIDAndPrefix(char selection, pluginID_t &pluginID, String &logPrefix);
void logErrorGpioOffline(const String& prefix, int port);
void logErrorGpioOutOfRange(const String& prefix, int port, const char* Line = nullptr);
void logErrorGpioNotOutput(const String& prefix, int port);
void logErrorModeOutOfRange(const String& prefix, int port);
bool gpio_monitor_helper(int port, struct EventStruct *event, const char* Line);
bool gpio_unmonitor_helper(int port, struct EventStruct *event, const char* Line);
bool mcpgpio_range_pattern_helper(struct EventStruct *event, const char* Line, bool isWritePattern);
bool pcfgpio_range_pattern_helper(struct EventStruct *event, const char* Line, bool isWritePattern);
bool gpio_mode_range_helper(byte pin, byte pinMode, struct EventStruct *event, const char* Line);
byte getPcfAddress(uint8_t pin);
bool setGPIOMode(byte pin, byte mode);
bool setPCFMode(byte pin, byte mode);
bool setMCPMode(byte pin, byte mode);
bool mcpgpio_plugin_range_helper(byte pin1, byte pin2, uint16_t &result);
bool pcfgpio_plugin_range_helper(byte pin1, byte pin2, uint16_t &result);

/*************************************************************************/

String Command_GPIO_Monitor(struct EventStruct *event, const char* Line)
{
  if (gpio_monitor_helper(event->Par2, event, Line))
    return return_command_success();
  else 
    return return_command_failed();
}

String Command_GPIO_MonitorRange(struct EventStruct *event, const char* Line) 
{
  bool success=true;
  for (byte i=event->Par2;i<=event->Par3;i++) {
    success &= gpio_monitor_helper(i, event, Line);
  }
  return success?return_command_success():return_command_failed();
}

bool gpio_monitor_helper(int port, struct EventStruct *event, const char* Line)
{
  String logPrefix;
  pluginID_t pluginID = INVALID_PLUGIN_ID;
  //parseString(Line, 2).charAt(0)='g':gpio; ='p':pcf; ='m':mcp
  bool success = getPluginIDAndPrefix(parseString(Line, 2).charAt(0), pluginID, logPrefix);
  if (success && checkValidPortRange(pluginID, port))
  {
    const uint32_t key = createKey(pluginID, port); // WARNING: 'monitor' uses Par2 instead of Par1
    //if (!existPortStatus(key)) globalMapPortStatus[key].mode=PIN_MODE_OUTPUT;
    addMonitorToPort(key);

    int8_t state;
    //giig1967g: Comment next 3 lines to receive an EVENT just after calling the monitor command
    GPIO_Read(pluginID, port, state);
    globalMapPortStatus[key].state = state;
    if (state == -1) globalMapPortStatus[key].mode=PIN_MODE_OFFLINE;

    String log = logPrefix + String(F(" port #")) + String(port) + String(F(": added to monitor list."));
    addLog(LOG_LEVEL_INFO, log);
    String dummy;
    SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, dummy, 0);

    return true;
  } else {
    logErrorGpioOutOfRange(logPrefix,port, Line);
    return false;
  }
}

String Command_GPIO_UnMonitor(struct EventStruct *event, const char* Line)
{
  if (gpio_unmonitor_helper(event->Par2, event, Line))
    return return_command_success();
  else 
    return return_command_failed();
}

String Command_GPIO_UnMonitorRange(struct EventStruct *event, const char* Line) 
{
  bool success=true;
  for (byte i=event->Par2;i<=event->Par3;i++) {
    success &= gpio_unmonitor_helper(i, event, Line);
  }
  return success?return_command_success():return_command_failed();
}

bool gpio_unmonitor_helper(int port, struct EventStruct *event, const char* Line)
{
  String logPrefix;
  pluginID_t pluginID = INVALID_PLUGIN_ID;
  //parseString(Line, 2).charAt(0)='g':gpio; ='p':pcf; ='m':mcp
  bool success = getPluginIDAndPrefix(parseString(Line, 2).charAt(0), pluginID, logPrefix);

  if (success && checkValidPortRange(pluginID, port))
  {
    const uint32_t key = createKey(pluginID, port); // WARNING: 'monitor' uses Par2 instead of Par1
    String dummy;
    SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, dummy, 0);

    removeMonitorFromPort(key);
    String log = logPrefix + String(F(" port #")) + String(port) + String(F(": removed from monitor list."));
    addLog(LOG_LEVEL_INFO, log);

    return true;
  } else {
    logErrorGpioOutOfRange(logPrefix, port, Line);
    return false;
  }
}

String Command_GPIO_LongPulse(struct EventStruct *event, const char* Line)
{
  event->Par3 = event->Par3 * 1000;
  return Command_GPIO_LongPulse_Ms( event, Line);
}

String Command_GPIO_LongPulse_Ms(struct EventStruct *event, const char* Line)
{
  String logPrefix;// = ;
  pluginID_t pluginID=INVALID_PLUGIN_ID;
  //Line[0]='l':longpulse; ='p':pcflongpulse; ='m':mcplongpulse
  bool success = getPluginIDAndPrefix(Line[0], pluginID, logPrefix);
  if (success && checkValidPortRange(pluginID, event->Par1))
  {
    const uint32_t key = createKey(pluginID,event->Par1);
    createAndSetPortStatus_Mode_State(key,PIN_MODE_OUTPUT,event->Par2);
    GPIO_Write(pluginID, event->Par1, event->Par2);

    Scheduler.setGPIOTimer(event->Par3, pluginID, event->Par1, !event->Par2);

    String log = logPrefix + String(F(" : port ")) + String(event->Par1);
    log += String(F(". Pulse set for ")) + String(event->Par3)+String(F(" ms"));
    addLog(LOG_LEVEL_INFO, log);
    SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);

    return return_command_success();
  } else {
    logErrorGpioOutOfRange(logPrefix,event->Par1, Line);
    return return_command_failed();
  }
}

String Command_GPIO_Status(struct EventStruct *event, const char* Line)
{
	bool success = true;
  bool sendStatusFlag;
  byte pluginID;

  switch (tolower(parseString(Line, 2).charAt(0)))
  {
    case 'g': //gpio
      pluginID=PLUGIN_GPIO;
	    sendStatusFlag = true;
      break;
    case 'm': //mcp
      pluginID=PLUGIN_MCP;
	    sendStatusFlag = GPIO_MCP_Read(event->Par2)==-1;
      break;
    case 'p': //pcf
      pluginID=PLUGIN_PCF;
	    sendStatusFlag = GPIO_PCF_Read(event->Par2)==-1;
      break;
    default:
      success=false;
  }

  if (success && checkValidPortRange(pluginID, event->Par2))
  {
    const uint32_t key = createKey(pluginID, event->Par2); // WARNING: 'status' uses Par2 instead of Par1
	  String dummy;
	  SendStatusOnlyIfNeeded(event, sendStatusFlag, key, dummy, 0);
    return return_command_success();
  } else {
    return return_command_failed();
  }
}

String Command_GPIO_PWM(struct EventStruct *event, const char *Line)
{
  // Par1: GPIO
  // Par2: Duty Cycle
  // Par3: Fade duration
  // Par4: Frequency

  // For now, we only support the internal GPIO pins.
  String logPrefix = F("GPIO");
  uint32_t frequency = event->Par4;
  uint32_t key = 0;
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

String Command_GPIO_Tone(struct EventStruct *event, const char* Line)
{
  // play a tone on pin par1, with frequency par2 and duration in msec par3.
  unsigned long duration = event->Par3;
  bool mustScheduleToneOff = false;
  if (duration > 50) {
    duration = 0;
    mustScheduleToneOff = true;
  }
  if (tone_espEasy(event->Par1, event->Par2, duration)) {
    if (mustScheduleToneOff) {
      // For now, we only support the internal GPIO pins.
      byte   pluginID  = PLUGIN_GPIO;
      Scheduler.setGPIOTimer(event->Par3, pluginID, event->Par1, 0);
    }
    return return_command_success();
  }
  return return_command_failed();
}


String Command_GPIO_RTTTL(struct EventStruct *event, const char* Line)
{
  #ifdef USE_RTTTL
  // FIXME: Absolutely no error checking in play_rtttl, until then keep it only in testing
  // play a tune via a RTTTL string, look at https://www.letscontrolit.com/forum/viewtopic.php?f=4&t=343&hilit=speaker&start=10 for
  // more info.

  String melody = parseStringToEndKeepCase(Line, 2);
  melody.replace('-', '#');
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("RTTTL : pin: ");
    log += event->Par1;
    log += F(" melody: ");
    log += melody;
    addLog(LOG_LEVEL_INFO, log);
  }
  if (play_rtttl(event->Par1, melody.c_str())) {
    return return_command_success();
  }
  #else 
  addLog(LOG_LEVEL_ERROR, F("RTTTL : command not included in build"));
  #endif
  return return_command_failed();
}

String Command_GPIO_Pulse(struct EventStruct *event, const char* Line)
{
  String logPrefix;
  bool success = false;
  byte pluginID=INVALID_PLUGIN_ID;
  switch (tolower(Line[0]))
  {
    case 'p': // pulse or pcfpulse
      if (tolower(Line[1])=='u') { //pulse
        pluginID=PLUGIN_GPIO;
        logPrefix=String(F("GPIO"));
        success=true;
      } else if (tolower(Line[1])=='c'){ //pcfpulse
        pluginID=PLUGIN_PCF;
        logPrefix=String(F("PCF"));
        success=true;
      }
      break;
    case 'm': //mcp
      pluginID=PLUGIN_MCP;
      logPrefix=String(F("MCP"));
      success=true;
      break;
  }

  if (success && checkValidPortRange(pluginID, event->Par1))
  {
    const uint32_t key = createKey(pluginID,event->Par1);

    createAndSetPortStatus_Mode_State(key,PIN_MODE_OUTPUT,event->Par2);
    GPIO_Write(pluginID, event->Par1, event->Par2);

    delay(event->Par3);

    createAndSetPortStatus_Mode_State(key,PIN_MODE_OUTPUT,!event->Par2);
    GPIO_Write(pluginID, event->Par1, !event->Par2);

    String log = logPrefix + String(F(" : port ")) + String(event->Par1);
    log += String(F(". Pulse set for ")) + String(event->Par3)+String(F(" ms"));
    addLog(LOG_LEVEL_INFO, log);
    SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);

    return return_command_success();
  } else {
    logErrorGpioOutOfRange(logPrefix,event->Par1, Line);
    return return_command_failed();
  }
}

String Command_GPIO_Toggle(struct EventStruct *event, const char* Line)
{
  String logPrefix;
  pluginID_t pluginID=INVALID_PLUGIN_ID;
  //Line[0]='g':gpiotoggle; ='p':pcfgpiotoggle; ='m':mcpgpiotoggle
  bool success = getPluginIDAndPrefix(Line[0], pluginID, logPrefix);
  if (success && checkValidPortRange(pluginID, event->Par1))
  {
    const uint32_t key = createKey(pluginID,event->Par1);
    // WARNING: operator [] creates an entry in the map if key does not exist
    // So the next command should be part of each command:
    byte mode;
    int8_t state;

    auto it = globalMapPortStatus.find(key);
    if (it != globalMapPortStatus.end()) {
      mode  = it->second.mode;
      state = it->second.state;
    } else {
      GPIO_Read(pluginID, event->Par1, state);
      mode = (state==-1)?PIN_MODE_OFFLINE:PIN_MODE_OUTPUT;
    }

    switch (mode) {
      case PIN_MODE_OUTPUT:
      case PIN_MODE_UNDEFINED:
        {
          createAndSetPortStatus_Mode_State(key,PIN_MODE_OUTPUT,!state);
          GPIO_Write(pluginID, event->Par1, !state);

          String log = logPrefix + String(F(" toggle: port#")) + String(event->Par1) + String(F(": set to ")) + String(!state);
          addLog(LOG_LEVEL_ERROR, log);
      	  SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);

          return return_command_success();
        }
        break;
      case PIN_MODE_OFFLINE:
        logErrorGpioOffline(logPrefix,event->Par1);
        return return_command_failed();
        break;
      default:
        logErrorGpioNotOutput(logPrefix,event->Par1);
        return return_command_failed();
        break;
    }
  } else {
    logErrorGpioOutOfRange(logPrefix,event->Par1, Line);
    return return_command_failed();
  }
}

String Command_GPIO(struct EventStruct *event, const char* Line)
{
  String logPrefix;// = new char;
  pluginID_t pluginID=INVALID_PLUGIN_ID;
  //Line[0]='g':gpio; ='p':pcfgpio; ='m':mcpgpio
  bool success = getPluginIDAndPrefix(Line[0], pluginID, logPrefix);
  if (success && checkValidPortRange(pluginID, event->Par1))
  {
	  int8_t state=0;
	  byte mode;

	  if (event->Par2 == 2) { //INPUT
		  mode = PIN_MODE_INPUT_PULLUP;
      switch (pluginID) {
        case PLUGIN_GPIO:
          setInternalGPIOPullupMode(event->Par1);
          state = GPIO_Read_Switch_State(event->Par1, PIN_MODE_INPUT_PULLUP);
          break;
        case PLUGIN_MCP:
          setMCPInputAndPullupMode(event->Par1,true);
          GPIO_Read(PLUGIN_MCP, event->Par1, state);
          break;
        case PLUGIN_PCF:
          // PCF8574 specific: only can read 0/low state, so we must send 1
          state = 1;
          break;
		  }
    } else { // OUTPUT
      mode=PIN_MODE_OUTPUT;
      state=(event->Par2==0)?0:1;
    }

    const uint32_t key = createKey(pluginID,event->Par1);

    if (globalMapPortStatus[key].mode != PIN_MODE_OFFLINE)
    {
      int8_t currentState;
      GPIO_Read(pluginID, event->Par1, currentState);
      if (currentState==-1) {
        mode=PIN_MODE_OFFLINE;
        state = -1;
      }

      createAndSetPortStatus_Mode_State(key,mode,state);
      if (mode==PIN_MODE_OUTPUT || pluginID==PLUGIN_PCF) GPIO_Write(pluginID,event->Par1,state,mode);

  		String log = logPrefix + String(F(" : port#")) + String(event->Par1) + String(F(": set to ")) + String(state);
  		addLog(LOG_LEVEL_INFO, log);
  		SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);
  		return return_command_success();
  	} else {
      logErrorGpioOffline(logPrefix,event->Par1);
      return return_command_failed();
    }
  } else {
    logErrorGpioOutOfRange(logPrefix,event->Par1, Line);
    return return_command_failed();
  }
}

void logErrorGpio(const String& prefix, int port, const String& description)
{
  if (port >= 0) {
    addLog(LOG_LEVEL_ERROR, prefix + String(F(" : port#")) + String(port) + description);
  }
}

void logErrorModeOutOfRange(const String& prefix, int port)
{
  logErrorGpio(prefix, port, F(" mode selection is incorrect. Valid values are: 0, 1 or 2."));
}

void logErrorGpioOffline(const String& prefix, int port)
{
  logErrorGpio(prefix, port, F(" is offline."));
}

void logErrorGpioOutOfRange(const String& prefix, int port, const char* Line)
{
  logErrorGpio(prefix, port, F(" is out of range"));
  if (port >= 0) {
    if (Line != nullptr) {
      addLog(LOG_LEVEL_DEBUG, Line);
    }
  }
}

void logErrorGpioNotOutput(const String& prefix, int port)
{
  logErrorGpio(prefix, port, F(" is not an output port"));
}

void createAndSetPortStatus_Mode_State(uint32_t key, byte newMode, int8_t newState)
{
  // WARNING: operator [] creates an entry in the map if key does not exist

  // If it doesn't exist, it is now created.
  globalMapPortStatus[key].mode     = newMode;
  auto it = globalMapPortStatus.find(key);
  if (it != globalMapPortStatus.end()) {
    // Should always be true, as it would be created if it didn't exist.
    it->second.command  = 1; //set to 1 in order to display the status in the PinStatus page
    //only force events if state has changed
    if (it->second.state != newState) {
      it->second.state        = newState;
      it->second.output       = newState;
      it->second.forceEvent   = 1;
      it->second.forceMonitor = 1;
    }
  }
}

bool getPluginIDAndPrefix(char selection, pluginID_t &pluginID, String &logPrefix)
{
  bool success = true;
  switch(tolower(selection))
  {
    case 'g': //gpio
    case 'l': //longpulse (gpio)
      pluginID=PLUGIN_GPIO;
      logPrefix=F("GPIO");
      break;
    case 'm': //mcp & mcplongpulse
      pluginID=PLUGIN_MCP;
      logPrefix=F("MCP");
      break;
    case 'p': //pcf & pcflongpulse
      pluginID=PLUGIN_PCF;
      logPrefix=F("PCF");
      break;
    default:
      logPrefix=F("PluginID out of range. Error");
      success=false;
  }
  return success;
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
String Command_GPIO_McpGPIOPattern(struct EventStruct *event, const char* Line) 
{
  return mcpgpio_range_pattern_helper(event, Line, true)?return_command_success():return_command_failed();
}

String Command_GPIO_PcfGPIOPattern(struct EventStruct *event, const char* Line) 
{
  return pcfgpio_range_pattern_helper(event, Line, true)?return_command_success():return_command_failed();
}


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
String Command_GPIO_McpGPIORange(struct EventStruct *event, const char* Line) 
{
  return mcpgpio_range_pattern_helper(event, Line, false)?return_command_success():return_command_failed();
}

String Command_GPIO_PcfGPIORange(struct EventStruct *event, const char* Line) 
{
  return pcfgpio_range_pattern_helper(event, Line, false)?return_command_success():return_command_failed();
}

bool mcpgpio_range_pattern_helper(struct EventStruct *event, const char* Line, bool isWritePattern) 
{
  String log;
  String logPrefix = isWritePattern?String(F("McpGPIOPattern")):String(F("McpGPIORange"));

  if ((event->Par2 < event->Par1) || !checkValidPortRange(PLUGIN_MCP, event->Par1) || !checkValidPortRange(PLUGIN_MCP, event->Par2) || (event->Par2 - event->Par1 + 1)>16 ) {
    log=logPrefix + String(F(": pin numbers out of range."));
    addLog(LOG_LEVEL_INFO,log);
    return false;
  }
  
  bool isMask = (parseString(Line, 5) == "")? false : true;

  uint32_t write;
  uint32_t mask;  
  
  byte firstPin     = int((event->Par1-1)/8)*8 + 1;
  byte lastPin      = int((event->Par2-1)/8)*8 + 8;
  byte numBytes     = (lastPin - firstPin + 1)/8;
  byte deltaStart   = event->Par1 - firstPin;
  byte numBits      = event->Par2 - event->Par1 + 1;
  byte firstAddress = int((event->Par1 - 1)/16)+0x20; 
  byte firstBank    = (((firstPin-1)/8)+2) % 2;
  byte initVal      = 2 * firstAddress + firstBank;
  bool onLine       = false;

  if (isMask) {
    mask = event->Par4 & ((1 << numBytes*8)-1);
    //mask &= (byte(pow(2,numBits))-1);
    mask &= ((1 << numBits)-1);
    mask = mask << deltaStart;
  } else {
    mask = (1 << numBits) - 1;
    mask = mask << (deltaStart); 
  }

  if (isWritePattern) { //write pattern is present
    write = event->Par3 & ((1 << numBytes*8)-1); //limit number of bytes
    write &= ((1 << numBits)-1); //limit to number of bits
    write = write << deltaStart; //shift to start from starting pin
  } else { //write pattern not present
    if (event->Par3 == 0) {
      write = 0;
    } else if (event->Par3 == 1) {
      write = (1 << numBits) - 1;
      write = write << deltaStart;
    } else {
      log=logPrefix + String(F(": Write value must be 0 or 1."));
      addLog(LOG_LEVEL_INFO,log);
      return false;
    }
  }  

  for (byte i=0; i<numBytes; i++) {
    uint8_t readValue;
    byte currentVal = initVal + i;
    byte currentAddress = int(currentVal/2);
    byte currentMask  = (mask  >> (8*i)) & 0xFF;
    byte currentInvertedMask  = 0xFF - currentMask;
    byte currentWrite = (write >> (8*i)) & 0xFF;
    byte currentGPIORegister = ((currentVal % 2)==0) ? MCP23017_GPIOA : MCP23017_GPIOB ;
    byte currentIOModeRegister = ((currentVal % 2)==0) ? MCP23017_IODIRA : MCP23017_IODIRB ;
    byte writeGPIOValue=0;

    if (GPIO_MCP_ReadRegister(currentAddress,currentIOModeRegister,&readValue)) {
      // set type to output only for the pins of the mask
      byte writeModeValue = (readValue & currentInvertedMask);
      GPIO_MCP_WriteRegister(currentAddress,currentIOModeRegister,writeModeValue);
      GPIO_MCP_ReadRegister(currentAddress,currentGPIORegister,&readValue);
     
      // write to port
      writeGPIOValue = (readValue & currentInvertedMask) | (currentWrite & mask);
      GPIO_MCP_WriteRegister(currentAddress,currentGPIORegister,writeGPIOValue);

      onLine=true;
    } else {
      onLine=false;    
    }

    byte mode = (onLine)? PIN_MODE_OUTPUT : PIN_MODE_OFFLINE;
    int8_t state;

    for (byte j=0; j<8; j++) {
      //if ((currentMask & (byte(pow(2,j)))) >> j) { //only for the pins in the mask
      if ((currentMask & (1 << j)) >> j) { //only for the pins in the mask
        byte currentPin = firstPin + j + 8*i;
        const uint32_t key = createKey(PLUGIN_MCP,currentPin);

        //state = onLine ? ((writeGPIOValue & byte(pow(2,j))) >> j) : -1;
        state = onLine ? ((writeGPIOValue & (1 << j)) >> j) : -1;

        createAndSetPortStatus_Mode_State(key,mode,state);
        log = logPrefix + String(F(": port#")) + String(currentPin) + String(F(": set to ")) + String(state);
        addLog(LOG_LEVEL_INFO, log);
        SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);
      }
    }
  }
  return onLine;
}

byte getPcfAddress(uint8_t pin)
{
  byte retValue=int((pin - 1)/8)+0x20;
  if (retValue>0x27) retValue+=0x10;
  return retValue;
}

bool pcfgpio_range_pattern_helper(struct EventStruct *event, const char* Line, bool isWritePattern) 
{
  String log;
  String logPrefix = isWritePattern?String(F("PcfGPIOPattern")):String(F("PcfGPIORange"));

  if ((event->Par2 < event->Par1) || !checkValidPortRange(PLUGIN_PCF, event->Par1) || !checkValidPortRange(PLUGIN_PCF, event->Par2) || (event->Par2 - event->Par1 + 1)>16 ) {
    log=logPrefix + String(F(": pin numbers out of range."));
    addLog(LOG_LEVEL_INFO,log);
    return false;
  }
  
  bool isMask = (parseString(Line, 5) == "")? false : true;

  uint32_t write;
  uint32_t mask;  
  
  byte firstPin     = int((event->Par1-1)/8)*8 + 1;
  byte lastPin      = int((event->Par2-1)/8)*8 + 8;
  byte numBytes     = (lastPin - firstPin + 1)/8;
  byte deltaStart   = event->Par1 - firstPin;
  byte numBits      = event->Par2 - event->Par1 + 1;
  
  if (isMask) {
//    mask = event->Par4 & (byte(pow(256,numBytes))-1);
    mask = event->Par4 & ((1 << numBytes*8)-1);
    mask &= ((1 << numBits)-1);
    mask = mask << deltaStart;
  } else {
    mask = (1 << numBits) - 1;
    mask = mask << (deltaStart); 
  }

  if (isWritePattern) { //write pattern is present
    write = event->Par3 & ((1 << numBytes*8)-1); //limit number of bytes
    write &= ((1 << numBits)-1); //limit to number of bits
    write = write << deltaStart; //shift to start from starting pin
  } else { //write pattern not present
    if (event->Par3 == 0) {
      write = 0;
    } else if (event->Par3 == 1) {
      write = (1 << numBits) - 1;
      write = write << deltaStart;
    } else {
      log=logPrefix + String(F(": Write value must be 0 or 1."));
      addLog(LOG_LEVEL_INFO,log);
      return false;
    }
  }  

  bool onLine=false;

  for (byte i=0; i<numBytes; i++) {
    uint8_t readValue;
    byte currentAddress = getPcfAddress(event->Par1+8*i);

    byte currentMask  = (mask  >> (8*i)) & 0xFF;
    byte currentInvertedMask  = 0xFF - currentMask;
    byte currentWrite = (write >> (8*i)) & 0xFF;
    byte writeGPIOValue=255;

    onLine = GPIO_PCF_ReadAllPins(currentAddress,&readValue);
    if (onLine) writeGPIOValue = (readValue & currentInvertedMask) | (currentWrite & mask);

    byte mode = (onLine)? PIN_MODE_OUTPUT : PIN_MODE_OFFLINE;
    int8_t state;

    for (byte j=0; j<8; j++) {
      byte currentPin = firstPin + j + 8*i;
      const uint32_t key = createKey(PLUGIN_PCF,currentPin);

      if ((currentMask & (1 << j)) >> j) { //only for the pins in the mask
        state = onLine ? ((writeGPIOValue & (1 << j) ) >> j) : -1;

        createAndSetPortStatus_Mode_State(key,mode,state);
        log = logPrefix + String(F(": port#")) + String(currentPin) + String(F(": set to ")) + String(state);
        addLog(LOG_LEVEL_INFO, log);
        SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);
      } else {
        //set to 1 the INPUT pins and the PIN that have not been initialized yet.
        if (!existPortStatus(key) || (existPortStatus(key) && (globalMapPortStatus[key].mode == PIN_MODE_INPUT || globalMapPortStatus[key].mode == PIN_MODE_INPUT_PULLUP)))
          readValue |= (1 << j); //set port j = 1   
      }
    }
    if (onLine) {
      writeGPIOValue = (readValue & currentInvertedMask) | (currentWrite & mask);        
      // write to port
      GPIO_PCF_WriteAllPins(currentAddress,writeGPIOValue);
    }
  }
  return onLine;
}

bool setGPIOMode(byte pin, byte mode) 
{
  if (checkValidPortRange(PLUGIN_GPIO, pin)) {
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
  } else 
    return false;
}

bool setMCPMode(byte pin, byte mode) 
{
  if (checkValidPortRange(PLUGIN_MCP, pin)) {
    switch (mode) {
      case PIN_MODE_OUTPUT:
        setMCPOutputMode(pin); 
        break;
      case PIN_MODE_INPUT_PULLUP:
        setMCPInputAndPullupMode(pin,true);
        break;
      case PIN_MODE_INPUT:
        setMCPInputAndPullupMode(pin,false);
        break;
    }
    return true;
  } else 
    return false;
}

bool setPCFMode(byte pin, byte mode) 
{
  if (checkValidPortRange(PLUGIN_PCF, pin)) {
    switch (mode) {
      case PIN_MODE_OUTPUT:
      //do nothing
        break;
      case PIN_MODE_INPUT_PULLUP:
      case PIN_MODE_INPUT:
        setPCFInputMode(pin); 
        break;
    }
    return true;
  } else 
    return false;
}

/***********************************************
 *event->Par1: PIN to be set
 *event->Par2: MODE to be set:
 *             0 = OUTPUT
 *             1 = INPUT PULLUP or INPUT PULLDOWN (only for GPIO16)
 *             2 = INPUT
 **********************************************/
String Command_GPIO_Mode(struct EventStruct *event, const char* Line)
{
  if (gpio_mode_range_helper(event->Par1, event->Par2, event, Line))
    return return_command_success();
  else 
    return return_command_failed();
}

String Command_GPIO_ModeRange(struct EventStruct *event, const char* Line)
{
  bool success=true;
  for (byte i=event->Par1;i<=event->Par2;i++) {
    success &= gpio_mode_range_helper(i, event->Par3, event, Line);
  }
  return success?return_command_success():return_command_failed();
}

bool gpio_mode_range_helper(byte pin, byte pinMode, struct EventStruct *event, const char* Line)
{
  String logPrefix;// = new char;
  String logPostfix;// = new char;
  pluginID_t pluginID=INVALID_PLUGIN_ID;
  //Line[0]='g':gpio; ='p':pcfgpio; ='m':mcpgpio
  bool success = getPluginIDAndPrefix(Line[0], pluginID, logPrefix);

  if (success && checkValidPortRange(pluginID, pin))
  {
	  //int8_t state=0;
	  byte mode=255;
    //bool setSuccess=false;

    switch (pinMode) {
      case 0:
        mode = PIN_MODE_OUTPUT;
        logPostfix = F("OUTPUT");
        break;
      case 1:
        mode = PIN_MODE_INPUT_PULLUP;
        logPostfix = F("INPUT PULLUP");
        break;
      case 2:
        mode = PIN_MODE_INPUT;
        logPostfix = F("INPUT");
        break;
    }
    
    if (mode < 255) { 
      switch(pluginID) {
        case PLUGIN_GPIO:
          /* setSuccess = */ setGPIOMode(pin, mode);
          break;
        case PLUGIN_PCF:
          //set pin = 1 when INPUT
          /* setSuccess = */ setPCFMode(pin, mode);
          break;
        case PLUGIN_MCP:
          /* setSuccess = */ setMCPMode(pin, mode);
          break;
      }

      const uint32_t key = createKey(pluginID,pin);

      if (globalMapPortStatus[key].mode != PIN_MODE_OFFLINE)
      {
        int8_t currentState;
        GPIO_Read(pluginID, pin, currentState);
        //state = currentState;

        if (currentState==-1) {
          mode=PIN_MODE_OFFLINE;
          //state = -1;
        }

        createAndSetPortStatus_Mode_State(key,mode,currentState);
        
        String log = logPrefix + String(F(" : port#")) + String(pin) + String(F(": MODE set to ")) + logPostfix + String(F(". Value = ")) + String(currentState);
        addLog(LOG_LEVEL_INFO, log);
        SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);
        return return_command_success();
      } else {
        logErrorGpioOffline(logPrefix,pin);
        return return_command_failed();
      }
    } else {
      logErrorModeOutOfRange(logPrefix,pin);
      return return_command_failed();
    }
  } else {
    logErrorGpioOutOfRange(logPrefix,pin, Line);
    return return_command_failed();
  }
}

bool getGPIOPinStateValues(String& str) {
  // parseString(string, 1) = device (gpio,mcpgpio,pcfgpio) that can be shortened to g, m or p
  // parseString(string, 2) = command (pinstate,pinrange)
  // parseString(string, 3) = gpio 1st number or a range separated by '-'
  bool success=false;
  String logPrefix="";

  if ((parseString(str, 2).length() >= 8) && parseString(str, 2).equalsIgnoreCase(F("pinstate"))) {
    // returns pin value using syntax: [plugin#xxxxxxx#pinstate#x]
    int par1;
    switch (parseString(str, 1)[0]) {
      case 'g':
        if (validIntFromString(parseString(str, 3), par1)) {
          str = String(digitalRead(par1));
          logPrefix="GPIO";
          success=true;
        }
        break;

      case 'm':
        if (validIntFromString(parseString(str, 3), par1)) {
          str = String(GPIO_MCP_Read(par1));
          logPrefix="MCP";
          success=true;
        }
        break;

      case 'p':
        if (validIntFromString(parseString(str, 3), par1)) {
          str = GPIO_PCF_Read(par1);
          logPrefix="PCF";
          success=true;
        }
        break;
    } 
    if (success) {
      addLog(LOG_LEVEL_DEBUG,logPrefix+String(F(" PLUGIN PINSTATE pin ="))+String(par1)+String(F("; value="))+str);
    } else {
      str="0";
      addLog(LOG_LEVEL_INFO,logPrefix+String(F(" PLUGIN PINSTATE. Syntax error. Pin parameter is not numeric")));
    }
  } else if ((parseString(str, 2).length() >= 8) && parseString(str, 2).equalsIgnoreCase(F("pinrange"))) {
    // returns pin value using syntax: [plugin#xxxxxxx#pinrange#x-y]
    int par1,par2;
    bool successPar = false;
    int dashpos = parseString(str, 3).indexOf('-');

    if (dashpos != -1) {
      // Found an extra '-' in the 4th param, will split.
      successPar = validIntFromString(parseString(str, 3).substring(dashpos + 1),par2);
      successPar &= validIntFromString(parseString(str, 3).substring(0, dashpos),par1);
    } 

    if (successPar) {
      uint16_t tempValue = 0;
      switch (parseString(str, 1)[0]) {
        case 'm':
          logPrefix="MCP";
          success=mcpgpio_plugin_range_helper(par1,par2,tempValue);
          str = String(tempValue);
          break;
          
        case 'p':
          logPrefix="PCF";
          success=mcpgpio_plugin_range_helper(par1,par2,tempValue);
          str = String(tempValue);
          break;
      } 
      if (success) {
        addLog(LOG_LEVEL_DEBUG,logPrefix+String(F(" PLUGIN RANGE pin start="))+String(par1)+String(F("; pin end="))+String(par2)+String(F("; value="))+str);
      } else {
        str="0";
        addLog(LOG_LEVEL_INFO,logPrefix+String(F(" IS OFFLINE. PLUGIN RANGE pin start="))+String(par1)+String(F("; pin end="))+String(par2)+String(F("; value="))+str);
      }
    } else {
      str="0";
      addLog(LOG_LEVEL_INFO,logPrefix+String(F(" PLUGIN PINRANGE. Syntax error. Pin parameters are not numeric.")));
    }
  } else {
    str="0";
    addLog(LOG_LEVEL_INFO,String(F("Syntax error. Invalid command. Valid commands are 'pinstate' and 'pinrange'.")));
  }
  return success;
}

bool mcpgpio_plugin_range_helper(byte pin1, byte pin2, uint16_t &result) 
{
  String log;
  String logPrefix = String(F("McpPluginRead"));

  if ((pin2 < pin1) || !checkValidPortRange(PLUGIN_MCP, pin1) || !checkValidPortRange(PLUGIN_MCP, pin2) || (pin2 - pin1 + 1)>16 ) {
    log=logPrefix + String(F(": pin numbers out of range."));
    addLog(LOG_LEVEL_INFO,log);
    return false;
  }
    
  byte firstPin     = int((pin1-1)/8)*8 + 1;
  byte lastPin      = int((pin2-1)/8)*8 + 8;
  byte numBytes     = (lastPin - firstPin + 1)/8;
  byte deltaStart   = pin1 - firstPin;
  byte numBits      = pin2 - pin1 + 1;
  byte firstAddress = int((pin1 - 1)/16)+0x20; 
  byte firstBank    = (((firstPin-1)/8)+2) % 2;
  byte initVal      = 2 * firstAddress + firstBank;
  bool onLine       = false;

  uint32_t tempResult=0;

  for (byte i=0; i<numBytes; i++) {
    uint8_t readValue;
    byte currentVal = initVal + i;
    byte currentAddress = int(currentVal/2);
    byte currentGPIORegister = ((currentVal % 2)==0) ? MCP23017_GPIOA : MCP23017_GPIOB ;

    onLine = GPIO_MCP_ReadRegister(currentAddress,currentGPIORegister,&readValue);
    if (onLine) tempResult += (readValue << (8*i));
  }

  tempResult = tempResult >> deltaStart;
  tempResult &= ((1 << numBits)-1);
  result = uint16_t(tempResult);

  return onLine;
}

bool pcfgpio_plugin_range_helper(byte pin1, byte pin2, uint16_t &result)
{
  String log;
  String logPrefix = String(F("PcfPluginRead"));

  if ((pin2 < pin1) || !checkValidPortRange(PLUGIN_PCF, pin1) || !checkValidPortRange(PLUGIN_PCF, pin2) || (pin2 - pin1 + 1)>16 ) {
    log=logPrefix + String(F(": pin numbers out of range."));
    addLog(LOG_LEVEL_INFO,log);
    return false;
  }
  
  byte firstPin     = int((pin1-1)/8)*8 + 1;
  byte lastPin      = int((pin2-1)/8)*8 + 8;
  byte numBytes     = (lastPin - firstPin + 1)/8;
  byte deltaStart   = pin1 - firstPin;
  byte numBits      = pin2 - pin1 + 1;
  bool onLine=false;

  uint32_t tempResult=0;

  for (byte i=0; i<numBytes; i++) {
    uint8_t readValue;
    byte currentAddress = getPcfAddress(pin1+8*i);
    
    onLine = GPIO_PCF_ReadAllPins(currentAddress,&readValue);
    if (onLine) tempResult += (readValue << (8*i));
  }

  tempResult = tempResult >> deltaStart;
  tempResult &= ((1 << numBits)-1);
  result = uint16_t(tempResult);

  return onLine;
}
