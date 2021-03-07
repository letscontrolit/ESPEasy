#include "_Plugin_Helper.h"
#ifdef USES_P022

#include "src/DataStructs/PinMode.h"
#include "src/Helpers/PortStatus.h"
#include "src/PluginStructs/P022_data_struct.h"

// #######################################################################################################
// #################################### Plugin 022: PCA9685 ##############################################
// #######################################################################################################


#define PLUGIN_022
#define PLUGIN_ID_022         22
#define PLUGIN_NAME_022       "Extra IO - PCA9685"
#define PLUGIN_VALUENAME1_022 "PWM"


// FIXME TD-er: This plugin uses a lot of calls to the P022_data_struct, which could be combined in single functions.

boolean Plugin_022(byte function, struct EventStruct *event, String& string)
{
  boolean  success = false;
  int      address = 0;
  int      mode2   = 0x10;
  uint16_t freq    = PCA9685_MAX_FREQUENCY;
  uint16_t range   = PCA9685_MAX_PWM;

  if ((event != NULL) && (event->TaskIndex >= 0))
  {
    address = CONFIG_PORT;
    mode2   = PCONFIG(0);
    freq    = PCONFIG(1);
    range   = PCONFIG(2);
  }

  if ((address < PCA9685_ADDRESS) || (address > PCA9685_MAX_ADDRESS)) {
    address = PCA9685_ADDRESS;
  }

  if (freq == 0) {
    freq = PCA9685_MAX_FREQUENCY;
  }

  if (range == 0) {
    range = PCA9685_MAX_PWM;
  }

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_022;
      Device[deviceCount].Type               = DEVICE_TYPE_I2C;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_NONE;
      Device[deviceCount].Ports              = 1;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 0;
      Device[deviceCount].Custom             = true;
      Device[deviceCount].TimerOption        = false;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_022);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_022));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    {
      int optionValues[PCA9685_NUMS_ADDRESS];

      for (int i = 0; i < PCA9685_NUMS_ADDRESS; i++)
      {
        optionValues[i] = PCA9685_ADDRESS + i;
      }
      addFormSelectorI2C(F("i2c_addr"), PCA9685_NUMS_ADDRESS, optionValues, address);
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      // The options lists are quite long.
      // To prevent stack overflow issues, each selection has its own scope.
      {
        String m2Options[PCA9685_MODE2_VALUES];
        int    m2Values[PCA9685_MODE2_VALUES];

        for (int i = 0; i < PCA9685_MODE2_VALUES; i++)
        {
          m2Values[i]  = i;
          m2Options[i] = formatToHex_decimal(i);

          if (i == 0x10) {
            m2Options[i] += F(" - (default)");
          }
        }
        addFormSelector(F("MODE2"), F("p022_mode2"), PCA9685_MODE2_VALUES, m2Options, m2Values, mode2);
      }
      {
        String freqString = F("Frequency (");
        freqString += PCA9685_MIN_FREQUENCY;
        freqString += '-';
        freqString += PCA9685_MAX_FREQUENCY;
        freqString += ')';
        addFormNumericBox(freqString, F("p022_freq"), freq, PCA9685_MIN_FREQUENCY, PCA9685_MAX_FREQUENCY);
      }
      {
        String funitString = F("default ");
        funitString += PCA9685_MAX_FREQUENCY;
        addUnit(funitString);
      }
      {
        addFormNumericBox(F("Range (1-10000)"), F("p022_range"), range, 1, 10000);
        String runitString = F("default ");
        runitString += PCA9685_MAX_PWM;
        addUnit(runitString);
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      const uint8_t oldAddress = CONFIG_PORT;

      CONFIG_PORT = getFormItemInt(F("i2c_addr"));
      PCONFIG(0)  = getFormItemInt(F("p022_mode2"));
      PCONFIG(1)  = getFormItemInt(F("p022_freq"));
      PCONFIG(2)  = getFormItemInt(F("p022_range"));

      P022_data_struct *P022_data =
        static_cast<P022_data_struct *>(getPluginTaskData(event->TaskIndex));
      if (nullptr != P022_data) {
        P022_data->p022_clear_init(oldAddress);

        if (!P022_data->p022_is_init(CONFIG_PORT))
        {
          P022_data->Plugin_022_initialize(address);

          if (PCONFIG(0) != mode2) {
            P022_data->Plugin_022_writeRegister(address, PCA9685_MODE2, PCONFIG(0));
          }

          if (PCONFIG(1) != freq) {
            P022_data->Plugin_022_Frequency(address, PCONFIG(1));
          }
        }
      }
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P022_data_struct());
      P022_data_struct *P022_data =
        static_cast<P022_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P022_data) {
        success = true;
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P022_data_struct *P022_data =
        static_cast<P022_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P022_data) {
        break;
      }
      String log;
      String line           = string;
      String command;
      int    dotPos          = line.indexOf('.');
      bool   instanceCommand = false;

      if (dotPos > -1)
      {
        LoadTaskSettings(event->TaskIndex);
        String name = line.substring(0, dotPos);
        name.replace(F("["), F(""));
        name.replace(F("]"), F(""));

        if (name.equalsIgnoreCase(getTaskDeviceName(event->TaskIndex))) {
          line            = line.substring(dotPos + 1);
          instanceCommand = true;
        } else {
          break;
        }
      }
      command = parseString(line, 1);

      if ((command == F("pcapwm")) || (instanceCommand && (command == F("pwm"))))
      {
        success = true;
        log     = String(F("PCA 0x")) + String(address, HEX) + String(F(": PWM ")) + String(event->Par1);

        if ((event->Par1 >= 0) && (event->Par1 <= PCA9685_MAX_PINS))
        {
          if ((event->Par2 >= 0) && (event->Par2 <= range))
          {
            if (!P022_data->p022_is_init(address))
            {
              P022_data->Plugin_022_initialize(address);
              P022_data->Plugin_022_writeRegister(address, PCA9685_MODE2, mode2);
              P022_data->Plugin_022_Frequency(address, freq);
            }
            P022_data->Plugin_022_Write(address, event->Par1, map(event->Par2, 0, range, 0, PCA9685_MAX_PWM));

            // setPinState(PLUGIN_ID_022, event->Par1, PIN_MODE_PWM, event->Par2);
            portStatusStruct newStatus;
            const uint32_t   key = createKey(PLUGIN_ID_022, event->Par1);

            // WARNING: operator [] creates an entry in the map if key does not exist
            newStatus         = globalMapPortStatus[key];
            newStatus.command = 1;
            newStatus.mode    = PIN_MODE_PWM;
            newStatus.state   = event->Par2;
            savePortStatus(key, newStatus);

            addLog(LOG_LEVEL_INFO, log);

            // SendStatus(event, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_022, event->Par1, log, 0));
            SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);
          }
          else {
            addLog(LOG_LEVEL_ERROR, log + String(F(" the pwm value ")) + String(event->Par2) + String(F(" is invalid value.")));
          }
        }
        else {
          addLog(LOG_LEVEL_ERROR, log + String(F(" is invalid value.")));
        }
      }

      if ((command == F("pcafrq")) || (instanceCommand && (command == F("frq"))))
      {
        success = true;

        if ((event->Par1 >= PCA9685_MIN_FREQUENCY) && (event->Par1 <= PCA9685_MAX_FREQUENCY))
        {
          if (!P022_data->p022_is_init(address))
          {
            P022_data->Plugin_022_initialize(address);
            P022_data->Plugin_022_writeRegister(address, PCA9685_MODE2, mode2);
          }
          P022_data->Plugin_022_Frequency(address, event->Par1);

          // setPinState(PLUGIN_ID_022, 99, PIN_MODE_UNDEFINED, event->Par1);
          portStatusStruct newStatus;
          const uint32_t   key = createKey(PLUGIN_ID_022, 99);

          // WARNING: operator [] creates an entry in the map if key does not exist
          newStatus         = globalMapPortStatus[key];
          newStatus.command = 1;
          newStatus.mode    = PIN_MODE_UNDEFINED;
          newStatus.state   = event->Par1;
          savePortStatus(key, newStatus);

          log = String(F("PCA 0x")) + String(address, HEX) + String(F(": FREQ ")) + String(event->Par1);
          addLog(LOG_LEVEL_INFO, log);

          // SendStatus(event, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_022, 99, log, 0));
          SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);
        }
        else {
          addLog(LOG_LEVEL_ERROR,
                 String(F("PCA 0x")) +
                 String(address, HEX) + String(F(" The frequency ")) + String(event->Par1) + String(F(" is out of range.")));
        }
      }

      if (instanceCommand && (command == F("mode2")))
      {
        success = true;

        if ((event->Par1 >= 0) && (event->Par1 < PCA9685_MODE2_VALUES))
        {
          if (!P022_data->p022_is_init(address))
          {
            P022_data->Plugin_022_initialize(address);
            P022_data->Plugin_022_Frequency(address, freq);
          }
          P022_data->Plugin_022_writeRegister(address, PCA9685_MODE2, event->Par1);
          log = String(F("PCA 0x")) + String(address, HEX) + String(F(": MODE2 0x")) + String(event->Par1, HEX);
          addLog(LOG_LEVEL_INFO, log);
        }
        else {
          addLog(LOG_LEVEL_ERROR,
                 String(F("PCA 0x")) +
                 String(address, HEX) + String(F(" MODE2 0x")) + String(event->Par1, HEX) + String(F(" is out of range.")));
        }
      }

      if (command == F("status"))
      {
        if (parseString(line, 2) == F("pca"))
        {
          if (!P022_data->p022_is_init(address))
          {
            P022_data->Plugin_022_initialize(address);
            P022_data->Plugin_022_writeRegister(address, PCA9685_MODE2, mode2);
            P022_data->Plugin_022_Frequency(address, freq);
          }
          success = true;

          // SendStatus(event, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_022, event->Par2, dummyString, 0));
          SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, createKey(PLUGIN_ID_022, event->Par2), dummyString, 0);
        }
      }

      if (instanceCommand && (command == F("gpio")))
      {
        success = true;
        log     = String(F("PCA 0x")) + String(address, HEX) + String(F(": GPIO "));

        if ((event->Par1 >= 0) && (event->Par1 <= PCA9685_MAX_PINS))
        {
          if (!P022_data->p022_is_init(address))
          {
            P022_data->Plugin_022_initialize(address);
            P022_data->Plugin_022_writeRegister(address, PCA9685_MODE2, mode2);
            P022_data->Plugin_022_Frequency(address, freq);
          }
          int pin = event->Par1;

          if (parseString(line, 2) == "all")
          {
            pin  = -1;
            log += String(F("all"));
          }
          else
          {
            log += String(pin);
          }

          if (event->Par2 == 0)
          {
            log += F(" off");
            P022_data->Plugin_022_Off(address, pin);
          }
          else
          {
            log += F(" on");
            P022_data->Plugin_022_On(address, pin);
          }
          addLog(LOG_LEVEL_INFO, log);

          // setPinState(PLUGIN_ID_022, pin, PIN_MODE_OUTPUT, event->Par2);
          portStatusStruct newStatus;
          const uint32_t   key = createKey(PLUGIN_ID_022, pin);

          // WARNING: operator [] creates an entry in the map if key does not exist
          newStatus         = globalMapPortStatus[key];
          newStatus.command = 1;
          newStatus.mode    = PIN_MODE_OUTPUT;
          newStatus.state   = event->Par2;
          savePortStatus(key, newStatus);

          // SendStatus(event, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_022, pin, log, 0));
          SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);
        }
        else {
          addLog(LOG_LEVEL_ERROR, log + String(F(" is invalid value.")));
        }
      }

      if (instanceCommand && (command == F("pulse")))
      {
        success = true;
        log     = String(F("PCA 0x")) + String(address, HEX) + String(F(": GPIO ")) + String(event->Par1);

        if ((event->Par1 >= 0) && (event->Par1 <= PCA9685_MAX_PINS))
        {
          if (!P022_data->p022_is_init(address))
          {
            P022_data->Plugin_022_initialize(address);
            P022_data->Plugin_022_writeRegister(address, PCA9685_MODE2, mode2);
            P022_data->Plugin_022_Frequency(address, freq);
          }

          if (event->Par2 == 0)
          {
            log += F(" off");
            P022_data->Plugin_022_Off(address, event->Par1);
          }
          else
          {
            log += F(" on");
            P022_data->Plugin_022_On(address, event->Par1);
          }
          log += String(F(" Pulse set for ")) + event->Par3;
          log += String(F("ms"));
          int autoreset = 0;

          if (event->Par3 > 0)
          {
            if (parseString(line, 5) == F("auto"))
            {
              autoreset = -1;
              log      += String(F(" with autoreset infinity"));
            }
            else
            {
              autoreset = event->Par4;

              if (autoreset > 0)
              {
                log += String(F(" for "));
                log += String(autoreset);
              }
            }
          }
          Scheduler.setPluginTaskTimer(event->Par3
                                       , event->TaskIndex
                                       , event->Par1
                                       , !event->Par2
                                       , event->Par3
                                       , autoreset);

          // setPinState(PLUGIN_ID_022, event->Par1, PIN_MODE_OUTPUT, event->Par2);
          portStatusStruct newStatus;
          const uint32_t   key = createKey(PLUGIN_ID_022, event->Par1);

          // WARNING: operator [] creates an entry in the map if key does not exist
          newStatus         = globalMapPortStatus[key];
          newStatus.command = 1;
          newStatus.mode    = PIN_MODE_OUTPUT;
          newStatus.state   = event->Par2;
          savePortStatus(key, newStatus);

          addLog(LOG_LEVEL_INFO, log);

          // SendStatus(event, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_022, event->Par1, log, 0));
          SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);
        }
        else {
          addLog(LOG_LEVEL_ERROR, log + String(F(" is invalid value.")));
        }
      }

      break;
    }
    case PLUGIN_TIMER_IN:
    {
      P022_data_struct *P022_data =
        static_cast<P022_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P022_data) {
        String log       = String(F("PCA 0x")) + String(address, HEX) + String(F(": GPIO ")) + String(event->Par1);
        int    autoreset = event->Par4;

        if (event->Par2 == 0)
        {
          log += F(" off");
          P022_data->Plugin_022_Off(address, event->Par1);
        }
        else
        {
          log += F(" on");
          P022_data->Plugin_022_On(address, event->Par1);
        }

        if ((autoreset > 0) || (autoreset == -1))
        {
          if (autoreset > -1)
          {
            log += String(F(" Pulse auto restart for "));
            log += String(autoreset);
            autoreset--;
          }
          Scheduler.setPluginTaskTimer(event->Par3
                                      , event->TaskIndex
                                      , event->Par1
                                      , !event->Par2
                                      , event->Par3
                                      , autoreset);
        }

        // setPinState(PLUGIN_ID_022, event->Par1, PIN_MODE_OUTPUT, event->Par2);
        portStatusStruct newStatus;
        const uint32_t   key = createKey(PLUGIN_ID_022, event->Par1);

        // WARNING: operator [] creates an entry in the map if key does not exist
        newStatus         = globalMapPortStatus[key];
        newStatus.command = 1;
        newStatus.mode    = PIN_MODE_OUTPUT;
        newStatus.state   = event->Par2;
        savePortStatus(key, newStatus);

        // SendStatus(event, getPinStateJSON(SEARCH_PIN_STATE, PLUGIN_ID_022, event->Par1, log, 0));
        SendStatusOnlyIfNeeded(event, SEARCH_PIN_STATE, key, log, 0);
      }
      break;
    }
  }
  return success;
}

#endif // USES_P022
