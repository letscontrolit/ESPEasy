//#######################################################################################################
//#################################### Plugin 001: Input Switch #########################################
//#######################################################################################################

#define PLUGIN_001
#define PLUGIN_ID_001         1
#define PLUGIN_NAME_001       "Switch input"
#define PLUGIN_VALUENAME1_001 "Switch"

boolean Plugin_001(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte switchstate[TASKS_MAX];
  static byte outputstate[TASKS_MAX];

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_001;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = true;
        Device[deviceCount].InverseLogicOption = true;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_001);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_001));
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
        string += F("<TR><TD>Switch Type:<TD><select name='plugin_001_type'>");
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

        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == 2)
        {
          char tmpString[128];
          sprintf_P(tmpString, PSTR("<TR><TD>Dim value:<TD><input type='text' name='plugin_001_dimvalue' value='%u'>"), Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
          string += tmpString;
        }

        choice = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        String buttonOptions[3];
        buttonOptions[0] = F("Normal Switch");
        buttonOptions[1] = F("Push Button Active Low");
        buttonOptions[2] = F("Push Button Active High");
        int buttonOptionValues[3];
        buttonOptionValues[0] = 0;
        buttonOptionValues[1] = 1;
        buttonOptionValues[2] = 2;
        string += F("<TR><TD>Switch Button Type:<TD><select name='plugin_001_button'>");
        for (byte x = 0; x < 3; x++)
        {
          string += F("<option value='");
          string += buttonOptionValues[x];
          string += "'";
          if (choice == buttonOptionValues[x])
            string += F(" selected");
          string += ">";
          string += buttonOptions[x];
          string += F("</option>");
        }
        string += F("</select>");

        string += F("<TR><TD>Send Boot state:<TD>");
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][3])
          string += F("<input type=checkbox name=plugin_001_boot checked>");
        else
          string += F("<input type=checkbox name=plugin_001_boot>");

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin1 = WebServer.arg("plugin_001_type");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin1.toInt();
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == 2)
        {
          String plugin2 = WebServer.arg("plugin_001_dimvalue");
          Settings.TaskDevicePluginConfig[event->TaskIndex][1] = plugin2.toInt();
        }
        String plugin3 = WebServer.arg("plugin_001_button");
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = plugin3.toInt();

        String plugin4 = WebServer.arg("plugin_001_boot");
        Settings.TaskDevicePluginConfig[event->TaskIndex][3] = (plugin4 == "on");

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (Settings.TaskDevicePin1PullUp[event->TaskIndex])
          pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT_PULLUP);
        else
          pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT);

        setPinState(PLUGIN_ID_001, Settings.TaskDevicePin1[event->TaskIndex], PIN_MODE_INPUT, 0);

        switchstate[event->TaskIndex] = digitalRead(Settings.TaskDevicePin1[event->TaskIndex]);
        outputstate[event->TaskIndex] = switchstate[event->TaskIndex];
        
        // if boot state must be send, inverse default state
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][3])
        {
          switchstate[event->TaskIndex] = !switchstate[event->TaskIndex];
          outputstate[event->TaskIndex] = !outputstate[event->TaskIndex];
        }
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        byte state = digitalRead(Settings.TaskDevicePin1[event->TaskIndex]);
        if (state != switchstate[event->TaskIndex])
        {
          switchstate[event->TaskIndex] = state;
          byte currentOutputState = outputstate[event->TaskIndex];

          if (Settings.TaskDevicePluginConfig[event->TaskIndex][2] == 0) //normal switch
            outputstate[event->TaskIndex] = state;
          else
          {
            if (Settings.TaskDevicePluginConfig[event->TaskIndex][2] == 1)  // active low push button
            {
              if (state == 0)
                outputstate[event->TaskIndex] = !outputstate[event->TaskIndex];
            }
            else  // active high push button
            {
              if (state == 1)
                outputstate[event->TaskIndex] = !outputstate[event->TaskIndex];
            }
          }

          // send if output needs to be changed
          if (currentOutputState != outputstate[event->TaskIndex])
          {
            byte sendState = outputstate[event->TaskIndex];
            if (Settings.TaskDevicePin1Inversed[event->TaskIndex])
              sendState = !outputstate[event->TaskIndex];
            UserVar[event->BaseVarIndex] = sendState;
            event->sensorType = SENSOR_TYPE_SWITCH;
            if ((sendState == 1) && (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == 2))
            {
              event->sensorType = SENSOR_TYPE_DIMMER;
              UserVar[event->BaseVarIndex] = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
            }
            String log = F("SW   : State ");
            log += sendState;
            addLog(LOG_LEVEL_INFO, log);
            sendData(event);
          }
        }
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        // We do not actually read the pin state as this is already done 10x/second
        // Instead we just send the last known state stored in Uservar
        String log = F("SW   : State ");
        log += UserVar[event->BaseVarIndex];
        addLog(LOG_LEVEL_INFO, log);
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String log = "";
        String command = parseString(string, 1);

        if (command == F("gpio"))
        {
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= 16)
          {
            pinMode(event->Par1, OUTPUT);
            digitalWrite(event->Par1, event->Par2);
            setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, event->Par2);
            log = String(F("SW   : GPIO ")) + String(event->Par1) + String(F(" Set to ")) + String(event->Par2);
            addLog(LOG_LEVEL_INFO, log);
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par1, log, 0));
          }
        }

        if (command == F("pwm"))
        {
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= 16)
          {
            pinMode(event->Par1, OUTPUT);
            
            if(event->Par3 != NULL)
            {
              byte prev_mode;
              uint16_t prev_value;            
              getPinState(PLUGIN_ID_001, event->Par1, &prev_mode, &prev_value);
              if(prev_mode != PIN_MODE_PWM)
                prev_value = 0;

              int32_t step_value = ((event->Par2 - prev_value) << 12) / event->Par3;
              int32_t curr_value = prev_value << 12;
              int16_t new_value;
              int i = event->Par3;
              while(i--){
                curr_value += step_value;
                new_value = (uint16_t)(curr_value >> 12);
                analogWrite(event->Par1, new_value);
                delay(1);
              }
            }
            
            analogWrite(event->Par1, event->Par2);
            setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_PWM, event->Par2);
            log = String(F("SW   : GPIO ")) + String(event->Par1) + String(F(" Set PWM to ")) + String(event->Par2);
            addLog(LOG_LEVEL_INFO, log);
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par1, log, 0));
          }
        }

        if (command == F("pulse"))
        {
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= 16)
          {
            pinMode(event->Par1, OUTPUT);
            digitalWrite(event->Par1, event->Par2);
            delay(event->Par3);
            digitalWrite(event->Par1, !event->Par2);
            setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, event->Par2);
            log = String(F("SW   : GPIO ")) + String(event->Par1) + String(F(" Pulsed for ")) + String(event->Par3) + String(F(" mS"));
            addLog(LOG_LEVEL_INFO, log);
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par1, log, 0));
          }
        }

        if (command == F("longpulse"))
        {
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= 16)
          {
            pinMode(event->Par1, OUTPUT);
            digitalWrite(event->Par1, event->Par2);
            setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, event->Par2);
            setSystemTimer(event->Par3 * 1000, PLUGIN_ID_001, event->Par1, !event->Par2, 0);
            log = String(F("SW   : GPIO ")) + String(event->Par1) + String(F(" Pulse set for ")) + String(event->Par3) + String(F(" S"));
            addLog(LOG_LEVEL_INFO, log);
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par1, log, 0));
          }
        }

        if (command == F("servo"))
        {
          success = true;
          if (event->Par1 >= 0 && event->Par1 <= 2)
            switch (event->Par1)
            {
              case 1:
                myservo1.attach(event->Par2);
                myservo1.write(event->Par3);
                break;
              case 2:
                myservo2.attach(event->Par2);
                myservo2.write(event->Par3);
                break;
            }
          setPinState(PLUGIN_ID_001, event->Par2, PIN_MODE_SERVO, event->Par3);
          log = String(F("SW   : GPIO ")) + String(event->Par2) + String(F(" Servo set to ")) + String(event->Par3);
          addLog(LOG_LEVEL_INFO, log);
          SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par2, log, 0));
        }

        if (command == F("status"))
        {
          if (parseString(string, 2) == F("gpio"))
          {
            success = true;
            SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_001, event->Par2, dummyString, 0));
          }
        }

        if (command == F("inputswitchstate"))
        {
          success = true;
          UserVar[event->Par1 * VARS_PER_TASK] = event->Par2;
          outputstate[event->Par1] = event->Par2;
        }

        break;
      }

    case PLUGIN_TIMER_IN:
      {
        digitalWrite(event->Par1, event->Par2);
        setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, event->Par2);
        break;
      }
  }
  return success;
}
