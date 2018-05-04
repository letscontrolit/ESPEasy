#ifdef USES_P022
//#######################################################################################################
//#################################### Plugin 022: PCA9685 ##############################################
//#######################################################################################################

#define PLUGIN_022
#define PLUGIN_ID_022         22
#define PLUGIN_NAME_022       "Extra IO - PCA9685"
#define PLUGIN_VALUENAME1_022 "PWM"

#define PLUGIN_022_PCA9685_MODE1   0x00  // location for Mode1 register address
#define PCA9685_MODE2   0x01  // location for Mode2 register address
#define PCA9685_LED0    0x06  // location for start of LED0 registers
#define PCA9685_ADDRESS 0x40  // I2C address

/*
is bit flag any bit rapresent the initialization state of PCA9685 
es:  bit 3 is set 1 PCA9685 with adddress 0X40 + 0x03 is intin
*/
#define IS_INIT(state, bit) ((state & 1 << bit) == 1 << bit)
#define SET_INIT(state, bit) (state|= 1 << bit)
long long initializeState; // 

boolean Plugin_022(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  int port = 0;
  if(event != NULL && event->TaskIndex >- 1)
  {
    port = Settings.TaskDevicePort[event->TaskIndex];
  }

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_022;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_NONE;
        Device[deviceCount].Ports = 1;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 0;
        Device[deviceCount].Custom = false;
        Device[deviceCount].TimerOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_022);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_022));
        break;
      }

    case PLUGIN_WRITE:
      {
        String log = "";
        String line = String(string);
        String command = "";
        int dotPos = line.indexOf('.');
        if(dotPos > -1)
        {
          LoadTaskSettings(event->TaskIndex);
          String name = line.substring(0,dotPos);
          name.replace(F("["),F(""));
          name.replace(F("]"),F(""));
          if(name.equalsIgnoreCase(ExtraTaskSettings.TaskDeviceName) == true)
          {
            line = line.substring(dotPos + 1);
          }
          else
          {
             break;
          }
        }
        command = parseString(line, 1);

        if (command == F("pcapwm"))
        {
          if (!IS_INIT(initializeState, port)) Plugin_022_initialize(port);
          success = true;
          Plugin_022_Write(port, event->Par1, event->Par2);
          setPinState(PLUGIN_ID_022, event->Par1, PIN_MODE_PWM, event->Par2);
          log = String(F("PCA ")) + String(PCA9685_ADDRESS + port) + String(F(": GPIO ")) + String(event->Par1) + String(F(" Set PWM to ")) + String(event->Par2);
          addLog(LOG_LEVEL_INFO, log);
          SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_022, event->Par1, log, 0));
        }
        if (command == F("pcafrq"))
        {
          if (!IS_INIT(initializeState, port)) Plugin_022_initialize(port);
          success = true;
          Plugin_022_Frequency(port, event->Par1);
          setPinState(PLUGIN_ID_022, 99, PIN_MODE_UNDEFINED, event->Par1);
          log = String(F("PCA ")) + String(PCA9685_ADDRESS + port) + String(F(": FREQ ")) + String(event->Par1);
          addLog(LOG_LEVEL_INFO, log);
          SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_022, 99, log, 0));
        }

        if (command == F("status"))
        {
          if (parseString(line, 2) == F("pca"))
          {
            if (!IS_INIT(initializeState, port)) Plugin_022_initialize(port);
            success = true;
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_022, event->Par2, dummyString, 0));
          }
        }
        break;
      }
  }
  return success;
}


//********************************************************************************
// PCA9685 config
//********************************************************************************
void Plugin_022_writeRegister(int i2cAddress, int regAddress, byte data) {
  Wire.beginTransmission(i2cAddress);
  Wire.write(regAddress);
  Wire.write(data);
  Wire.endTransmission();
}

uint8_t Plugin_022_readRegister(int i2cAddress, int regAddress) {
  uint8_t res = 0;
  Wire.requestFrom(i2cAddress,1,1);
  while (Wire.available()) {
    res = Wire.read();
  }
  return res;
}


//********************************************************************************
// PCA9685 write
//********************************************************************************
void Plugin_022_Write(int port, byte Par1, int Par2)
{
  int i2cAddress = PCA9685_ADDRESS + port;
  // boolean success = false;
  uint16_t LED_ON = 0;
  uint16_t LED_OFF = Par2;
  Wire.beginTransmission(i2cAddress);
  Wire.write(0x06 + 4 * Par1);
  Wire.write(lowByte(LED_ON));
  Wire.write(highByte(LED_ON));
  Wire.write(lowByte(LED_OFF));
  Wire.write(highByte(LED_OFF));
  Wire.endTransmission();
}
void Plugin_022_Frequency(int port, uint16_t freq)
{
  int i2cAddress = PCA9685_ADDRESS + port;
  Plugin_022_writeRegister(i2cAddress, PLUGIN_022_PCA9685_MODE1, (byte)0x0);
  freq *= 0.9;
  //  prescale = 25000000 / 4096;
  uint16_t prescale = 6103;
  prescale /=  freq;
  prescale -= 1;
  uint8_t oldmode = Plugin_022_readRegister(i2cAddress, 0);
  uint8_t newmode = (oldmode&0x7f) | 0x10;
  Plugin_022_writeRegister(i2cAddress, PLUGIN_022_PCA9685_MODE1, (byte)newmode);
  Plugin_022_writeRegister(i2cAddress, 0xfe, (byte)prescale);  //prescale register
  Plugin_022_writeRegister(i2cAddress, PLUGIN_022_PCA9685_MODE1, (byte)oldmode);
  delayMicroseconds(5000);
  Plugin_022_writeRegister(i2cAddress, PLUGIN_022_PCA9685_MODE1, (byte)oldmode | 0xa1);
}

void Plugin_022_initialize(int port)
{
  int i2cAddress = PCA9685_ADDRESS + port;
  // default mode is open drain output, drive leds connected to VCC
  Plugin_022_writeRegister(i2cAddress, PLUGIN_022_PCA9685_MODE1, (byte)0x01); // reset the device
  delay(1);
  Plugin_022_writeRegister(i2cAddress, PLUGIN_022_PCA9685_MODE1, (byte)B10100000);  // set up for auto increment
  Plugin_022_writeRegister(i2cAddress, PCA9685_MODE2, (byte)0x10); // set to output
  SET_INIT(initializeState, port);
}
#endif // USES_P022
