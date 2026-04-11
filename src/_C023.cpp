#include "src/Helpers/_CPlugin_Helper.h"

#ifdef USES_C023

// #######################################################################################################
// ########################### Controller Plugin 023: AT-command LoRaWAN ####################
// #######################################################################################################


// Mainly tested on Dragino LA66
// However nearly all LoRaWAN modules supporting AT-commands will very likely work just fine.

// Useful links:
// Notes for TTN: https://wiki.dragino.com/xwiki/bin/view/Main/Notes%20for%20TTN/#H4.A0ConfigurenodeconnectiontoTTNv3
// Dragino WiKi LA66 User Manual:
// LA66 LoRaWAN Shield User Manual"
//   https://wiki.dragino.com/xwiki/bin/view/Main/User%20Manual%20for%20LoRaWAN%20End%20Nodes/LA66%20LoRaWAN%20Shield%20User%20Manual/
//
// https://wiki.dragino.com/xwiki/bin/view/Main/User%20Manual%20for%20LoRaWAN%20End%20Nodes/LA66%20LoRaWAN%20Shield%20User%20Manual/#H1.6A0Example:JoinTTNnetworkandsendanuplinkmessage2Cgetdownlinkmessage.
// End Device AT-Commands and Downlink Command
//   https://wiki.dragino.com/xwiki/bin/view/Main/End%20Device%20AT%20Commands%20and%20Downlink%20Command/
// Semtech LoRa Calculator:
//   https://www.semtech.com/design-support/lora-calculator
// Examples of AT commands on I-Cube-LRWAN
//   https://www.thethingsnetwork.org/forum/uploads/short-url/a5S0svOkbG9bTkvHkcfTNqRDlsq.pdf

// Dragino LGT92_AT_Commands: https://www.dragino.com/downloads/downloads/LGT_92/DRAGINO_LGT92_AT_Commands_v1.5.3.pdf
// Dragino LT IO Controller AT Command Sets:
//  https://www.digikey.nl/htmldatasheets/production/5521267/0/0/1/114992158.pdf

// eWBM LoRa AT command: https://www.tme.eu/Document/6fd083ba11ae9feedaadc9c7221ab46e/eWBM_LoRa_AT_Command_v0.6.pdf

// M5Stack LoRaWAN Atom DTU RAK 3172  AT-commands:
//  https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/docs/products/unit/Unit%20LoRaWAN-CN470/AT%20command%20manual.pdf
//  https://github.com/m5stack/M5-LoRaWAN-RAK
// RAK RUI3 AT Command Manual
//  https://docs.rakwireless.com/product-categories/software-apis-and-libraries/rui3/at-command-manual/
// RAK AT Command Migration Guide to RUI3
//  https://learn.rakwireless.com/hc/en-us/articles/26687498449559-AT-Command-Migration-Guide-of-RAK3172-to-RUI3-RAKwireless-Unified-Interface-V3?_gl=1*kgdaj4*_gcl_au*MTI0NDgyNDMyMy4xNzY1ODQzMTg4
// RAK3172 Overlooked but Useful LoRaWAN® AT Commands
//  https://news.rakwireless.com/rak3172-overlooked-but-useful-lorawan-at-commands/


# define CPLUGIN_023
# define CPLUGIN_ID_023         23
# define CPLUGIN_NAME_023       "AT-command LoRaWAN"


# include <ESPeasySerial.h>

# include "src/ControllerQueue/C023_queue_element.h"
# include "src/Controller_config/C023_config.h"
# include "src/Controller_struct/C023_data_struct.h"
# include "src/DataTypes/ESPEasy_plugin_functions.h"
# include "src/Globals/CPlugins.h"
# include "src/Helpers/_Plugin_Helper_serial.h"
# include "src/Helpers/StringGenerator_GPIO.h"
# include "src/WebServer/Markup.h"
# include "src/WebServer/Markup_Forms.h"
# include "src/WebServer/HTML_wrappers.h"
# include "src/WebServer/KeyValueWriter_WebForm.h"


// Have this define after the includes, so we can set it in Custom.h
# ifndef C023_FORCE_SW_SERIAL
#  define C023_FORCE_SW_SERIAL false
# endif // ifndef C023_FORCE_SW_SERIAL


