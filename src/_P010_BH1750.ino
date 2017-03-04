//#######################################################################################################
//#################################### Plugin-010: LuxRead   ############################################
//#######################################################################################################

#define PLUGIN_010
#define PLUGIN_ID_010         10
#define PLUGIN_NAME_010       "Luminosity - BH1750"
#define PLUGIN_VALUENAME1_010 "Lux"

#define BH1750_ADDRESS_1    0x23
#define BH1750_ADDRESS_2    0x5c

// Start measurement at 1lx resolution. Measurement time is approx 120ms.
#define BH1750_CONTINUOUS_HIGH_RES_MODE  0x10

// Start measurement at 0.5lx resolution. Measurement time is approx 120ms.
#define BH1750_CONTINUOUS_HIGH_RES_MODE_2  0x11

// Start measurement at 4lx resolution. Measurement time is approx 16ms.
#define BH1750_CONTINUOUS_LOW_RES_MODE  0x13

// Start measurement at 1lx resolution. Measurement time is approx 120ms.
// Device is automatically set to Power Down after measurement.
#define BH1750_ONE_TIME_HIGH_RES_MODE  0x20

// Start measurement at 0.5lx resolution. Measurement time is approx 120ms.
// Device is automatically set to Power Down after measurement.
#define BH1750_ONE_TIME_HIGH_RES_MODE_2  0x21

// Start measurement at 1lx resolution. Measurement time is approx 120ms.
// Device is automatically set to Power Down after measurement.
#define BH1750_ONE_TIME_LOW_RES_MODE  0x23


boolean Plugin_010_init_1 = false;
boolean Plugin_010_init_2 = false;

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

    case PLUGIN_WEBFORM_LOAD:
      {
        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String options[2];
        options[0] = F("0x23 - default settings (ADDR Low)");
        options[1] = F("0x5c - alternate settings (ADDR High)");
        int optionValues[2];
        optionValues[0] = 0;
        optionValues[1] = 1;
        string += F("<TR><TD>I2C Address:<TD><select name='plugin_010'>");
        for (byte x = 0; x < 2; x++)
        {
          string += F("<option value='");
          string += optionValues[x];
          string += "'";
          if (choice == optionValues[x])
            string += F(" selected");
          string += ">";
          string += options[x];
          string += F("</option>");
        }
        string += F("</select>");

        byte choiceMode = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        String optionsMode[6];
        optionsMode[0] = F("BH1750_CONTINUOUS_HIGH_RES_MODE");
        optionsMode[1] = F("BH1750_CONTINUOUS_HIGH_RES_MODE_2");
        optionsMode[2] = F("BH1750_CONTINUOUS_LOW_RES_MODE");
        optionsMode[3] = F("BH1750_ONE_TIME_HIGH_RES_MODE");
        optionsMode[4] = F("BH1750_ONE_TIME_HIGH_RES_MODE_2");
        optionsMode[5] = F("BH1750_ONE_TIME_LOW_RES_MODE");
        int optionValuesMode[6];
        optionValuesMode[0] = 0;
        optionValuesMode[1] = 1;
        optionValuesMode[2] = 2;
        optionValuesMode[3] = 3;
        optionValuesMode[4] = 4;
        optionValuesMode[5] = 5;
        string += F("<TR><TD>measurment mode:<TD><select name='plugin_010_mode'>");
        for (byte x = 0; x < 6; x++)
        {
          string += F("<option value='");
          string += optionValuesMode[x];
          string += "'";
          if (choiceMode == optionValuesMode[x])
            string += F(" selected");
          string += ">";
          string += optionsMode[x];
          string += F("</option>");
        }
        string += F("</select>");

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg(F("plugin_010"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        String plugin2 = WebServer.arg(F("plugin_010_mode"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = plugin2.toInt();
        success = true;
        break;
      }

  case PLUGIN_READ:
    {
      uint8_t address = -1;
      boolean *Plugin_010_init;

      if(Settings.TaskDevicePluginConfig[event->TaskIndex][0]==0)
        {
            address = BH1750_ADDRESS_1;
            Plugin_010_init = &Plugin_010_init_1;
        }
      else
        {
            address = BH1750_ADDRESS_2;
            Plugin_010_init = &Plugin_010_init_2;
        }

      byte mode = -1;
      switch (Settings.TaskDevicePluginConfig[event->TaskIndex][1]) {
        case 0:
          mode = BH1750_CONTINUOUS_HIGH_RES_MODE;
          break;
        case 1:
          mode = BH1750_CONTINUOUS_HIGH_RES_MODE_2;
          break;
        case 2:
          mode = BH1750_CONTINUOUS_LOW_RES_MODE;
          break;
        case 3:
          mode = BH1750_ONE_TIME_HIGH_RES_MODE;
          break;
        case 4:
          mode = BH1750_ONE_TIME_HIGH_RES_MODE_2;
          break;
        case 5:
          mode = BH1750_ONE_TIME_LOW_RES_MODE;
          break;
        }


        *Plugin_010_init = Plugin_010_setResolution(address,mode);
        delay(150);  //need this otherwise we get one wrong reading after mode change


      if (Wire.requestFrom(address, (uint8_t)2) == 2)
        {
          byte b1 = Wire.read();
          byte b2 = Wire.read();
          float val = 0xffff; //pm-cz: Maximum obtainable value
          if (b1 != 0xff || b2 != 0xff) { //pm-cz: Add maximum range check
            val=((b1<<8)|b2)/1.2;
          }
          if (mode==BH1750_CONTINUOUS_HIGH_RES_MODE_2 || mode==BH1750_ONE_TIME_HIGH_RES_MODE_2) {
            UserVar[event->BaseVarIndex] = val/2;
          }
          else {
            UserVar[event->BaseVarIndex] = val;
          }
          String log = F("LUX 0x");
          log += String(address,HEX);
          log += F(" Mode: 0x");
          log += String(mode,HEX);
          log += F(" : Light intensity: ");
          log += UserVar[event->BaseVarIndex];
          addLog(LOG_LEVEL_INFO,log);
          success=true;
        }
      break;
    }
  }
  return success;
}

boolean Plugin_010_setResolution(uint8_t address, byte mode){
          Wire.beginTransmission(address);
          Wire.write(mode);
          Wire.endTransmission();
          return true;
}
