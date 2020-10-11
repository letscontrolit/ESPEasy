#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C012
//#######################################################################################################
//########################### Controller Plugin 012: Blynk  #############################################
//#######################################################################################################

// #ifdef PLUGIN_BUILD_TESTING

#include "src/Commands/Blynk.h"

#define CPLUGIN_012
#define CPLUGIN_ID_012         12
#define CPLUGIN_NAME_012       "Blynk HTTP [TESTING]"

bool CPlugin_012(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_012;
        Protocol[protocolCount].usesMQTT = false;
        Protocol[protocolCount].usesAccount = false;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].usesExtCreds = true;
        Protocol[protocolCount].defaultPort = 80;
        Protocol[protocolCount].usesID = true;
        break;
      }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_012);
        break;
      }

    case CPlugin::Function::CPLUGIN_INIT:
      {
        success = init_c012_delay_queue(event->ControllerIndex);
        break;
      }

    case CPlugin::Function::CPLUGIN_EXIT:
      {
        exit_c012_delay_queue();
        break;
      }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
      {
        if (C012_DelayHandler == nullptr) {
          break;
        }
        LoadTaskSettings(event->TaskIndex);

        // Collect the values at the same run, to make sure all are from the same sample
        byte valueCount = getValueCountForTask(event->TaskIndex);
        C012_queue_element element(event, valueCount);

        for (byte x = 0; x < valueCount; x++)
        {
          bool isvalid;
          String formattedValue = formatUserVar(event, x, isvalid);
          if (isvalid) {
            element.txt[x] = F("update/V");
            element.txt[x] += event->idx + x;
            element.txt[x] += F("?value=");
            element.txt[x] += formattedValue;
            addLog(LOG_LEVEL_DEBUG_MORE, element.txt[x]);
          }
        }
        // FIXME TD-er must define a proper move operator
        success = C012_DelayHandler->addToQueue(C012_queue_element(element));
        Scheduler.scheduleNextDelayQueue(ESPEasy_Scheduler::IntervalTimer_e::TIMER_C012_DELAY_QUEUE, C012_DelayHandler->getNextScheduleTime());
        break;
      }

    case CPlugin::Function::CPLUGIN_FLUSH:
      {
        process_c012_delay_queue();
        delay(0);
        break;
      }

    default:
      break;

  }
  return success;
}

//********************************************************************************
// Process Queued Blynk request, with data set to NULL
//********************************************************************************

// Uncrustify may change this into multi line, which will result in failed builds
// *INDENT-OFF*
bool do_process_c012_delay_queue(int controller_number, const C012_queue_element& element, ControllerSettingsStruct& ControllerSettings);
// *INDENT-ON*

bool do_process_c012_delay_queue(int controller_number, const C012_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
  while (element.txt[element.valuesSent] == "") {
    // A non valid value, which we are not going to send.
    // Increase sent counter until a valid value is found.
    if (element.checkDone(true))
      return true;
  }
  if (!NetworkConnected()) {
    return false;
  }
  return element.checkDone(Blynk_get(element.txt[element.valuesSent], element.controller_idx));
}


#endif