// FIXME TD-er: Must add a controller data struct vector, like with plugins.
C023_data_struct *C023_data = nullptr;


// Forward declarations
bool C023_init(struct EventStruct *event);

bool CPlugin_023(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      ProtocolStruct& proto = getProtocolStruct(event->idx); //        = CPLUGIN_ID_023;
      proto.usesMQTT       = false;
      proto.usesAccount    = true;
      proto.usesPassword   = true;
      proto.defaultPort    = 1;
      proto.usesID         = true;
      proto.usesHost       = false;
      proto.usesCheckReply = false;
      proto.usesTimeout    = false;
      proto.usesSampleSets = true;
      proto.needsNetwork   = false;
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_023);
      break;
    }

    case CPlugin::Function::CPLUGIN_WEBFORM_SHOW_HOST_CONFIG:
    {
      if ((C023_data != nullptr) && C023_data->isInitialized()) {
        string  = F("Dev addr: ");
        string += C023_data->getDevaddr();
        string += C023_data->useOTAA() ? F(" (OTAA)") : F(" (ABP)");
      } else {
        string = F("-");
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_INIT:
    {
      success = init_c023_delay_queue(event->ControllerIndex);

      if (success) {
        C023_init(event);
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_EXIT:
    {
      if (C023_data != nullptr) {
        C023_data->reset();
        delete C023_data;
        C023_data = nullptr;
      }
      exit_c023_delay_queue();
      break;
    }

    case CPlugin::Function::CPLUGIN_WEBFORM_LOAD:
    {
      {
        // Script to toggle visibility of OTAA/ABP field, based on the activation method selector.
        protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(event->ControllerIndex);
        html_add_script(false);
        addHtml(F(
                  "function joinChanged(elem){ var styleOTAA = elem.value == 0 ? '' : 'none'; var styleABP = elem.value == 1 ? '' : 'none';"));
        LoRa_Helper::add_joinChanged_script_element_line(getControllerParameterInternalName(ProtocolIndex,
                                                                                            ControllerSettingsStruct::CONTROLLER_USER),
                                                         LoRa_Helper::LoRaWAN_JoinMethod::OTAA);
        LoRa_Helper::add_joinChanged_script_element_line(getControllerParameterInternalName(ProtocolIndex,
                                                                                            ControllerSettingsStruct::CONTROLLER_PASS),
                                                         LoRa_Helper::LoRaWAN_JoinMethod::OTAA);
        LoRa_Helper::add_joinChanged_script_element_line(F("deveui"),      LoRa_Helper::LoRaWAN_JoinMethod::OTAA);
        LoRa_Helper::add_joinChanged_script_element_line(F("deveui_note"), LoRa_Helper::LoRaWAN_JoinMethod::OTAA);

        LoRa_Helper::add_joinChanged_script_element_line(F("devaddr"),     LoRa_Helper::LoRaWAN_JoinMethod::ABP);
        LoRa_Helper::add_joinChanged_script_element_line(F("nskey"),       LoRa_Helper::LoRaWAN_JoinMethod::ABP);
        LoRa_Helper::add_joinChanged_script_element_line(F("appskey"),     LoRa_Helper::LoRaWAN_JoinMethod::ABP);
        addHtml('}');
        html_add_script_end();
      }

      {
        // Keep this object in a small scope so we can destruct it as soon as possible again.

        constexpr unsigned size = sizeof(C023_ConfigStruct);
        void *ptr               = special_calloc(1, size);

        if (ptr == nullptr) {
          break;
        }
        UP_C023_ConfigStruct customConfig(new (ptr) C023_ConfigStruct);

        if (!customConfig) {
          break;
        }
        LoadCustomControllerSettings(event->ControllerIndex, reinterpret_cast<uint8_t *>(customConfig.get()), sizeof(C023_ConfigStruct));
        customConfig->webform_load(C023_data);
      }

      if (C023_data)
      {
        KeyValueWriter_WebForm writer(true);

        const __FlashStringHelper *separators[] = {
          F("Keys, IDs and EUIs"),
          F("Access LoRa Network"),
          F("LoRa Network Management"),
          F("Information")
        };

        const C023_AT_commands::AT_cmd commands[] = {
          C023_AT_commands::AT_cmd::UUID,
          C023_AT_commands::AT_cmd::CFM,
          C023_AT_commands::AT_cmd::ADR,
          C023_AT_commands::AT_cmd::RSSI,

          C023_AT_commands::AT_cmd::Unknown
        };

        for (size_t i = 0; i < NR_ELEMENTS(separators); ++i) {
          C023_data->writeCachedValues(
            writer.createChild(separators[i]).get(),
            commands[i], commands[i + 1]);
        }
      }

      break;
    }
    case CPlugin::Function::CPLUGIN_WEBFORM_SAVE:
    {
      constexpr unsigned size = sizeof(C023_ConfigStruct);
      void *ptr               = special_calloc(1, size);

      if (ptr == nullptr) {
        break;
      }
      UP_C023_ConfigStruct customConfig(new (ptr) C023_ConfigStruct);

      if (customConfig) {
        customConfig->webform_save();
        SaveCustomControllerSettings(event->ControllerIndex, reinterpret_cast<const uint8_t *>(customConfig.get()),
                                     sizeof(C023_ConfigStruct));
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_PROTOCOL_DISPLAY_NAME:
    {
      success = true;

      switch (event->idx)
      {
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
      if (C023_DelayHandler == nullptr) {
        break;
      }

      if (C023_DelayHandler->queueFull(event->ControllerIndex)) {
        break;
      }

      if (C023_data != nullptr) {
        {
          constexpr unsigned size = sizeof(C023_queue_element);
          void *ptr               = special_calloc(1, size);

          if (ptr == nullptr) {
            break;
          }

          UP_C023_queue_element element(new (ptr) C023_queue_element(event, C023_data->getSampleSetCount(event->TaskIndex)));
          success = C023_DelayHandler->addToQueue(std::move(element));
          Scheduler.scheduleNextDelayQueue(SchedulerIntervalTimer_e::TIMER_C023_DELAY_QUEUE,
                                           C023_DelayHandler->getNextScheduleTime());
        }

        if (!C023_data->isInitialized()) {
          // Sometimes the module does need some time after power on to respond.
          // So it may not be initialized well at the call of CPLUGIN_INIT
          // We try to trigger its init again when sending data.
          C023_init(event);
        }
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_RECV:
    {
      // FIXME TD-er: WHen should this be scheduled?
      // schedule_controller_event_timer(event->ControllerIndex, CPlugin::Function::CPLUGIN_PROTOCOL_RECV, event);
      break;
    }

    case CPlugin::Function::CPLUGIN_WRITE:
    {
      if (C023_data != nullptr) {
        if (C023_data->isInitialized())
        {
          const String command = parseString(string, 1);

          if (equals(command, F("lorawan"))) {
            const String subcommand = parseString(string, 2);

            if (equals(subcommand, F("write"))) {
              const String loraWriteCommand = parseStringToEnd(string, 3);
              const String res              = C023_data->sendRawCommand(loraWriteCommand);
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
      if (C023_data != nullptr) {
        C023_data->async_loop();
      }

      // FIXME TD-er: Handle reading error state or return values.
      break;
    }

    case CPlugin::Function::CPLUGIN_FLUSH:
    {
      process_c023_delay_queue();
      delay(0);
      break;
    }

    default:
      break;
  }
  return success;
}

bool C023_init(struct EventStruct *event) {
  String AppEUI;
  String AppKey;
  taskIndex_t  SampleSetInitiator = INVALID_TASK_INDEX;
  unsigned int Port               = 0;

  // Check if the object is already created.
  // If so, delete it to make sure the module is initialized according to the full set parameters.
  if (C023_data != nullptr) {
    C023_data->reset();
    delete C023_data;
    C023_data = nullptr;
  }

  {
    constexpr unsigned size = sizeof(C023_data_struct);
    void *ptr               = special_calloc(1, size);

    if (ptr == nullptr) {
      return false;
    }

    C023_data = new (ptr) C023_data_struct;

    if (C023_data == nullptr) {
      return false;
    }
  }
  {
    // Allocate ControllerSettings object in a scope, so we can destruct it as soon as possible.
    MakeControllerSettings(ControllerSettings); // -V522

    if (!AllocatedControllerSettings()) {
      return false;
    }

    LoadControllerSettings(event->ControllerIndex, *ControllerSettings);
    C023_DelayHandler->cacheControllerSettings(*ControllerSettings);
    AppEUI             = getControllerUser(event->ControllerIndex, *ControllerSettings);
    AppKey             = getControllerPass(event->ControllerIndex, *ControllerSettings);
    SampleSetInitiator = ControllerSettings->SampleSetInitiator;
    Port               = ControllerSettings->Port;
  }

  constexpr unsigned size = sizeof(C023_ConfigStruct);
  void *ptr               = special_calloc(1, size);

  if (ptr == nullptr) {
    return false;
  }
  UP_C023_ConfigStruct customConfig(new (ptr) C023_ConfigStruct);

  if (!customConfig) {
    return false;
  }
  LoadCustomControllerSettings(event->ControllerIndex, reinterpret_cast<uint8_t *>(customConfig.get()), sizeof(C023_ConfigStruct));
  customConfig->validate();

  if (!C023_data->init(*customConfig,
                       SampleSetInitiator))
  {
    return false;
  }

  // C023_data->setFrequencyPlan(static_cast<RN2xx3_datatypes::Freq_plan>(customConfig->frequencyplan), customConfig->rx2_freq);

  //  if (!C023_data->setSF(customConfig->sf)) {
  //    return false;
  //  }

  /*
     if (!C023_data->setTTNstack(static_cast<RN2xx3_datatypes::TTN_stack_version>(customConfig->stackVersion))) {
     return false;
     }
   */

  if (customConfig->getJoinMethod() == LoRa_Helper::LoRaWAN_JoinMethod::OTAA) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLogMove(LOG_LEVEL_INFO, strformat(
                   F("OTAA: AppEUI: %s AppKey: %s DevEUI: %s"),
                   AppEUI.c_str(),
                   AppKey.c_str(),
                   customConfig->DeviceEUI));
    }

    if (!C023_data->initOTAA(AppEUI, AppKey, customConfig->DeviceEUI)) {
      return false;
    }
  }
  else {
    if (!C023_data->initABP(customConfig->DeviceAddr, customConfig->AppSessionKey, customConfig->NetworkSessionKey)) {
      return false;
    }
  }
  

  if (!C023_data->join()) { return false; }


  if (!C023_data->txUncnf(F("ESPeasy (TTN)"), Port)) {
    return false;
  }
  return true;
}

// Uncrustify may change this into multi line, which will result in failed builds
// *INDENT-OFF*
bool do_process_c023_delay_queue(cpluginID_t cpluginID, const Queue_element_base& element_base, ControllerSettingsStruct& ControllerSettings) {
  const C023_queue_element& element = static_cast<const C023_queue_element&>(element_base);
// *INDENT-ON*
uint8_t pl           = (element.packed.length() / 2);
float   airtime_ms   = C023_data->getLoRaAirTime(pl);
bool    mustSetDelay = false;
bool    success      = false;

if (!C023_data->command_finished()) {
  mustSetDelay = true;
} else {
  success = C023_data->txHexBytes(element.packed, ControllerSettings.Port);

  if (success) {
    if (airtime_ms > 0.0f) {
      ADD_TIMER_STAT(C023_AIR_TIME, static_cast<unsigned long>(airtime_ms * 1000));

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
String error = C023_data->getLastError(); // Clear the error string.

if (error.indexOf(F("no_free_ch")) != -1) {
  mustSetDelay = true;
}

if (loglevelActiveFor(LOG_LEVEL_INFO)) {
  String log = F("C023 : Sent: ");
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
  C023_DelayHandler->setAdditionalDelay(10 * airtime_ms);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("LoRaWAN : Unable to send. Delay for ");
    log += 10 * airtime_ms;
    log += F(" ms");
    addLogMove(LOG_LEVEL_INFO, log);
  }
}

return success;
}

#endif // ifdef USES_C023
