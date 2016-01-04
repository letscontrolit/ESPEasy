//#######################################################################################################
//#################################### Plugin 027: MLX90614 Ambient temperature I2C 0x5A)  ###############################################
//#######################################################################################################

// MyMessage *msgAmbTemp027; // Mysensors

#define PLUGIN_027
#define PLUGIN_ID_027 27
#define PLUGIN_NAME_027 "Ambient temperature - MLX90614"
#define PLUGIN_VALUENAME1_027 "Temperature"

boolean Plugin_027_init = false;

uint16_t readRegister027(uint8_t i2cAddress) {
  uint16_t ret;
  uint8_t reg;
  reg = 0x06;
  Wire.beginTransmission(i2cAddress);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(i2cAddress, (uint8_t)3);
  ret = Wire.read(); // receive DATA
  ret |= Wire.read() << 8; // receive DATA
  uint8_t pec = Wire.read();
  return ret;  
}

float readTemp027(uint8_t i2c_addr)
{
  float temp;
  temp = readRegister027(i2c_addr);
  temp *= .02;
  temp -= 273.15;
  return temp;
}

boolean Plugin_027(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte portValue = 0;
  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_027;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 16;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_027);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_027));
        break;
      }

    case PLUGIN_INIT:
      {
        Plugin_027_init = true;
//        if (!msgAmbTemp027) // Mysensors
//          msgAmbTemp027 = new MyMessage(event->BaseVarIndex, V_TEMP); //Mysensors
//        present(event->BaseVarIndex, S_TEMP); //Mysensors
//        Serial.print("Present MLX90614: "); //Mysensors
//        Serial.println(event->BaseVarIndex); //Mysensors
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
  //      noInterrupts();
        int value;
        value = 0;
        byte unit = Settings.TaskDevicePort[event->TaskIndex];
        uint8_t address = 0x5A + unit;
        UserVar[event->BaseVarIndex] = (float) readTemp027(address);
        String log = F("MLX90614  : Temperature: ");
        log += UserVar[event->BaseVarIndex];
//        send(msgAmbTemp027->set(UserVar[event->BaseVarIndex], 1)); // Mysensors
        addLog(LOG_LEVEL_INFO,log);
        success = true;
   //     interrupts();
        break;
      }
  }
  return success;
}
