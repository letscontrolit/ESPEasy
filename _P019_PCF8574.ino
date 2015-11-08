//#######################################################################################################
//#################################### Plugin 019: PCF8574 ##############################################
//#######################################################################################################

#define PLUGIN_019
#define PLUGIN_ID_019         19
#define PLUGIN_NAME_019       "Switch input - PCF8574"
#define PLUGIN_VALUENAME1_019 "Switch"

boolean Plugin_019(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte switchstate[TASKS_MAX];

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_019;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 8;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 1;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_019);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_019));
        break;
      }

    case PLUGIN_INIT:
      {
        // read and store current state to prevent switching at boot time
        switchstate[event->TaskIndex] = Plugin_019_Read(Settings.TaskDevicePort[event->TaskIndex]);
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        int state = Plugin_019_Read(Settings.TaskDevicePort[event->TaskIndex]);
        if (state != -1)
        {
          if (state != switchstate[event->TaskIndex])
          {
            String log = F("PCF  : State ");
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
        if (tmpString.equalsIgnoreCase("PCFGPIO"))
        {
          success = true;
          Plugin_019_Write(event->Par1, event->Par2);
          if (printToWeb)
          {
            printWebString += F("PCFGPIO ");
            printWebString += event->Par1;
            printWebString += F(" Set to ");
            printWebString += event->Par2;
            printWebString += F("<BR>");
          }
        }
        break;
      }
  }
  return success;
}


//********************************************************************************
// PCF8574 read
//********************************************************************************
int Plugin_019_Read(byte Par1)
{
  int8_t state = -1;
  byte unit = (Par1 - 1) / 8;
  byte port = Par1 - (unit * 8);
  uint8_t address = 0x20 + unit;
  // get the current pin status
  Wire.requestFrom(address, (uint8_t)0x1);
  if (Wire.available())
  {
    state = ((Wire.read() & _BV(port - 1)) >> (port - 1));
  }
  return state;
}


//********************************************************************************
// PCF8574 write
//********************************************************************************
boolean Plugin_019_Write(byte Par1, byte Par2)
{
  boolean success = false;
  byte portvalue = 0;
  byte unit = (Par1 - 1) / 8;
  byte port = Par1 - (unit * 8);
  uint8_t address = 0x20 + unit;
  // get the current pin status
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
    Wire.write(portvalue);
    Wire.endTransmission();
    success = true;
  }
}
