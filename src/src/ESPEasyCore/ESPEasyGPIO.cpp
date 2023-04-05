
#include "../ESPEasyCore/ESPEasyGPIO.h"

/****************************************************************************/
// Central functions for GPIO handling
// **************************************************************************/
#include "../../_Plugin_Helper.h"
#include "../ESPEasyCore/ESPEasyRules.h"
#include "../Globals/GlobalMapPortStatus.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/PortStatus.h"



//********************************************************************************
// Internal GPIO write
//********************************************************************************
void GPIO_Internal_Write(int pin, uint8_t value)
{
  if (checkValidPortRange(PLUGIN_GPIO, pin)) {
    const uint32_t key = createKey(PLUGIN_GPIO, pin);
    auto it = globalMapPortStatus.find(key);
    if (it != globalMapPortStatus.end()) {
      if (it->second.mode == PIN_MODE_PWM) {
        set_Gpio_PWM(pin, value);
      } else {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, value);
      }
    }
  }
}

//********************************************************************************
// Internal GPIO read
//********************************************************************************
bool GPIO_Internal_Read(int pin)
{
  if (checkValidPortRange(PLUGIN_GPIO, pin))
    return digitalRead(pin)==HIGH;
  return false;
}

bool GPIO_Read_Switch_State(struct EventStruct *event) {
  const int pin     = CONFIG_PIN1;
  if (checkValidPortRange(PLUGIN_GPIO, pin)) {
    const uint32_t key = createKey(PLUGIN_GPIO, pin);

    auto it = globalMapPortStatus.find(key);
    if (it != globalMapPortStatus.end()) {
      return GPIO_Read_Switch_State(pin, it->second.mode);
    }
  }
  return false;
}

bool GPIO_Read_Switch_State(int pin, uint8_t pinMode) {
  bool canRead = false;
  if (checkValidPortRange(PLUGIN_GPIO, pin)) {
    switch (pinMode)
    {
      case PIN_MODE_UNDEFINED:
      case PIN_MODE_INPUT:
      case PIN_MODE_INPUT_PULLUP:
      case PIN_MODE_OUTPUT:
        canRead = true;
        break;
/*
      case PIN_MODE_PWM:
        break;
      case PIN_MODE_SERVO:
        break;
      case PIN_MODE_OFFLINE:
        break;
*/
      default:
        break;
    }
  }

  if (!canRead) { return false; }

  // Do not read from the pin while mode is set to PWM or servo.
  // See https://github.com/letscontrolit/ESPEasy/issues/2117#issuecomment-443516794
  return digitalRead(pin) == HIGH;
}

#ifdef USES_P009
//********************************************************************************
// MCP23017 PIN read
//********************************************************************************
int8_t GPIO_MCP_Read(int Par1)
{
  int8_t pinState = -1;
  if (checkValidPortRange(PLUGIN_MCP, Par1)) {
    uint8_t unit = (Par1 - 1) / 16;
    uint8_t port = Par1 - (unit * 16) - 1;
    uint8_t address = 0x20 + unit;
    uint8_t IOBankValueReg = (port<8)? MCP23017_GPIOA : MCP23017_GPIOB;
    port = port % 8;

    uint8_t retValue;
    if (GPIO_MCP_ReadRegister(address,IOBankValueReg,&retValue)) {
      retValue = (retValue & (1 << port)) >> port;
      pinState = (retValue==0)?0:1;

      /*
      // get the current pin status
      Wire.beginTransmission(address);
      Wire.write(IOBankValueReg); // IO data register
      Wire.endTransmission();
      Wire.requestFrom(address, (uint8_t)0x1);
      if (Wire.available())
      {
        state = (Wire.read() & (1 << port)) >> port;
      }
      */
    }
  }
  return pinState;
}

//********************************************************************************
// MCP23017 read register
//********************************************************************************

bool GPIO_MCP_ReadRegister(uint8_t mcpAddr, uint8_t regAddr, uint8_t *retValue) {
  bool success = false;
  // Read the register
  Wire.beginTransmission(mcpAddr);
  Wire.write(regAddr);
  Wire.endTransmission();

  Wire.requestFrom(mcpAddr, (uint8_t)0x1);
  if (Wire.available()) {
    success=true;
    *retValue = Wire.read();
  }
  return success;
}

//********************************************************************************
// MCP23017 write register
//********************************************************************************

