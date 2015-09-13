//#######################################################################################################
//#################################### Plugin 099: Smart PIR    #########################################
//#######################################################################################################

#define PLUGIN_099
#define PLUGIN_ID_099        99

unsigned long pirOffTimer;
boolean pirOffActive = false;

boolean Plugin_099(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte pirState[TASKS_MAX];  
  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_099;
        strcpy(Device[deviceCount].Name, "SmartPir");
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = true;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 1;
        strcpy(Device[deviceCount].ValueNames[0], "State");
        break;
      }
    case PLUGIN_WEBFORM_LOAD:
      {
        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String options[2];
        options[0] = F("Switch");
        options[1] = F("Dimmer");
        int optionValues[2];
        optionValues[0] = 1;
        optionValues[1] = 2;
        string += F("<TR><TD>Switch Type:<TD><select name='plugin_099_type'>");
        for (byte x = 0; x < 2; x++)
        {
          string += F("<option value='");
          string += optionValues[x];
          string += "'";
          if (choice == optionValues[x])
            string += " selected";
          string += ">";
          string += options[x];
          string += "</option>";
        }
        string += F("</select>");
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == 2)
        {
          char tmpString[80];
          sprintf(tmpString, "<TR><TD>Dim value:<TD><input type='text' name='plugin_099_dimvalue' value='%u'>", Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
          string += tmpString;
        }  
        string += F("<TR><TD>Pir delay active:<TD>");
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][2])
        {
          char tmpString[80];
          string += F("<input type=checkbox name=plugin_099_pirDelayActive checked>");
          sprintf(tmpString, "<TR><TD>Off Delay:<TD><input type='text' name='plugin_099_offDelay' value='%u'>", Settings.TaskDevicePluginConfig[event->TaskIndex][3]);
          string += tmpString;
        }else
         {
           string += F("<input type=checkbox name=plugin_099_pirDelayActive>");
         }
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg("plugin_099_type");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == 2)
        {
          String plugin2 = WebServer.arg("plugin_099_dimvalue");
          Settings.TaskDevicePluginConfig[event->TaskIndex][1] = plugin2.toInt();
        }
        String pirDelayActive = WebServer.arg("plugin_099_pirDelayActive");
        String offDelay = WebServer.arg("plugin_099_offDelay");
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = (pirDelayActive == "on");
        Settings.TaskDevicePluginConfig[event->TaskIndex][3] = offDelay.toInt();  
        success = true;
        break;
      }
    case PLUGIN_INIT:
      {
        if (Settings.TaskDevicePin1PullUp[event->TaskIndex])
        {
          Serial.print(F("INIT : InputPullup "));
          pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT_PULLUP);
        }
        else
        {
          Serial.print(F("INIT : Input "));
          pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT);
        }
        pirOffTimer = 0;
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        byte SensorType = SENSOR_TYPE_SWITCH;
        byte pState = digitalRead(Settings.TaskDevicePin1[event->TaskIndex]);
        if (pState != pirState[event->TaskIndex])
        {
          Serial.print(F("Pir   : State "));
          Serial.println(pState);
          pirState[event->TaskIndex] = pState;
          UserVar[event->BaseVarIndex] = pState; 
          
          if ((pState == 1) && (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == 2))
            {
              SensorType = SENSOR_TYPE_DIMMER;
              UserVar[event->BaseVarIndex] = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
            }
          if(pState)
          {
            // Pir port is high:
            pirOffActive = false;
            sendData(event->TaskIndex, SensorType, Settings.TaskDeviceID[event->TaskIndex], event->BaseVarIndex);
          }else{
            // Pir is low:
            if(Settings.TaskDevicePluginConfig[event->TaskIndex][2] && Settings.TaskDevicePluginConfig[event->TaskIndex][3])
            {
              pirOffActive = true;
              pirOffTimer = millis() + Settings.TaskDevicePluginConfig[event->TaskIndex][3] * 1000;
              Serial.print("Pir is low, off delay timer started");
              Serial.print(" will shutdown in: ");
              Serial.print(Settings.TaskDevicePluginConfig[event->TaskIndex][3]);
              Serial.println(" seconds");
            }else{
               // No off delay active. Just process the state 
               sendData(event->TaskIndex, SensorType, Settings.TaskDeviceID[event->TaskIndex], event->BaseVarIndex);
            }
          }
        }
        
         if (millis() > pirOffTimer && pirOffActive)
          {
            // On delay timer finished
            Serial.println("Off timer finished");
            pirOffActive = false;
            sendData(event->TaskIndex, SensorType, Settings.TaskDeviceID[event->TaskIndex], event->BaseVarIndex);
          }
        success = true;
        break;
      }

  }
  return success;
}
