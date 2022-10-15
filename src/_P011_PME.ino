#include "_Plugin_Helper.h"
#ifdef USES_P011


// #######################################################################################################
// #################################### Plugin 011: Pro Mini Extender ####################################
// #######################################################################################################


#define PLUGIN_011
#define PLUGIN_ID_011         11
#define PLUGIN_NAME_011       "Extra IO - ProMini Extender"
#define PLUGIN_VALUENAME1_011 "Value"

#define PLUGIN_011_I2C_ADDRESS 0x7f

boolean Plugin_011(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_011;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].Ports              = 14;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
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

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = (event->Par1 == 0x7f);
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      uint8_t   choice     = PCONFIG(0);
      const __FlashStringHelper * options[2] = { F("Digital"), F("Analog") };
      addFormSelector(F("Port Type"), F("p011"), 2, options, nullptr, choice);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("p011"));
      success    = true;
      break;
    }

    case PLUGIN_INIT:
    {
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      UserVar[event->BaseVarIndex] = Plugin_011_Read(PCONFIG(0), CONFIG_PORT);
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("PME  : PortValue: ");
        log += formatUserVarNoCheck(event->TaskIndex, 0);
        addLogMove(LOG_LEVEL_INFO, log);
      }
      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      String log;
      String command = parseString(string, 1);

      if (command.equals(F("extgpio")))
      {
        success = true;
        portStatusStruct tempStatus;
        const uint32_t   key = createKey(PLUGIN_ID_011, event->Par1);

        // WARNING: operator [] creates an entry in the map if key does not exist
        // So the next command should be part of each command:
        tempStatus = globalMapPortStatus[key];

        tempStatus.mode    = PIN_MODE_OUTPUT;
        tempStatus.state   = event->Par2;
        tempStatus.command = 1; // set to 1 in order to display the status in the PinStatus page
        savePortStatus(key, tempStatus);

        Plugin_011_Write(event->Par1, event->Par2);

        // setPinState(PLUGIN_ID_011, event->Par1, PIN_MODE_OUTPUT, event->Par2);
        log = F("PME  : GPIO ");
        log += event->Par1;
        log += F(" Set to ");
        log += event->Par2;
        addLog(LOG_LEVEL_INFO, log);

        // SendStatus(event, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_011, event->Par1, log, 0));
        SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);
      }

      if (command.equals(F("extpwm")))
      {
        success = true;
        uint8_t address = PLUGIN_011_I2C_ADDRESS;
        Wire.beginTransmission(address);
        Wire.write(3);
        Wire.write(event->Par1);
        Wire.write(event->Par2 & 0xff);
        Wire.write((event->Par2 >> 8));
        Wire.endTransmission();

        portStatusStruct tempStatus;
        const uint32_t   key = createKey(PLUGIN_ID_011, event->Par1);

        // WARNING: operator [] creates an entry in the map if key does not exist
        // So the next command should be part of each command:
        tempStatus         = globalMapPortStatus[key];
        tempStatus.mode    = PIN_MODE_PWM;
        tempStatus.state   = event->Par2;
        tempStatus.command = 1; // set to 1 in order to display the status in the PinStatus page
        savePortStatus(key, tempStatus);

        // setPinState(PLUGIN_ID_011, event->Par1, PIN_MODE_PWM, event->Par2);
        log = F("PME  : GPIO ");
        log += event->Par1;
        log += F(" Set PWM to ");
        log += event->Par2;
        addLog(LOG_LEVEL_INFO, log);

        // SendStatus(event, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_011, event->Par1, log, 0));
        SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);
      }

      if (command.equals(F("extpulse")))
      {
        success = true;

        if ((event->Par1 >= 0) && (event->Par1 <= 13))
        {
          Plugin_011_Write(event->Par1, event->Par2);
          delay(event->Par3);
          Plugin_011_Write(event->Par1, !event->Par2);

          portStatusStruct tempStatus;
          const uint32_t   key = createKey(PLUGIN_ID_011, event->Par1);

          // WARNING: operator [] creates an entry in the map if key does not exist
          // So the next command should be part of each command:
          tempStatus         = globalMapPortStatus[key];
          tempStatus.mode    = PIN_MODE_OUTPUT;
          tempStatus.state   = event->Par2;
          tempStatus.command = 1; // set to 1 in order to display the status in the PinStatus page
          savePortStatus(key, tempStatus);

          // setPinState(PLUGIN_ID_011, event->Par1, PIN_MODE_OUTPUT, event->Par2);
          log = F("PME  : GPIO ");
          log += event->Par1;
          log += F(" Pulsed for ");
          log += event->Par3;
          log += F(" mS");
          addLog(LOG_LEVEL_INFO, log);

          // SendStatus(event, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_011, event->Par1, log, 0));
          SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);
        }
      }

      if (command.equals(F("extlongpulse")))
      {
        success = true;

        if ((event->Par1 >= 0) && (event->Par1 <= 13))
        {
          Plugin_011_Write(event->Par1, event->Par2);
          Scheduler.setPluginTaskTimer(event->Par3 * 1000, event->TaskIndex, event->Par1, !event->Par2);

          portStatusStruct tempStatus;
          const uint32_t   key = createKey(PLUGIN_ID_011, event->Par1);

          // WARNING: operator [] creates an entry in the map if key does not exist
          // So the next command should be part of each command:
          tempStatus         = globalMapPortStatus[key];
          tempStatus.mode    = PIN_MODE_OUTPUT;
          tempStatus.state   = event->Par2;
          tempStatus.command = 1; // set to 1 in order to display the status in the PinStatus page
          savePortStatus(key, tempStatus);

          // setPinState(PLUGIN_ID_011, event->Par1, PIN_MODE_OUTPUT, event->Par2);
          log = F("PME  : GPIO ");
          log += event->Par1;
          log += F(" Pulse set for ");
          log += event->Par3;
          log += F(" S");
          addLog(LOG_LEVEL_INFO, log);

          // SendStatus(event, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_011, event->Par1, log, 0));
          SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);
        }
      }

      if (command.equals(F("status"))) {
        if (parseString(string, 2).equals(F("ext")))
        {
          success = true;
          const uint32_t key = createKey(PLUGIN_ID_011, event->Par2); // WARNING: 'status' uses Par2 instead of Par1
          String dummyString;

          if (!existPortStatus(key)) {                                // tempStatus.mode == PIN_MODE_OUTPUT) // has been set as output
            SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, dummyString, 0);
          }
          else
          {
            uint8_t port = event->Par2; // port 0-13 is digital, ports 20-27 are mapped to A0-A7
            uint8_t type = 0;           // digital

            if (port > 13)
            {
              type  = 1;
              port -= 20;
            }
            int state = Plugin_011_Read(type, port); // report as input (todo: analog reading)

            if (state != -1) {
              SendStatusOnlyIfNeeded(event, NO_SEARCH_PIN_STATE, key, dummyString, state);
            }

            // status = getPinStateJSON(NO_SEARCH_PIN_STATE, PLUGIN_ID_011, event->Par2, dummyString, state);
          }
        }
      }
      break;
    }

    case PLUGIN_TASKTIMER_IN:
    {
      Plugin_011_Write(event->Par1, event->Par2);
      portStatusStruct tempStatus;

      // WARNING: operator [] creates an entry in the map if key does not exist
      const uint32_t key = createKey(PLUGIN_ID_011, event->Par1);
      tempStatus = globalMapPortStatus[key];

      tempStatus.state = event->Par2;
      tempStatus.mode  = PIN_MODE_OUTPUT;
      savePortStatus(key, tempStatus);

      // setPinState(PLUGIN_ID_011, event->Par1, PIN_MODE_OUTPUT, event->Par2);
      break;
    }
  }
  return success;
}

