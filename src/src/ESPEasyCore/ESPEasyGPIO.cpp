#include "ESPEasyGPIO.h"

/****************************************************************************/
// Central functions for GPIO handling
// **************************************************************************/
#include "../../_Plugin_Helper.h"
#include "../Commands/GPIO.h"
#include "../DataStructs/PinMode.h"
#include "../ESPEasyCore/ESPEasyRules.h"
#include "../Globals/GlobalMapPortStatus.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/PortStatus.h"



//********************************************************************************
// Internal GPIO write
//********************************************************************************
void GPIO_Internal_Write(int pin, byte value)
{
  if (checkValidPortRange(PLUGIN_GPIO, pin)) {
    const uint32_t key = createKey(PLUGIN_GPIO, pin);
    auto it = globalMapPortStatus.find(key);
    if (it != globalMapPortStatus.end()) {
      switch (it->second.mode) {
        case PIN_MODE_PWM:
          set_Gpio_PWM(pin, value);
          break;
        default:
          pinMode(pin, OUTPUT);
          digitalWrite(pin, value);
          break;
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

bool GPIO_Read_Switch_State(int pin, byte pinMode) {
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
      case PIN_MODE_PWM:
        break;
      case PIN_MODE_SERVO:
        break;
      case PIN_MODE_OFFLINE:
        break;
      default:
        break;
    }
  }

  if (!canRead) { return false; }

  // Do not read from the pin while mode is set to PWM or servo.
  // See https://github.com/letscontrolit/ESPEasy/issues/2117#issuecomment-443516794
  return digitalRead(pin) == HIGH;
}

//********************************************************************************
// MCP23017 read
//********************************************************************************
int8_t GPIO_MCP_Read(int Par1)
{
  int8_t state = -1;
  if (checkValidPortRange(PLUGIN_MCP, Par1)) {
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
  }
  return state;
}


//********************************************************************************
// MCP23017 write
//********************************************************************************
bool GPIO_MCP_Write(int Par1, byte Par2)
{
  if (!checkValidPortRange(PLUGIN_MCP, Par1)) {
    return false;
  }
  bool success = false;
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
void GPIO_MCP_Config(int Par1, byte Par2)
{
  if (!checkValidPortRange(PLUGIN_MCP, Par1)) {
    return;
  }

  // bool success = false;
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

//********************************************************************************
// PCF8574 read
//********************************************************************************
//@giig1967g-20181023: changed to int8_t
int8_t GPIO_PCF_Read(int Par1)
{
  int8_t state = -1;
  if (checkValidPortRange(PLUGIN_PCF, Par1)) {
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
  }
  return state;
}

uint8_t GPIO_PCF_ReadAllPins(uint8_t address)
{
  uint8_t rawState = 0;

  Wire.requestFrom(address, (uint8_t)0x1);
  if (Wire.available())
  {
    rawState =Wire.read();
  }
  return rawState;
}

//********************************************************************************
// PCF8574 write
//********************************************************************************
bool GPIO_PCF_Write(int Par1, byte Par2)
{
  if (!checkValidPortRange(PLUGIN_PCF, Par1)) {
    return false;
  }
  uint8_t unit = (Par1 - 1) / 8;
  uint8_t port = Par1 - (unit * 8);
  uint8_t address = 0x20 + unit;
  if (unit > 7) address += 0x10;

  //generate bitmask
  int i = 0;
  uint8_t portmask = 255;
  unit = unit * 8 + 1; // calculate first pin

  uint32_t key;

  for(i=0; i<8; i++){
    key = createKey(PLUGIN_PCF,unit+i);

    auto it = globalMapPortStatus.find(key);
    if (it != globalMapPortStatus.end()) {
      if (it->second.mode == PIN_MODE_OUTPUT && it->second.state == 0) {
        portmask &= ~(1 << i); //set port i = 0
      }
    }
  }

  key = createKey(PLUGIN_PCF,Par1);

  if (Par2 == 1)
    portmask |= (1 << (port-1));
  else
    portmask &= ~(1 << (port-1));

  Wire.beginTransmission(address);
  Wire.write(portmask);
  Wire.endTransmission();

  return true;
}

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
      String eventString;
      bool caseFound = true;

      switch (pluginID)
      {
        case PLUGIN_GPIO:
          currentState = GPIO_Read_Switch_State(gpioPort, it->second.mode);
          eventString = F("GPIO");
          break;
        case PLUGIN_MCP:
          currentState = GPIO_MCP_Read(gpioPort);
          if (currentState==-1){
            it->second.mode=PIN_MODE_OFFLINE;
          } else if (it->second.mode==PIN_MODE_OFFLINE) {
            it->second.mode=PIN_MODE_UNDEFINED;
          }
          eventString = F("MCP");
          break;
        case PLUGIN_PCF:
          currentState = GPIO_PCF_Read(gpioPort);
          if (currentState==-1){
            it->second.mode=PIN_MODE_OFFLINE;
          } else if (it->second.mode==PIN_MODE_OFFLINE) {
            it->second.mode=PIN_MODE_UNDEFINED;
          }
          eventString = F("PCF");
          break;
        default:
          caseFound=false;
        break;
      }
      if (caseFound && ((it->second.state != currentState) || (it->second.forceMonitor && it->second.monitor))) {
        // update state
        if (!it->second.task) {
          it->second.state = currentState; //update state ONLY if task flag=false otherwise it will not be picked up by 10xSEC function
          // send event if not task, otherwise is sent in the task PLUGIN_TEN_PER_SECOND
          if (it->second.monitor) sendMonitorEvent(eventString.c_str(), gpioPort, currentState);
        }
      }
      it->second.forceMonitor=0; //reset flag
    }
  }
}

// prefix should be either "GPIO", "PCF", "MCP"
void sendMonitorEvent(const char* prefix, int port, int8_t state)
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
    {
      int  pinnr = -1;
      bool input, output, warning;
      return getGpioInfo(port, pinnr, input, output, warning);
    }

    case PLUGIN_MCP:
    case PLUGIN_PCF:
      return (port>=1 && port<=128);
  }
  return false;
}

void setInternalGPIOPullupMode(int port)
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

bool GPIO_Write(pluginID_t pluginID, int port, byte value, byte pinMode)
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
      GPIO_MCP_Write(port, value);
      break;
    case PLUGIN_PCF:
      GPIO_PCF_Write(port, value);
      break;
    default:
      success=false;
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
      value = GPIO_MCP_Read(port);
      break;
    case PLUGIN_PCF:
      value = GPIO_PCF_Read(port);
      break;
    default:
      success=false;
  }
  return success;
}
