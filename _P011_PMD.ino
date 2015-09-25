//#######################################################################################################
//#################################### Plugin 011: Pro Mini Digital #####################################
//#######################################################################################################

#define PLUGIN_011
#define PLUGIN_ID_011         11
#define PLUGIN_NAME_011       "ProMini Extender Digital"
#define PLUGIN_VALUENAME1_011 "Switch"

boolean Plugin_011(byte function, struct EventStruct *event, String& string)
  {
  boolean success=false;

  switch(function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_011;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].Ports = 14;
        Device[deviceCount].ValueCount = 1;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_011);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_011));
        break;
      }
    
  case PLUGIN_COMMAND:
    {
      uint8_t address = 0x7f;
      Wire.beginTransmission(address);
      Wire.write(2); // Digital Read
      Wire.write(Settings.TaskDevicePort[event->TaskIndex]);
      Wire.write(0);
      Wire.write(0);
      Wire.endTransmission();
      delay(1);  // remote unit needs some time ?
      Wire.requestFrom(address, (uint8_t)0x1);
      if (Wire.available())
        UserVar[event->BaseVarIndex] = Wire.read();
      Serial.print(F("PMD  : Digital: "));
      Serial.println(UserVar[event->BaseVarIndex]);
      success=true;
      break;
    }
  }      
  return success;
}
