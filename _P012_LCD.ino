//#######################################################################################################
//#################################### Plugin 012: LCD ##################################################
//#######################################################################################################

// Sample templates
//  Temp: [DHT11#Temperature]   Hum:[DHT11#humidity]
//  DS Temp:[Dallas1#Temperature#R]
//  Lux:[Lux#Lux#R]
//  Baro:[Baro#Pressure#R]

#define PLUGIN_012
#define PLUGIN_ID_012         12
#define PLUGIN_NAME_012       "LCD Display"
#define PLUGIN_VALUENAME1_012 "LCD"

boolean Plugin_012(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

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

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        char deviceTemplate[4][80];
        for (byte varNr = 0; varNr < 4; varNr++)
        {
          char argc[25];
          String arg = "Plugin_012_template";
          arg += varNr + 1;
          arg.toCharArray(argc, 25);
          String tmpString = WebServer.arg(argc);
          char tmp[81];
          tmpString.toCharArray(tmp, 81);
          urlDecode(tmp);
          strcpy(deviceTemplate[varNr], tmp);
        }
        
        Settings.TaskDeviceID[event->TaskIndex] = 1; // temp fix, needs a dummy value
        
        SaveCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        char deviceTemplate[4][80];
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));

        for (byte x = 0; x < 4; x++)
        {
          String newString = "";
          String tmpString = deviceTemplate[x];
          String tmpStringMid = "";
          int leftBracketIndex = tmpString.indexOf('[');
          if (leftBracketIndex == -1)
            newString = tmpString;
          byte count = 0;
          while (leftBracketIndex >= 0 && count < 10 - 1)
          {
            newString += tmpString.substring(0, leftBracketIndex);
            tmpString = tmpString.substring(leftBracketIndex + 1);
            int rightBracketIndex = tmpString.indexOf(']');
            if (rightBracketIndex)
            {
              tmpStringMid = tmpString.substring(0, rightBracketIndex);
              tmpString = tmpString.substring(rightBracketIndex + 1);
              int hashtagIndex = tmpStringMid.indexOf('#');
              String deviceName = tmpStringMid.substring(0, hashtagIndex);
              String valueName = tmpStringMid.substring(hashtagIndex + 1);
              String valueFormat = "";
              hashtagIndex = valueName.indexOf('#');
              if (hashtagIndex >= 0)
              {
                valueFormat = valueName.substring(hashtagIndex + 1);
                valueName = valueName.substring(0, hashtagIndex);
              }
              for (byte y = 0; y < TASKS_MAX; y++)
              {
                LoadTaskSettings(y);
                if (ExtraTaskSettings.TaskDeviceName[0] != 0)
                {
                  if (deviceName.equalsIgnoreCase(ExtraTaskSettings.TaskDeviceName))
                  {
                    for (byte z = 0; z < VARS_PER_TASK; z++)
                      if (valueName.equalsIgnoreCase(ExtraTaskSettings.TaskDeviceValueNames[z]))
                      {
                        // here we know the task and value, so find the uservar
                        String value = String(UserVar[y * VARS_PER_TASK + z]);
                        if (valueFormat == "R")
                        {
                          int filler = 20 - newString.length() - value.length() - tmpString.length() ;
                          for (byte f = 0; f < filler; f++)
                            newString += " ";
                        }
                        newString += String(value);
                      }
                  }
                }
              }
            }
            leftBracketIndex = tmpString.indexOf('[');
            count++;
          }
          newString += tmpString;
          //Serial.print("LCD  : ");
          //Serial.println(newString);
          lcd.setCursor(0, x);
          lcd.print(newString);
        }
        success = false;
        break;
      }
  }
  return success;
}
