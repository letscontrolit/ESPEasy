//#######################################################################################################
//#################################### Plugin 009: MCP23017 input #######################################
//#######################################################################################################

#define PLUGIN_009
#define PLUGIN_ID_009         9
#define PLUGIN_NAME_009       "Switch input - MCP23017"
#define PLUGIN_VALUENAME1_009 "Switch"

boolean Plugin_009(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte switchstate[TASKS_MAX];

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_009;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 16;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_009);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_009));
        break;
      }

    case PLUGIN_INIT:
      {
        // read and store current state to prevent switching at boot time
        switchstate[event->TaskIndex] = Plugin_009_Read(Settings.TaskDevicePort[event->TaskIndex]);
        // Turn on Pullup resistor
        Plugin_009_Config(Settings.TaskDevicePort[event->TaskIndex], 1);
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        int state = Plugin_009_Read(Settings.TaskDevicePort[event->TaskIndex]);
        if (state != -1)
        {
          if (state != switchstate[event->TaskIndex])
          {
            String log = F("MCP  : State ");
            log += state;
            addLog(LOG_LEVEL_INFO,log);
            switchstate[event->TaskIndex] = state;
            UserVar[event->BaseVarIndex] = state;
            event->sensorType = SENSOR_TYPE_SWITCH;
            sendData(event);
          }
        }
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String tmpString  = string;
        int argIndex = tmpString.indexOf(',');
        if (argIndex)
          tmpString = tmpString.substring(0, argIndex);
        if (tmpString.equalsIgnoreCase("MCPGPIO"))
        {
          success = true;
          Plugin_009_Write(event->Par1, event->Par2);
          if (printToWeb)
          {
            printWebString += F("MCPGPIO ");
            printWebString += event->Par1;
            printWebString += F(" Set to ");
            printWebString += event->Par2;
            printWebString += F("<BR>");
          }
        }
		if (tmpString.equalsIgnoreCase("MCPGPIOPulse"))
        {
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= 1023)
          {
            Plugin_009_Write(event->Par1, event->Par2);
            delay(event->Par3);
            Plugin_009_Write(event->Par1, !event->Par2);
            if (printToWeb)
            {
              printWebString += F("MCPGPIO ");
              printWebString += event->Par1;
              printWebString += F(" Pulsed for ");
              printWebString += event->Par3;
              printWebString += F(" mS<BR>");
            }
          }
        }
        break;
      }
  }
  return success;
}


//********************************************************************************
// MCP23017 read
//********************************************************************************
int Plugin_009_Read(byte Par1)
{
  int8_t state = -1;
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
  return state;
}


//********************************************************************************
// MCP23017 write
//********************************************************************************
boolean Plugin_009_Write(byte Par1, byte Par2)
{
  boolean success = false;
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
}


//********************************************************************************
// MCP23017 config
//********************************************************************************
boolean Plugin_009_Config(byte Par1, byte Par2)
{
  boolean success = false;
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

