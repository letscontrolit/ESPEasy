#include "_Plugin_Helper.h"

#ifdef USES_P059

// #######################################################################################################
// #################################### Plugin 059: Rotary Encoder #######################################
// #######################################################################################################

/** Changelog:
 * 2025-01-04 tonhuisman: Minor code cleanup
 */

// ESPEasy Plugin to process the quadrature encoder interface signals (e.g. rotary encoder)
// written by Jochen Krapf (jk@nerd2nerd.org)

// Connection:
// Use 1st and 2nd GPIO for encoders A and B signal.
// Optional use 3rd GPIO for encoders I signal to reset counter to 0 at first trigger.
// If counter runs in wrong direction, change A and B GPIOs in settings page

// Note: Up to 4 encoders can be used simultaneously


# define PLUGIN_059
# define PLUGIN_ID_059         59
# define PLUGIN_NAME_059       "Switch Input - Rotary Encoder"
# define PLUGIN_VALUENAME1_059 "Counter"

# include <QEIx4.h>

std::map<unsigned int, std::shared_ptr<QEIx4> > P_059_sensordefs;

boolean Plugin_059(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_059;
      dev.Type           = DEVICE_TYPE_TRIPLE;
      dev.VType          = Sensor_VType::SENSOR_TYPE_SWITCH;
      dev.ValueCount     = 1;
      dev.SendDataOption = true;
      dev.TimerOption    = true;
      dev.TimerOptional  = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_059);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_059));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_input(F("A (CLK)"));
      event->String2 = formatGpioName_input(F("B (DT)"));
      event->String3 = formatGpioName_input_optional(F("I (Z)"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      // default values
      if ((PCONFIG_LONG(0) == 0) && (PCONFIG_LONG(1) == 0)) {
        PCONFIG_LONG(1) = 100;
      }

      {
        const __FlashStringHelper *options[] = { F("1"), F("2"), F("4") };
        const int optionValues[]             = { 1, 2, 4 };
        constexpr size_t optionCount         = NR_ELEMENTS(optionValues);
        addFormSelector(F("Mode"), F("mode"), optionCount, options, optionValues, PCONFIG(0));
        addUnit(F("pulses per cycle"));
      }

      addFormNumericBox(F("Limit min."), F("limitmin"), PCONFIG_LONG(0));
      addFormNumericBox(F("Limit max."), F("limitmax"), PCONFIG_LONG(1));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("mode"));

      PCONFIG_LONG(0) = getFormItemInt(F("limitmin"));
      PCONFIG_LONG(1) = getFormItemInt(F("limitmax"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      portStatusStruct newStatus;

      // create sensor instance and add to std::map
      P_059_sensordefs.erase(event->TaskIndex);
      P_059_sensordefs[event->TaskIndex] = std::shared_ptr<QEIx4>(new QEIx4);

      P_059_sensordefs[event->TaskIndex]->begin(CONFIG_PIN1, CONFIG_PIN2, CONFIG_PIN3, PCONFIG(0));
      P_059_sensordefs[event->TaskIndex]->setLimit(PCONFIG_LONG(0), PCONFIG_LONG(1));
      P_059_sensordefs[event->TaskIndex]->setIndexTrigger(true);

      ExtraTaskSettings.TaskDeviceValueDecimals[event->BaseVarIndex] = 0;

      String log = F("QEI  : GPIO: ");

      for (uint8_t i = 0; i < 3; ++i)
      {
        const int pin = PIN(i);

        if (validGpio(pin))
        {
          // pinMode(pin, (Settings.TaskDevicePin1PullUp[event->TaskIndex]) ? INPUT_PULLUP : INPUT);
          constexpr pluginID_t P059_PLUGIN_ID{ PLUGIN_ID_059 };

          const uint32_t key = createKey(P059_PLUGIN_ID, pin);

          // WARNING: operator [] creates an entry in the map if key does not exist
          newStatus = globalMapPortStatus[key];
          newStatus.task++; // add this GPIO/port as a task
          newStatus.mode  = PIN_MODE_INPUT;
          newStatus.state = 0;
          savePortStatus(key, newStatus);

          // setPinState(PLUGIN_ID_059, pin, PIN_MODE_INPUT, 0);
        }
        log += pin;
        log += ' ';
      }
      addLogMove(LOG_LEVEL_INFO, log);

      success = true;
      break;
    }

    case PLUGIN_EXIT:
    {
      P_059_sensordefs.erase(event->TaskIndex);
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      if (P_059_sensordefs.count(event->TaskIndex) != 0)
      {
        if (P_059_sensordefs[event->TaskIndex]->hasChanged())
        {
          const long c = P_059_sensordefs[event->TaskIndex]->read();
          UserVar.setFloat(event->TaskIndex, 0, c);

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            addLog(LOG_LEVEL_INFO, concat(F("QEI  : "), c));
          }

          sendData(event);
        }
      }
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      if (P_059_sensordefs.count(event->TaskIndex) != 0)
      {
        UserVar.setFloat(event->TaskIndex, 0, P_059_sensordefs[event->TaskIndex]->read());
      }
      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      if (P_059_sensordefs.count(event->TaskIndex) != 0)
      {
        const String command = parseString(string, 1);

        if (equals(command, F("encwrite")))
        {
          if (event->Par1 >= 0)
          {
            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              addLog(LOG_LEVEL_INFO, concat(F("QEI  : "), string));
            }
            P_059_sensordefs[event->TaskIndex]->write(event->Par1);
            Scheduler.schedule_task_device_timer(event->TaskIndex, millis());
          }
          success = true; // Command is handled.
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P059
