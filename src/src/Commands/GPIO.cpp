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


//predeclaration of functions used in this module
void createAndSetPortStatus_Mode_State(uint32_t key, byte newMode, int8_t newState);
bool getPluginIDAndPrefix(char selection, pluginID_t &pluginID, String &logPrefix);
void logErrorGpioOffline(const String& prefix, int port);
void logErrorGpioOutOfRange(const String& prefix, int port, const char* Line = nullptr);
void logErrorGpioNotOutput(const String& prefix, int port);

String Command_GPIO_Monitor(struct EventStruct *event, const char* Line)
{
  String logPrefix;
  pluginID_t pluginID = INVALID_PLUGIN_ID;
  //parseString(Line, 2).charAt(0)='g':gpio; ='p':pcf; ='m':mcp
  bool success = getPluginIDAndPrefix(parseString(Line, 2).charAt(0), pluginID, logPrefix);
  if (success && checkValidPortRange(pluginID, event->Par2))
  {
    const uint32_t key = createKey(pluginID, event->Par2); // WARNING: 'monitor' uses Par2 instead of Par1
    //if (!existPortStatus(key)) globalMapPortStatus[key].mode=PIN_MODE_OUTPUT;
    addMonitorToPort(key);

    int8_t state;
    //giig1967g: Comment next 3 lines to receive an EVENT just after calling the monitor command
    GPIO_Read(pluginID, event->Par2, state);
    globalMapPortStatus[key].state = state;
    if (state == -1) globalMapPortStatus[key].mode=PIN_MODE_OFFLINE;

    String log = logPrefix + String(F(" port #")) + String(event->Par2) + String(F(": added to monitor list."));
    addLog(LOG_LEVEL_INFO, log);
    String dummy;
    SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, dummy, 0);

    return return_command_success();
  } else {
    logErrorGpioOutOfRange(logPrefix,event->Par2, Line);
    return return_command_failed();
  }
}

