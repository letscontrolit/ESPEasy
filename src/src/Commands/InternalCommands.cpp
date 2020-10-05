#include "InternalCommands.h"

#include "../../ESPEasy_common.h"
#include "../../ESPEasy_fdwdecl.h"
#include "../../ESPEasy_Log.h"
#include "../../_Plugin_Helper.h"
#include "../Globals/Settings.h"

#ifdef USES_BLYNK
# include "../Commands/Blynk.h"
# include "../Commands/Blynk_c015.h"
#endif // ifdef USES_BLYNK

#include "../Commands/Common.h"
#include "../Commands/Controller.h"
#include "../Commands/Diagnostic.h"
#include "../Commands/HTTP.h"
#include "../Commands/i2c.h"

#ifdef USES_MQTT
# include "../Commands/MQTT.h"
#endif // USES_MQTT

#include "../Commands/Networks.h"
#include "../Commands/Notifications.h"
#include "../Commands/RTC.h"
#include "../Commands/Rules.h"
#include "../Commands/SDCARD.h"
#include "../Commands/Settings.h"
#include "../Commands/System.h"
#include "../Commands/Tasks.h"
#include "../Commands/Time.h"
#include "../Commands/Timer.h"
#include "../Commands/UPD.h"
#include "../Commands/wd.h"
#include "../Commands/WiFi.h"

#include "../Helpers/Misc.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringParser.h"


bool checkNrArguments(const char *cmd, const char *Line, int nrArguments) {
  if (nrArguments < 0) { return true; }

  // 0 arguments means argument on pos1 is valid (the command) and argpos 2 should not be there.
  if (HasArgv(Line, nrArguments + 2)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log;
      log.reserve(128);
      log += F("Too many arguments: cmd=");
      log += cmd;

      if (nrArguments < 1) {
        log += Line;
      } else {
        // Check for one more argument than allowed, since we apparently have one.
        bool done = false;
        int i = 1;
        while (!done) {
          String parameter;
          if (i == nrArguments) {
            parameter = tolerantParseStringKeepCase(Line, i+ 1);
            done = true;
          } else {
            parameter = parseStringKeepCase(Line, i + 1);
          }
          done = parameter.length() == 0;
          if (!done) {
            if (i <= nrArguments) {
              if (Settings.TolerantLastArgParse() && i == nrArguments) {
                log += F(" (fixed)");
              }
              log += F(" Arg");
            } else {
              log += F(" ExtraArg");
            }
            log += String(i);
            log += '=';
            log += parameter;
          }
          ++i;
        }
      }
      log += F(" lineLength=");
      log += strlen(Line);
      addLog(LOG_LEVEL_ERROR, log);
      log = F("Line: _");
      log += Line;
      log += '_';
      addLog(LOG_LEVEL_ERROR, log);

      if (!Settings.TolerantLastArgParse()) {
        log = F("Command not executed!");
      } else {
        log = F("Command executed, but may fail.");
      }
      log += F(" See: https://github.com/letscontrolit/ESPEasy/issues/2724");
      addLog(LOG_LEVEL_ERROR, log);
    }
    if (Settings.TolerantLastArgParse()) {
      return true;
    }
    return false;
  }
  return true;
}

bool do_command_case(const String& cmd_lc, const char *cmd, struct EventStruct *event, const char *line, String& status, const String& cmd_test, command_function pFunc, int nrArguments, bool& retval)
{
  if (cmd_lc.equals(cmd_test)) {
    if (!checkNrArguments(cmd, line, nrArguments)) {
      status = return_incorrect_nr_arguments(); 
      retval = false;
    } else  {
      status = pFunc(event, line); 
      retval = true;
    }
    return true; // Command is handled
  }
  return false;
}


