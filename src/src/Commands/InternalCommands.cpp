#include "../Commands/InternalCommands.h"

#include "../../ESPEasy_common.h"

#include "../../_Plugin_Helper.h"
#include "../Globals/Settings.h"

#if FEATURE_BLYNK
# include "../Commands/Blynk.h"
# include "../Commands/Blynk_c015.h"
#endif // if FEATURE_BLYNK

#include "../Commands/Common.h"
#include "../Commands/Controller.h"
#include "../Commands/Diagnostic.h"
#include "../Commands/GPIO.h"
#include "../Commands/HTTP.h"
#include "../Commands/InternalCommands_decoder.h"
#include "../Commands/i2c.h"

#if FEATURE_MQTT
# include "../Commands/MQTT.h"
#endif // if FEATURE_MQTT

#include "../Commands/Networks.h"
#if FEATURE_NOTIFIER
# include "../Commands/Notifications.h"
#endif // if FEATURE_NOTIFIER
#include "../Commands/Provisioning.h"
#include "../Commands/RTC.h"
#include "../Commands/Rules.h"
#include "../Commands/SDCARD.h"
#include "../Commands/Settings.h"
#if FEATURE_SERVO
# include "../Commands/Servo.h"
#endif // if FEATURE_SERVO
#include "../Commands/System.h"
#include "../Commands/Tasks.h"
#include "../Commands/Time.h"
#include "../Commands/Timer.h"
#include "../Commands/UPD.h"
#include "../Commands/wd.h"
#include "../Commands/WiFi.h"

#include "../DataStructs/TimingStats.h"

#include "../ESPEasyCore/ESPEasy_Log.h"

#include "../Helpers/Misc.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringParser.h"


bool checkNrArguments(const char *cmd, const String& Line, int nrArguments) {
  if (nrArguments < 0) { return true; }

  // 0 arguments means argument on pos1 is valid (the command) and argpos 2 should not be there.
  if (HasArgv(Line.c_str(), nrArguments + 2)) {
    #ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log;

      if (log.reserve(128)) {
        log += F("Too many arguments: cmd=");
        log += cmd;

        if (nrArguments < 1) {
          log += Line;
        } else {
          // Check for one more argument than allowed, since we apparently have one.
          bool done = false;
          int  i    = 1;

          while (!done) {
            String parameter;

            if (i == nrArguments) {
              parameter = tolerantParseStringKeepCase(Line, i + 1);
            } else {
              parameter = parseStringKeepCase(Line, i + 1);
            }
            done = parameter.isEmpty();

            if (!done) {
              if (i <= nrArguments) {
                if (Settings.TolerantLastArgParse() && (i == nrArguments)) {
                  log += F(" (fixed)");
                }
                log += F(" Arg");
              } else {
                log += F(" ExtraArg");
              }
              log += i;
              log += '=';
              log += parameter;
            }
            ++i;
          }
        }
        log += F(" lineLength=");
        log += Line.length();
        addLogMove(LOG_LEVEL_ERROR, log);
      }
      addLogMove(LOG_LEVEL_ERROR, strformat(F("Line: _%s_"), Line.c_str()));

      addLogMove(LOG_LEVEL_ERROR, concat(Settings.TolerantLastArgParse() ?
                                         F("Command executed, but may fail.") : F("Command not executed!"),
                                         F(" See: https://github.com/letscontrolit/ESPEasy/issues/2724")));
    }
    #endif // ifndef BUILD_NO_DEBUG

    if (Settings.TolerantLastArgParse()) {
      return true;
    }
    return false;
  }
  return true;
}

bool checkSourceFlags(EventValueSource::Enum source, EventValueSourceGroup::Enum group) {
  if (EventValueSource::partOfGroup(source, group)) {
    return true;
  }
  addLog(LOG_LEVEL_ERROR, return_incorrect_source());
  return false;
}

command_case_data::command_case_data(const char *cmd, struct EventStruct *event, const char *line) :
  cmd(cmd), event(event), line(line)
{
  cmd_lc = cmd;
  cmd_lc.toLowerCase();
}

