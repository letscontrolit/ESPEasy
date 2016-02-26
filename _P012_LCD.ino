//#######################################################################################################
//#################################### Plugin 012: LCD ##################################################
//#######################################################################################################

// Sample templates
//  Temp: [DHT11#Temperature]   Hum:[DHT11#humidity]
//  DS Temp:[Dallas1#Temperature#R]
//  Lux:[Lux#Lux#R]
//  Baro:[Baro#Pressure#R]

LiquidCrystal_I2C *lcd;

#define PLUGIN_012
#define PLUGIN_ID_012         12
#define PLUGIN_NAME_012       "Display - LCD2004"
#define PLUGIN_VALUENAME1_012 "LCD"

boolean Plugin_012(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte displayTimer = 0;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_012;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 0;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_012);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_012));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        int optionValues[16];
        for (byte x = 0; x < 17; x++)
          if (x < 8)
            optionValues[x] = 0x20 + x;
          else
            optionValues[x] = 0x30 + x;

        string += F("<TR><TD>I2C Address:<TD><select name='plugin_012_adr'>");
        for (byte x = 0; x < 16; x++)
        {
          string += F("<option value='");
          string += optionValues[x];
          string += "'";
          if (choice == optionValues[x])
            string += F(" selected");
          string += ">";
          string += String(optionValues[x], HEX);
          string += F("</option>");
        }
        string += F("</select>");

        byte choice2 = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        String options2[2];
        options2[0] = F("2 x 16");
        options2[1] = F("4 x 20");
        int optionValues2[2];
        optionValues2[0] = 1;
        optionValues2[1] = 2;
        string += F("<TR><TD>Display Size:<TD><select name='plugin_012_size'>");
        for (byte x = 0; x < 2; x++)
        {
          string += F("<option value='");
          string += optionValues2[x];
          string += "'";
          if (choice2 == optionValues2[x])
            string += F(" selected");
          string += ">";
          string += options2[x];
          string += F("</option>");
        }
        string += F("</select>");

        char deviceTemplate[4][80];
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));
        for (byte varNr = 0; varNr < 4; varNr++)
        {
          string += F("<TR><TD>Line ");
          string += varNr + 1;
          string += F(":<TD><input type='text' size='80' maxlength='80' name='Plugin_012_template");
          string += varNr + 1;
          string += F("' value='");
          string += deviceTemplate[varNr];
          string += F("'>");
        }

        string += F("<TR><TD>Display button:<TD>");
        addPinSelect(false, string, "taskdevicepin3", Settings.TaskDevicePin3[event->TaskIndex]);

        char tmpString[128];

        sprintf_P(tmpString, PSTR("<TR><TD>Display Timeout:<TD><input type='text' name='plugin_12_timer' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][2]);
        string += tmpString;

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg("plugin_012_adr");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        String plugin2 = WebServer.arg("plugin_012_size");
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = plugin2.toInt();
        String plugin3 = WebServer.arg("plugin_12_timer");
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = plugin3.toInt();

        char deviceTemplate[4][80];
        for (byte varNr = 0; varNr < 4; varNr++)
        {
          char argc[25];
          String arg = F("Plugin_012_template");
          arg += varNr + 1;
          arg.toCharArray(argc, 25);
          String tmpString = WebServer.arg(argc);
          strncpy(deviceTemplate[varNr], tmpString.c_str(), sizeof(deviceTemplate[varNr]));
        }

        Settings.TaskDeviceID[event->TaskIndex] = 1; // temp fix, needs a dummy value

        SaveCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (!lcd)
        {
          byte row = 2;
          byte col = 16;
          if (Settings.TaskDevicePluginConfig[event->TaskIndex][1] == 2)
          {
            row = 4;
            col = 20;
          }
          lcd = new LiquidCrystal_I2C(Settings.TaskDevicePluginConfig[event->TaskIndex][0], col, row);
        }
        // Setup LCD display
        lcd->init();                      // initialize the lcd
        lcd->backlight();
        lcd->print("ESP Easy");
        displayTimer = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        if (Settings.TaskDevicePin3[event->TaskIndex] != -1)
          pinMode(Settings.TaskDevicePin3[event->TaskIndex], INPUT_PULLUP);
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (Settings.TaskDevicePin3[event->TaskIndex] != -1)
        {
          if (!digitalRead(Settings.TaskDevicePin3[event->TaskIndex]))
          {
            lcd->backlight();
            displayTimer = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
          }
        }
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        if ( displayTimer > 0)
        {
          displayTimer--;
          if (displayTimer == 0)
            lcd->noBacklight();
        }
        break;
      }

    case PLUGIN_READ:
      {
        char deviceTemplate[4][80];
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));

        byte row = 2;
        byte col = 16;
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][1] == 2)
        {
          row = 4;
          col = 20;
        }

        for (byte x = 0; x < row; x++)
        {
          String tmpString = deviceTemplate[x];
          if (tmpString.length())
          {
            String newString = parseTemplate(tmpString, col);
            lcd->setCursor(0, x);
            lcd->print(newString);
          }
        }
        success = false;
        break;
      }

    case PLUGIN_WRITE:
      {
        String tmpString  = string;
        int argIndex = tmpString.indexOf(',');
        if (argIndex)
          tmpString = tmpString.substring(0, argIndex);
        if (tmpString.equalsIgnoreCase(F("LCD")))
        {
          success = true;
          argIndex = string.lastIndexOf(',');
          tmpString = string.substring(argIndex + 1);
          lcd->setCursor(event->Par2 - 1, event->Par1 - 1);
          lcd->print(tmpString.c_str());
        }
        if (tmpString.equalsIgnoreCase(F("LCDCMD")))
        {
          success = true;
          argIndex = string.lastIndexOf(',');
          tmpString = string.substring(argIndex + 1);
          if (tmpString.equalsIgnoreCase(F("Off")))
            lcd->noBacklight();
          else if (tmpString.equalsIgnoreCase(F("On")))
            lcd->backlight();
          else if (tmpString.equalsIgnoreCase(F("Clear")))
            lcd->clear();
        }
        break;
      }

  }
  return success;
}
