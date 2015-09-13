//#######################################################################################################
//#################################### Plugin-010: LuxRead   ############################################
//#######################################################################################################

#define PLUGIN_010
#define PLUGIN_ID_010     10

#define BH1750_ADDRESS    0x23
boolean Plugin_010_init = false;

boolean Plugin_010(byte function, struct EventStruct *event, String& string)
  {
  boolean success=false;

  switch(function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_010;
        strcpy(Device[deviceCount].Name,"LUX BH1750");
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].ValueCount = 1;
        strcpy(Device[deviceCount].ValueNames[0],"Lux");
        break;
      }
    
  case PLUGIN_COMMAND:
    {
      if (!Plugin_010_init)
        {
          Plugin_010_init=true;
          Serial.println("Lux init");
          Wire.beginTransmission(BH1750_ADDRESS);
          Wire.write(0x10);                             // 1 lx resolution
          Wire.endTransmission();
        }
      Wire.requestFrom(BH1750_ADDRESS, 2);
      byte b1 = Wire.read();
      byte b2 = Wire.read();
      float val=0;     
      val=((b1<<8)|b2)/1.2;
      val=val+15;
      UserVar[event->BaseVarIndex] = val;
      Serial.print("LUX  : Light intensity: ");
      Serial.println(UserVar[event->BaseVarIndex]);
      success=true;
      break;
    }
  }      
  return success;
}