void GPIO_MCP_WriteRegister(uint8_t mcpAddr, uint8_t regAddr, uint8_t regValue) {
  // Write the register
  Wire.beginTransmission(mcpAddr);
  Wire.write(regAddr);
  Wire.write(regValue);
  Wire.endTransmission();
}


//********************************************************************************
// MCP23017 write pin
//********************************************************************************
bool GPIO_MCP_Write(int Par1, uint8_t Par2)
{
  if (!checkValidPortRange(PLUGIN_MCP, Par1)) {
    return false;
  }
  bool success = false;
  uint8_t unit = (Par1 - 1) / 16;
  uint8_t port = Par1 - (unit * 16) - 1;
  uint8_t address = int((Par1-1) / 16) + 0x20;
  uint8_t IOBankConfigReg = (port<8)? MCP23017_IODIRA : MCP23017_IODIRB;
  uint8_t IOBankValueReg = (port<8)? MCP23017_GPIOA : MCP23017_GPIOB;
  port = port % 8;

  uint8_t retValue;

  // turn this port into output, first read current config
  if (GPIO_MCP_ReadRegister(address,IOBankConfigReg,&retValue)) {
    retValue &= ~(1 << port ); // change pin to output (0)

    // write new IO config
    GPIO_MCP_WriteRegister(address,IOBankConfigReg,retValue);

    if (GPIO_MCP_ReadRegister(address,IOBankValueReg,&retValue)) {
      (Par2 == 1) ? retValue |= (1 << port) : retValue &= ~(1 << port);
      
      // write new IO config
      GPIO_MCP_WriteRegister(address,IOBankValueReg,retValue);
      success = true;
    }
  }
  return(success);

/*
  uint8_t portvalue = 0;
  // turn this port into output, first read current config
  Wire.beginTransmission(address);
  Wire.write(IOBankConfigReg); // IO config register
  Wire.endTransmission();
  Wire.requestFrom(address, (uint8_t)0x1);
  if (Wire.available())
  {
    portvalue = Wire.read();
    portvalue &= ~(1 << port ); // change pin from (default) input to output

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
      portvalue |= (1 << port);
    else
      portvalue &= ~(1 << port);

    // write back new data
    Wire.beginTransmission(address);
    Wire.write(IOBankValueReg);
    Wire.write(portvalue);
    Wire.endTransmission();
    success = true;
  }
  return(success);
  */
}

//********************************************************************************
// MCP23017 config
// Par2: 0: Pullup disabled
// Par2: 1: Pullup enabled
//********************************************************************************
bool setMCPInputAndPullupMode(uint8_t Par1, bool enablePullUp)
{
  if (!checkValidPortRange(PLUGIN_MCP, Par1)) {
    return false;
  }

  bool success = false;
  uint8_t unit = (Par1 - 1) / 16;
  uint8_t port = Par1 - (unit * 16) -1;
  uint8_t address = 0x20 + unit;
  uint8_t IOBankPullUpReg = (port<8)? MCP23017_GPPUA : MCP23017_GPPUB;
  uint8_t IOBankIODirReg = (port<8)? MCP23017_IODIRA : MCP23017_IODIRB;
  port = port % 8;

  uint8_t retValue;
  // set this port mode to INPUT (bit=1)
  if (GPIO_MCP_ReadRegister(address, IOBankIODirReg, &retValue)) {
    retValue |= (1 << port);
    GPIO_MCP_WriteRegister(address, IOBankIODirReg, retValue);

    // turn this port pullup on or off
    if (GPIO_MCP_ReadRegister(address, IOBankPullUpReg, &retValue)) {
    enablePullUp ? retValue |= (1 << port) : retValue &= ~(1 << port);
    GPIO_MCP_WriteRegister(address, IOBankPullUpReg, retValue);

    success=true;
    }
  }
  return success;
}

bool setMCPOutputMode(uint8_t Par1)
{
  if (!checkValidPortRange(PLUGIN_MCP, Par1)) {
    return false;
  }

  bool success = false;
  uint8_t retValue;
  uint8_t unit = (Par1 - 1) / 16;
  uint8_t port = Par1 - (unit * 16) - 1;
  uint8_t address = 0x20 + unit;
  uint8_t IOBankIODirReg = (port<8)? MCP23017_IODIRA : MCP23017_IODIRB;
  port = port % 8;

  // set this port mode to OUTPUT (bit=0)
  if (GPIO_MCP_ReadRegister(address, IOBankIODirReg, &retValue)) {
   retValue &= ~(1 << port);
   GPIO_MCP_WriteRegister(address, IOBankIODirReg, retValue);
   success=true;
  }
  return success;
}
#endif

