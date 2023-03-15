#include "_Plugin_Helper.h"
#ifdef USES_P086
//#######################################################################################################
//################################## Plugin 086: Homie receiver##########################################
//#######################################################################################################


#define PLUGIN_086
#define PLUGIN_ID_086         86
#define PLUGIN_NAME_086       "Generic - Homie receiver"

// empty default names because settings will be ignored / not used if value name is empty
#define PLUGIN_VALUENAME1_086 ""
#define PLUGIN_VALUENAME2_086 ""
#define PLUGIN_VALUENAME3_086 ""
#define PLUGIN_VALUENAME4_086 ""

#define PLUGIN_086_VALUE_INTEGER    0
#define PLUGIN_086_VALUE_FLOAT      1
#define PLUGIN_086_VALUE_BOOLEAN    2
#define PLUGIN_086_VALUE_STRING     3
#define PLUGIN_086_VALUE_ENUM       4
#define PLUGIN_086_VALUE_RGB        5
#define PLUGIN_086_VALUE_HSV        6

#define PLUGIN_086_VALUE_TYPES      7
#define PLUGIN_086_VALUE_MAX        4

#define PLUGIN_086_DEBUG            true

boolean Plugin_086(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_086;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_NONE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].DecimalsOnly = true;
        Device[deviceCount].ValueCount = PLUGIN_086_VALUE_MAX;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = false;
        Device[deviceCount].Custom = true;
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
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_086));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_086));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_086));

        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        addFormNote(F("Translation Plugin for controllers able to receive value updates according to the Homie convention."));

        uint8_t choice = 0;
        String labelText;
        String keyName;
        const __FlashStringHelper * options[PLUGIN_086_VALUE_TYPES] = {
          F("integer"),
          F("float"),
          F("boolean"),
          F("string"),
          F("enum"),
          F("rgb"),
          F("hsv")
        };
        const int optionValues[PLUGIN_086_VALUE_TYPES] = {
          PLUGIN_086_VALUE_INTEGER,
          PLUGIN_086_VALUE_FLOAT,
          PLUGIN_086_VALUE_BOOLEAN,
          PLUGIN_086_VALUE_STRING,
          PLUGIN_086_VALUE_ENUM,
          PLUGIN_086_VALUE_RGB,
         PLUGIN_086_VALUE_HSV
        };
        for (int i=0;i<PLUGIN_086_VALUE_MAX;i++) {
          labelText = F("Function #");
          labelText += (i+1);
          addFormSubHeader(labelText);
          choice = PCONFIG(i);
          if (i==0) addFormNote(F("Triggers an event when a ../%event%/set topic arrives"));
          labelText = F("Event Name");
          keyName = F("functionName");
          keyName += i;
          addFormTextBox(labelText, keyName, Cache.getTaskDeviceValueName(event->TaskIndex, i), NAME_FORMULA_LENGTH_MAX);
          labelText = F("Parameter Type");
          keyName = F("valueType");
          keyName += i;
          addFormSelector(labelText, keyName, PLUGIN_086_VALUE_TYPES, options, optionValues, choice );
          keyName += F("_min");
          addFormNumericBox(F("Min"),keyName,Cache.getTaskDevicePluginConfig(event->TaskIndex, i));
          keyName = F("valueType");
          keyName += i;
          keyName += F("_max");
          addFormNumericBox(F("Max"),keyName,Cache.getTaskDevicePluginConfig(event->TaskIndex, i+PLUGIN_086_VALUE_MAX));
          if (i==0) addFormNote(F("min max values only valid for numeric parameter"));
          keyName = F("decimals");
          keyName += i;
          addFormNumericBox(F("Decimals"),keyName,Cache.getTaskDeviceValueDecimals(event->TaskIndex, i) ,0,8);
          if (i==0) addFormNote(F("Decimal counts for float parameter"));
          keyName = F("string");
          keyName += i;
          addFormTextBox(F("String or enum"), keyName, Cache.getTaskDeviceFormula(event->TaskIndex, i), NAME_FORMULA_LENGTH_MAX);
          if (i==0) addFormNote(F("Default string or enumumeration list (comma seperated)."));
        }
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String keyName;
        for (int i=0;i<PLUGIN_086_VALUE_MAX;i++) {
          keyName = F("valueType");
          keyName += i;
          PCONFIG(i) = getFormItemInt(keyName);
          keyName = F("functionName");
          keyName += i;
          strncpy_webserver_arg(ExtraTaskSettings.TaskDeviceValueNames[i], keyName);
          keyName = F("valueType");
          keyName += i;
          keyName += F("_min");
          ExtraTaskSettings.TaskDevicePluginConfig[i]=getFormItemInt(keyName);
          keyName = F("valueType");
          keyName += i;
          keyName += F("_max");
          ExtraTaskSettings.TaskDevicePluginConfig[i+PLUGIN_086_VALUE_MAX]=getFormItemInt(keyName);
          keyName = F("decimals");
          keyName += i;
          ExtraTaskSettings.TaskDeviceValueDecimals[i]=getFormItemInt(keyName);
          keyName = F("string");
          keyName += i;
          strncpy_webserver_arg(ExtraTaskSettings.TaskDeviceFormula[i], keyName);
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
        for (uint8_t x=0; x<PLUGIN_086_VALUE_MAX;x++)
        {
          String log = F("P086 : Value ");
          log += x+1;
          log += F(": ");
          log += formatUserVarNoCheck(event->TaskIndex, x);
          addLogMove(LOG_LEVEL_INFO, log);
        }
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String command = parseString(string, 1);
        if (equals(command, F("homievalueset")))
        {
          const taskVarIndex_t taskVarIndex = event->Par2 - 1;
          const userVarIndex_t userVarIndex = event->BaseVarIndex + taskVarIndex;
          if (validTaskIndex(event->TaskIndex) && 
              validTaskVarIndex(taskVarIndex) && 
              validUserVarIndex(userVarIndex) &&  
              (event->Par1 == (event->TaskIndex + 1))) {// make sure that this instance is the target
            String parameter = parseStringToEndKeepCase(string,4);
            String log;
/*            if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
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
            float floatValue = 0.0f;
            String enumList;
            int i = 0;
            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              log = F("P086 : deviceNr:");
              log += event->TaskIndex + 1;
              log += F(" valueNr:");
              log += event->Par2;
              log += F(" valueType:");
              log += Settings.TaskDevicePluginConfig[event->TaskIndex][taskVarIndex];
            }

            switch (Settings.TaskDevicePluginConfig[event->TaskIndex][taskVarIndex]) {
              case PLUGIN_086_VALUE_INTEGER:
              case PLUGIN_086_VALUE_FLOAT:
                if (!parameter.isEmpty()) {
                  if (string2float(parameter,floatValue)) {
                    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                      log += F(" integer/float set to ");
                      log += floatValue;
                      addLogMove(LOG_LEVEL_INFO, log);
                    }
                    UserVar[userVarIndex]=floatValue;
                  } else { // float conversion failed!
                    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
                      log += F(" parameter:");
                      log += parameter;
                      log += F(" not a float value!");
                      addLogMove(LOG_LEVEL_ERROR, log);
                    }
                  }
                } else {
                  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                    log += F(" value:");
                    log += UserVar[userVarIndex];
                    addLogMove(LOG_LEVEL_INFO, log);
                  }
                }
                break;

              case PLUGIN_086_VALUE_BOOLEAN:
                if (parameter=="false") {
                  floatValue = 0.0f;
                } else {
                  floatValue = 1.0f;
                }
                UserVar[userVarIndex]=floatValue;
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  log += F(" boolean set to ");
                  log += floatValue;
                  addLogMove(LOG_LEVEL_INFO, log);
                }
                break;

              case PLUGIN_086_VALUE_STRING:
                //String values not stored to conserve flash memory
                //safe_strncpy(ExtraTaskSettings.TaskDeviceFormula[taskVarIndex], parameter.c_str(), sizeof(ExtraTaskSettings.TaskDeviceFormula[taskVarIndex]));
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  log += F(" string set to ");
                  log += parameter;
                  addLogMove(LOG_LEVEL_INFO, log);
                }
                break;

              case PLUGIN_086_VALUE_ENUM:
                enumList = Cache.getTaskDeviceFormula(event->TaskIndex, taskVarIndex);
                i = 1;
                while (!parseString(enumList,i).isEmpty()) { // lookup result in enum List
                  if (parseString(enumList,i)==parameter) {
                    floatValue = i;
                    break;
                  }
                  i++;
                }
                UserVar[userVarIndex]=floatValue;
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  log += F(" enum set to ");
                  log += floatValue;
                  log += ' ';
                  log += wrap_braces(parameter);
                  addLogMove(LOG_LEVEL_INFO, log);
                }
                break;

              case PLUGIN_086_VALUE_RGB:
                //String values not stored to conserve flash memory
                //safe_strncpy(ExtraTaskSettings.TaskDeviceFormula[taskVarIndex], parameter.c_str(), sizeof(ExtraTaskSettings.TaskDeviceFormula[taskVarIndex]));
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  log += F(" RGB received ");
                  log += parameter;
                  addLogMove(LOG_LEVEL_INFO, log);
                }
                break;

              case PLUGIN_086_VALUE_HSV:
                //String values not stored to conserve flash memory
                //safe_strncpy(ExtraTaskSettings.TaskDeviceFormula[taskVarIndex], parameter.c_str(), sizeof(ExtraTaskSettings.TaskDeviceFormula[taskVarIndex]));
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  log += F(" HSV received ");
                  log += parameter;
                  addLogMove(LOG_LEVEL_INFO, log);
                }
                break;
            }
            success = true;
          }
        }
      }
      break;
    }
  return success;
}
#endif // USES_P086