// ********************************************************************************
// PME read
// ********************************************************************************
int Plugin_011_Read(uint8_t Par1, uint8_t Par2)
{
  int value       = -1;
  uint8_t address = PLUGIN_011_I2C_ADDRESS;

  Wire.beginTransmission(address);

  if (Par1 == 0) {
    Wire.write(2); // Digital Read
  }
  else {
    Wire.write(4); // Analog Read
  }
  Wire.write(Par2);
  Wire.write(0);
  Wire.write(0);
  Wire.endTransmission();
  delay(1); // remote unit needs some time for conversion...
  Wire.requestFrom(address, (uint8_t)0x4);
  uint8_t buffer[4];

  if (Wire.available() == 4)
  {
    for (uint8_t x = 0; x < 4; x++) {
      buffer[x] = Wire.read();
    }
    value = buffer[0] + 256 * buffer[1];
  }
  return value;
}

// ********************************************************************************
// PME write
// ********************************************************************************
void Plugin_011_Write(uint8_t Par1, uint8_t Par2)
{
  uint8_t address = 0x7f;

  Wire.beginTransmission(address);
  Wire.write(1);
  Wire.write(Par1);
  Wire.write(Par2 & 0xff);
  Wire.write((Par2 >> 8));
  Wire.endTransmission();
}

#endif // USES_P011
