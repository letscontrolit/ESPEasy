#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C002

# include "src/Helpers/_CPlugin_DomoticzHelper.h"

// #######################################################################################################
// ########################### Controller Plugin 002: Domoticz MQTT ######################################
// #######################################################################################################

# define CPLUGIN_002
# define CPLUGIN_ID_002         2
# define CPLUGIN_NAME_002       "Domoticz MQTT"

# include "src/Commands/InternalCommands.h"
# include "src/Commands/GPIO.h"
# include "src/ESPEasyCore/ESPEasyGPIO.h"
# include "src/ESPEasyCore/ESPEasyRules.h"
# include "src/Globals/Settings.h"
# include "src/Helpers/PeriodicalActions.h"
# include "src/Helpers/StringParser.h"

# include <ArduinoJson.h>

String CPlugin_002_pubname;
bool   CPlugin_002_mqtt_retainFlag = false;

bool CPlugin_002(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      Protocol[++protocolCount].Number     = CPLUGIN_ID_002;
      Protocol[protocolCount].usesMQTT     = true;
      Protocol[protocolCount].usesTemplate = true;
      Protocol[protocolCount].usesAccount  = true;
      Protocol[protocolCount].usesPassword = true;
      Protocol[protocolCount].usesExtCreds = true;
      Protocol[protocolCount].defaultPort  = 1883;
      Protocol[protocolCount].usesID       = true;
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_002);
      break;
    }

    case CPlugin::Function::CPLUGIN_INIT:
    {
      success = init_mqtt_delay_queue(event->ControllerIndex, CPlugin_002_pubname, CPlugin_002_mqtt_retainFlag);
      break;
    }

    case CPlugin::Function::CPLUGIN_EXIT:
    {
      exit_mqtt_delay_queue();
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_TEMPLATE:
    {
      event->String1 = F("domoticz/out");
      event->String2 = F("domoticz/in");
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_RECV:
    {
      // char json[512];
      // json[0] = 0;
      // event->String2.toCharArray(json, 512);
      // Find first enabled controller index with this protocol
      controllerIndex_t ControllerID = findFirstEnabledControllerWithId(CPLUGIN_ID_002);

      if (validControllerIndex(ControllerID)) {
        DynamicJsonDocument root(512);
        deserializeJson(root, event->String2.c_str());

        if (!root.isNull())
        {
          unsigned int idx = root[F("idx")];
          float nvalue     = root[F("nvalue")];
          long  nvaluealt  = root[F("nvalue")];

          // const char* name = root["name"]; // Not used
          // const char* svalue = root["svalue"]; // Not used
          const char *svalue1 = root[F("svalue1")];

          // const char* svalue2 = root["svalue2"]; // Not used
          // const char* svalue3 = root["svalue3"]; // Not used
          const char *switchtype = root[F("switchType")]; // Expect "On/Off" or "dimmer"

          if (nvalue == 0) {
            nvalue = nvaluealt;
          }

          if ((int)switchtype == 0) {
            switchtype = "?";
          }

          for (taskIndex_t x = 0; x < TASKS_MAX; x++) {
            // We need the index of the controller we are: 0...CONTROLLER_MAX
            if (Settings.TaskDeviceEnabled[x] && (Settings.TaskDeviceID[ControllerID][x] == idx)) // get idx for our controller index
            {
              String action;
              bool   mustSendEvent = false;

              switch (Settings.TaskDeviceNumber[x]) {
                case 1: // temp solution, if input switch, update state
                {
                  action  = F("inputSwitchState,");
                  action += x;
                  action += ',';
                  action += nvalue;
                  break;
                }
                case 29: // temp solution, if plugin 029, set gpio
                {
                  int baseVar = x * VARS_PER_TASK;

                  if (strcasecmp_P(switchtype, PSTR("dimmer")) == 0)
                  {
                    mustSendEvent = true;
                    int pwmValue = UserVar[baseVar];

                    switch ((int)nvalue)
                    {
                      case 0: // Off
                        pwmValue         = 0;
                        UserVar[baseVar] = pwmValue;
                        break;
                      case 1: // On
                      case 2: // Update dimmer value
                        pwmValue = 0;

                        if (validIntFromString(svalue1, pwmValue)) {
                          pwmValue *= 10;
                        }
                        UserVar[baseVar] = pwmValue;
                        break;
                    }

                    if (checkValidPortRange(PLUGIN_GPIO, Settings.TaskDevicePin1[x])) {
                      action  = F("pwm,");
                      action += Settings.TaskDevicePin1[x];
                      action += ',';
                      action += pwmValue;
                    }
                  } else {
                    mustSendEvent    = true;
                    UserVar[baseVar] = nvalue;

                    if (checkValidPortRange(PLUGIN_GPIO, Settings.TaskDevicePin1[x])) {
                      action  = F("gpio,");
                      action += Settings.TaskDevicePin1[x];
                      action += ',';
                      action += static_cast<int>(nvalue);
                    }
                  }
                  break;
                }
# if defined(USES_P088) || defined(USES_P115)
                case 88:             // Send heatpump IR (P088) if IDX matches
                case 115:            // Send heatpump IR (P115) if IDX matches
                {
                  action  = F("heatpumpir,");
                  action += svalue1; // svalue1 is like 'gree,1,1,0,22,0,0'
                  break;
                }
# endif // USES_P088 || USES_P115
                default:
                  break;
              }

              const bool validCommand = action.length() > 0;

              if (validCommand) {
                mustSendEvent = true;

                // Try plugin and internal
                ExecuteCommand(x, EventValueSource::Enum::VALUE_SOURCE_MQTT, action.c_str(), true, true, false);
              }

              if (mustSendEvent) {
                // trigger rulesprocessing
                if (Settings.UseRules) {
                  struct EventStruct TempEvent(x);
                  parseCommandString(&TempEvent, action);
                  createRuleEvents(&TempEvent);
                }
              }
            }
          }
          LoadTaskSettings(event->TaskIndex);
        }
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      if (event->idx != 0)
      {
        String json;
        {
          DynamicJsonDocument root(200);
          root[F("idx")]  = event->idx;
          root[F("RSSI")] = mapRSSItoDomoticz();
            # if FEATURE_ADC_VCC
          root[F("Battery")] = mapVccToDomoticz();
            # endif // if FEATURE_ADC_VCC

          const Sensor_VType sensorType = event->getSensorType();

          switch (sensorType)
          {
            case Sensor_VType::SENSOR_TYPE_SWITCH:
              root[F("command")] = String(F("switchlight"));

              if (UserVar[event->BaseVarIndex] == 0) {
                root[F("switchcmd")] = String(F("Off"));
              }
              else {
                root[F("switchcmd")] = String(F("On"));
              }
              break;
            case Sensor_VType::SENSOR_TYPE_DIMMER:
              root[F("command")] = String(F("switchlight"));

              if (UserVar[event->BaseVarIndex] == 0) {
                root[F("switchcmd")] = String(F("Off"));
              }
              else {
                root[F("Set%20Level")] = UserVar[event->BaseVarIndex];
              }
              break;

            case Sensor_VType::SENSOR_TYPE_SINGLE:
            case Sensor_VType::SENSOR_TYPE_LONG:
            case Sensor_VType::SENSOR_TYPE_DUAL:
            case Sensor_VType::SENSOR_TYPE_TRIPLE:
            case Sensor_VType::SENSOR_TYPE_QUAD:
            case Sensor_VType::SENSOR_TYPE_TEMP_HUM:
            case Sensor_VType::SENSOR_TYPE_TEMP_BARO:
            case Sensor_VType::SENSOR_TYPE_TEMP_EMPTY_BARO:
            case Sensor_VType::SENSOR_TYPE_TEMP_HUM_BARO:
            case Sensor_VType::SENSOR_TYPE_WIND:
            case Sensor_VType::SENSOR_TYPE_STRING:
            default:
              root[F("nvalue")] = 0;
              root[F("svalue")] = formatDomoticzSensorType(event);
              break;
          }

          serializeJson(root, json);
        }
# ifndef BUILD_NO_DEBUG
        String log = F("MQTT : ");
        log += json;
        addLog(LOG_LEVEL_DEBUG, log);
# endif // ifndef BUILD_NO_DEBUG

        String pubname = CPlugin_002_pubname;
        parseControllerVariables(pubname, event, false);

        success = MQTTpublish(event->ControllerIndex, event->TaskIndex, pubname.c_str(), json.c_str(), CPlugin_002_mqtt_retainFlag);
      } // if ixd !=0
      else
      {
        String log = F("MQTT : IDX cannot be zero!");
        addLog(LOG_LEVEL_ERROR, log);
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_FLUSH:
    {
      processMQTTdelayQueue();
      delay(0);
      break;
    }

    default:
      break;
  }
  return success;
}

#endif // ifdef USES_C002