#ifdef USES_P019
//********************************************************************************
// PCF8574 read pin
//********************************************************************************
//@giig1967g-20181023: changed to int8_t
int8_t GPIO_PCF_Read(int Par1)
{
  int8_t state = -1;
  if (checkValidPortRange(PLUGIN_PCF, Par1)) {
    uint8_t unit = (Par1 - 1) / 8;
    uint8_t port = Par1 - (unit * 8) - 1;
    uint8_t address = 0x20 + unit;
    if (unit > 7) address += 0x10;

    // get the current pin status
    Wire.requestFrom(address, (uint8_t)0x1);
    if (Wire.available())
    {
      state = ((Wire.read() & _BV(port)) >> (port));
    }
  }
  return state;
}

bool GPIO_PCF_ReadAllPins(uint8_t address, uint8_t *retValue)
{
  bool success = false;

  Wire.requestFrom(address, (uint8_t)0x1);
  if (Wire.available())
  {
    success=true;
    *retValue = Wire.read();
  }
  return success;
}


//********************************************************************************
// PCF8574 write pin
//*******************************************************************************
void GPIO_PCF_WriteAllPins(uint8_t address, uint8_t value)
{
  Wire.beginTransmission(address);
  Wire.write(value);
  Wire.endTransmission();
}

bool GPIO_PCF_Write(int Par1, uint8_t Par2)
{
  if (!checkValidPortRange(PLUGIN_PCF, Par1)) {
    return false;
  }
  uint8_t unit = (Par1 - 1) / 8;
  uint8_t port = Par1 - (unit * 8) - 1;
  uint8_t address = 0x20 + unit;
  if (unit > 7) address += 0x10;

  //generate bitmask
  int i = 0;
  uint8_t portmask = 255;
  unit = unit * 8 + 1; // calculate first pin

  uint32_t key;

  //REMEMBER: all input pins must be set to 1 when writing to the unit
  for(i=0; i<8; i++){
    key = createKey(PLUGIN_PCF,unit+i);

    auto it = globalMapPortStatus.find(key);
    if (it != globalMapPortStatus.end()) {
      if (it->second.mode == PIN_MODE_OUTPUT && it->second.state == 0) {
        portmask &= ~(1 << i); //set port i = 0
      }
    }
  }

  if (Par2 == 1)
    portmask |= (1 << port);
  else
    portmask &= ~(1 << port);

  GPIO_PCF_WriteAllPins(address,portmask);
  return true;
}

bool setPCFInputMode(uint8_t pin) 
{
  if (!checkValidPortRange(PLUGIN_PCF, pin)) {
    return false;
  }
  uint8_t unit = (pin - 1) / 8;
  const uint8_t port = pin - (unit * 8)-1;
  uint8_t address = 0x20 + unit;
  if (unit > 7) address += 0x10;

  //generate bitmask
  int i = 0;
  uint8_t portmask;
  if (GPIO_PCF_ReadAllPins(address, &portmask)) {
    unit = unit * 8 + 1; // calculate first pin
    uint32_t key;

    //REMEMBER: all input pins must be set to 1 when writing to the unit
    for(i=0; i<8; i++){
      key = createKey(PLUGIN_PCF,unit+i);

      if (!existPortStatus(key) || 
          (existPortStatus(key) && (globalMapPortStatus[key].mode == PIN_MODE_INPUT_PULLUP || globalMapPortStatus[key].mode == PIN_MODE_INPUT)) ||
          port==i) //set to 1 the PIN to be set as INPUT
        portmask |= (1 << i); //set port i = 1
    }

    GPIO_PCF_WriteAllPins(address,portmask);

    return true;
  } else
    return false;  
}
#endif


//*********************************************************
// GPIO_Monitor10xSec:
// What it does:
// a) It updates the pinstate structure
// b) Returns an EVENT in case monitor is enabled
// c) Does not update the pinstate state if the port is defined in a task, as it will be picked up by thePLUGIN_TEN_PER_SECOND event
// called from: run10TimesPerSecond in ESPEasy.ino
//*********************************************************

