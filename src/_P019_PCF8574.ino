#ifdef USES_P019
//#######################################################################################################
//#################################### Plugin 019: PCF8574 ##############################################
//#######################################################################################################

#define PLUGIN_019
#define PLUGIN_ID_019         19
#define PLUGIN_NAME_019       "Switch input - PCF8574"
#define PLUGIN_VALUENAME1_019 "Switch"

boolean Plugin_019(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte switchstate[TASKS_MAX];

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_019;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 8;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
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
        string = F(PLUGIN_NAME_019);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_019));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        addFormCheckBox(F("Send Boot state"), F("plugin_019_boot"), Settings.TaskDevicePluginConfig[event->TaskIndex][0]);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = isFormItemChecked(F("plugin_019_boot"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        // read and store current state to prevent switching at boot time
        switchstate[event->TaskIndex] = Plugin_019_Read(Settings.TaskDevicePort[event->TaskIndex]);

        // if boot state must be send, inverse default state
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0])
          switchstate[event->TaskIndex] = !switchstate[event->TaskIndex];

        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        int state = Plugin_019_Read(Settings.TaskDevicePort[event->TaskIndex]);
        if (state != -1)
        {
          if (state != switchstate[event->TaskIndex])
          {
            String log = F("PCF  : State ");
            log += state;
            addLog(LOG_LEVEL_INFO, log);
            switchstate[event->TaskIndex] = state;
            UserVar[event->BaseVarIndex] = state;
            event->sensorType = SENSOR_TYPE_SWITCH;
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
        String log = F("PCF  : State ");
        log += UserVar[event->BaseVarIndex];
        addLog(LOG_LEVEL_INFO, log);
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String log = "";
        String command = parseString(string, 1);

        if (command == F("pcfgpio"))
        {
          success = true;
          if (event->Par2 == 2) { //INPUT
        	  // PCF8574 specific: only can read 0/low state, so we must send 1
        	  setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_INPUT, 1);
        	  Plugin_019_Write(event->Par1,1);
        	  log = String(F("PCF  : GPIO ")) + String(event->Par1) + String(F(" Set to 1"));
          }
          else { // OUTPUT
        	  setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, event->Par2);
        	  Plugin_019_Write(event->Par1, event->Par2);
        	  log = String(F("PCF  : GPIO ")) + String(event->Par1) + String(F(" Set to ")) + String(event->Par2);
          }
          addLog(LOG_LEVEL_INFO, log);
          SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_019, event->Par1, log, 0));
        }

        if (command == F("pcfpulse"))
        {
          success = true;
          Plugin_019_Write(event->Par1, event->Par2);
          delay(event->Par3);
          Plugin_019_Write(event->Par1, !event->Par2);
          setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, event->Par2);
          log = String(F("PCF  : GPIO ")) + String(event->Par1) + String(F(" Pulsed for ")) + String(event->Par3) + String(F(" mS"));
          addLog(LOG_LEVEL_INFO, log);
          SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_019, event->Par1, log, 0));
        }

        if (command == F("pcflongpulse"))
        {
          success = true;
          Plugin_019_Write(event->Par1, event->Par2);
          setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, event->Par2);
          setSystemTimer(event->Par3 * 1000, PLUGIN_ID_019, event->Par1, !event->Par2, 0);
          log = String(F("PCF  : GPIO ")) + String(event->Par1) + String(F(" Pulse set for ")) + String(event->Par3) + String(F(" S"));
          addLog(LOG_LEVEL_INFO, log);
          SendStatus(event->Source, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_019, event->Par1, log, 0));
        }

        if (command == F("status"))
        {
          if (parseString(string, 2) == F("pcf"))
          {
            success = true;
            String status = "";
            if (hasPinState(PLUGIN_ID_019, event->Par2))  // has been set as output
              status = getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_019, event->Par2, dummyString, 0);
            else
            {
              int state = Plugin_019_Read(event->Par2); // report as input
              if (state != -1)
                status = getPinStateJSON(NO_SEARCH_PIN_STATE, PLUGIN_ID_019, event->Par2, dummyString, state);
            }
            SendStatus(event->Source, status);
          }
        }

        break;
      }

    case PLUGIN_TIMER_IN:
      {
        Plugin_019_Write(event->Par1, event->Par2);
        setPinState(PLUGIN_ID_019, event->Par1, PIN_MODE_OUTPUT, event->Par2);
        break;
      }
  }
  return success;
}


//********************************************************************************
// PCF8574 read
//********************************************************************************
int Plugin_019_Read(byte Par1)
{
  int8_t state = -1;
  byte unit = (Par1 - 1) / 8;
  byte port = Par1 - (unit * 8);
  uint8_t address = 0x20 + unit;
  if (unit > 7) address += 0x10;

  // get the current pin status
  Wire.requestFrom(address, (uint8_t)0x1);
  if (Wire.available())
  {
    state = ((Wire.read() & _BV(port - 1)) >> (port - 1));
  }
  return state;
}


//********************************************************************************
// PCF8574 write
//********************************************************************************
boolean Plugin_019_Write(byte Par1, byte Par2)
{
  byte unit = (Par1 - 1) / 8;
  byte port = Par1 - (unit * 8);
  uint8_t address = 0x20 + unit;
  if (unit > 7) address += 0x10;

  //generate bitmask
  int i = 0;
  byte portmask = 0;
  byte mode = 0;
  uint16_t value = 0;
  unit *= 8; // calculate first pin
  unit += 1;
  for(i =0;i<8;i++){
	  mode =0;
	  if(!getPinState(PLUGIN_ID_019, unit, &mode, &value) || mode == PIN_MODE_INPUT || (mode == PIN_MODE_OUTPUT && value == 1))
		  portmask |= (1 << i);
	  unit++;
  }

  if (Par2 == 1)
    portmask |= (1 << (port - 1));
  else
    portmask &= ~(1 << (port - 1));

  Wire.beginTransmission(address);
  Wire.write(portmask);
  Wire.endTransmission();

  return true;
}
#endif // USES_P019
