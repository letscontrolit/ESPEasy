#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C016

// #######################################################################################################
// ########################### Controller Plugin 016: Controller - Cache #################################
// #######################################################################################################

/*
   This is a cache layer to collect data while not connected to a network.
   The data will first be stored in RTC memory, which will survive a crash/reboot and even an OTA update.
   If this RTC buffer is full, it will be flushed to whatever is set here as storage.

   Typical sample sets contain:
   - UNIX timestamp
   - task index delivering the data
   - 4 float values

   These are the result of any plugin sending data to this controller.

   The controller can save the samples from RTC memory to several places on the flash:
   - Files on FS
   - Part reserved for OTA update (TODO)
   - Unused flash after the partitioned space (TODO)

   The controller can deliver the data to:
   <TODO>
 */

# include "src/Globals/C016_ControllerCache.h"
# include "src/Globals/ESPEasy_time.h"

# define CPLUGIN_016
# define CPLUGIN_ID_016         16
# define CPLUGIN_NAME_016       "Cache Controller [Experimental]"

// #include <ArduinoJson.h>

bool CPlugin_016(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      Protocol[++protocolCount].Number       = CPLUGIN_ID_016;
      Protocol[protocolCount].usesMQTT       = false;
      Protocol[protocolCount].usesTemplate   = true;
      Protocol[protocolCount].usesAccount    = false;
      Protocol[protocolCount].usesPassword   = false;
      Protocol[protocolCount].defaultPort    = 80;
      Protocol[protocolCount].usesID         = false;
      Protocol[protocolCount].usesHost       = false;
      Protocol[protocolCount].usesPort       = false;
      Protocol[protocolCount].usesSampleSets = false;
      Protocol[protocolCount].needsNetwork   = false;
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_016);
      break;
    }

    case CPlugin::Function::CPLUGIN_INIT:
    {
      success = init_c016_delay_queue(event->ControllerIndex);
      ControllerCache.init();
      break;
    }

    case CPlugin::Function::CPLUGIN_EXIT:
    {
      exit_c016_delay_queue();
      break;
    }

    case CPlugin::Function::CPLUGIN_WEBFORM_LOAD:
    {
      break;
    }

    case CPlugin::Function::CPLUGIN_WEBFORM_SAVE:
    {
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_TEMPLATE:
    {
      event->String1 = "";
      event->String2 = "";
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      // Collect the values at the same run, to make sure all are from the same sample
      byte valueCount = getValueCountForTask(event->TaskIndex);
      C016_queue_element element(event, valueCount, node_time.getUnixTime());
      success = ControllerCache.write((uint8_t *)&element, sizeof(element));

      /*
              if (C016_DelayHandler == nullptr) {
                break;
              }

              MakeControllerSettings(ControllerSettings);
              LoadControllerSettings(event->ControllerIndex, ControllerSettings);
              success = C016_DelayHandler->addToQueue(element);
              Scheduler.scheduleNextDelayQueue(ESPEasy_Scheduler::IntervalTimer_e::TIMER_C016_DELAY_QUEUE,
                 C016_DelayHandler->getNextScheduleTime());
       */
      break;
    }

    case CPlugin::Function::CPLUGIN_FLUSH:
    {
      process_c016_delay_queue();
      delay(0);
      break;
    }

    default:
      break;
  }
  return success;
}

// ********************************************************************************
// Process the data from the cache
// ********************************************************************************
// Uncrustify may change this into multi line, which will result in failed builds
// *INDENT-OFF*
bool do_process_c016_delay_queue(int controller_number, const C016_queue_element& element, ControllerSettingsStruct& ControllerSettings);

bool do_process_c016_delay_queue(int controller_number, const C016_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
// *INDENT-ON*
  return true;

  // FIXME TD-er: Hand over data to wherever it needs to be.
  // Ideas:
  // - Upload bin files to some server (HTTP post?)
  // - Provide a sample to any connected controller
  // - Do nothing and let some extern host pull the data from the node.
  // - JavaScript to process the data inside the browser.
  // - Feed it to some plugin (e.g. a display to show a chart)
}

#endif // ifdef USES_C016
