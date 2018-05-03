#ifdef USES_P059
//#######################################################################################################
//#################################### Plugin 059: Rotary Encoder #######################################
//#######################################################################################################

// ESPEasy Plugin to process the quadrature encoder interface signals (e.g. rotary encoder)
// written by Jochen Krapf (jk@nerd2nerd.org)

// Connection:
// Use 1st and 2nd GPIO for encoders A and B signal.
// Optional use 3rd GPIO for encoders I signal to reset counter to 0 at first trigger.
// If counter runs in wrong direction, change A and B GPIOs in settings page

// Note: Up to 4 encoders can be used simultaneously



#define PLUGIN_059
#define PLUGIN_ID_059         59
#define PLUGIN_NAME_059       "Switch Input - Rotary Encoder"
#define PLUGIN_VALUENAME1_059 "Counter"

#include <QEIx4.h>

QEIx4* Plugin_059_QE = NULL;

#ifndef CONFIG
#define CONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][n])
#endif
#ifndef CONFIG_L
#define CONFIG_L(n) (Settings.TaskDevicePluginConfigLong[event->TaskIndex][n])
#endif
#ifndef PIN
#define PIN(n) (Settings.TaskDevicePin[n][event->TaskIndex])
#endif


boolean Plugin_059(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_059;
        Device[deviceCount].Type = DEVICE_TYPE_TRIPLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
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
        event->String1 = F("GPIO &larr; A");
        event->String2 = F("GPIO &larr; B");
        event->String3 = F("GPIO &#8672; I (optional)");
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        // default values
        if (CONFIG_L(0) == 0 && CONFIG_L(1) == 0)
          CONFIG_L(1) = 100;

        String options[3] = { F("1 pulse per cycle"), F("2 pulses per cycle"), F("4 pulses per cycle") };
        int optionValues[3] = { 1, 2, 4 };
        addFormSelector(F("Mode"), F("qei_mode"), 3, options, optionValues, CONFIG(0));

        addFormNumericBox(F("Limit min."), F("qei_limitmin"), CONFIG_L(0));
        addFormNumericBox(F("Limit max."), F("qei_limitmax"), CONFIG_L(1));

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        CONFIG(0) = getFormItemInt(F("qei_mode"));

        CONFIG_L(0) = getFormItemInt(F("qei_limitmin"));
        CONFIG_L(1) = getFormItemInt(F("qei_limitmax"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (!Plugin_059_QE)
          Plugin_059_QE = new QEIx4;

        Plugin_059_QE->begin(PIN(0),PIN(1),PIN(2),CONFIG(0));
        Plugin_059_QE->setLimit(CONFIG_L(0), CONFIG_L(1));
        Plugin_059_QE->setIndexTrigger(true);

        ExtraTaskSettings.TaskDeviceValueDecimals[event->BaseVarIndex] = 0;

        String log = F("QEI  : GPIO: ");
        for (byte i=0; i<3; i++)
        {
          int pin = PIN(i);
          if (pin >= 0)
          {
            //pinMode(pin, (Settings.TaskDevicePin1PullUp[event->TaskIndex]) ? INPUT_PULLUP : INPUT);
            setPinState(PLUGIN_ID_059, pin, PIN_MODE_INPUT, 0);
          }
          log += pin;
          log += F(" ");
        }
        addLog(LOG_LEVEL_INFO, log);

        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (Plugin_059_QE)
        {
          if (Plugin_059_QE->hasChanged())
          {
            long c = Plugin_059_QE->read();
            UserVar[event->BaseVarIndex] = (float)c;
            event->sensorType = SENSOR_TYPE_SWITCH;

            String log = F("QEI  : ");
            log += c;
            addLog(LOG_LEVEL_INFO, log);

            sendData(event);
          }

        }
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        if (Plugin_059_QE)
        {
          UserVar[event->BaseVarIndex] = (float)Plugin_059_QE->read();
        }
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        if (Plugin_059_QE)
        {
            String log = "";
            String command = parseString(string, 1);
            if (command == F("encwrite"))
            {
              if (event->Par1 >= 0)
              {
                log = String(F("QEI  : ")) + string;
                addLog(LOG_LEVEL_INFO, log);
                Plugin_059_QE->write(event->Par1);
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
