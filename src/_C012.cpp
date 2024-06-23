#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C012

// #######################################################################################################
// ########################### Controller Plugin 012: Blynk  #############################################
// #######################################################################################################

# include "src/Commands/Blynk.h"

# define CPLUGIN_012
# define CPLUGIN_ID_012         12
# define CPLUGIN_NAME_012       "Blynk HTTP"

bool CPlugin_012(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      ProtocolStruct& proto = getProtocolStruct(event->idx); //      = CPLUGIN_ID_012;
      proto.usesMQTT     = false;
      proto.usesAccount  = false;
      proto.usesPassword = true;
      proto.usesExtCreds = true;
      proto.defaultPort  = 80;
      proto.usesID       = true;
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
      if (C012_DelayHandler->queueFull(event->ControllerIndex)) {
        break;
      }
      //LoadTaskSettings(event->TaskIndex); // FIXME TD-er: This can probably be removed

      // Collect the values at the same run, to make sure all are from the same sample
      uint8_t valueCount = getValueCountForTask(event->TaskIndex);
      std::unique_ptr<C012_queue_element> element(new (std::nothrow) C012_queue_element(event, valueCount));

      for (uint8_t x = 0; x < valueCount; x++)
      {
        bool   isvalid;
        const String formattedValue = formatUserVar(event, x, isvalid);

        if (isvalid) {
          move_special(element->txt[x], strformat(
            F("update/V%d?value=%s"), 
            event->idx + x, 
            formattedValue.c_str()));

          #ifndef BUILD_NO_DEBUG
          if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
            addLog(LOG_LEVEL_DEBUG_MORE, element->txt[x]);
          }
          #endif
        }
      }

      
      success = C012_DelayHandler->addToQueue(std::move(element));
      Scheduler.scheduleNextDelayQueue(SchedulerIntervalTimer_e::TIMER_C012_DELAY_QUEUE, C012_DelayHandler->getNextScheduleTime());
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

// ********************************************************************************
// Process Queued Blynk request, with data set to NULL
// ********************************************************************************

// Uncrustify may change this into multi line, which will result in failed builds
// *INDENT-OFF*
bool do_process_c012_delay_queue(cpluginID_t cpluginID, const Queue_element_base& element_base, ControllerSettingsStruct& ControllerSettings) {
  const C012_queue_element& element = static_cast<const C012_queue_element&>(element_base);
// *INDENT-ON*
  while (element.txt[element.valuesSent].isEmpty()) {
    // A non valid value, which we are not going to send.
    // Increase sent counter until a valid value is found.
    if (element.checkDone(true)) {
      return true;
    }
  }

  if (!NetworkConnected()) {
    return false;
  }
  return element.checkDone(Blynk_get(element.txt[element.valuesSent], element._controller_idx));
}

#endif // ifdef USES_C012
