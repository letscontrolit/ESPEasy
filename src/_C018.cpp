#include "src/Helpers/_CPlugin_Helper.h"

#ifdef USES_C018

// #######################################################################################################
// ########################### Controller Plugin 018: LoRa TTN - RN2483/RN2903 ###########################
// #######################################################################################################

# define CPLUGIN_018
# define CPLUGIN_ID_018         18
# define CPLUGIN_NAME_018       "LoRa TTN - RN2483/RN2903"



# include <ESPeasySerial.h>

# include "src/ControllerQueue/C018_queue_element.h"
# include "src/Controller_config/C018_config.h"
# include "src/Controller_struct/C018_data_struct.h"
# include "src/DataTypes/ESPEasy_plugin_functions.h"
# include "src/Globals/CPlugins.h"
# include "src/Globals/Protocol.h"
# include "src/Helpers/_Plugin_Helper_serial.h"
# include "src/Helpers/StringGenerator_GPIO.h"
# include "src/WebServer/Markup.h"
# include "src/WebServer/Markup_Forms.h"
# include "src/WebServer/HTML_wrappers.h"


// Have this define after the includes, so we can set it in Custom.h
# ifndef C018_FORCE_SW_SERIAL
#  define C018_FORCE_SW_SERIAL false
# endif // ifndef C018_FORCE_SW_SERIAL


// FIXME TD-er: Must add a controller data struct vector, like with plugins.
C018_data_struct *C018_data = nullptr;


// Forward declarations
bool   C018_init(struct EventStruct *event);
String c018_add_joinChanged_script_element_line(const String& id,
                                                bool          forOTAA);


