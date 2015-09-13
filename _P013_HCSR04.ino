//#######################################################################################################
//#################################### Plugin 013: HC-SR04 ##############################################
//#######################################################################################################

#define PLUGIN_013
#define PLUGIN_ID_013        13

boolean Plugin_013_init = false;
volatile unsigned long Plugin_013_timer = 0;
volatile unsigned long Plugin_013_state = 0;
byte Plugin_013_TRIG_Pin = 0;
byte Plugin_013_IRQ_Pin = 0;

boolean Plugin_013(byte function, struct EventStruct *event, String& string)
{
  static byte switchstate[TASKS_MAX];
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_013;
        strcpy(Device[deviceCount].Name, "Ultrasonic Sensor HC-SR04");
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 1;
        strcpy(Device[deviceCount].ValueNames[0], "Distance");
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String options[2];
        options[0] = F("Value");
        options[1] = F("State");
        int optionValues[2];
        optionValues[0] = 1;
        optionValues[1] = 2;
        string += F("<TR><TD>Mode:<TD><select name='plugin_013_mode'>");
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
          sprintf(tmpString, "<TR><TD>Threshold:<TD><input type='text' name='plugin_013_threshold' value='%u'>", Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
          string += tmpString;
        }
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg("plugin_013_mode");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == 2)
        {
          String plugin2 = WebServer.arg("plugin_013_threshold");
          Settings.TaskDevicePluginConfig[event->TaskIndex][1] = plugin2.toInt();
        }
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        Plugin_013_init = true;
        Serial.print(F("INIT : HC-SR04 "));
        Serial.print(Settings.TaskDevicePin1[event->TaskIndex]);
        Serial.print(" & ");
        Serial.println(Settings.TaskDevicePin2[event->TaskIndex]);
        pinMode(Settings.TaskDevicePin1[event->TaskIndex], OUTPUT);
        pinMode(Settings.TaskDevicePin2[event->TaskIndex], INPUT_PULLUP);
        Plugin_013_IRQ_Pin = Settings.TaskDevicePin2[event->TaskIndex];
        attachInterrupt(Settings.TaskDevicePin2[event->TaskIndex], Plugin_013_interrupt, CHANGE);
        success = true;
        break;
      }

    case PLUGIN_COMMAND: // If we select value mode, read and send the value based on global timer
      {
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == 1)
        {
          Plugin_013_TRIG_Pin = Settings.TaskDevicePin1[event->TaskIndex];
          float value = Plugin_013_read();
          if (value != -1)
          {
            UserVar[event->BaseVarIndex] = (float)Plugin_013_timer / 58;
            Serial.print("SR04 : Distance: ");
            Serial.println(UserVar[event->BaseVarIndex]);
            success = true;
          }
          else
            Serial.println("SR04 : Distance: No reading!");
        }
        break;
      }

    case PLUGIN_TEN_PER_SECOND: // If we select state mode, do more frequent checks and send only state changes
      {
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == 2)
        {
          Plugin_013_TRIG_Pin = Settings.TaskDevicePin1[event->TaskIndex];
          byte state = 0;
          float value = Plugin_013_read();
          if (value != -1)
          {
            if (value < Settings.TaskDevicePluginConfig[event->TaskIndex][1])
              state = 1;
            if (state != switchstate[event->TaskIndex])
            {
              Serial.print(F("SR04 : State "));
              Serial.println(state);
              switchstate[event->TaskIndex] = state;
              UserVar[event->BaseVarIndex] = state;
              sendData(event->TaskIndex, SENSOR_TYPE_SWITCH, Settings.TaskDeviceID[event->TaskIndex], event->BaseVarIndex);
            }
          }
        }
        success = true;
        break;
      }
  }
  return success;
}

/*********************************************************************/
float Plugin_013_read()
/*********************************************************************/
{
  float value = -1;
  Plugin_013_timer = 0;
  Plugin_013_state = 0;
  noInterrupts();
  digitalWrite(Plugin_013_TRIG_Pin, LOW);
  delayMicroseconds(2);
  digitalWrite(Plugin_013_TRIG_Pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(Plugin_013_TRIG_Pin, LOW);
  interrupts();

  delay(25);  // wait for measurement to finish (max 400 cm * 58 uSec = 23uSec)
  if (Plugin_013_state == 2)
  {
    value = (float)Plugin_013_timer / 58;
  }
  return value;
}

/*********************************************************************/
void Plugin_013_interrupt()
/*********************************************************************/
{
  byte pinState = digitalRead(Plugin_013_IRQ_Pin);
  if (pinState == 1) // Start of pulse
  {
    Plugin_013_state = 1;
    Plugin_013_timer = micros();
  }
  else // End of pulse, calculate timelapse between start & end
  {
    Plugin_013_state = 2;
    Plugin_013_timer = micros() - Plugin_013_timer;
  }
}

