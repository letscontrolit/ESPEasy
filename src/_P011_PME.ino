#include "_Plugin_Helper.h"
#ifdef USES_P011


// #######################################################################################################
// #################################### Plugin 011: Pro Mini Extender ####################################
// #######################################################################################################

/** Changelog:
 * 2024-04-14 tonhuisman: Add support for Get Config Values, to obtain a port state/value without instantiating a task for each pin.
 *                        Only a single, enabled, task is required to handle the Get Config Values.
 *                        Variables: [<TaskName>#D<port>] and [<TaskName>#A,<port>]
 * 2024-03-29 tonhuisman: Add support for Input (switch) behavior, that only generates an event if the input pin changes state
 *                        De-duplicate (merge) extpulse and extlongpulse code, and make extpulse only blocking for durations up to 10 msec
 * 2024-03-28 tonhuisman: Start changelog.
 */

# define PLUGIN_011
# define PLUGIN_ID_011         11
# define PLUGIN_NAME_011       "Extra IO - ProMini Extender"
# define PLUGIN_VALUENAME1_011 "Value"

# define PLUGIN_011_I2C_ADDRESS 0x7f

# define PLUGIN_011_PORTS       14
# define PLUGIN_011_A_PORTS     8

# define P011_PORT_TYPE         PCONFIG(0)
# define P011_TYPE_DIGITAL      0
# define P011_TYPE_ANALOG       1
# define P011_TYPE_SWITCH       2

constexpr pluginID_t P011_PLUGIN_ID{ PLUGIN_ID_011 };

