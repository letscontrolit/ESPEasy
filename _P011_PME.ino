//#######################################################################################################
//#################################### Plugin 011: Pro Mini Extender ####################################
//#######################################################################################################

#define PLUGIN_011
#define PLUGIN_ID_011         11
#define PLUGIN_NAME_011       "ProMini Extender"
#define PLUGIN_VALUENAME1_011 "Value"

boolean Plugin_011(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_011;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].Ports = 14;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
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

    case PLUGIN_WEBFORM_LOAD:
      {
        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String options[2];
        options[0] = F("Digital");
        options[1] = F("Analog");
        int optionValues[2];
        optionValues[0] = 0;
        optionValues[1] = 1;
        string += F("<TR><TD>Port Type:<TD><select name='plugin_011'>");
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

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg("plugin_011");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        uint8_t address = 0x7f;
        Wire.beginTransmission(address);
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == 0)
          Wire.write(2); // Digital Read
        else
          Wire.write(4); // Analog Read
        Wire.write(Settings.TaskDevicePort[event->TaskIndex]);
        Wire.write(0);
        Wire.write(0);
        Wire.endTransmission();
        delay(1);  // remote unit needs some time for conversion...
        Wire.requestFrom(address, (uint8_t)0x4);
        byte buffer[4];
        if (Wire.available() == 4)
        {
          for (byte x=0; x < 4; x++)
            buffer[x]=Wire.read();
          UserVar[event->BaseVarIndex] = buffer[0] + 256 * buffer[1];
        }
        String log = F("PME  : PortValue: ");
        log += UserVar[event->BaseVarIndex];
        addLog(LOG_LEVEL_INFO,log);
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String tmpString  = string;
        int argIndex = tmpString.indexOf(',');
        if (argIndex)
          tmpString = tmpString.substring(0, argIndex);
        if (tmpString.equalsIgnoreCase("EXTGPIO"))
        {
          success = true;
          uint8_t address = 0x7f;
          Wire.beginTransmission(address);
          Wire.write(1);
          Wire.write(event->Par1);
          Wire.write(event->Par2 & 0xff);
          Wire.write((event->Par2 >> 8));
          Wire.endTransmission();
          if (printToWeb)
          {
            printWebString += F("EXTGPIO ");
            printWebString += event->Par1;
            printWebString += F(" Set to ");
            printWebString += event->Par2;
            printWebString += F("<BR>");
          }
        }

        if (tmpString.equalsIgnoreCase("EXTPWM"))
        {
          success = true;
          uint8_t address = 0x7f;
          Wire.beginTransmission(address);
          Wire.write(3);
          Wire.write(event->Par1);
          Wire.write(event->Par2 & 0xff);
          Wire.write((event->Par2 >> 8));
          Wire.endTransmission();
          if (printToWeb)
          {
            printWebString += F("EXTPWM ");
            printWebString += event->Par1;
            printWebString += F(" Set to ");
            printWebString += event->Par2;
            printWebString += F("<BR>");
          }
        }
        break;
      }
  }
  return success;
}
