#include "_Plugin_Helper.h"

#ifdef USES_P043

// #######################################################################################################
// #################################### Plugin 043: Clock Output #########################################
// #######################################################################################################


# define PLUGIN_043
# define PLUGIN_ID_043         43
# define PLUGIN_NAME_043       "Output - Clock"
# define PLUGIN_VALUENAME_043 "Output"

// #define PLUGIN_VALUENAME1_043 "Output"
// #define PLUGIN_VALUENAME2_043 "Output2"
# define PLUGIN_043_MAX_SETTINGS 8
# define P043_SENSOR_TYPE_INDEX  2
# define P043_NR_OUTPUT_VALUES   getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P043_SENSOR_TYPE_INDEX)))


boolean Plugin_043(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_043;
      Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SWITCH;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 2;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].OutputDataType     = Output_Data_type_t::Simple;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_043);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      ExtraTaskSettings.populateDeviceValueNamesSeq(F("Output"), P043_NR_OUTPUT_VALUES, 2, false);
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P043_NR_OUTPUT_VALUES;
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(P043_SENSOR_TYPE_INDEX));
      event->idx        = P043_SENSOR_TYPE_INDEX;
      success           = true;
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_output(F("Clock Event"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      const __FlashStringHelper *options[3] = {
        F(""),
        F("Off"),
        F("On"),
      };

      for (int x = 0; x < PLUGIN_043_MAX_SETTINGS; x++)
      {
        addFormTextBox(
          concat(F("Day,Time "), x + 1),
          concat(F("clock"),     x),
          timeLong2String(Cache.getTaskDevicePluginConfigLong(event->TaskIndex, x)), 32);

        if (CONFIG_PIN1 >= 0) {
          addHtml(' ');
          const uint8_t choice = Cache.getTaskDevicePluginConfig(event->TaskIndex, x);
          addSelector(concat(F("state"), x), 3, options, nullptr, nullptr, choice);
        }
        else { addFormNumericBox(
                 concat(F("Value"), x + 1),
                 concat(F("state"), x),
                 Cache.getTaskDevicePluginConfig(event->TaskIndex, x)); }
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      for (int x = 0; x < PLUGIN_043_MAX_SETTINGS; x++)
      {
        const String plugin1 = webArg(concat(F("clock"), x));
        ExtraTaskSettings.TaskDevicePluginConfigLong[x] = string2TimeLong(plugin1);
        const String plugin2 = webArg(concat(F("state"), x));
        ExtraTaskSettings.TaskDevicePluginConfig[x] = plugin2.toInt();
      }
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      success = true;
      break;
    }

    case PLUGIN_CLOCK_IN:
    {
      for (uint8_t x = 0; x < PLUGIN_043_MAX_SETTINGS; x++)
      {
        unsigned long clockEvent = (unsigned long)node_time.minute() % 10 | (unsigned long)(node_time.minute() / 10) <<
        4 | (unsigned long)(node_time.hour() % 10) << 8 | (unsigned long)(node_time.hour() / 10) << 12 | (unsigned long)node_time.weekday() <<
        16;
        unsigned long clockSet = Cache.getTaskDevicePluginConfigLong(event->TaskIndex, x);

        if (matchClockEvent(clockEvent, clockSet))
        {
          uint8_t state = Cache.getTaskDevicePluginConfig(event->TaskIndex, x);

          if (state != 0)
          {
            if (CONFIG_PIN1 >= 0) { // if GPIO is specified, use the old behavior
              state--;
              pinMode(CONFIG_PIN1, OUTPUT);
              digitalWrite(CONFIG_PIN1, state);
              UserVar[event->BaseVarIndex] = state;
            }
            else {
              UserVar[event->BaseVarIndex]     = x + 1;
              UserVar[event->BaseVarIndex + 1] = state;
            }

            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              String log = F("TCLK : State ");
              log += state;
              addLogMove(LOG_LEVEL_INFO, log);
            }
            sendData(event);
          }
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P043
