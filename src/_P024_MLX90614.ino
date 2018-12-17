#ifdef USES_P024
//#######################################################################################################
//#################################### Plugin 024: MLX90614 IR temperature I2C 0x5A)  ###############################################
//#######################################################################################################

// MyMessage *msgTemp024; // Mysensors

#define PLUGIN_024
#define PLUGIN_ID_024 24
#define PLUGIN_NAME_024 "Environment - MLX90614"
#define PLUGIN_VALUENAME1_024 "Temperature"

boolean Plugin_024_init = false;

uint16_t readRegister024(uint8_t i2cAddress, uint8_t reg) {
  uint16_t ret;
  Wire.beginTransmission(i2cAddress);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(i2cAddress, (uint8_t)3);
  ret = Wire.read(); // receive DATA
  ret |= Wire.read() << 8; // receive DATA
  Wire.read();
  return ret;
}

float readTemp024(uint8_t i2c_addr, uint8_t i2c_reg)
{
  float temp;
  temp = readRegister024(i2c_addr, i2c_reg);
  temp *= .02;
  temp -= 273.15;
  return temp;
}

boolean Plugin_024(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  // static byte portValue = 0;
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
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
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

    case PLUGIN_WEBFORM_LOAD:
      {
        #define MLX90614_OPTION 2

        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String options[MLX90614_OPTION];
        int optionValues[MLX90614_OPTION];
        optionValues[0] = (0x07);
        options[0] = F("IR object temperature");
        optionValues[1] = (0x06);
        options[1] = F("Ambient temperature");
        addFormSelector(F("Option"), F("p024_option"), MLX90614_OPTION, options, optionValues, choice);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("p024_option"));
        Plugin_024_init = false; // Force device setup next time
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        Plugin_024_init = true;
//        if (!msgTemp024) // Mysensors
//          msgTemp024 = new MyMessage(event->BaseVarIndex, V_TEMP); //Mysensors
//        present(event->BaseVarIndex, S_TEMP); //Mysensors
//        serialPrint("Present MLX90614: "); //Mysensors
//        serialPrintln(event->BaseVarIndex); //Mysensors
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
  //      noInterrupts();
        // int value;
        // value = 0;
        byte unit = Settings.TaskDevicePort[event->TaskIndex];
        uint8_t address = 0x5A + unit;
        UserVar[event->BaseVarIndex] = (float) readTemp024(address, Settings.TaskDevicePluginConfig[event->TaskIndex][0]);
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
#endif // USES_P024