boolean Plugin_011(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_011;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_SINGLE;
      dev.FormulaOption  = true;
      dev.Ports          = PLUGIN_011_PORTS;
      dev.ValueCount     = 1;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
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
      success = (event->Par1 == PLUGIN_011_I2C_ADDRESS);
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = PLUGIN_011_I2C_ADDRESS;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_WEBFORM_LOAD:
    {
      const __FlashStringHelper *options[] = { F("Digital"), F("Analog"), F("Input (switch)") };
      const int optionValues[]             = { P011_TYPE_DIGITAL, P011_TYPE_ANALOG, P011_TYPE_SWITCH };
      constexpr size_t optionCount         = NR_ELEMENTS(options);
      addFormSelector(F("Port Type"), F("p011"), optionCount, options, optionValues, P011_PORT_TYPE);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P011_PORT_TYPE = getFormItemInt(F("p011"));
      success        = true;
      break;
    }

    case PLUGIN_INIT:
    {
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      if (P011_TYPE_SWITCH != P011_PORT_TYPE) { // Not for Switch type
        UserVar.setFloat(event->TaskIndex, 0, Plugin_011_Read(P011_PORT_TYPE, CONFIG_PORT));

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLogMove(LOG_LEVEL_INFO, concat(F("PME  : PortValue: "), formatUserVarNoCheck(event, 0)));
        }
        success = true;
      }
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      if (P011_TYPE_SWITCH == P011_PORT_TYPE) { // Only for Switch type
        const int oldValue = static_cast<int>(UserVar.getFloat(event->TaskIndex, 0));
        const int newValue = Plugin_011_Read(P011_TYPE_DIGITAL, CONFIG_PORT);

        if (oldValue != newValue) { // Changed?
          UserVar.setFloat(event->TaskIndex, 0, newValue);
          sendData(event);

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLogMove(LOG_LEVEL_INFO, strformat(F("PME  : Switch state: %d"), newValue));
          }
          success = true;
        }
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      String log;
      const String command = parseString(string, 1);

      if (equals(command, F("extgpio")))
      {
        success = true;
        portStatusStruct tempStatus;
        const uint32_t   key = createKey(P011_PLUGIN_ID, event->Par1);

        // WARNING: operator [] creates an entry in the map if key does not exist
        // So the next command should be part of each command:
        tempStatus = globalMapPortStatus[key];

        tempStatus.mode    = PIN_MODE_OUTPUT;
        tempStatus.state   = event->Par2;
        tempStatus.command = 1; // set to 1 in order to display the status in the PinStatus page
        savePortStatus(key, tempStatus);

        Plugin_011_Write(event->Par1, event->Par2);

        // setPinState(PLUGIN_ID_011, event->Par1, PIN_MODE_OUTPUT, event->Par2);
        log = strformat(F("PME  : GPIO %d Set to %d"), event->Par1, event->Par2);
        addLog(LOG_LEVEL_INFO, log);

        // SendStatus(event, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_011, event->Par1, log, 0));
        SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);
      }
      else
      if (equals(command, F("extpwm")))
      {
        success = true;
        uint8_t address = PLUGIN_011_I2C_ADDRESS;
        Wire.beginTransmission(address);
        Wire.write(3);
        Wire.write(event->Par1);
        Wire.write(event->Par2 & 0xff);
        Wire.write(event->Par2 >> 8);
        Wire.endTransmission();

        portStatusStruct tempStatus;
        const uint32_t   key = createKey(P011_PLUGIN_ID, event->Par1);

        // WARNING: operator [] creates an entry in the map if key does not exist
        // So the next command should be part of each command:
        tempStatus         = globalMapPortStatus[key];
        tempStatus.mode    = PIN_MODE_PWM;
        tempStatus.state   = event->Par2;
        tempStatus.command = 1; // set to 1 in order to display the status in the PinStatus page
        savePortStatus(key, tempStatus);

        // setPinState(PLUGIN_ID_011, event->Par1, PIN_MODE_PWM, event->Par2);
        log = strformat(F("PME  : GPIO %d Set PWM to %d"), event->Par1, event->Par2);
        addLog(LOG_LEVEL_INFO, log);

        // SendStatus(event, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_011, event->Par1, log, 0));
        SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);
      }
      else
      if (equals(command, F("extpulse")) || equals(command, F("extlongpulse"))) // De-duplicated
      {
        if ((event->Par1 >= 0) && (event->Par1 < PLUGIN_011_PORTS))
        {
          const int factor   = equals(command, F("extlongpulse")) ? 1000 : 1;
          const int duration = event->Par3 * factor;
          success = true;
          Plugin_011_Write(event->Par1, event->Par2);

          if (duration <= 10) { // Short pulses (<= 10 msec) only use direct delay
            delay(event->Par3);
            Plugin_011_Write(event->Par1, !event->Par2);
            log = strformat(F("PME  : GPIO %d Pulsed for %d mS"), event->Par1, duration);
          } else {
            Scheduler.setPluginTaskTimer(duration, event->TaskIndex, event->Par1, !event->Par2);
            log = strformat(F("PME  : GPIO %d Pulse set for %d %cS"), event->Par1, duration, factor == 1 ? 'm' : ' ');
          }

          portStatusStruct tempStatus;
          const uint32_t   key = createKey(P011_PLUGIN_ID, event->Par1);

          // WARNING: operator [] creates an entry in the map if key does not exist
          // So the next command should be part of each command:
          tempStatus         = globalMapPortStatus[key];
          tempStatus.mode    = PIN_MODE_OUTPUT;
          tempStatus.state   = event->Par2;
          tempStatus.command = 1; // set to 1 in order to display the status in the PinStatus page
          savePortStatus(key, tempStatus);

          // setPinState(PLUGIN_ID_011, event->Par1, PIN_MODE_OUTPUT, event->Par2);
          addLog(LOG_LEVEL_INFO, log);

          // SendStatus(event, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_011, event->Par1, log, 0));
          SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);
        }
      }
      else
      if (equals(command, F("status"))) {
        if (equals(parseString(string, 2), F("ext")))
        {
          success = true;
          const uint32_t key = createKey(P011_PLUGIN_ID, event->Par2); // WARNING: 'status' uses Par2 instead of Par1
          String dummyString;

          if (!existPortStatus(key)) {                                 // tempStatus.mode == PIN_MODE_OUTPUT) // has been set as output
            SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, dummyString, 0);
          }
          else
          {
            uint8_t port = event->Par2; // port 0-13 is digital, ports 20-27 are mapped to A0-A7
            uint8_t type = 0;           // digital

            if (port >= PLUGIN_011_PORTS)
            {
              type  = 1;
              port -= 20;
            }
            int state = Plugin_011_Read(type, port); // report as input (todo: analog reading)

            if (state != -1) {
              SendStatusOnlyIfNeeded(event, NO_SEARCH_PIN_STATE, key, dummyString, state);
            }
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
      const uint32_t key = createKey(P011_PLUGIN_ID, event->Par1);
      tempStatus = globalMapPortStatus[key];

      tempStatus.state = event->Par2;
      tempStatus.mode  = PIN_MODE_OUTPUT;
      savePortStatus(key, tempStatus);

      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      char sep = '.';

      if ((-1 == string.indexOf(sep)) && (string.indexOf(',') >= 0)) {
        sep = ',';
      }
      const String typ  = parseString(string, 1, sep);
      const String port = parseString(string, 2, sep);
      int32_t portnr    = -1;
      validIntFromString(port, portnr);

      // [<taskname>#D.<port>] : Read Digital value from <port>
      if (equals(typ, F("d")) && !port.isEmpty() && (portnr >= 0) && (portnr < PLUGIN_011_PORTS)) {
        string  = Plugin_011_Read(0, portnr);
        success = true;
      } else

      // [<taskname>#A.<port>] : Read Analog value from <port>
      if (equals(typ, F("a")) && !port.isEmpty() && (portnr >= 0) && (portnr < PLUGIN_011_A_PORTS)) {
        string  = Plugin_011_Read(1, portnr);
        success = true;
      }

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

  if (Par1 == P011_TYPE_DIGITAL) {
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
    for (uint8_t x = 0; x < 4; ++x) {
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
  uint8_t address = PLUGIN_011_I2C_ADDRESS;

  Wire.beginTransmission(address);
  Wire.write(1);
  Wire.write(Par1);
  Wire.write(Par2 & 0xff);
  Wire.write(Par2 >> 8);
  Wire.endTransmission();
}

#endif // USES_P011
