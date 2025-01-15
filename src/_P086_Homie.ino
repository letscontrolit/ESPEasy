#include "_Plugin_Helper.h"
#ifdef USES_P086

// #######################################################################################################
// ################################## Plugin 086: Homie receiver##########################################
// #######################################################################################################

/** Changelog:
 * 2023-09-10 tonhuisman: Reduce string usage to lower the .bin footprint
 * 2023-09-10 tonhuisman: Add changelog, uncrustify source
 */

# define PLUGIN_086
# define PLUGIN_ID_086         86
# define PLUGIN_NAME_086       "Generic - Homie receiver"

// empty default names because settings will be ignored / not used if value name is empty
# define PLUGIN_VALUENAME1_086 ""

# define PLUGIN_086_VALUE_INTEGER    0
# define PLUGIN_086_VALUE_FLOAT      1
# define PLUGIN_086_VALUE_BOOLEAN    2
# define PLUGIN_086_VALUE_STRING     3
# define PLUGIN_086_VALUE_ENUM       4
# define PLUGIN_086_VALUE_RGB        5
# define PLUGIN_086_VALUE_HSV        6

// Unsupported (yet) value types:
// - Percent
// - DateTime (convert to linuxtime?)
// - Duration

# define PLUGIN_086_VALUE_MAX        VARS_PER_TASK

# define PLUGIN_086_DEBUG            true