bool executeInternalCommand(const char *cmd, struct EventStruct *event, const char *line, String& status)
{
  String cmd_lc;

  cmd_lc = cmd;
  cmd_lc.toLowerCase();
  bool retval;
  // Simple macro to match command to function call.
  #define COMMAND_CASE(S, C, NARGS) \
    if (do_command_case(cmd_lc, cmd, event, line, status, F(S), &C, NARGS, retval)) { return retval; }

  // FIXME TD-er: Should we execute command when number of arguments is wrong?

  // FIXME TD-er: must determine nr arguments where NARGS is set to -1

  switch (cmd_lc[0]) {
    case 'a': {
      COMMAND_CASE("accessinfo", Command_AccessInfo_Ls, 0); // Network Command
      COMMAND_CASE("asyncevent", Command_Rules_Async_Events,  -1); // Rule.h
      break;
    }
    case 'b': {
    #ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
      COMMAND_CASE("background", Command_Background,     1); // Diagnostic.h
    #endif
    #ifdef USES_C012
      COMMAND_CASE(  "blynkget", Command_Blynk_Get,     -1);
    #endif // ifdef USES_C012
    #ifdef USES_C015
      COMMAND_CASE(  "blynkset", Command_Blynk_Set,     -1);
    #endif // ifdef USES_C015
      COMMAND_CASE(     "build", Command_Settings_Build, 1); // Settings.h
      break;
    }
    case 'c': {
      COMMAND_CASE( "clearaccessblock", Command_AccessInfo_Clear,   0); // Network Command
      COMMAND_CASE(      "clearrtcram", Command_RTC_Clear,          0); // RTC.h
      COMMAND_CASE(           "config", Command_Task_RemoteConfig, -1); // Tasks.h
      COMMAND_CASE("controllerdisable", Command_Controller_Disable, 1); // Controller.h
      COMMAND_CASE( "controllerenable", Command_Controller_Enable,  1); // Controller.h

      break;
    }
    case 'd': {
      COMMAND_CASE( "datetime", Command_DateTime,         2);      // Time.h
      COMMAND_CASE(    "debug", Command_Debug,            1); // Diagnostic.h
      COMMAND_CASE("deepsleep", Command_System_deepSleep, 1); // System.h
      COMMAND_CASE(    "delay", Command_Delay,            1); // Timers.h
      COMMAND_CASE(      "dns", Command_DNS,              1); // Network Command
      COMMAND_CASE(      "dst", Command_DST,              1); // Time.h
      break;
    }
    case 'e': {
    #ifdef HAS_ETHERNET
      COMMAND_CASE(   "ethphyadr", Command_ETH_Phy_Addr,   1); // Network Command
      COMMAND_CASE(   "ethpinmdc", Command_ETH_Pin_mdc,    1); // Network Command
      COMMAND_CASE(  "ethpinmdio", Command_ETH_Pin_mdio,   1); // Network Command
      COMMAND_CASE( "ethpinpower", Command_ETH_Pin_power,  1); // Network Command
      COMMAND_CASE(  "ethphytype", Command_ETH_Phy_Type,   1); // Network Command
      COMMAND_CASE("ethclockmode", Command_ETH_Clock_Mode, 1); // Network Command
      COMMAND_CASE(       "ethip", Command_ETH_IP,         1); // Network Command
      COMMAND_CASE(  "ethgateway", Command_ETH_Gateway,    1); // Network Command
      COMMAND_CASE(   "ethsubnet", Command_ETH_Subnet,     1); // Network Command  
      COMMAND_CASE(      "ethdns", Command_ETH_DNS,        1); // Network Command
      COMMAND_CASE( "ethwifimode", Command_ETH_Wifi_Mode,  1); // Network Command
    #endif // HAS_ETHERNET
      COMMAND_CASE("erasesdkwifi", Command_WiFi_Erase,     0); // WiFi.h
      COMMAND_CASE(       "event", Command_Rules_Events,  -1); // Rule.h
      COMMAND_CASE("executerules", Command_Rules_Execute, -1); // Rule.h
      break;
    }
    case 'g': {
      COMMAND_CASE("gateway", Command_Gateway, 1); // Network Command
      break;
    }
    case 'i': {
      COMMAND_CASE("i2cscanner", Command_i2c_Scanner, -1); // i2c.h
      COMMAND_CASE(        "ip", Command_IP,           1); // Network Command
      break;
    }
    case 'j': {
      #ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
      COMMAND_CASE("jsonportstatus", Command_JSONPortStatus, -1); // Diagnostic.h
      #endif
      break;
    }
    case 'l': {
      COMMAND_CASE(            "let", Command_Rules_Let,         2);    // Rules.h
      COMMAND_CASE(           "load", Command_Settings_Load,     0);    // Settings.h
      COMMAND_CASE(       "logentry", Command_logentry,          1);    // Diagnostic.h
      COMMAND_CASE(   "looptimerset", Command_Loop_Timer_Set,    3); // Timers.h
      COMMAND_CASE("looptimerset_ms", Command_Loop_Timer_Set_ms, 3); // Timers.h

    #ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
      COMMAND_CASE("logportstatus", Command_logPortStatus, 0);    // Diagnostic.h
      COMMAND_CASE(       "lowmem", Command_Lowmem,        0);    // Diagnostic.h
    #endif
      break;
    }
    case 'm': {
    #ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
      COMMAND_CASE(        "malloc", Command_Malloc,            1); // Diagnostic.h
      COMMAND_CASE(       "meminfo", Command_MemInfo,           0); // Diagnostic.h
      COMMAND_CASE( "meminfodetail", Command_MemInfo_detail,    0); // Diagnostic.h
    #endif
      break;
    }
    case 'n': {
      COMMAND_CASE(   "name", Command_Settings_Name,        1); // Settings.h
      COMMAND_CASE("nosleep", Command_System_NoSleep,       1); // System.h
      COMMAND_CASE( "notify", Command_Notifications_Notify, 2); // Notifications.h
      COMMAND_CASE("ntphost", Command_NTPHost,              1); // Time.h
      break;
    }
    case 'p': {
      COMMAND_CASE("password", Command_Settings_Password, 1); // Settings.h
#ifdef USES_MQTT
      COMMAND_CASE( "publish", Command_MQTT_Publish,      2); // MQTT.h
#endif // USES_MQTT
      break;
    }
    case 'r': {
      COMMAND_CASE(                "reboot", Command_System_Reboot,              0); // System.h
      COMMAND_CASE(                 "reset", Command_Settings_Reset,             0); // Settings.h
      COMMAND_CASE("resetflashwritecounter", Command_RTC_resetFlashWriteCounter, 0); // RTC.h
      COMMAND_CASE(               "restart", Command_System_Restart,             0); // System.h
      COMMAND_CASE(                 "rules", Command_Rules_UseRules,             1); // Rule.h
      break;
    }
    case 's': {
      COMMAND_CASE(       "save", Command_Settings_Save,   0); // Settings.h
    #ifdef FEATURE_SD
      COMMAND_CASE(     "sdcard", Command_SD_LS,           0); // SDCARDS.h
      COMMAND_CASE(   "sdremove", Command_SD_Remove,       1); // SDCARDS.h
    #endif // ifdef FEATURE_SD
    if (cmd_lc[1] == 'e') {
      COMMAND_CASE(     "sendto", Command_UPD_SendTo,      2); // UDP.h    // FIXME TD-er: These send commands, can we determine the nr of
                                                               // arguments?
      COMMAND_CASE( "sendtohttp", Command_HTTP_SendToHTTP, 3); // HTTP.h
      COMMAND_CASE(  "sendtoudp", Command_UDP_SendToUPD,   3); // UDP.h
    #ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
      COMMAND_CASE("serialfloat", Command_SerialFloat,     0); // Diagnostic.h
    #endif
      COMMAND_CASE(   "settings", Command_Settings_Print,  0); // Settings.h
    }
      COMMAND_CASE(     "subnet", Command_Subnet,          1); // Network Command
    #ifdef USES_MQTT
      COMMAND_CASE(  "subscribe", Command_MQTT_Subscribe,  1);      // MQTT.h
    #endif // USES_MQTT
    #ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
      COMMAND_CASE(    "sysload", Command_SysLoad,         0); // Diagnostic.h
    #endif
      break;
    }
    case 't': {
    if (cmd_lc[1] == 'a') {
      COMMAND_CASE(         "taskclear", Command_Task_Clear,          1); // Tasks.h
      COMMAND_CASE(      "taskclearall", Command_Task_ClearAll,       0); // Tasks.h
      COMMAND_CASE(       "taskdisable", Command_Task_Disable,        1); // Tasks.h
      COMMAND_CASE(        "taskenable", Command_Task_Enable,         1); // Tasks.h
      COMMAND_CASE(           "taskrun", Command_Task_Run,            1); // Tasks.h
      COMMAND_CASE(      "taskvalueset", Command_Task_ValueSet,       3); // Tasks.h
      COMMAND_CASE(   "taskvaluetoggle", Command_Task_ValueToggle,    2); // Tasks.h
      COMMAND_CASE("taskvaluesetandrun", Command_Task_ValueSetAndRun, 3); // Tasks.h
    } else if (cmd_lc[1] == 'i') {
      COMMAND_CASE(        "timerpause", Command_Timer_Pause,         1); // Timers.h
      COMMAND_CASE(       "timerresume", Command_Timer_Resume,        1); // Timers.h
      COMMAND_CASE(          "timerset", Command_Timer_Set,           2); // Timers.h
      COMMAND_CASE(       "timerset_ms", Command_Timer_Set_ms,        2); // Timers.h
      COMMAND_CASE(          "timezone", Command_TimeZone,            1); // Time.h
    }
      break;
    }
    case 'u': {
      COMMAND_CASE("udpport", Command_UDP_Port,      1); // UDP.h
      COMMAND_CASE("udptest", Command_UDP_Test,      2); // UDP.h
      COMMAND_CASE(   "unit", Command_Settings_Unit, 1); // Settings.h
      COMMAND_CASE( "usentp", Command_useNTP,        1); // Time.h
      break;
    }
    case 'w': {
      COMMAND_CASE(      "wdconfig", Command_WD_Config,       3); // WD.h
      COMMAND_CASE(        "wdread", Command_WD_Read,         2); // WD.h
    if (cmd_lc[1] == 'i') {
      COMMAND_CASE(    "wifiapmode", Command_Wifi_APMode,     0); // WiFi.h
      COMMAND_CASE(   "wificonnect", Command_Wifi_Connect,    0); // WiFi.h
      COMMAND_CASE("wifidisconnect", Command_Wifi_Disconnect, 0); // WiFi.h
      COMMAND_CASE(       "wifikey", Command_Wifi_Key,        1); // WiFi.h
      COMMAND_CASE(      "wifikey2", Command_Wifi_Key2,       1); // WiFi.h
      COMMAND_CASE(      "wifimode", Command_Wifi_Mode,       1); // WiFi.h
      COMMAND_CASE(      "wifiscan", Command_Wifi_Scan,       0); // WiFi.h
      COMMAND_CASE(      "wifissid", Command_Wifi_SSID,       1); // WiFi.h
      COMMAND_CASE(     "wifissid2", Command_Wifi_SSID2,      1); // WiFi.h
      COMMAND_CASE(   "wifistamode", Command_Wifi_STAMode,    0); // WiFi.h
    }
      break;
    }
    default:
      break;
  }

  #undef COMMAND_CASE
  return false;
}