bool CPlugin_018(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      Protocol[++protocolCount].Number       = CPLUGIN_ID_018;
      Protocol[protocolCount].usesMQTT       = false;
      Protocol[protocolCount].usesAccount    = true;
      Protocol[protocolCount].usesPassword   = true;
      Protocol[protocolCount].defaultPort    = 1;
      Protocol[protocolCount].usesID         = true;
      Protocol[protocolCount].usesHost       = false;
      Protocol[protocolCount].usesCheckReply = false;
      Protocol[protocolCount].usesTimeout    = false;
      Protocol[protocolCount].usesSampleSets = true;
      Protocol[protocolCount].needsNetwork   = false;
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_018);
      break;
    }

    case CPlugin::Function::CPLUGIN_WEBFORM_SHOW_HOST_CONFIG:
    {
      if ((C018_data != nullptr) && C018_data->isInitialized()) {
        string  = F("Dev addr: ");
        string += C018_data->getDevaddr();
        string += C018_data->useOTAA() ? F(" (OTAA)") : F(" (ABP)");
      } else {
        string = F("-");
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_INIT:
    {
      success = init_c018_delay_queue(event->ControllerIndex);

      if (success) {
        C018_init(event);
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_EXIT:
    {
      if (C018_data != nullptr) {
        C018_data->reset();
        delete C018_data;
        C018_data = nullptr;
      }
      exit_c018_delay_queue();
      break;
    }

    case CPlugin::Function::CPLUGIN_WEBFORM_LOAD:
    {
      {
        // Script to toggle visibility of OTAA/ABP field, based on the activation method selector.
        protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(event->ControllerIndex);
        html_add_script(false);
        addHtml(F("function joinChanged(elem){ var styleOTAA = elem.value == 0 ? '' : 'none'; var styleABP = elem.value == 1 ? '' : 'none';"));
        addHtml(c018_add_joinChanged_script_element_line(getControllerParameterInternalName(ProtocolIndex,
                                                                                            ControllerSettingsStruct::CONTROLLER_USER),
                                                         true));
        addHtml(c018_add_joinChanged_script_element_line(getControllerParameterInternalName(ProtocolIndex,
                                                                                            ControllerSettingsStruct::CONTROLLER_PASS),
                                                         true));
        addHtml(c018_add_joinChanged_script_element_line(F("deveui"), true));
        addHtml(c018_add_joinChanged_script_element_line(F("deveui_note"), true));

        addHtml(c018_add_joinChanged_script_element_line(F("devaddr"), false));
        addHtml(c018_add_joinChanged_script_element_line(F("nskey"), false));
        addHtml(c018_add_joinChanged_script_element_line(F("appskey"), false));
        addHtml('}');
        html_add_script_end();
      }

      {
        // Keep this object in a small scope so we can destruct it as soon as possible again.
        std::shared_ptr<C018_ConfigStruct> customConfig;
        {
          // Try to allocate on 2nd heap
          # ifdef USE_SECOND_HEAP

          //          HeapSelectIram ephemeral;
          # endif // ifdef USE_SECOND_HEAP
          std::shared_ptr<C018_ConfigStruct> tmp_shared(new (std::nothrow) C018_ConfigStruct);
          customConfig = std::move(tmp_shared);
        }

        if (!customConfig) {
          break;
        }
        LoadCustomControllerSettings(event->ControllerIndex, reinterpret_cast<uint8_t *>(customConfig.get()), sizeof(C018_ConfigStruct));
        customConfig->webform_load(C018_data);
      }

      break;
    }
    case CPlugin::Function::CPLUGIN_WEBFORM_SAVE:
    {
      std::shared_ptr<C018_ConfigStruct> customConfig;
      {
        // Try to allocate on 2nd heap
        # ifdef USE_SECOND_HEAP

        //        HeapSelectIram ephemeral;
        # endif // ifdef USE_SECOND_HEAP
        std::shared_ptr<C018_ConfigStruct> tmp_shared(new (std::nothrow) C018_ConfigStruct);
        customConfig = std::move(tmp_shared);
      }

      if (customConfig) {
        customConfig->webform_save();
        SaveCustomControllerSettings(event->ControllerIndex, reinterpret_cast<const uint8_t *>(customConfig.get()),
                                     sizeof(C018_ConfigStruct));
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_PROTOCOL_DISPLAY_NAME:
    {
      success = true;

      switch (event->idx) {
        case ControllerSettingsStruct::CONTROLLER_USER:
          string = F("AppEUI");
          break;
        case ControllerSettingsStruct::CONTROLLER_PASS:
          string = F("AppKey");
          break;
        case ControllerSettingsStruct::CONTROLLER_TIMEOUT:
          string = F("Module Timeout");
          break;
        case ControllerSettingsStruct::CONTROLLER_PORT:
          string = F("Port");
          break;
        default:
          success = false;
          break;
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      if (C018_DelayHandler == nullptr) {
        break;
      }

      if (C018_DelayHandler->queueFull(event->ControllerIndex)) {
        break;
      }

      if (C018_data != nullptr) {
        std::unique_ptr<C018_queue_element> element(new C018_queue_element(event, C018_data->getSampleSetCount(event->TaskIndex)));
        success = C018_DelayHandler->addToQueue(std::move(element));
        Scheduler.scheduleNextDelayQueue(ESPEasy_Scheduler::IntervalTimer_e::TIMER_C018_DELAY_QUEUE,
                                         C018_DelayHandler->getNextScheduleTime());

        if (!C018_data->isInitialized()) {
          // Sometimes the module does need some time after power on to respond.
          // So it may not be initialized well at the call of CPLUGIN_INIT
          // We try to trigger its init again when sending data.
          C018_init(event);
        }
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_RECV:
    {
      // FIXME TD-er: WHen should this be scheduled?
      // protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(event->ControllerIndex);
      // schedule_controller_event_timer(ProtocolIndex, CPlugin::Function::CPLUGIN_PROTOCOL_RECV, event);
      break;
    }

    case CPlugin::Function::CPLUGIN_WRITE:
    {
      if (C018_data != nullptr) {
        if (C018_data->isInitialized())
        {
          const String command    = parseString(string, 1);
          if (equals(command, F("lorawan"))) {
            const String subcommand = parseString(string, 2);
            if (equals(subcommand, F("write"))) {
              const String loraWriteCommand = parseStringToEnd(string, 3);
              const String res              = C018_data->sendRawCommand(loraWriteCommand);
              String logstr                 = F("LoRaWAN cmd: ");
              logstr += loraWriteCommand;
              logstr += F(" -> ");
              logstr += res;
              addLog(LOG_LEVEL_INFO, logstr);
              SendStatus(event, logstr);
              success = true;
            }
          }
        }
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_FIFTY_PER_SECOND:
    {
      if (C018_data != nullptr) {
        C018_data->async_loop();
      }

      // FIXME TD-er: Handle reading error state or return values.
      break;
    }

    case CPlugin::Function::CPLUGIN_FLUSH:
    {
      process_c018_delay_queue();
      delay(0);
      break;
    }

    default:
      break;
  }
  return success;
}

bool C018_init(struct EventStruct *event) {
  String AppEUI;
  String AppKey;
  taskIndex_t  SampleSetInitiator = INVALID_TASK_INDEX;
  unsigned int Port               = 0;

  // Check if the object is already created.
  // If so, delete it to make sure the module is initialized according to the full set parameters.
  if (C018_data != nullptr) {
    C018_data->reset();
    delete C018_data;
    C018_data = nullptr;
  }


  C018_data = new (std::nothrow) C018_data_struct;

  if (C018_data == nullptr) {
    return false;
  }
  {
    // Allocate ControllerSettings object in a scope, so we can destruct it as soon as possible.
    MakeControllerSettings(ControllerSettings); // -V522

    if (!AllocatedControllerSettings()) {
      return false;
    }

    LoadControllerSettings(event->ControllerIndex, ControllerSettings);
    C018_DelayHandler->configureControllerSettings(ControllerSettings);
    AppEUI             = getControllerUser(event->ControllerIndex, ControllerSettings);
    AppKey             = getControllerPass(event->ControllerIndex, ControllerSettings);
    SampleSetInitiator = ControllerSettings.SampleSetInitiator;
    Port               = ControllerSettings.Port;
  }

  std::shared_ptr<C018_ConfigStruct> customConfig;
  {
    // Try to allocate on 2nd heap
    # ifdef USE_SECOND_HEAP

    //    HeapSelectIram ephemeral;
    # endif // ifdef USE_SECOND_HEAP
    std::shared_ptr<C018_ConfigStruct> tmp_shared(new (std::nothrow) C018_ConfigStruct);
    customConfig = std::move(tmp_shared);
  }

  if (!customConfig) {
    return false;
  }
  LoadCustomControllerSettings(event->ControllerIndex, reinterpret_cast<uint8_t *>(customConfig.get()), sizeof(C018_ConfigStruct));
  customConfig->validate();

  if (!C018_data->init(customConfig->serialPort, customConfig->rxpin, customConfig->txpin, customConfig->baudrate,
                       (customConfig->joinmethod == C018_USE_OTAA),
                       SampleSetInitiator, customConfig->resetpin))
  {
    return false;
  }

  C018_data->setFrequencyPlan(static_cast<RN2xx3_datatypes::Freq_plan>(customConfig->frequencyplan), customConfig->rx2_freq);

  if (!C018_data->setSF(customConfig->sf)) {
    return false;
  }

  if (!C018_data->setAdaptiveDataRate(customConfig->adr != 0)) {
    return false;
  }

  if (!C018_data->setTTNstack(static_cast<RN2xx3_datatypes::TTN_stack_version>(customConfig->stackVersion))) {
    return false;
  }

  if (customConfig->joinmethod == C018_USE_OTAA) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("OTAA: AppEUI: ");
      log += AppEUI;
      log += F(" AppKey: ");
      log += AppKey;
      log += F(" DevEUI: ");
      log += customConfig->DeviceEUI;

      addLogMove(LOG_LEVEL_INFO, log);
    }

    if (!C018_data->initOTAA(AppEUI, AppKey, customConfig->DeviceEUI)) {
      return false;
    }
  }
  else {
    if (!C018_data->initABP(customConfig->DeviceAddr, customConfig->AppSessionKey, customConfig->NetworkSessionKey)) {
      return false;
    }
  }


  if (!C018_data->txUncnf(F("ESPeasy (TTN)"), Port)) {
    return false;
  }
  return true;
}

// Uncrustify may change this into multi line, which will result in failed builds
// *INDENT-OFF*
bool do_process_c018_delay_queue(int controller_number, const Queue_element_base& element_base, ControllerSettingsStruct& ControllerSettings) {
  const C018_queue_element& element = static_cast<const C018_queue_element&>(element_base);
// *INDENT-ON*
uint8_t pl           = (element.packed.length() / 2);
float   airtime_ms   = C018_data->getLoRaAirTime(pl);
bool    mustSetDelay = false;
bool    success      = false;

if (!C018_data->command_finished()) {
  mustSetDelay = true;
} else {
  success = C018_data->txHexBytes(element.packed, ControllerSettings.Port);

  if (success) {
    if (airtime_ms > 0.0f) {
      ADD_TIMER_STAT(C018_AIR_TIME, static_cast<unsigned long>(airtime_ms * 1000));

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("LoRaWAN : Payload Length: ");
        log += pl + 13; // We have a LoRaWAN header of 13 bytes.
        log += F(" Air Time: ");
        log += toString(airtime_ms, 3);
        log += F(" ms");
        addLogMove(LOG_LEVEL_INFO, log);
      }
    }
  }
}
String error = C018_data->getLastError(); // Clear the error string.

if (error.indexOf(F("no_free_ch")) != -1) {
  mustSetDelay = true;
}

if (loglevelActiveFor(LOG_LEVEL_INFO)) {
  String log = F("C018 : Sent: ");
  log += element.packed;
  log += F(" length: ");
  log += String(element.packed.length());

  if (success) {
    log += F(" (success) ");
  }
  log += error;
  addLogMove(LOG_LEVEL_INFO, log);
}

if (mustSetDelay) {
  // Module is still sending, delay for 10x expected air time, which is equivalent of 10% air time duty cycle.
  // This can be retried a few times, so at most 10 retries like these are needed to get below 1% air time again.
  // Very likely only 2 - 3 of these delays are needed, as we have 8 channels to send from and messages are likely sent in bursts.
  C018_DelayHandler->setAdditionalDelay(10 * airtime_ms);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("LoRaWAN : Unable to send. Delay for ");
    log += 10 * airtime_ms;
    log += F(" ms");
    addLogMove(LOG_LEVEL_INFO, log);
  }
}

return success;
}

String c018_add_joinChanged_script_element_line(const String& id, bool forOTAA) {
  String result = F("document.getElementById('tr_");

  result += id;
  result += F("').style.display = style");
  result += forOTAA ? F("OTAA") : F("ABP");
  result += ';';
  return result;
}

#endif // ifdef USES_C018
