#include "src/Helpers/_CPlugin_Helper.h"

#ifdef USES_C008

// #######################################################################################################
// ########################### Controller Plugin 008: Generic HTTP #######################################
// #######################################################################################################

# define CPLUGIN_008
# define CPLUGIN_ID_008         8
# define CPLUGIN_NAME_008       "Generic HTTP"
# include <ArduinoJson.h>

bool CPlugin_008(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      Protocol[++protocolCount].Number     = CPLUGIN_ID_008;
      Protocol[protocolCount].usesMQTT     = false;
      Protocol[protocolCount].usesTemplate = true;
      Protocol[protocolCount].usesAccount  = true;
      Protocol[protocolCount].usesPassword = true;
      Protocol[protocolCount].usesExtCreds = true;
      Protocol[protocolCount].defaultPort  = 80;
      Protocol[protocolCount].usesID       = true;
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_008);
      break;
    }

    case CPlugin::Function::CPLUGIN_INIT:
    {
      success = init_c008_delay_queue(event->ControllerIndex);
      break;
    }

    case CPlugin::Function::CPLUGIN_EXIT:
    {
      exit_c008_delay_queue();
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_TEMPLATE:
    {
      event->String1 = "";
      event->String2 = F("demo.php?name=%sysname%&task=%tskname%&valuename=%valname%&value=%value%");
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      if (C008_DelayHandler == nullptr) {
        break;
      }

      String pubname;
      {
        // Place the ControllerSettings in a scope to free the memory as soon as we got all relevant information.
        MakeControllerSettings(ControllerSettings);

        if (!AllocatedControllerSettings()) {
          addLog(LOG_LEVEL_ERROR, F("C008 : Generic HTTP - Cannot send, out of RAM"));
          break;
        }
        LoadControllerSettings(event->ControllerIndex, ControllerSettings);
        pubname = ControllerSettings.Publish;
      }

      // FIXME TD-er must define a proper move operator
      byte valueCount = getValueCountForTask(event->TaskIndex);
      success = C008_DelayHandler->addToQueue(C008_queue_element(event, valueCount));

      if (success) {
        // Element was added.
        // Now we try to append to the existing element
        // and thus preventing the need to create a long string only to copy it to a queue element.
        C008_queue_element& element = C008_DelayHandler->sendQueue.back();

        // Collect the values at the same run, to make sure all are from the same sample
        LoadTaskSettings(event->TaskIndex);
        parseControllerVariables(pubname, event, true);

        for (byte x = 0; x < valueCount; x++)
        {
          String tmppubname = pubname;
          bool   isvalid;
          String formattedValue = formatUserVar(event, x, isvalid);

          if (isvalid) {
            element.txt[x]  = "/";
            element.txt[x] += tmppubname;
            parseSingleControllerVariable(element.txt[x], event, x, true);
            element.txt[x].replace(F("%value%"), formattedValue);
# ifndef BUILD_NO_DEBUG
            addLog(LOG_LEVEL_DEBUG_MORE, element.txt[x]);
# endif // ifndef BUILD_NO_DEBUG
          }
        }
      }
      Scheduler.scheduleNextDelayQueue(ESPEasy_Scheduler::IntervalTimer_e::TIMER_C008_DELAY_QUEUE, C008_DelayHandler->getNextScheduleTime());
      break;
    }

    case CPlugin::Function::CPLUGIN_FLUSH:
    {
      process_c008_delay_queue();
      delay(0);
      break;
    }

    default:
      break;
  }
  return success;
}

// ********************************************************************************
// Generic HTTP get request
// ********************************************************************************

// Uncrustify may change this into multi line, which will result in failed builds
// *INDENT-OFF*
bool do_process_c008_delay_queue(int controller_number, const C008_queue_element& element, ControllerSettingsStruct& ControllerSettings);

bool do_process_c008_delay_queue(int controller_number, const C008_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
// *INDENT-ON*
  while (element.txt[element.valuesSent] == "") {
    // A non valid value, which we are not going to send.
    // Increase sent counter until a valid value is found.
    if (element.checkDone(true)) {
      return true;
    }
  }

  WiFiClient client;

  if (!try_connect_host(controller_number, client, ControllerSettings)) {
    return false;
  }

  String request =
    create_http_request_auth(controller_number, element.controller_idx, ControllerSettings, F("GET"), element.txt[element.valuesSent]);

  return element.checkDone(send_via_http(controller_number, client, request, ControllerSettings.MustCheckReply));
}

#endif // ifdef USES_C008