// Execute command which may be plugin or internal commands
bool ExecuteCommand_all(EventValueSource::Enum source, const char *Line)
{
  return ExecuteCommand(INVALID_TASK_INDEX, source, Line, true, true, false);
}

bool ExecuteCommand_all_config(EventValueSource::Enum source, const char *Line)
{
  return ExecuteCommand(INVALID_TASK_INDEX, source, Line, true, true, true);
}

bool ExecuteCommand_plugin_config(EventValueSource::Enum source, const char *Line)
{
  return ExecuteCommand(INVALID_TASK_INDEX, source, Line, true, false, true);
}

bool ExecuteCommand_all_config_eventOnly(EventValueSource::Enum source, const char *Line)
{
  bool tryInternal = false;
  {
    String cmd;
    if (GetArgv(Line, cmd, 1)) {
      tryInternal = cmd.equalsIgnoreCase(F("event"));
    }
  }
  return ExecuteCommand(INVALID_TASK_INDEX, source, Line, true, tryInternal, true);
}

bool ExecuteCommand_internal(EventValueSource::Enum source, const char *Line)
{
  return ExecuteCommand(INVALID_TASK_INDEX, source, Line, false, true, false);
}

bool ExecuteCommand_plugin(EventValueSource::Enum source, const char *Line)
{
  return ExecuteCommand(INVALID_TASK_INDEX, source, Line, true, false, false);
}

