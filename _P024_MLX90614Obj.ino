//#######################################################################################################
//#################################### Plugin 024: MLX90614 IR temperature I2C 0x5A)  ###############################################
//#######################################################################################################

// MyMessage *msgObjTemp024; // Mysensors

#define PLUGIN_024
#define PLUGIN_ID_024 24
#define PLUGIN_NAME_024 "IR temperature - MLX90614"
#define PLUGIN_VALUENAME1_024 "Temperature"

boolean Plugin_024_init = false;

uint16_t readRegister024(uint8_t i2cAddress) {
  uint16_t ret;
  uint8_t reg;
  reg = 0x07;
  Wire.beginTransmission(i2cAddress);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(i2cAddress, (uint8_t)3);
  ret = Wire.read(); // receive DATA
  ret |= Wire.read() << 8; // receive DATA
  uint8_t pec = Wire.read();
  return ret;  
}

float readTemp024(uint8_t i2c_addr)
{
  float temp;
  temp = readRegister024(i2c_addr);
  temp *= .02;
  temp -= 273.15;
  return temp;
}

boolean Plugin_024(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte portValue = 0;
  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_024;
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
        string = F(PLUGIN_NAME_024);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_024));
        break;
      }

    case PLUGIN_INIT:
      {
        Plugin_024_init = true;
//        if (!msgObjTemp024) // Mysensors
//          msgObjTemp024 = new MyMessage(event->BaseVarIndex, V_TEMP); //Mysensors
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
        UserVar[event->BaseVarIndex] = (float) readTemp024(address);
        String log = F("MLX90614  : Temperature: ");
        log += UserVar[event->BaseVarIndex];
//        send(msgObjTemp024->set(UserVar[event->BaseVarIndex], 1)); // Mysensors
        addLog(LOG_LEVEL_INFO,log);
        success = true;
   //     interrupts();
        break;
      }
  }
  return success;
}