InternalCommands::InternalCommands(const char *cmd, struct EventStruct *event, const char *line)
  : _data(cmd, event, line) {}


// Wrapper to reduce generated code by macro
bool InternalCommands::do_command_case_all(command_function_fs pFunc,
                                           int                 nrArguments)
{
  return do_command_case(_data, pFunc, nrArguments, EventValueSourceGroup::Enum::ALL);
}

bool InternalCommands::do_command_case_all(command_function pFunc,
                                           int              nrArguments)
{
  return do_command_case(_data, pFunc, nrArguments, EventValueSourceGroup::Enum::ALL);
}

// Wrapper to reduce generated code by macro
bool InternalCommands::do_command_case_all_restricted(command_function_fs pFunc,
                                                      int                 nrArguments)
{
  return do_command_case(_data,  pFunc, nrArguments, EventValueSourceGroup::Enum::RESTRICTED);
}

bool InternalCommands::do_command_case_all_restricted(command_function pFunc,
                                                      int              nrArguments)
{
  return do_command_case(_data,  pFunc, nrArguments, EventValueSourceGroup::Enum::RESTRICTED);
}

bool do_command_case_check(command_case_data         & data,
                           int                         nrArguments,
                           EventValueSourceGroup::Enum group)
{
  // The data struct is re-used on each attempt to process an internal command.
  // Re-initialize the only two members that may have been altered by a previous call.
  data.retval = false;
  data.status = String();

  if (!checkSourceFlags(data.event->Source, group)) {
    data.status = return_incorrect_source();
    return false;
  }

  // FIXME TD-er: Do not check nr arguments from MQTT source.
  // See https://github.com/letscontrolit/ESPEasy/issues/3344
  // C005 does recreate command partly from topic and published message
  // e.g. ESP_Easy/Bathroom_pir_env/GPIO/14 with data 0 or 1
  // This only allows for 2 parameters, but some commands need more arguments (default to "0")
  const bool mustCheckNrArguments = data.event->Source != EventValueSource::Enum::VALUE_SOURCE_MQTT;

  if (mustCheckNrArguments) {
    if (!checkNrArguments(data.cmd, data.line, nrArguments)) {
      data.status = return_incorrect_nr_arguments();

      // data.retval = false;
      return true;    // Command is handled
    }
  }
  data.retval = true; // Mark the command should be executed.
  return true;        // Command is handled
}

bool InternalCommands::do_command_case(command_case_data         & data,
                                       command_function_fs         pFunc,
                                       int                         nrArguments,
                                       EventValueSourceGroup::Enum group)
{
  if (do_command_case_check(data, nrArguments, group)) {
    // It has been handled, check if we need to execute it.
    // FIXME TD-er: Must change command function signature to use const String&
    START_TIMER;
    data.status = pFunc(data.event, data.line.c_str());
    STOP_TIMER(COMMAND_EXEC_INTERNAL);
    return true;
  }
  return false;
}

bool InternalCommands::do_command_case(command_case_data         & data,
                                       command_function            pFunc,
                                       int                         nrArguments,
                                       EventValueSourceGroup::Enum group)
{
  if (do_command_case_check(data, nrArguments, group)) {
    // It has been handled, check if we need to execute it.
    // FIXME TD-er: Must change command function signature to use const String&
    START_TIMER;
    data.status = pFunc(data.event, data.line.c_str());
    STOP_TIMER(COMMAND_EXEC_INTERNAL);
    return true;
  }
  return false;
}