bool ExecuteCommand_plugin(taskIndex_t taskIndex, EventValueSource::Enum source, const char *Line)
{
  return ExecuteCommand(taskIndex, source, Line, true, false, false);
}

bool ExecuteCommand(taskIndex_t taskIndex, EventValueSource::Enum source, const char *Line, bool tryPlugin, bool tryInternal, bool tryRemoteConfig)
{
  checkRAM(F("ExecuteCommand"));
  String cmd;

  if (!GetArgv(Line, cmd, 1)) {
    SendStatus(source, return_command_failed());
    return false;
  }

  if (tryInternal) {
    // Small optimization for events, which happen frequently
    // FIXME TD-er: Make quick check to see if a command is an internal command, so we don't need to try all
    if (cmd.equalsIgnoreCase(F("event"))) {
      tryPlugin = false;
      tryRemoteConfig = false;      
    }
  }

  // FIXME TD-er: Not sure what happens now, but TaskIndex cannot always be set here
  // since commands can originate from anywhere.
  struct EventStruct TempEvent;
  TempEvent.setTaskIndex(taskIndex);
  checkDeviceVTypeForTask(&TempEvent);
  TempEvent.Source = source;

  String action(Line);
  action = parseTemplate(action); // parseTemplate before executing the command

  // Split the arguments into Par1...5 of the event.
  // Do not split it in executeInternalCommand, since that one will be called from the scheduler with pre-set events.
  // FIXME TD-er: Why call this for all commands? The CalculateParam function is quite heavy.
  parseCommandString(&TempEvent, action);

  // FIXME TD-er: This part seems a bit strange.
  // It can't schedule a call to PLUGIN_WRITE.
  // Maybe ExecuteCommand can be scheduled?
  delay(0);

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("Command: ");
    log += cmd;
    addLog(LOG_LEVEL_DEBUG, log);
#ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_DEBUG, Line); // for debug purposes add the whole line.
    String parameters;
    parameters.reserve(64);
    parameters += F("Par1: ");
    parameters += TempEvent.Par1;
    parameters += F(" Par2: ");
    parameters += TempEvent.Par2;
    parameters += F(" Par3: ");
    parameters += TempEvent.Par3;
    parameters += F(" Par4: ");
    parameters += TempEvent.Par4;
    parameters += F(" Par5: ");
    parameters += TempEvent.Par5;
    addLog(LOG_LEVEL_DEBUG, parameters);
#endif // ifndef BUILD_NO_DEBUG
  }


  if (tryInternal) {
    String status;
    bool handled = executeInternalCommand(cmd.c_str(), &TempEvent, action.c_str(), status);
    if (status.length() > 0) {
      delay(0);
      SendStatus(source, status);
      delay(0);
    }
    if (handled) {
      return true;
    }
  }
  if (tryPlugin) {
    // Use a tmp string to call PLUGIN_WRITE, since PluginCall may inadvertenly
    // alter the string.
    String tmpAction(action);
    bool handled = PluginCall(PLUGIN_WRITE, &TempEvent, tmpAction);
    if (!tmpAction.equals(action)) {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        String log = F("PLUGIN_WRITE altered the string: ");
        log += action;
        log += F(" to: ");
        log += tmpAction;
        addLog(LOG_LEVEL_ERROR, log);
      }
    }
    if (handled) {
      SendStatus(source, return_command_success());
      return true;
    }
  }
  if (tryRemoteConfig) {
    if (remoteConfig(&TempEvent, action)) {
      SendStatus(source, return_command_success());
      return true;
    }
  }
  String errorUnknown = F("Command unknown: \"");
  errorUnknown += action;
  errorUnknown += '\"';
  addLog(LOG_LEVEL_INFO, errorUnknown);
  SendStatus(source, errorUnknown);
  delay(0);
  return false;
}