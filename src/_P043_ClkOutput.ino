#include "_Plugin_Helper.h"

#ifdef USES_P043

// #######################################################################################################
// #################################### Plugin 043: Clock Output #########################################
// #######################################################################################################

/** Changelog:
 * 2023-12-28 tonhuisman: Exclude some code that won't work for PLUGIN_BUILD_MINIMAL_OTA builds
 * 2023-12-19 tonhuisman: Fix Value input On/Off only to behave exactly like when using a GPIO, except for changing the GPIO state
 * 2023-12-16 tonhuisman: Add support for _GET_CONFIG_VALUE function, with [Clock#GetTimeX] and [Clock#GetValueX], where X is
 *                        in range 1..Nr. of Day,Time fields (PLUGIN_EXTRACONFIGVAR_MAX = 16)
 *                        Renamed setting PLUGIN_043_MAX_SETTINGS to P043_MAX_SETTINGS (to avoid confusion)
 * 2023-12-14 tonhuisman: Fix 'Simplified' mode to behave like GPIO mode, so state Off = 0 and On = 1
 *                        Add support for config command: config,task,<taskName>,SetTime,<timeIndex>,<timeString>[],<value>]
 *                        Use convention of accepting '$' for '%' in <timeString> value, to prevent todays values to be used
 * 2023-12-12 tonhuisman: Add option to choose simplified Off/On input instead of full numeric input for non-GPIO configuration
 * 2023-12-11 tonhuisman: Put Value X input on same line as Day,Time X inputs, just like the On/Off combobox.
 *                        Code optimization, calculating the current time to compare to only once.
 * 2023-12-10 tonhuisman: Change input layout for non-LIMIT_BUILD_SIZE builds, to select a day and a time string in separate
 *                        inputs.
 *                        Add setting for number of Day,Time settings (range 1..16), default is 8.
 * 2023-12-07 tonhuisman: Add support for %sunrise[+/-offsetHMS]% and %sunset[+/-offsetHMS]% format in constants
 *                        also supports long offsets up to 32767 seconds using S suffix in offset
 * 2023-12-06 tonhuisman: Add changelog
 */

/** Supported command:
 * config,task,<taskName>,SetTime,<timeIndex>,<timeString>[],<value>]
 * <timeIndex>: Range 1..number of Day,Time fields
 * <timeString>: As entered in the UI: Mon,12:34, can also be quoted: "Mon,12:34". Day name has to be 3 letters
 * To enter %Sunrise% or %Sunset-1h% etc. use $Sunrise$ or $Sunset-1h$ to prevent todays values to be used
 * ($ for % is a convention introduced in P036)
 * <value>: (Optional) Use 0 or 1 for GPIO configuration or when 'Value input On/Off only' is enabled
 *          For non-GPIO and 'Value input On/Off only' is off, then value 0 won't cause an event to be generated!
 */

/** Supported values:
 * GetTimeX:  Get the time string for Day,Time line X
 * GetValueX: Get the configured value for Day,Time line X.
 *            With a configured GPIO or 'Value input On/Off only' enabled, the value will be converted to Off=0, On=1
 * NB: X is in range 1..Nr. of Day,Time fields
 */

# include "src/Helpers/StringGenerator_Web.h"

# define PLUGIN_043
# define PLUGIN_ID_043         43
# define PLUGIN_NAME_043       "Output - Clock"
# define PLUGIN_VALUENAME_043  "Output"

