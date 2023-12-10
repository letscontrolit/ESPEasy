#include "_Plugin_Helper.h"

#ifdef USES_P043

// #######################################################################################################
// #################################### Plugin 043: Clock Output #########################################
// #######################################################################################################

/** Changelog:
 * 2023-12-10 tonhuisman: Change input layout for non-LIMIT_BUILD_SIZE builds, to select a day and a time string in separate
 *                        inputs.
 *                        Add setting for number of Day,Time settings (range 1..16), default is 8.
 * 2023-12-07 tonhuisman: Add support for %sunrise[+/-offsetHMS]% and %sunset[+/-offsetHMS]% format in constants
 *                        also supports long offsets up to 32767 seconds using S suffix in offset
 * 2023-12-06 tonhuisman: Add changelog
 */

# include "src/Helpers/StringGenerator_Web.h"

# define PLUGIN_043
# define PLUGIN_ID_043         43
# define PLUGIN_NAME_043       "Output - Clock"
# define PLUGIN_VALUENAME_043  "Output"

// #define PLUGIN_VALUENAME1_043 "Output"
// #define PLUGIN_VALUENAME2_043 "Output2"
# define PLUGIN_043_MAX_SETTINGS PCONFIG(7)
# define P043_DEFAULT_MAX        8
# define P043_SENSOR_TYPE_INDEX  2
# define P043_NR_OUTPUT_VALUES   getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P043_SENSOR_TYPE_INDEX)))


boolean Plugin_043(uint8_t function, struct EventStruct *event, String& string)
{
  # ifndef LIMIT_BUILD_SIZE
  const String weekDays = F("AllSunMonTueWedThuFriSatWrkWkd");
  # endif // ifndef LIMIT_BUILD_SIZE
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

    case PLUGIN_SET_DEFAULTS:
    {
      PLUGIN_043_MAX_SETTINGS = P043_DEFAULT_MAX;
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
      if (PLUGIN_043_MAX_SETTINGS == 0) { PLUGIN_043_MAX_SETTINGS = P043_DEFAULT_MAX; }
      addFormNumericBox(F("Nr. of Day,Time fields"), F("vcount"), PLUGIN_043_MAX_SETTINGS, 1, 16);
      addUnit(F("1..16"));
      # ifndef LIMIT_BUILD_SIZE
      addFormNote(F("Page will be updated after Submit"));
      # endif // ifndef LIMIT_BUILD_SIZE

      const __FlashStringHelper *options[] = {
        F(""),
        F("Off"),
        F("On"),
      };

      # ifndef LIMIT_BUILD_SIZE
      const unsigned int daysCount = weekDays.length() / 3;
      String days[daysCount];

      for (unsigned int n = 0; n < weekDays.length() / 3u; ++n) {
        days[n] = weekDays.substring(n * 3, n * 3 + 3);
      }

      datalistStart(F("timepatternlist"));
      datalistAddValue(F("00:00"));
      datalistAddValue(F("%sunrise%"));
      datalistAddValue(F("%sunset%"));
      datalistFinish();
      # endif // ifndef LIMIT_BUILD_SIZE

      for (int x = 0; x < PLUGIN_043_MAX_SETTINGS; x++)
      {
        const String timeStr = timeLong2String(Cache.getTaskDevicePluginConfigLong(event->TaskIndex, x));
        # ifndef LIMIT_BUILD_SIZE
        addRowLabel(concat(F("Day,Time "), x + 1));
        int thisDay = weekDays.indexOf(timeStr.substring(0, 3));

        if (thisDay > 0) { thisDay /= 3; }
        addSelector(concat(F("day"), x), daysCount, days, nullptr, nullptr, thisDay, false, true, F(""));
        addHtml(',');
        addTextBox(concat(F("clock"), x),
                   parseString(timeStr, 2), 32
                   , false, false, EMPTY_STRING, F("")
                   #  if FEATURE_TOOLTIPS
                   , EMPTY_STRING
                   #  endif // if FEATURE_TOOLTIPS
                   , F("timepatternlist"));
        # else // ifndef LIMIT_BUILD_SIZE
        addFormTextBox(concat(F("Day,Time "), x + 1),
                       concat(F("clock"), x),
                       timeStr, 32);
        # endif // ifndef LIMIT_BUILD_SIZE

        if (validGpio(CONFIG_PIN1)) {
          addHtml(' ');
          const uint8_t choice       = Cache.getTaskDevicePluginConfig(event->TaskIndex, x);
          constexpr int optionsCount = NR_ELEMENTS(options);
          addSelector(concat(F("state"), x), optionsCount, options, nullptr, nullptr, choice);
        }
        else {
          addFormNumericBox(concat(F("Value"), x + 1),
                            concat(F("state"), x),
                            Cache.getTaskDevicePluginConfig(event->TaskIndex, x));
        }
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PLUGIN_043_MAX_SETTINGS = getFormItemInt(F("vcount"));

      for (int x = 0; x < PLUGIN_043_MAX_SETTINGS; x++)
      {
        String plugin1;
        # ifndef LIMIT_BUILD_SIZE
        const int day = getFormItemInt(concat(F("day"), x));
        plugin1 = strformat(F("%s,%s"), weekDays.substring(day * 3, day * 3 + 3).c_str(), webArg(concat(F("clock"), x)).c_str());
        # else // ifndef LIMIT_BUILD_SIZE
        plugin1 = webArg(concat(F("clock"), x));
        # endif // ifndef LIMIT_BUILD_SIZE
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
      if (PLUGIN_043_MAX_SETTINGS == 0) { PLUGIN_043_MAX_SETTINGS = P043_DEFAULT_MAX; }

      for (uint8_t x = 0; x < PLUGIN_043_MAX_SETTINGS; x++)
      {
        unsigned long clockEvent = (unsigned long)node_time.minute() % 10
                                   | (unsigned long)(node_time.minute() / 10) << 4
                                   | (unsigned long)(node_time.hour() % 10) << 8
                                   | (unsigned long)(node_time.hour() / 10) << 12
                                   | (unsigned long)node_time.weekday() << 16;
        unsigned long clockSet = Cache.getTaskDevicePluginConfigLong(event->TaskIndex, x);

        if (bitRead(clockSet, 28) || bitRead(clockSet, 29)) { // sunrise or sunset string, apply todays values
          String specialTime = timeLong2String(clockSet);

          parseSystemVariables(specialTime, false);           // Parse systemvariables only, to reduce processing
          clockSet = string2TimeLong(specialTime);
        }

        if (matchClockEvent(clockEvent, clockSet))
        {
          uint8_t state = Cache.getTaskDevicePluginConfig(event->TaskIndex, x);

          if (state != 0)
          {
            if (validGpio(CONFIG_PIN1)) { // if GPIO is specified, use the old behavior
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
              addLog(LOG_LEVEL_INFO, strformat(F("TCLK : State %d"), state));
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