void GPIO_Monitor10xSec()
{
  for (auto it=globalMapPortStatus.begin(); it!=globalMapPortStatus.end(); ++it) {
    //only call monitor function if there is the need to
    if (it->second.monitor || it->second.command || it->second.init) {
      const uint16_t gpioPort = getPortFromKey(it->first);
      const uint16_t pluginID = getPluginFromKey(it->first);
      int8_t currentState = -1;
      const __FlashStringHelper * eventString = F("");
      bool caseFound = true;

      switch (pluginID)
      {
        case PLUGIN_GPIO:
          currentState = GPIO_Read_Switch_State(gpioPort, it->second.mode);
          eventString = F("GPIO");
          break;
        case PLUGIN_MCP:
#ifdef USES_P009
          currentState = GPIO_MCP_Read(gpioPort);
          eventString = F("MCP");
#endif
          break;
        case PLUGIN_PCF:
#ifdef USES_P019
          currentState = GPIO_PCF_Read(gpioPort);
          eventString = F("PCF");
#endif
          break;
        default:
          caseFound=false;
        break;
      }
      if (caseFound) {
        if (currentState == -1) {
          it->second.mode = PIN_MODE_OFFLINE;
        } else if (it->second.mode == PIN_MODE_OFFLINE) {
          it->second.mode = PIN_MODE_UNDEFINED;
        }
        if (((it->second.state != currentState) || (it->second.forceMonitor && it->second.monitor))) {
          // update state
          if (!it->second.task) {
            it->second.state = currentState; //update state ONLY if task flag=false otherwise it will not be picked up by 10xSEC function
            // send event if not task, otherwise is sent in the task PLUGIN_TEN_PER_SECOND
            if (it->second.monitor) { 
              sendMonitorEvent(eventString, gpioPort, currentState);
            }
          }
        }
      }
      it->second.forceMonitor = 0; //reset flag
    }
  }
}

// prefix should be either "GPIO", "PCF", "MCP"
void sendMonitorEvent(const __FlashStringHelper * prefix, int port, int8_t state)
{
  String eventString = prefix;
  eventString += '#';
  eventString += port;
  eventString += '=';
  eventString += state;
  rulesProcessing(eventString);
}

bool checkValidPortRange(pluginID_t pluginID, int port)
{
  switch (pluginID)
  {
    case PLUGIN_GPIO:
      return validGpio(port);

    case PLUGIN_MCP:
    case PLUGIN_PCF:
      return (port>=1 && port<=128);
  }
  return false;
}

void setInternalGPIOPullupMode(uint8_t port)
{
  if (checkValidPortRange(PLUGIN_GPIO, port)) {
  #if defined(ESP8266)
    if (port == 16) {
      pinMode(port, INPUT_PULLDOWN_16);
    }
    else {
      pinMode(port, INPUT_PULLUP);
    }
  #else // if defined(ESP8266)
    pinMode(port, INPUT_PULLUP);
  #endif // if defined(ESP8266)
  }
}

bool GPIO_Write(pluginID_t pluginID, int port, uint8_t value, uint8_t pinMode)
{
  bool success=true;
  switch (pluginID)
  {
    case PLUGIN_GPIO:
      if (pinMode==PIN_MODE_OUTPUT)
        GPIO_Internal_Write(port, value);
      else
        success=false;
      break;
    case PLUGIN_MCP:
#ifdef USES_P009
      GPIO_MCP_Write(port, value);
#endif
      break;
    case PLUGIN_PCF:
#ifdef USES_P019
      GPIO_PCF_Write(port, value);
#endif
      break;
    default:
      success=false;
  }
  if (success) {
    Scheduler.clearGPIOTimer(pluginID, port);
  }
  return success;
}

bool GPIO_Read(pluginID_t pluginID, int port, int8_t &value)
{
  bool success=true;
  switch (pluginID)
  {
    case PLUGIN_GPIO:
      value = GPIO_Internal_Read(port);
      break;
    case PLUGIN_MCP:
#ifdef USES_P009
      value = GPIO_MCP_Read(port);
#endif
      break;
    case PLUGIN_PCF:
#ifdef USES_P019
      value = GPIO_PCF_Read(port);
#endif
      break;
    default:
      success=false;
  }
  return success;
}