# define P043_SIMPLE_VALUE       PCONFIG(6)
# define P043_MAX_SETTINGS       PCONFIG(7)
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
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_043;
      dev.Type           = DEVICE_TYPE_SINGLE;
      dev.VType          = Sensor_VType::SENSOR_TYPE_SWITCH;
      dev.ValueCount     = 2;
      dev.SendDataOption = true;
      dev.OutputDataType = Output_Data_type_t::Simple;
      dev.setPin1Direction(gpio_direction::gpio_output);
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
      P043_MAX_SETTINGS = P043_DEFAULT_MAX;
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
      if (P043_MAX_SETTINGS == 0) { P043_MAX_SETTINGS = P043_DEFAULT_MAX; }
      addFormNumericBox(F("Nr. of Day,Time fields"), F("vcount"), P043_MAX_SETTINGS, 1, PLUGIN_EXTRACONFIGVAR_MAX);
      addUnit(concat(F("1.."), PLUGIN_EXTRACONFIGVAR_MAX));

      addFormCheckBox(F("Value input On/Off only"), F("simpl"), P043_SIMPLE_VALUE == 1);
      # ifndef LIMIT_BUILD_SIZE
      addFormNote(F("Page will be updated after Submit"));
      # endif // ifndef LIMIT_BUILD_SIZE

      const __FlashStringHelper *options[] = {
        F(""),
        F("Off"),
        F("On"),
      };
      constexpr int optionsCount = NR_ELEMENTS(options);

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

      for (int x = 0; x < P043_MAX_SETTINGS; x++)
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

        if (validGpio(CONFIG_PIN1) || (P043_SIMPLE_VALUE == 1)) {
          addHtml(' ');
          const uint8_t choice = Cache.getTaskDevicePluginConfig(event->TaskIndex, x);
          addSelector(concat(F("state"), x), optionsCount, options, nullptr, nullptr, choice);
        }
        else {
          addHtml(strformat(F("Value %d:"), x + 1));
          addNumericBox(concat(F("state"), x),
                        Cache.getTaskDevicePluginConfig(event->TaskIndex, x), INT_MIN, INT_MAX);
        }
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P043_MAX_SETTINGS = getFormItemInt(F("vcount"));
      P043_SIMPLE_VALUE = isFormItemChecked(F("simpl")) ? 1 : 0;

      for (int x = 0; x < P043_MAX_SETTINGS; x++)
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
      if (P043_MAX_SETTINGS == 0) { P043_MAX_SETTINGS = P043_DEFAULT_MAX; }

      unsigned long clockEvent = (unsigned long)node_time.minute() % 10
                                 | (unsigned long)(node_time.minute() / 10) << 4
                                 | (unsigned long)(node_time.hour() % 10) << 8
                                 | (unsigned long)(node_time.hour() / 10) << 12
                                 | (unsigned long)node_time.weekday() << 16;

      for (uint8_t x = 0; x < P043_MAX_SETTINGS; x++)
      {
        unsigned long clockSet = Cache.getTaskDevicePluginConfigLong(event->TaskIndex, x);

        # ifndef PLUGIN_BUILD_MINIMAL_OTA

        if (bitRead(clockSet, 28) || bitRead(clockSet, 29)) { // sunrise or sunset string, apply todays values
          String specialTime = timeLong2String(clockSet);

          parseSystemVariables(specialTime, false);           // Parse systemvariables only, to reduce processing
          clockSet = string2TimeLong(specialTime);
        }
        # endif // ifndef PLUGIN_BUILD_MINIMAL_OTA

        if (matchClockEvent(clockEvent, clockSet))
        {
          uint8_t state = Cache.getTaskDevicePluginConfig(event->TaskIndex, x);

          if (state != 0)
          {
            const bool hasGpio = validGpio(CONFIG_PIN1);

            if (hasGpio || (P043_SIMPLE_VALUE == 1)) { // if GPIO or Yes/No selection is specified, use the old behavior
              state--;

              if (hasGpio) {
                pinMode(CONFIG_PIN1, OUTPUT);
                digitalWrite(CONFIG_PIN1, state);
              }
              UserVar.setFloat(event->TaskIndex, 0, state);
            }
            else {
              UserVar.setFloat(event->TaskIndex, 0, x + 1);
              UserVar.setFloat(event->TaskIndex, 1, state);
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

    case PLUGIN_SET_CONFIG:
    {
      const String cmd = parseString(string, 1);

      // command: config,task,<taskname>,SetTime,<timeIndex>,<timeString>,<value>
      if (equals(cmd, F("settime"))) {
        String  para      = parseString(string, 2);
        int32_t timeIndex = 0;

        if (validIntFromString(para, timeIndex) && (timeIndex > 0) && (timeIndex <= P043_MAX_SETTINGS)) {
          para = parseString(string, 3);
          String para4       = parseString(string, 4);
          const String para5 = parseString(string, 5);
          int32_t value      = INT_MIN;

          if ((para.length() == 3) && !para4.isEmpty() && !para5.isEmpty()) {
            // handle timeString without quotes: All,12:34,<value> instead of "All,12:34",<value>
            para += ','; // (most compact code...)
            para += para4;
            para4 = para5;
          }

          # ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG, concat(F("P043: Time received "), para));
          # endif // ifndef BUILD_NO_DEBUG

          para.replace('$', '%');             // Allow %Sunrise%/%Sunset% by using $ instead of %

          LoadTaskSettings(event->TaskIndex); // Not preloaded

          ExtraTaskSettings.TaskDevicePluginConfigLong[timeIndex - 1] = string2TimeLong(para);

          if (validIntFromString(para4, value)) { // Value is optional
            if (validGpio(CONFIG_PIN1) || (P043_SIMPLE_VALUE == 1)) { value++; } // Off is stored as 1, On is stored as 2 for GPIO action

            ExtraTaskSettings.TaskDevicePluginConfig[timeIndex - 1] = value;
          }

          Cache.updateExtraTaskSettingsCache();
          SaveTaskSettings(event->TaskIndex); // Unfortunately we have to save the settings here, or they will get lost
          // Using too often will wear out the flash memory quickly!

          # ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG, strformat(F("P043: Time received %s, value %d, stored %s, long: %d"), para.c_str(), value,
                                            timeLong2String(ExtraTaskSettings.TaskDevicePluginConfigLong[timeIndex - 1]).c_str(),
                                            string2TimeLong(para)));
          # endif // ifndef BUILD_NO_DEBUG
          success = true;
        }
      }
      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      # define P043_GETTIME_LENGTH  7u // Length of 'gettime'
      # define P043_GETVALUE_LENGTH 8u // Length of 'getvalue'
      const String cmd  = parseString(string, 1);
      unsigned int idx  = 0u;
      int32_t timeIndex = -1;

      if (cmd.startsWith(F("gettime"))) {
        idx = P043_GETTIME_LENGTH;
      } else
      if (cmd.startsWith(F("getvalue"))) {
        idx = P043_GETVALUE_LENGTH;
      }

      if ((idx > 0) && validIntFromString(cmd.substring(idx), timeIndex) &&
          (timeIndex > 0) && (timeIndex <= P043_MAX_SETTINGS)) {
        LoadTaskSettings(event->TaskIndex); // Not preloaded

        if (idx == P043_GETTIME_LENGTH) {   // gettime
          string = timeLong2String(ExtraTaskSettings.TaskDevicePluginConfigLong[timeIndex - 1]);
        } else {                            // getvalue
          const int16_t offset = (validGpio(CONFIG_PIN1) || (P043_SIMPLE_VALUE == 1)) ? 1 : 0;
          string = ExtraTaskSettings.TaskDevicePluginConfig[timeIndex - 1] - offset;
        }
        success = true;
      }
      # undef P043_GETTIME_LENGTH // No longer needed
      # undef P043_GETVALUE_LENGTH
      break;
    }
  }
  return success;
}

#endif // USES_P043
