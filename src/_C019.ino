#ifdef USES_C019

// #######################################################################################################
// ################### Controller Plugin 019: ESPEasy-Now ################################################
// #######################################################################################################

#define CPLUGIN_019
#define CPLUGIN_ID_019         19
#define CPLUGIN_NAME_019       "ESPEasy-Now"

bool CPlugin_019(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      Protocol[++protocolCount].Number       = CPLUGIN_ID_019;
      Protocol[protocolCount].usesMQTT       = false;
      Protocol[protocolCount].usesTemplate   = false;
      Protocol[protocolCount].usesAccount    = false;
      Protocol[protocolCount].usesPassword   = true;
      Protocol[protocolCount].usesExtCreds   = false;
      Protocol[protocolCount].defaultPort    = 0;
      Protocol[protocolCount].usesID         = false;
      Protocol[protocolCount].Custom         = false;
      Protocol[protocolCount].usesHost       = false;
      Protocol[protocolCount].usesPort       = false;
      Protocol[protocolCount].usesQueue      = false;
      Protocol[protocolCount].usesCheckReply = false;
      Protocol[protocolCount].usesTimeout    = false;
      Protocol[protocolCount].usesSampleSets = false;
      Protocol[protocolCount].needsWiFi      = false;
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_019);
      break;
    }

    case CPlugin::Function::CPLUGIN_INIT:
    {
      MakeControllerSettings(ControllerSettings);
      LoadControllerSettings(event->ControllerIndex, ControllerSettings);

      // FIXME TD-er: Not sure if MQTT like formatting is the best here.
      C019_DelayHandler.configureControllerSettings(ControllerSettings);
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_TEMPLATE:
    {
      event->String1 = F("%sysname%/#");
      event->String2 = F("%sysname%/%tskname%/%valname%");
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      success = C019_DelayHandler.addToQueue(C019_queue_element(event));
      scheduleNextDelayQueue(TIMER_C019_DELAY_QUEUE, C019_DelayHandler.getNextScheduleTime());
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_RECV:
    {
      // FIXME TD-er: WHen should this be scheduled?
      // protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(event->ControllerIndex);
      // schedule_controller_event_timer(ProtocolIndex, CPlugin::Function::CPLUGIN_PROTOCOL_RECV, event);
      break;
    }

    case CPlugin::Function::CPLUGIN_FIFTY_PER_SECOND:
    {
      //      C019_data.async_loop();

      // FIXME TD-er: Handle reading error state or return values.
      break;
    }

    case CPlugin::Function::CPLUGIN_FLUSH:
    {
      process_c019_delay_queue();
      delay(0);
      break;
    }

    default:
      break;
  }

  return success;
}

// Uncrustify may change this into multi line, which will result in failed builds
// *INDENT-OFF*
bool do_process_c019_delay_queue(int controller_number, const C019_queue_element& element, ControllerSettingsStruct& ControllerSettings);
// *INDENT-ON*

bool do_process_c019_delay_queue(int controller_number, const C019_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
  bool success = true;

  return success;
}

#endif // ifdef USES_C019