bool InternalCommands::executeInternalCommand()
{
  // Simple macro to match command to function call.

  // EventValueSourceGroup::Enum::ALL
  #define COMMAND_CASE_A(C, NARGS) \
  do_command_case_all(&C, NARGS); break;

  // EventValueSourceGroup::Enum::RESTRICTED
  #define COMMAND_CASE_R(C, NARGS) \
   do_command_case_all_restricted(&C, NARGS); break;


  const ESPEasy_cmd_e cmd = match_ESPEasy_internal_command(_data.cmd_lc);

  _data.retval = false;

  if (cmd == ESPEasy_cmd_e::NotMatched) {
    return false;
  }

  // FIXME TD-er: Should we execute command when number of arguments is wrong?

  // FIXME TD-er: must determine nr arguments where NARGS is set to -1
  switch (cmd) {
    case ESPEasy_cmd_e::accessinfo:                 COMMAND_CASE_A(Command_AccessInfo_Ls,       0); // Network Command
    case ESPEasy_cmd_e::asyncevent:                 COMMAND_CASE_A(Command_Rules_Async_Events, -1); // Rule.h
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
    case ESPEasy_cmd_e::background:                 COMMAND_CASE_R(Command_Background, 1);          // Diagnostic.h
#endif // ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
#ifdef USES_C012
    case ESPEasy_cmd_e::blynkget:                   COMMAND_CASE_A(Command_Blynk_Get, -1);
#endif // ifdef USES_C012
#ifdef USES_C015
    case ESPEasy_cmd_e::blynkset:                   COMMAND_CASE_R(Command_Blynk_Set, -1);
#endif // ifdef USES_C015
    case ESPEasy_cmd_e::build:                      COMMAND_CASE_A(Command_Settings_Build, 1);      // Settings.h
    case ESPEasy_cmd_e::clearaccessblock:           COMMAND_CASE_R(Command_AccessInfo_Clear,   0);  // Network Command
    case ESPEasy_cmd_e::clearpassword:              COMMAND_CASE_R(Command_Settings_Password_Clear, 1);      // Settings.h
    case ESPEasy_cmd_e::clearrtcram:                COMMAND_CASE_R(Command_RTC_Clear,          0);           // RTC.h
#ifdef ESP8266
    case ESPEasy_cmd_e::clearsdkwifi:               COMMAND_CASE_R(Command_System_Erase_SDK_WiFiconfig,  0); // System.h
    case ESPEasy_cmd_e::clearwifirfcal:             COMMAND_CASE_R(Command_System_Erase_RFcal,  0);          // System.h
#endif // ifdef ESP8266
    case ESPEasy_cmd_e::config:                     COMMAND_CASE_R(Command_Task_RemoteConfig, -1);           // Tasks.h
    case ESPEasy_cmd_e::controllerdisable:          COMMAND_CASE_R(Command_Controller_Disable, 1);           // Controller.h
    case ESPEasy_cmd_e::controllerenable:           COMMAND_CASE_R(Command_Controller_Enable,  1);           // Controller.h
    case ESPEasy_cmd_e::datetime:                   COMMAND_CASE_R(Command_DateTime,             2);         // Time.h
    case ESPEasy_cmd_e::debug:                      COMMAND_CASE_R(Command_Debug,                1);         // Diagnostic.h
    case ESPEasy_cmd_e::dec:                        COMMAND_CASE_A(Command_Rules_Dec,           -1);         // Rules.h
    case ESPEasy_cmd_e::deepsleep:                  COMMAND_CASE_R(Command_System_deepSleep,     1);         // System.h
    case ESPEasy_cmd_e::delay:                      COMMAND_CASE_R(Command_Delay,                1);         // Timers.h
#if FEATURE_PLUGIN_PRIORITY
    case ESPEasy_cmd_e::disableprioritytask:        COMMAND_CASE_R(Command_PriorityTask_Disable, 1);         // Tasks.h
#endif // if FEATURE_PLUGIN_PRIORITY
    case ESPEasy_cmd_e::dns:                        COMMAND_CASE_R(Command_DNS,                  1);         // Network Command
    case ESPEasy_cmd_e::dst:                        COMMAND_CASE_R(Command_DST,                  1);         // Time.h
#if FEATURE_ETHERNET
    case ESPEasy_cmd_e::ethphyadr:                  COMMAND_CASE_R(Command_ETH_Phy_Addr,   1);               // Network Command
    case ESPEasy_cmd_e::ethpinmdc:                  COMMAND_CASE_R(Command_ETH_Pin_mdc,    1);               // Network Command
    case ESPEasy_cmd_e::ethpinmdio:                 COMMAND_CASE_R(Command_ETH_Pin_mdio,   1);               // Network Command
    case ESPEasy_cmd_e::ethpinpower:                COMMAND_CASE_R(Command_ETH_Pin_power,  1);               // Network Command
    case ESPEasy_cmd_e::ethphytype:                 COMMAND_CASE_R(Command_ETH_Phy_Type,   1);               // Network Command
    case ESPEasy_cmd_e::ethclockmode:               COMMAND_CASE_R(Command_ETH_Clock_Mode, 1);               // Network Command
    case ESPEasy_cmd_e::ethip:                      COMMAND_CASE_R(Command_ETH_IP,         1);               // Network Command
    case ESPEasy_cmd_e::ethgateway:                 COMMAND_CASE_R(Command_ETH_Gateway,    1);               // Network Command
    case ESPEasy_cmd_e::ethsubnet:                  COMMAND_CASE_R(Command_ETH_Subnet,     1);               // Network Command
    case ESPEasy_cmd_e::ethdns:                     COMMAND_CASE_R(Command_ETH_DNS,        1);               // Network Command
    case ESPEasy_cmd_e::ethdisconnect:              COMMAND_CASE_A(Command_ETH_Disconnect, 0);               // Network Command
    case ESPEasy_cmd_e::ethwifimode:                COMMAND_CASE_R(Command_ETH_Wifi_Mode,  1);               // Network Command
#endif // FEATURE_ETHERNET
    case ESPEasy_cmd_e::erasesdkwifi:               COMMAND_CASE_R(Command_WiFi_Erase,     0);               // WiFi.h
    case ESPEasy_cmd_e::event:                      COMMAND_CASE_A(Command_Rules_Events,  -1);               // Rule.h
    case ESPEasy_cmd_e::executerules:               COMMAND_CASE_A(Command_Rules_Execute, -1);               // Rule.h
    case ESPEasy_cmd_e::gateway:                    COMMAND_CASE_R(Command_Gateway,     1);                  // Network Command
    case ESPEasy_cmd_e::gpio:                       COMMAND_CASE_A(Command_GPIO,        2);                  // Gpio.h
    case ESPEasy_cmd_e::gpiotoggle:                 COMMAND_CASE_A(Command_GPIO_Toggle, 1);                  // Gpio.h
    case ESPEasy_cmd_e::hiddenssid:                 COMMAND_CASE_R(Command_Wifi_HiddenSSID, 1);              // wifi.h
    case ESPEasy_cmd_e::i2cscanner:                 COMMAND_CASE_R(Command_i2c_Scanner, -1);                 // i2c.h
    case ESPEasy_cmd_e::inc:                        COMMAND_CASE_A(Command_Rules_Inc,   -1);                 // Rules.h
    case ESPEasy_cmd_e::ip:                         COMMAND_CASE_R(Command_IP,           1);                 // Network Command
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
    case ESPEasy_cmd_e::jsonportstatus:             COMMAND_CASE_A(Command_JSONPortStatus, -1);              // Diagnostic.h
#endif // ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
    case ESPEasy_cmd_e::let:                        COMMAND_CASE_A(Command_Rules_Let,         2);            // Rules.h
    case ESPEasy_cmd_e::load:                       COMMAND_CASE_A(Command_Settings_Load,     0);            // Settings.h
    case ESPEasy_cmd_e::logentry:                   COMMAND_CASE_A(Command_logentry,         -1);            // Diagnostic.h
    case ESPEasy_cmd_e::looptimerset:               COMMAND_CASE_A(Command_Loop_Timer_Set,    3);            // Timers.h
    case ESPEasy_cmd_e::looptimerset_ms:            COMMAND_CASE_A(Command_Loop_Timer_Set_ms, 3);            // Timers.h
    case ESPEasy_cmd_e::longpulse:                  COMMAND_CASE_A(Command_GPIO_LongPulse,    5);            // GPIO.h
    case ESPEasy_cmd_e::longpulse_ms:               COMMAND_CASE_A(Command_GPIO_LongPulse_Ms, 5);            // GPIO.h
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
    case ESPEasy_cmd_e::logportstatus:              COMMAND_CASE_A(Command_logPortStatus,     0);            // Diagnostic.h
    case ESPEasy_cmd_e::lowmem:                     COMMAND_CASE_A(Command_Lowmem,            0);            // Diagnostic.h
#endif // ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
#ifdef USES_P009
    case ESPEasy_cmd_e::mcpgpio:                    COMMAND_CASE_A(Command_GPIO,              2);            // Gpio.h
    case ESPEasy_cmd_e::mcpgpiorange:               COMMAND_CASE_A(Command_GPIO_McpGPIORange, -1);           // Gpio.h
    case ESPEasy_cmd_e::mcpgpiopattern:             COMMAND_CASE_A(Command_GPIO_McpGPIOPattern, -1);         // Gpio.h
    case ESPEasy_cmd_e::mcpgpiotoggle:              COMMAND_CASE_A(Command_GPIO_Toggle,       1);            // Gpio.h
    case ESPEasy_cmd_e::mcplongpulse:               COMMAND_CASE_A(Command_GPIO_LongPulse,    3);            // GPIO.h
    case ESPEasy_cmd_e::mcplongpulse_ms:            COMMAND_CASE_A(Command_GPIO_LongPulse_Ms, 3);            // GPIO.h
    case ESPEasy_cmd_e::mcpmode:                    COMMAND_CASE_A(Command_GPIO_Mode,         2);            // Gpio.h
    case ESPEasy_cmd_e::mcpmoderange:               COMMAND_CASE_A(Command_GPIO_ModeRange,    3);            // Gpio.h
    case ESPEasy_cmd_e::mcppulse:                   COMMAND_CASE_A(Command_GPIO_Pulse,        3);            // GPIO.h
#endif // ifdef USES_P009
    case ESPEasy_cmd_e::monitor:                    COMMAND_CASE_A(Command_GPIO_Monitor,      2);            // GPIO.h
    case ESPEasy_cmd_e::monitorrange:               COMMAND_CASE_A(Command_GPIO_MonitorRange, 3);            // GPIO.h
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
    case ESPEasy_cmd_e::malloc:                     COMMAND_CASE_A(Command_Malloc,         1);               // Diagnostic.h
    case ESPEasy_cmd_e::meminfo:                    COMMAND_CASE_A(Command_MemInfo,        0);               // Diagnostic.h
    case ESPEasy_cmd_e::meminfodetail:              COMMAND_CASE_A(Command_MemInfo_detail, 0);               // Diagnostic.h
#endif // ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
    case ESPEasy_cmd_e::name:                       COMMAND_CASE_R(Command_Settings_Name,        1);         // Settings.h
    case ESPEasy_cmd_e::nosleep:                    COMMAND_CASE_R(Command_System_NoSleep,       1);         // System.h
#if FEATURE_NOTIFIER
    case ESPEasy_cmd_e::notify:                     COMMAND_CASE_R(Command_Notifications_Notify, 2);         // Notifications.h
#endif // if FEATURE_NOTIFIER
    case ESPEasy_cmd_e::ntphost:                    COMMAND_CASE_R(Command_NTPHost,              1);         // Time.h
#ifdef USES_P019
    case ESPEasy_cmd_e::pcfgpio:                    COMMAND_CASE_A(Command_GPIO,                 2);         // Gpio.h
    case ESPEasy_cmd_e::pcfgpiorange:               COMMAND_CASE_A(Command_GPIO_PcfGPIORange,   -1);         // Gpio.h
    case ESPEasy_cmd_e::pcfgpiopattern:             COMMAND_CASE_A(Command_GPIO_PcfGPIOPattern, -1);         // Gpio.h
    case ESPEasy_cmd_e::pcfgpiotoggle:              COMMAND_CASE_A(Command_GPIO_Toggle,          1);         // Gpio.h
    case ESPEasy_cmd_e::pcflongpulse:               COMMAND_CASE_A(Command_GPIO_LongPulse,       3);         // GPIO.h
    case ESPEasy_cmd_e::pcflongpulse_ms:            COMMAND_CASE_A(Command_GPIO_LongPulse_Ms,    3);         // GPIO.h
    case ESPEasy_cmd_e::pcfmode:                    COMMAND_CASE_A(Command_GPIO_Mode,            2);         // Gpio.h
    case ESPEasy_cmd_e::pcfmoderange:               COMMAND_CASE_A(Command_GPIO_ModeRange,       3);         // Gpio.h   ************
    case ESPEasy_cmd_e::pcfpulse:                   COMMAND_CASE_A(Command_GPIO_Pulse,           3);         // GPIO.h
#endif // ifdef USES_P019
    case ESPEasy_cmd_e::password:                   COMMAND_CASE_R(Command_Settings_Password, 1);            // Settings.h
#if FEATURE_POST_TO_HTTP
    case ESPEasy_cmd_e::posttohttp:                 COMMAND_CASE_A(Command_HTTP_PostToHTTP,  -1);            // HTTP.h
#endif // if FEATURE_POST_TO_HTTP
#if FEATURE_CUSTOM_PROVISIONING
    case ESPEasy_cmd_e::provision:                  COMMAND_CASE_A(Command_Provisioning_Dispatcher, -1);     // Provisioning.h
# ifdef PLUGIN_BUILD_MAX_ESP32

    // FIXME DEPRECATED: Fallback for temporary backward compatibility
    case ESPEasy_cmd_e::provisionconfig:            COMMAND_CASE_A(Command_Provisioning_ConfigFallback,       0); // Provisioning.h
    case ESPEasy_cmd_e::provisionsecurity:          COMMAND_CASE_A(Command_Provisioning_SecurityFallback,     0); // Provisioning.h
#  if FEATURE_NOTIFIER
    case ESPEasy_cmd_e::provisionnotification:      COMMAND_CASE_A(Command_Provisioning_NotificationFallback, 0); // Provisioning.h
#  endif // if FEATURE_NOTIFIER
    case ESPEasy_cmd_e::provisionprovision:         COMMAND_CASE_A(Command_Provisioning_ProvisionFallback,    0); // Provisioning.h
    case ESPEasy_cmd_e::provisionrules:             COMMAND_CASE_A(Command_Provisioning_RulesFallback,        1); // Provisioning.h
    case ESPEasy_cmd_e::provisionfirmware:          COMMAND_CASE_A(Command_Provisioning_FirmwareFallback,     1); // Provisioning.h
# endif // ifdef PLUGIN_BUILD_MAX_ESP32
#endif // if FEATURE_CUSTOM_PROVISIONING
    case ESPEasy_cmd_e::pulse:                      COMMAND_CASE_A(Command_GPIO_Pulse,        3);                 // GPIO.h
#if FEATURE_MQTT
    case ESPEasy_cmd_e::publish:                    COMMAND_CASE_A(Command_MQTT_Publish,     -1);                 // MQTT.h
#endif // if FEATURE_MQTT
#if FEATURE_PUT_TO_HTTP
    case ESPEasy_cmd_e::puttohttp:                  COMMAND_CASE_A(Command_HTTP_PutToHTTP,  -1);                  // HTTP.h
#endif // if FEATURE_PUT_TO_HTTP
    case ESPEasy_cmd_e::pwm:                        COMMAND_CASE_A(Command_GPIO_PWM,          4);                 // GPIO.h
    case ESPEasy_cmd_e::reboot:                     COMMAND_CASE_A(Command_System_Reboot,              0);        // System.h
    case ESPEasy_cmd_e::reset:                      COMMAND_CASE_R(Command_Settings_Reset,             0);        // Settings.h
    case ESPEasy_cmd_e::resetflashwritecounter:     COMMAND_CASE_A(Command_RTC_resetFlashWriteCounter, 0);        // RTC.h
    case ESPEasy_cmd_e::restart:                    COMMAND_CASE_A(Command_System_Reboot,              0);        // System.h
    case ESPEasy_cmd_e::rtttl:                      COMMAND_CASE_A(Command_GPIO_RTTTL,                -1);        // GPIO.h
    case ESPEasy_cmd_e::rules:                      COMMAND_CASE_A(Command_Rules_UseRules,             1);        // Rule.h
    case ESPEasy_cmd_e::save:                       COMMAND_CASE_R(Command_Settings_Save, 0);                     // Settings.h
    case ESPEasy_cmd_e::scheduletaskrun:            COMMAND_CASE_A(Command_ScheduleTask_Run, 2);                  // Tasks.h

#if FEATURE_SD
    case ESPEasy_cmd_e::sdcard:                     COMMAND_CASE_R(Command_SD_LS,         0);                     // SDCARDS.h
    case ESPEasy_cmd_e::sdremove:                   COMMAND_CASE_R(Command_SD_Remove,     1);                     // SDCARDS.h
#endif // if FEATURE_SD

#if FEATURE_ESPEASY_P2P

    // FIXME TD-er: These send commands, can we determine the nr of arguments?
    case ESPEasy_cmd_e::sendto:                     COMMAND_CASE_A(Command_UPD_SendTo,      2);      // UDP.h
#endif // if FEATURE_ESPEASY_P2P
#if FEATURE_SEND_TO_HTTP
    case ESPEasy_cmd_e::sendtohttp:                 COMMAND_CASE_A(Command_HTTP_SendToHTTP, 3);      // HTTP.h
#endif // FEATURE_SEND_TO_HTTP
    case ESPEasy_cmd_e::sendtoudp:                  COMMAND_CASE_A(Command_UDP_SendToUPD,   3);      // UDP.h
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
    case ESPEasy_cmd_e::serialfloat:                COMMAND_CASE_R(Command_SerialFloat,    0);       // Diagnostic.h
#endif // ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
    case ESPEasy_cmd_e::settings:                   COMMAND_CASE_R(Command_Settings_Print, 0);       // Settings.h
#if FEATURE_SERVO
    case ESPEasy_cmd_e::servo:                      COMMAND_CASE_A(Command_Servo,          3);       // Servo.h
#endif // if FEATURE_SERVO

    case ESPEasy_cmd_e::status:                     COMMAND_CASE_A(Command_GPIO_Status,          2); // GPIO.h
    case ESPEasy_cmd_e::subnet:                     COMMAND_CASE_R(Command_Subnet, 1);               // Network Command
#if FEATURE_MQTT
    case ESPEasy_cmd_e::subscribe:                  COMMAND_CASE_A(Command_MQTT_Subscribe, 1);       // MQTT.h
#endif // if FEATURE_MQTT
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
    case ESPEasy_cmd_e::sysload:                    COMMAND_CASE_A(Command_SysLoad,        0);       // Diagnostic.h
#endif // ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
    case ESPEasy_cmd_e::taskclear:                  COMMAND_CASE_R(Command_Task_Clear,    1);        // Tasks.h
    case ESPEasy_cmd_e::taskclearall:               COMMAND_CASE_R(Command_Task_ClearAll, 0);        // Tasks.h
    case ESPEasy_cmd_e::taskdisable:                COMMAND_CASE_R(Command_Task_Disable,  1);        // Tasks.h
    case ESPEasy_cmd_e::taskenable:                 COMMAND_CASE_R(Command_Task_Enable,   1);        // Tasks.h
    case ESPEasy_cmd_e::taskrun:                    COMMAND_CASE_A(Command_Task_Run,            1);  // Tasks.h
    case ESPEasy_cmd_e::taskrunat:                  COMMAND_CASE_A(Command_Task_Run,            2);  // Tasks.h
    case ESPEasy_cmd_e::taskvalueset:               COMMAND_CASE_A(Command_Task_ValueSet,       3);  // Tasks.h
    case ESPEasy_cmd_e::taskvaluetoggle:            COMMAND_CASE_A(Command_Task_ValueToggle,    2);  // Tasks.h
    case ESPEasy_cmd_e::taskvaluesetandrun:         COMMAND_CASE_A(Command_Task_ValueSetAndRun, 3);  // Tasks.h
    case ESPEasy_cmd_e::timerpause:                 COMMAND_CASE_A(Command_Timer_Pause,  1);         // Timers.h
    case ESPEasy_cmd_e::timerresume:                COMMAND_CASE_A(Command_Timer_Resume, 1);         // Timers.h
    case ESPEasy_cmd_e::timerset:                   COMMAND_CASE_A(Command_Timer_Set,    2);         // Timers.h
    case ESPEasy_cmd_e::timerset_ms:                COMMAND_CASE_A(Command_Timer_Set_ms, 2);         // Timers.h
    case ESPEasy_cmd_e::timezone:                   COMMAND_CASE_R(Command_TimeZone, 1);             // Time.h
    case ESPEasy_cmd_e::tone:                       COMMAND_CASE_A(Command_GPIO_Tone, 3);            // GPIO.h
    case ESPEasy_cmd_e::udpport:                    COMMAND_CASE_R(Command_UDP_Port,      1);        // UDP.h
#if FEATURE_ESPEASY_P2P
    case ESPEasy_cmd_e::udptest:                    COMMAND_CASE_R(Command_UDP_Test,      2);        // UDP.h
#endif // if FEATURE_ESPEASY_P2P
    case ESPEasy_cmd_e::unit:                       COMMAND_CASE_R(Command_Settings_Unit, 1);        // Settings.h
    case ESPEasy_cmd_e::unmonitor:                  COMMAND_CASE_A(Command_GPIO_UnMonitor, 2);       // GPIO.h
    case ESPEasy_cmd_e::unmonitorrange:             COMMAND_CASE_A(Command_GPIO_UnMonitorRange, 3);  // GPIO.h
    case ESPEasy_cmd_e::usentp:                     COMMAND_CASE_R(Command_useNTP, 1);               // Time.h
#ifndef LIMIT_BUILD_SIZE
    case ESPEasy_cmd_e::wdconfig:                   COMMAND_CASE_R(Command_WD_Config, 3);            // WD.h
    case ESPEasy_cmd_e::wdread:                     COMMAND_CASE_R(Command_WD_Read,   2);            // WD.h
#endif // ifndef LIMIT_BUILD_SIZE

    case ESPEasy_cmd_e::wifiallowap:                COMMAND_CASE_R(Command_Wifi_AllowAP,    0);      // WiFi.h
    case ESPEasy_cmd_e::wifiapmode:                 COMMAND_CASE_R(Command_Wifi_APMode,     0);      // WiFi.h
    case ESPEasy_cmd_e::wificonnect:                COMMAND_CASE_A(Command_Wifi_Connect,    0);      // WiFi.h
    case ESPEasy_cmd_e::wifidisconnect:             COMMAND_CASE_A(Command_Wifi_Disconnect, 0);      // WiFi.h
    case ESPEasy_cmd_e::wifikey:                    COMMAND_CASE_R(Command_Wifi_Key,        1);      // WiFi.h
    case ESPEasy_cmd_e::wifikey2:                   COMMAND_CASE_R(Command_Wifi_Key2,       1);      // WiFi.h
    case ESPEasy_cmd_e::wifimode:                   COMMAND_CASE_R(Command_Wifi_Mode,       1);      // WiFi.h
    case ESPEasy_cmd_e::wifiscan:                   COMMAND_CASE_R(Command_Wifi_Scan,       0);      // WiFi.h
    case ESPEasy_cmd_e::wifissid:                   COMMAND_CASE_R(Command_Wifi_SSID,       1);      // WiFi.h
    case ESPEasy_cmd_e::wifissid2:                  COMMAND_CASE_R(Command_Wifi_SSID2,      1);      // WiFi.h
    case ESPEasy_cmd_e::wifistamode:                COMMAND_CASE_R(Command_Wifi_STAMode,    0);      // WiFi.h


    case ESPEasy_cmd_e::NotMatched:
      return false;

      // Do not add default: here
      // The compiler will then warn when a command is not included
  }

  #undef COMMAND_CASE_R
  #undef COMMAND_CASE_A
  return _data.retval;
}