String Command_GPIO_UnMonitor(struct EventStruct *event, const char* Line)
{
  String logPrefix;
  pluginID_t pluginID = INVALID_PLUGIN_ID;
  //parseString(Line, 2).charAt(0)='g':gpio; ='p':pcf; ='m':mcp
  bool success = getPluginIDAndPrefix(parseString(Line, 2).charAt(0), pluginID, logPrefix);

  if (success && checkValidPortRange(pluginID, event->Par2))
  {
    const uint32_t key = createKey(pluginID, event->Par2); // WARNING: 'monitor' uses Par2 instead of Par1
    String dummy;
    SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, dummy, 0);

    removeMonitorFromPort(key);
    String log = logPrefix + String(F(" port #")) + String(event->Par2) + String(F(": removed from monitor list."));
    addLog(LOG_LEVEL_INFO, log);

    return return_command_success();
  } else {
    logErrorGpioOutOfRange(logPrefix,event->Par2, Line);
    return return_command_failed();
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
    SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);

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
	  SendStatusOnlyIfNeeded(event->Source, sendStatusFlag, key, dummy, 0);
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
    SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);

    // SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, pluginID, event->Par1, log, 0));

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
    SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);

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

    if (existPortStatus(key))
    {
      mode=globalMapPortStatus.at(key).mode;
      state=globalMapPortStatus.at(key).state;
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
      	  SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);

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
        case PLUGIN_PCF:
          // PCF8574/MCP specific: only can read 0/low state, so we must send 1
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
      GPIO_Write(pluginID,event->Par1,state,mode);

  		String log = logPrefix + String(F(" : port#")) + String(event->Par1) + String(F(": set to ")) + String(state);
  		addLog(LOG_LEVEL_INFO, log);
  		SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);
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

  globalMapPortStatus[key].mode     = newMode;
  globalMapPortStatus[key].command  = 1; //set to 1 in order to display the status in the PinStatus page

  //only force events if state has changed
  if (globalMapPortStatus[key].state != newState) {
    globalMapPortStatus[key].state        = newState;
    globalMapPortStatus[key].output       = newState;
    globalMapPortStatus[key].forceEvent   = 1;
    globalMapPortStatus[key].forceMonitor = 1;
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

String Command_GPIO_McpAll(struct EventStruct *event, const char* Line) {
  //Par1=starting pin
  //Par2=ending pin
  //Par3=value
  //Par4=mask (optional)

  if ((event->Par2 < event->Par1) || !checkValidPortRange(PLUGIN_MCP, event->Par1) || !checkValidPortRange(PLUGIN_MCP, event->Par2))
    return return_command_failed();

  String log;
  bool isWriteMask = true;
  if (event->Par3==0 || event->Par3==1) isWriteMask = false;

  uint32_t write = event->Par3;
  uint32_t mask  = event->Par4;  
  
  byte firstPin     = int((event->Par1-1)/8)*8 + 1;
  byte lastPin      = int((event->Par2-1)/8)*8 + 8;
  byte numBytes     = (lastPin - firstPin + 1)/8;
  byte deltaStart   = event->Par1 - firstPin;
  //byte deltaEnd     = lastPin - event->Par2;
  byte numBits      = event->Par2 - event->Par1 + 1;
  byte firstAddress = int((event->Par1 - 1)/16)+0x20; 
  byte firstBank    = (((firstPin-1)/8)+2) % 2;
  byte initVal      = 2 * firstAddress + firstBank;

  if (mask == 0) {
    mask = (1 << numBits) - 1;
    mask = mask << (deltaStart); 
 //   log = String(F("1a. mask==0. mask="))+String(mask);
 //   addLog(LOG_LEVEL_INFO,log);
  } else {
    mask = mask & (byte(pow(256,numBytes))-1);
    mask = mask & (byte(pow(2,numBits))-1);
    mask = mask << deltaStart;
  //  log = String(F("1b. mask<>0. mask="))+String(mask);
  //  addLog(LOG_LEVEL_INFO,log);
  }

  if (isWriteMask) {
    write = write & (byte(pow(256,numBytes))-1);
    write = write & (byte(pow(2,numBits))-1);
    write = write << deltaStart;
  } else {
    write = event->Par3==0 ? 0 : 1;
    if (write > 0) {
      write = (write << numBits) - 1;
      write = write << deltaStart;
    }
  }  
//  log = String(F("1c. write="))+String(write);
//  addLog(LOG_LEVEL_INFO,log);

  bool onLine = false;

  for (byte i=0; i<numBytes; i++) {
    uint8_t readValue;
    byte currentVal = initVal + i;
    byte currentAddress = int(currentVal/2);
    byte currentMask  = (mask  >> (8*i)) & 0xFF;
    byte currentInvertedMask  = 0xFF - currentMask;
    byte currentWrite = (write >> (8*i)) & 0xFF;
    byte currentGPIORegister = ((currentVal % 2)==0) ? MCP23017_GPIOA : MCP23017_GPIOB ;
    byte currentIOModeRegister = ((currentVal % 2)==0) ? MCP23017_IODIRA : MCP23017_IODIRB ;
    byte writeGPIOValue;
/*
    log = String(F("2a. i="))+String(i);
    addLog(LOG_LEVEL_INFO,log);
    log = String(F("2b. currentVal="))+String(currentVal);
    addLog(LOG_LEVEL_INFO,log);
    log = String(F("2c. currentAddress="))+String(currentAddress);
    addLog(LOG_LEVEL_INFO,log);
    log = String(F("2d. currentMask="))+String(currentMask);
    addLog(LOG_LEVEL_INFO,log);
    log = String(F("2e. currentWrite="))+String(currentWrite);
    addLog(LOG_LEVEL_INFO,log);
    log = String(F("2f. currentRegister="))+String(currentGPIORegister);
    addLog(LOG_LEVEL_INFO,log);
*/
    if (GPIO_MCP_ReadRegister(currentAddress,currentIOModeRegister,&readValue)) {
  //    log = String(F("3a. readValue register="))+String(readValue);
  //    addLog(LOG_LEVEL_INFO,log);

      byte writeModeValue = (readValue & currentInvertedMask); //| ((255-mask) & mask);
  //    log = String(F("3b. writeValue="))+String(writeModeValue);
  //    addLog(LOG_LEVEL_INFO,log);

      // set type to output
      GPIO_MCP_WriteRegister(currentAddress,currentIOModeRegister,writeModeValue);

      GPIO_MCP_ReadRegister(currentAddress,currentGPIORegister,&readValue);
  //    log = String(F("4a. readValue register="))+String(readValue);
  //    addLog(LOG_LEVEL_INFO,log);

      writeGPIOValue = (readValue & currentInvertedMask) | (currentWrite & mask);
  //    log = String(F("4b. writeValue="))+String(writeGPIOValue);
  //    addLog(LOG_LEVEL_INFO,log);
      
      // write to port
      GPIO_MCP_WriteRegister(currentAddress,currentGPIORegister,writeGPIOValue);

      onLine=true;
    } else {
      onLine=false;    
    }

    byte mode = (onLine)? PIN_MODE_OUTPUT : PIN_MODE_OFFLINE;
    int8_t state;

    for (byte j=0; j<8; j++) {
//      log=String(F("currentMask="))+String(currentMask)+String(F(". j="))+String(j)+String(F(". 2^j="))+String(2^j)+String(F(". currentMask & (2^j)) >> j="))+String(((currentMask & (2^j)) >> j));
//      addLog(LOG_LEVEL_INFO,log);
      if ((currentMask & byte(pow(2,j))) >> j) { //only for the pins in the mask
        byte currentPin = firstPin + j + 8*i;
        const uint32_t key = createKey(PLUGIN_MCP,currentPin);

        state = onLine ? (writeGPIOValue & byte(pow(2,j)) >> j) : -1;

        createAndSetPortStatus_Mode_State(key,mode,state);
        String log = String(F("MCPALL : port#")) + String(currentPin) + String(F(": set to ")) + String(state);
        addLog(LOG_LEVEL_INFO, log);
        SendStatusOnlyIfNeeded(event->Source, SEARCH_PIN_STATE, key, log, 0);
      }
    }
  }
  return onLine?return_command_success():return_command_failed();
}
