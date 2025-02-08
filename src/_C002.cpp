#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C002

# include "src/Helpers/_CPlugin_DomoticzHelper.h"

// #######################################################################################################
// ########################### Controller Plugin 002: Domoticz MQTT ######################################
// #######################################################################################################

/** Changelog:
 * 2024-03-24 tonhuisman: Add support for 'Invert On/Off value' in P029 - Domoticz MQTT Helper
 * 2024-03-24 tonhuisman: Start Changelog (newest on top)
 */

# define CPLUGIN_002
# define CPLUGIN_ID_002         2
# define CPLUGIN_NAME_002       "Domoticz MQTT"

# include "src/Commands/ExecuteCommand.h"
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
      ProtocolStruct& proto = getProtocolStruct(event->idx); //      = CPLUGIN_ID_002;
      proto.usesMQTT     = true;
      proto.usesTemplate = true;
      proto.usesAccount  = true;
      proto.usesPassword = true;
      proto.usesExtCreds = true;
      proto.defaultPort  = 1883;
      proto.usesID       = true;
      # if FEATURE_MQTT_TLS
      proto.usesTLS = true;
      # endif // if FEATURE_MQTT_TLS
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
            constexpr pluginID_t PLUGIN_ID_DOMOTICZ_HELPER(29);
            # if defined(USES_P088)
            constexpr pluginID_t PLUGIN_ID_HEATPUMP_IR(88);
            # endif // if defined(USES_P088)

            if (Settings.TaskDeviceEnabled[x] &&
                (Settings.TaskDeviceSendData[ControllerID][x]
                 || (Settings.getPluginID_for_task(x) == PLUGIN_ID_DOMOTICZ_HELPER) // Domoticz helper doesn't have controller checkboxes...
                 # if defined(USES_P088)
                 || (Settings.getPluginID_for_task(x) == PLUGIN_ID_HEATPUMP_IR)     // Heatpump IR doesn't have controller checkboxes...
                 # endif // if defined(USES_P088)
                ) &&
                (Settings.TaskDeviceID[ControllerID][x] == idx))                    // get idx for our controller index
            {
              String action;
              bool   mustSendEvent = false;

              switch (Settings.getPluginID_for_task(x).value) {
                case 1: // temp solution, if input switch, update state
                {
                  action = strformat(F("gpio,%d,%d"), Settings.TaskDevicePin1[x], static_cast<int>(nvalue));
                  break;
                }
                case 29: // temp solution, if plugin 029, set gpio
                {
                  if (switchtype.equalsIgnoreCase(F("dimmer")))
                  {
                    mustSendEvent = true;
                    int32_t pwmValue = UserVar.getFloat(x, 0);

                    switch (static_cast<int>(nvalue))
                    {
                      case 0: // Off
                        pwmValue = 0;
                        UserVar.setFloat(x, 0, pwmValue);
                        break;
                      case 1: // On
                      case 2: // Update dimmer value
                        pwmValue = 0;

                        if (validIntFromString(svalue1, pwmValue)) {
                          pwmValue *= 10;
                        }
                        UserVar.setFloat(x, 0, pwmValue);
                        break;
                    }

                    if (checkValidPortRange(PLUGIN_GPIO, Settings.TaskDevicePin1[x])) {
                      action = strformat(F("pwm,%d,%d"), Settings.TaskDevicePin1[x], pwmValue);
                    }
                  } else {
                    mustSendEvent = true;
                    int ivalue = static_cast<int>(nvalue);

                    if (1 == Settings.TaskDevicePluginConfig[x][0]) { // PCONFIG(0) = Invert On/Off value
                      ivalue = (1 == ivalue ? 0 : 1);
                    }
                    UserVar.setFloat(x, 0, ivalue);

                    if (checkValidPortRange(PLUGIN_GPIO, Settings.TaskDevicePin1[x])) {
                      action = strformat(F("gpio,%d,%d"), Settings.TaskDevicePin1[x], ivalue);
                    }
                  }
                  break;
                }
                # if defined(USES_P088)
                case 88:                                      // Send heatpump IR (P088) if IDX matches
                {
                  action = concat(F("heatpumpir,"), svalue1); // svalue1 is like 'gree,1,1,0,22,0,0'
                  break;
                }
                # endif // if defined(USES_P088)
                default:
                  break;
              }

              if (action.length() != 0) {
                mustSendEvent = true;

                // Try plugin and internal
                ExecuteCommandArgs args(
                  x,
                  EventValueSource::Enum::VALUE_SOURCE_MQTT,
                  action.c_str(),
                  true,
                  true,
                  false);
                ExecuteCommand(std::move(args), true);
              }

              if (mustSendEvent) {
                // trigger rulesprocessing
                if (Settings.UseRules) {
                  EventStruct TempEvent(x);
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
      if (MQTT_queueFull(event->ControllerIndex)) {
        break;
      }

      if (event->idx != 0)
      {
        String json = serializeDomoticzJson(event);
        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          addLogMove(LOG_LEVEL_DEBUG, concat(F("MQTT : "), json));
        }
        # endif // ifndef BUILD_NO_DEBUG

        String pubname = CPlugin_002_pubname;
        parseControllerVariables(pubname, event, false);

        // Publish using move operator, thus pubname and json are empty after this call
        success = MQTTpublish(event->ControllerIndex, event->TaskIndex, std::move(pubname), std::move(json), CPlugin_002_mqtt_retainFlag);
      } // if idx !=0
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
