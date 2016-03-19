//#######################################################################################################
//#################################### Plugin-010: LuxRead   ############################################
//#######################################################################################################

#define PLUGIN_010
#define PLUGIN_ID_010         10
#define PLUGIN_NAME_010       "Luminosity - BH1750"
#define PLUGIN_VALUENAME1_010 "Lux"

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
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_010);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_010));
        break;
      }
    
  case PLUGIN_READ:
    {
      if (!Plugin_010_init)
        {
          Plugin_010_init=true;
          Wire.beginTransmission(BH1750_ADDRESS);
          Wire.write(0x10);                             // 1 lx resolution
          Wire.endTransmission();
        }
      Wire.requestFrom(BH1750_ADDRESS, 2);
      byte b1 = Wire.read();
      byte b2 = Wire.read();
      float val=0;     
      val=((b1<<8)|b2)/1.2;
      UserVar[event->BaseVarIndex] = val;
      String log = F("LUX  : Light intensity: ");
      log += UserVar[event->BaseVarIndex];
      addLog(LOG_LEVEL_INFO,log);
      success=true;
      break;
    }
  }      
  return success;
}
