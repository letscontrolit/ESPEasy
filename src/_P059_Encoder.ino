#include "_Plugin_Helper.h"

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

std::map<unsigned int, std::shared_ptr<QEIx4> > P_059_sensordefs;

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
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_SWITCH;
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
        event->String1 = formatGpioName_input(F("A (CLK)"));
        event->String2 = formatGpioName_input(F("B (DT)"));
        event->String3 = formatGpioName_input_optional(F("I (Z)"));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        // default values
        if (PCONFIG_LONG(0) == 0 && PCONFIG_LONG(1) == 0)
          PCONFIG_LONG(1) = 100;

        String options[3] = { F("1 pulse per cycle"), F("2 pulses per cycle"), F("4 pulses per cycle") };
        int optionValues[3] = { 1, 2, 4 };
        addFormSelector(F("Mode"), F("qei_mode"), 3, options, optionValues, PCONFIG(0));

        addFormNumericBox(F("Limit min."), F("qei_limitmin"), PCONFIG_LONG(0));
        addFormNumericBox(F("Limit max."), F("qei_limitmax"), PCONFIG_LONG(1));

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        PCONFIG(0) = getFormItemInt(F("qei_mode"));

        PCONFIG_LONG(0) = getFormItemInt(F("qei_limitmin"));
        PCONFIG_LONG(1) = getFormItemInt(F("qei_limitmax"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        portStatusStruct newStatus;

        // create sensor instance and add to std::map
        P_059_sensordefs.erase(event->TaskIndex);
        P_059_sensordefs[event->TaskIndex] = std::shared_ptr<QEIx4>(new QEIx4);

        P_059_sensordefs[event->TaskIndex]->begin(CONFIG_PIN1,CONFIG_PIN2,CONFIG_PIN3,PCONFIG(0));
        P_059_sensordefs[event->TaskIndex]->setLimit(PCONFIG_LONG(0), PCONFIG_LONG(1));
        P_059_sensordefs[event->TaskIndex]->setIndexTrigger(true);

        ExtraTaskSettings.TaskDeviceValueDecimals[event->BaseVarIndex] = 0;

        String log = F("QEI  : GPIO: ");
        for (byte i=0; i<3; i++)
        {
          int pin = PIN(i);
          if (pin >= 0)
          {
            //pinMode(pin, (Settings.TaskDevicePin1PullUp[event->TaskIndex]) ? INPUT_PULLUP : INPUT);
            const uint32_t key = createKey(PLUGIN_ID_059,pin);
            // WARNING: operator [] creates an entry in the map if key does not exist
            newStatus = globalMapPortStatus[key];
            newStatus.task++; // add this GPIO/port as a task
            newStatus.mode = PIN_MODE_INPUT;
            newStatus.state = 0;
            savePortStatus(key,newStatus);
            //setPinState(PLUGIN_ID_059, pin, PIN_MODE_INPUT, 0);
          }
          log += pin;
          log += ' ';
        }
        addLog(LOG_LEVEL_INFO, log);

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
            long c = P_059_sensordefs[event->TaskIndex]->read();
            UserVar[event->BaseVarIndex] = (float)c;
            event->sensorType = Sensor_VType::SENSOR_TYPE_SWITCH;

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
        if (P_059_sensordefs.count(event->TaskIndex) != 0)
        {
          UserVar[event->BaseVarIndex] = (float)P_059_sensordefs[event->TaskIndex]->read();
        }
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        if (P_059_sensordefs.count(event->TaskIndex) != 0)
        {
            String log = "";
            String command = parseString(string, 1);
            if (command == F("encwrite"))
            {
              if (event->Par1 >= 0)
              {
                log = String(F("QEI  : ")) + string;
                addLog(LOG_LEVEL_INFO, log);
                P_059_sensordefs[event->TaskIndex]->write(event->Par1);
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
