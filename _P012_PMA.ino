//#######################################################################################################
//#################################### Plugin 012: Pro Mini Analog ######################################
//#######################################################################################################

#define PLUGIN_012
#define PLUGIN_ID_012     12

boolean Plugin_012(byte function, struct EventStruct *event, String& string)
  {
  boolean success=false;

  switch(function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_012;
        strcpy(Device[deviceCount].Name,"ProMini Extender Analog");
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 6;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        strcpy(Device[deviceCount].ValueNames[0],"Analog");
        break;
      }
    
  case PLUGIN_COMMAND:
    {
      uint8_t address = 0x7f;
      Wire.beginTransmission(address);
      Wire.write(4); // ADC Read
      Wire.write(Settings.TaskDevicePort[event->TaskIndex]);
      Wire.write(0);
      Wire.write(0);
      Wire.endTransmission();
      delay(1);  // remote unit needs some time to do the adc stuff
      Wire.requestFrom(address, (uint8_t)0x1);
      if (Wire.available())
        UserVar[event->BaseVarIndex] = Wire.read();
      Serial.print("PMADC: Analog: ");
      Serial.println(UserVar[event->BaseVarIndex]);
      success=true;
      break;
    }
  }      
  return success;
}
