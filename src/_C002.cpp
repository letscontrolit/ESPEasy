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
# include "src/ESPEasyCore/ESPEasyGPIO.h"
# include "src/ESPEasyCore/ESPEasyRules.h"
# include "src/Globals/Settings.h"
# include "src/Helpers/PeriodicalActions.h"
# include "src/Helpers/StringParser.h"

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
        unsigned int idx;
        float  nvalue;
        long   nvaluealt;
        String svalue1, switchtype;

        if (deserializeDomoticzJson(event->String2, idx, nvalue, nvaluealt, svalue1, switchtype)) {
          for (taskIndex_t x = 0; x < TASKS_MAX; x++) {
            // We need the index of the controller we are: 0...CONTROLLER_MAX
            if (Settings.TaskDeviceEnabled[x] &&
                (Settings.TaskDeviceSendData[ControllerID][x]
                 || (Settings.TaskDeviceNumber[x] == 29)         // Domoticz helper doesn't have controller checkboxes...
                 # if defined(USES_P088)
                 || (Settings.TaskDeviceNumber[x] == 88)         // Heatpump IR doesn't have controller checkboxes...
                 # endif // if defined(USES_P088)
                ) &&
                (Settings.TaskDeviceID[ControllerID][x] == idx)) // get idx for our controller index
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

                  if (switchtype.equalsIgnoreCase(F("dimmer")))
                  {
                    mustSendEvent = true;
                    int pwmValue = UserVar[baseVar];

                    switch (static_cast<int>(nvalue))
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
# if defined(USES_P088)  // || defined(USES_P115)
                case 88: // Send heatpump IR (P088) if IDX matches
                  //                case 115:            // Send heatpump IR (P115) if IDX matches
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
        }
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      if (event->idx != 0)
      {
        String json = serializeDomoticzJson(event);
# ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = F("MQTT : ");
          log += json;
          addLogMove(LOG_LEVEL_DEBUG, log);
        }
# endif // ifndef BUILD_NO_DEBUG

        String pubname = CPlugin_002_pubname;
        parseControllerVariables(pubname, event, false);

        // Publish using move operator, thus pubname and json are empty after this call
        success = MQTTpublish(event->ControllerIndex, event->TaskIndex, std::move(pubname), std::move(json), CPlugin_002_mqtt_retainFlag);
      } // if ixd !=0
      else
      {
        addLog(LOG_LEVEL_ERROR, F("MQTT : IDX cannot be zero!"));
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
