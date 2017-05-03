//#######################################################################################################
//#################################### Plugin-010: LuxRead   ############################################
//#######################################################################################################

#include <AS_BH1750.h>

#define PLUGIN_010
#define PLUGIN_ID_010         10
#define PLUGIN_NAME_010       "Luminosity - BH1750"
#define PLUGIN_VALUENAME1_010 "Lux"


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
        optionValues[0] = BH1750_DEFAULT_I2CADDR;
        optionValues[1] = BH1750_SECOND_I2CADDR;
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
        String optionsMode[4];
        optionsMode[0] = F("RESOLUTION_LOW");
        optionsMode[1] = F("RESOLUTION_NORMAL");
        optionsMode[2] = F("RESOLUTION_HIGH");
        optionsMode[3] = F("RESOLUTION_AUTO_HIGH");
        int optionValuesMode[4];
        optionValuesMode[0] = RESOLUTION_LOW;
        optionValuesMode[1] = RESOLUTION_NORMAL;
        optionValuesMode[2] = RESOLUTION_HIGH;
        optionValuesMode[3] = RESOLUTION_AUTO_HIGH;
        string += F("<TR><TD>measurment mode:<TD><select name='plugin_010_mode'>");
        for (byte x = 0; x < 4; x++)
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

        addFormCheckBox(string, F("Send sensor to sleep:"), F("plugin_010_sleep"),
        		Settings.TaskDevicePluginConfig[event->TaskIndex][2]);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg(F("plugin_010"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        String plugin2 = WebServer.arg(F("plugin_010_mode"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = plugin2.toInt();
        String plugin3 = WebServer.arg(F("plugin_010_sleep"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = (plugin3 == "on");
        success = true;
        break;
      }

  case PLUGIN_READ:
    {
    	uint8_t address = Settings.TaskDevicePluginConfig[event->TaskIndex][0];


      AS_BH1750 sensor = AS_BH1750(address);
      sensors_resolution_t mode;
      if (Settings.TaskDevicePluginConfig[event->TaskIndex][1]==RESOLUTION_LOW)
      	mode = RESOLUTION_LOW;
      if (Settings.TaskDevicePluginConfig[event->TaskIndex][1]==RESOLUTION_NORMAL)
      	mode = RESOLUTION_NORMAL;
      if (Settings.TaskDevicePluginConfig[event->TaskIndex][1]==RESOLUTION_HIGH)
      	mode = RESOLUTION_HIGH;
      if (Settings.TaskDevicePluginConfig[event->TaskIndex][1]==RESOLUTION_AUTO_HIGH)
      	mode = RESOLUTION_AUTO_HIGH;

      sensor.begin(mode,Settings.TaskDevicePluginConfig[event->TaskIndex][2]);

      float lux = sensor.readLightLevel();
      if (lux != -1) {
      	UserVar[event->BaseVarIndex] = lux;
  			String log = F("BH1750 Address: 0x");
  			log += String(address,HEX);
  			log += F(" Mode: 0x");
  			log += String(mode);
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