boolean Plugin_086(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number       = PLUGIN_ID_086;
      dev.Type         = DEVICE_TYPE_DUMMY;
      dev.VType        = Sensor_VType::SENSOR_TYPE_NONE;
      dev.DecimalsOnly = true;
      dev.ValueCount   = PLUGIN_086_VALUE_MAX;
      dev.Custom       = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_086);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_086));

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      # ifndef BUILD_NO_DEBUG
      addFormNote(F("Translation Plugin for controllers able to receive value updates according to the Homie convention."));
      # endif // ifndef BUILD_NO_DEBUG

      const __FlashStringHelper *options[] = {
        F("integer"),
        F("float"),
        F("boolean"),
        F("string"),
        F("enum"),
        F("rgb"),
        F("hsv")
      };
      const int optionValues[] = {
        PLUGIN_086_VALUE_INTEGER,
        PLUGIN_086_VALUE_FLOAT,
        PLUGIN_086_VALUE_BOOLEAN,
        PLUGIN_086_VALUE_STRING,
        PLUGIN_086_VALUE_ENUM,
        PLUGIN_086_VALUE_RGB,
        PLUGIN_086_VALUE_HSV
      };
      constexpr int PLUGIN_086_VALUE_TYPES = NR_ELEMENTS(optionValues);

      for (int i = 0; i < PLUGIN_086_VALUE_MAX; ++i) {
        addFormSubHeader(concat(F("Function #"), i + 1));

        if (i == 0) { addFormNote(F("Triggers an event when a ../%taskname%/%event%/set MQTT topic arrives")); }

        addFormTextBox(F("Event Name"), getPluginCustomArgName((i * 10) + 0),
                       Cache.getTaskDeviceValueName(event->TaskIndex, i), NAME_FORMULA_LENGTH_MAX);
        addFormSelector(F("Parameter Type"), getPluginCustomArgName((i * 10) + 1),
                        PLUGIN_086_VALUE_TYPES, options, optionValues, PCONFIG(i));

        addFormNumericBox(F("Min"), getPluginCustomArgName((i * 10) + 2),
                          Cache.getTaskDevicePluginConfig(event->TaskIndex, i));
        addFormNumericBox(F("Max"), getPluginCustomArgName((i * 10) + 3),
                          Cache.getTaskDevicePluginConfig(event->TaskIndex, i + PLUGIN_086_VALUE_MAX));

        if (i == 0) { addFormNote(F("min/max values only valid for numeric parameter")); }

        addFormNumericBox(F("Decimals"), getPluginCustomArgName((i * 10) + 4),
                          Cache.getTaskDeviceValueDecimals(event->TaskIndex, i), 0, 8);

        if (i == 0) { addFormNote(F("Decimal precision for float parameter")); }

        addFormTextBox(F("String or enum"), getPluginCustomArgName((i * 10) + 5),
                       Cache.getTaskDeviceFormula(event->TaskIndex, i), NAME_FORMULA_LENGTH_MAX);

        if (i == 0) { addFormNote(F("Default string or enumeration list (comma separated)")); }
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      for (int i = 0; i < PLUGIN_086_VALUE_MAX; ++i) {
        strncpy_webserver_arg(ExtraTaskSettings.TaskDeviceValueNames[i], getPluginCustomArgName((i * 10) + 0));
        PCONFIG(i)                                                         = getFormItemInt(getPluginCustomArgName((i * 10) + 1));
        ExtraTaskSettings.TaskDevicePluginConfig[i]                        = getFormItemInt(getPluginCustomArgName((i * 10) + 2));
        ExtraTaskSettings.TaskDevicePluginConfig[i + PLUGIN_086_VALUE_MAX] = getFormItemInt(getPluginCustomArgName((i * 10) + 3));
        ExtraTaskSettings.TaskDeviceValueDecimals[i]                       = getFormItemInt(getPluginCustomArgName((i * 10) + 4));
        strncpy_webserver_arg(ExtraTaskSettings.TaskDeviceFormula[i], getPluginCustomArgName((i * 10) + 5));
      }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        for (uint8_t x = 0; x < PLUGIN_086_VALUE_MAX; ++x) {
          addLogMove(LOG_LEVEL_INFO, strformat(F("P086 : Value %d: %s"), x + 1, formatUserVarNoCheck(event->TaskIndex, x).c_str()));
        }
      }
      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      const String command = parseString(string, 1);

      if (equals(command, F("homievalueset"))) {
        const taskVarIndex_t taskVarIndex = event->Par2 - 1;

        if (validTaskIndex(event->TaskIndex) &&
            validTaskVarIndex(taskVarIndex) &&
            (event->Par1 == (event->TaskIndex + 1))) { // make sure that this instance is the target
          const String parameter = parseStringToEndKeepCase(string, 4);
          String log;

          /*
             if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
              log = F("P086 : Acknowledge :");
              log += string;
              log += F(" / ");
              log += ExtraTaskSettings.TaskDeviceName;
              log += F(" / ");
              log += ExtraTaskSettings.TaskDeviceValueNames[taskVarIndex];
              log += F(" sensorType:");
              log += event->sensorType;
              log += F(" Source:");
              log += event->Source;
              log += F(" idx:");
              log += event->idx;
              log += F(" S1:");
              log += event->String1;
              log += F(" S2:");
              log += event->String2;
              log += F(" S3:");
              log += event->String3;
              log += F(" S4:");
              log += event->String4;
              log += F(" S5:");
              log += event->String5;
              log += F(" P1:");
              log += event->Par1;
              log += F(" P2:");
              log += event->Par2;
              log += F(" P3:");
              log += event->Par3;
              log += F(" P4:");
              log += event->Par4;
              log += F(" P5:");
              log += event->Par5;
              addLog(LOG_LEVEL_DEBUG, log);
             } */
          float  floatValue = 0.0f;
          String enumList;
          int    i = 0;

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            log = strformat(F("P086 : deviceNr: %d valueNr: %d valueType: %d"),
                            event->TaskIndex + 1, event->Par2, Settings.TaskDevicePluginConfig[event->TaskIndex][taskVarIndex]);
          }

          switch (Settings.TaskDevicePluginConfig[event->TaskIndex][taskVarIndex]) {
            case PLUGIN_086_VALUE_INTEGER:
            case PLUGIN_086_VALUE_FLOAT:

              if (!parameter.isEmpty()) {
                if (string2float(parameter, floatValue)) {
                  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                    log += concat(F(" integer/float set to "), floatValue);
                    addLogMove(LOG_LEVEL_INFO, log);
                  }
                  UserVar.setFloat(event->TaskIndex, taskVarIndex, floatValue);
                } else { // float conversion failed!
                  if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
                    log += strformat(F(" parameter: %s not a float value!"), parameter.c_str());
                    addLogMove(LOG_LEVEL_ERROR, log);
                  }
                }
              } else {
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  log += concat(F(" value: "), UserVar.getFloat(event->TaskIndex, taskVarIndex));
                  addLogMove(LOG_LEVEL_INFO, log);
                }
              }
              break;

            case PLUGIN_086_VALUE_BOOLEAN:

              if (parameter.equalsIgnoreCase(F("false"))) { // This should be a case-sensitive check...
                                                            // and also check for "true"
                floatValue = 0.0f;
              } else {
                floatValue = 1.0f;
              }
              UserVar.setFloat(event->TaskIndex, taskVarIndex, floatValue);

              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                log += concat(F(" boolean set to "), floatValue);
                addLogMove(LOG_LEVEL_INFO, log);
              }
              break;

            case PLUGIN_086_VALUE_STRING:

              // String values not stored to conserve flash memory
              // safe_strncpy(ExtraTaskSettings.TaskDeviceFormula[taskVarIndex], parameter.c_str(),
              // sizeof(ExtraTaskSettings.TaskDeviceFormula[taskVarIndex]));
              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                log += concat(F(" string set to "), parameter);
                addLogMove(LOG_LEVEL_INFO, log);
              }
              break;

            case PLUGIN_086_VALUE_ENUM:
            {
              enumList = Cache.getTaskDeviceFormula(event->TaskIndex, taskVarIndex);
              i        = 1;
              String enumItem = parseStringKeepCase(enumList, i);

              while (!enumItem.isEmpty()) {                 // lookup result in enum List
                if (enumItem.equalsIgnoreCase(parameter)) { // This should be a case-sensitive check...
                  floatValue = i;
                  break;
                }
                i++;
                enumItem = parseStringKeepCase(enumList, i);
              }
              UserVar.setFloat(event->TaskIndex, event->Par2 - 1, floatValue);

              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                log += strformat(F(" enum set to %.2f %s"), floatValue, wrap_braces(parameter).c_str());
                addLogMove(LOG_LEVEL_INFO, log);
              }
              break;
            }
            case PLUGIN_086_VALUE_RGB:

              // String values not stored to conserve flash memory
              // safe_strncpy(ExtraTaskSettings.TaskDeviceFormula[taskVarIndex], parameter.c_str(),
              // sizeof(ExtraTaskSettings.TaskDeviceFormula[taskVarIndex]));
              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                log += concat(F(" RGB received "), parameter);
                addLogMove(LOG_LEVEL_INFO, log);
              }
              break;

            case PLUGIN_086_VALUE_HSV:

              // String values not stored to conserve flash memory
              // safe_strncpy(ExtraTaskSettings.TaskDeviceFormula[taskVarIndex], parameter.c_str(),
              // sizeof(ExtraTaskSettings.TaskDeviceFormula[taskVarIndex]));
              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                log += concat(F(" HSV received "), parameter);
                addLogMove(LOG_LEVEL_INFO, log);
              }
              break;
          }
          success = true;
        }
      }
      break;
    }
  }
  return success;
}

#endif // USES_P086
