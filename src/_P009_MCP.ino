#ifdef USES_P009
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
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = true;
        Device[deviceCount].GlobalSyncOption = true;
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

    case PLUGIN_WEBFORM_LOAD:
      {
        addFormCheckBox(F("Send Boot state") ,F("plugin_009_boot"), Settings.TaskDevicePluginConfig[event->TaskIndex][0]);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = isFormItemChecked(F("plugin_009_boot"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        // Turn on Pullup resistor
        Plugin_009_Config(Settings.TaskDevicePort[event->TaskIndex], 1);

        // read and store current state to prevent switching at boot time
        switchstate[event->TaskIndex] = Plugin_009_Read(Settings.TaskDevicePort[event->TaskIndex]);

        // if boot state must be send, inverse default state
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0])
          switchstate[event->TaskIndex] = !switchstate[event->TaskIndex];

        setPinState(PLUGIN_ID_009, Settings.TaskDevicePort[event->TaskIndex], PIN_MODE_INPUT, 0);
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
            addLog(LOG_LEVEL_INFO, log);
            switchstate[event->TaskIndex] = state;
            UserVar[event->BaseVarIndex] = state;
            event->sensorType = SENSOR_TYPE_SWITCH;
            sendData(event);
          }
        }
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        // We do not actually read the pin state as this is already done 10x/second
        // Instead we just send the last known state stored in Uservar
        String log = F("MCP   : State ");
        log += UserVar[event->BaseVarIndex];
        addLog(LOG_LEVEL_INFO, log);
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String log = "";
        String command = parseString(string, 1);

        if (command == F("mcpgpio"))
        {
          success = true;
          Plugin_009_Write(event->Par1, event->Par2);
          setPinState(PLUGIN_ID_009, event->Par1, PIN_MODE_OUTPUT, event->Par2);
          log = String(F("MCP  : GPIO ")) + String(event->Par1) + String(F(" Set to ")) + String(event->Par2);
          addLog(LOG_LEVEL_INFO, log);
          SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_009, event->Par1, log, 0));
        }

        if (command == F("mcppulse"))
        {
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= 128)
          {
            Plugin_009_Write(event->Par1, event->Par2);
            delay(event->Par3);
            Plugin_009_Write(event->Par1, !event->Par2);
            setPinState(PLUGIN_ID_009, event->Par1, PIN_MODE_OUTPUT, event->Par2);
            log = String(F("MCP  : GPIO ")) + String(event->Par1) + String(F(" Pulsed for ")) + String(event->Par3) + String(F(" mS"));
            addLog(LOG_LEVEL_INFO, log);
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_009, event->Par1, log, 0));
          }
        }

        if (command == F("mcplongpulse"))
        {
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= 128)
          {
            Plugin_009_Write(event->Par1, event->Par2);
            setPinState(PLUGIN_ID_009, event->Par1, PIN_MODE_OUTPUT, event->Par2);
            setPluginTaskTimer(event->Par3 * 1000, PLUGIN_ID_009, event->TaskIndex, event->Par1, !event->Par2);
            log = String(F("MCP  : GPIO ")) + String(event->Par1) + String(F(" Pulse set for ")) + String(event->Par3) + String(F(" S"));
            addLog(LOG_LEVEL_INFO, log);
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_009, event->Par1, log, 0));
          }
        }

        if (command == F("status"))
        {
          if (parseString(string, 2) == F("mcp"))
          {
            success = true;
            String status = "";
            if (hasPinState(PLUGIN_ID_009, event->Par2))  // has been set as output
              status = getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_009, event->Par2, dummyString, 0);
            else
            {
              int state = Plugin_009_Read(event->Par2); // report as input
              if (state != -1)
                status = getPinStateJSON(NO_SEARCH_PIN_STATE, PLUGIN_ID_009, event->Par2, dummyString, state);
            }
            SendStatus(event->Source, status);
          }
        }

        break;
      }

    case PLUGIN_TIMER_IN:
      {
        Plugin_009_Write(event->Par1, event->Par2);
        setPinState(PLUGIN_ID_009, event->Par1, PIN_MODE_OUTPUT, event->Par2);
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
  return(success);
}


//********************************************************************************
// MCP23017 config
//********************************************************************************
void Plugin_009_Config(byte Par1, byte Par2)
{
  // boolean success = false;
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
#endif // USES_P009
