
#include "Commands/Common.h"
#include "Commands/Blynk.h"
#include "Commands/Diagnostic.h"
#include "Commands/HTTP.h"
#include "Commands/i2c.h"
#include "Commands/MQTT.h"
#include "Commands/Networks.h"
#include "Commands/Notifications.h"
#include "Commands/RTC.h"
#include "Commands/Rules.h"
#include "Commands/SDCARD.h"
#include "Commands/Settings.h"
#include "Commands/System.h"
#include "Commands/Tasks.h"
#include "Commands/Time.h"
#include "Commands/Timer.h"
#include "Commands/UPD.h"
#include "Commands/wd.h"
#include "Commands/WiFi.h"


/*********************************************************************************************\
 * Registers command
\*********************************************************************************************/
bool doExecuteCommand(const char * cmd, struct EventStruct *event, const char* line) {
  // Simple macro to match command to function call.
  #define COMMAND_CASE(S,C) if (strcmp_P(cmd_lc, PSTR(S)) == 0) return (C(event,line));

  String tmpcmd;
  tmpcmd = cmd;
  tmpcmd.toLowerCase();
  String log = F("Command: ");
  log += tmpcmd;
  addLog(LOG_LEVEL_INFO, log);
  char cmd_lc[INPUT_COMMAND_SIZE];
  tmpcmd.toCharArray(cmd_lc, tmpcmd.length() + 1);
  switch (cmd_lc[0]) {
    case 'a': {
	  COMMAND_CASE("accessinfo"             , Command_AccessInfo_Ls);              // Network Command
      break;
    }
    case 'b': {
	  COMMAND_CASE("background"             , Command_Background);                 // Diagnostic.h
    #ifdef CPLUGIN_012
	  COMMAND_CASE("blynkget"               , Command_Blynk_Get);
    #endif
	  COMMAND_CASE("build"                  , Command_Settings_Build);             // Settings.h
      break;
    }
    case 'c': {
	  COMMAND_CASE("clearaccessblock"       , Command_AccessInfo_Clear);           // Network Command
	  COMMAND_CASE("clearrtcram"            , Command_RTC_Clear);                  // RTC.h
	  COMMAND_CASE("config"                 , Command_Task_RemoteConfig);          // Tasks.h
      break;
    }
    case 'd': {
	  COMMAND_CASE("debug"                  , Command_Debug);                      // Diagnostic.h
	  COMMAND_CASE("deepsleep"              , Command_System_deepSleep);           // System.h
	  COMMAND_CASE("delay"                  , Command_Delay);                      // Timers.h
	  COMMAND_CASE("dns"                    , Command_DNS);                        // Network Command
	  COMMAND_CASE("dst"                    , Command_DST);                        // Time.h
      break;
    }
    case 'e': {
	  COMMAND_CASE("erase"                  , Command_WiFi_Erase);                 // WiFi.h
	  COMMAND_CASE("event"                  , Command_Rules_Events);               // Rule.h
	  COMMAND_CASE("executerules"           , Command_Rules_Execute);              // Rule.h
      break;
    }
    case 'g': {
	  COMMAND_CASE("gateway"                , Command_Gateway);                    // Network Command
      break;
    }
    case 'i': {
	  COMMAND_CASE("i2cscanner"             , Command_i2c_Scanner);                // i2c.h
	  COMMAND_CASE("ip"                     , Command_IP);                         // Network Command
      break;
    }
    case 'l': {
	  COMMAND_CASE("load"                   , Command_Settings_Load);              // Settings.h
	  COMMAND_CASE("logentry"               , Command_logentry);                   // Diagnostic.h
	  COMMAND_CASE("lowmem"                 , Command_Lowmem);                     // Diagnostic.h
      break;
    }
    case 'm': {
	  COMMAND_CASE("malloc"                 , Command_Malloc);                     // Diagnostic.h
	  COMMAND_CASE("meminfo"                , Command_MemInfo);                    // Diagnostic.h
    COMMAND_CASE("meminfodetail"          , Command_MemInfo_detail);             // Diagnostic.h
	  COMMAND_CASE("messagedelay"           , Command_MQTT_messageDelay);          // MQTT.h
	  COMMAND_CASE("mqttretainflag"         , Command_MQTT_Retain);                // MQTT.h
      break;
    }
    case 'n': {
	  COMMAND_CASE("name"                   , Command_Settings_Name);              // Settings.h
	  COMMAND_CASE("nosleep"                , Command_System_NoSleep);             // System.h
	  COMMAND_CASE("notify"                 , Command_Notifications_Notify);       // Notifications.h
	  COMMAND_CASE("ntphost"                , Command_NTPHost);                    // Time.h
      break;
    }
    case 'p': {
	  COMMAND_CASE("password"               , Command_Settings_Password);          // Settings.h
	  COMMAND_CASE("publish"                , Command_MQTT_Publish);               // MQTT.h
      break;
    }
    case 'r': {
	  COMMAND_CASE("reboot"                 , Command_System_Reboot);              // System.h
	  COMMAND_CASE("reset"                  , Command_Settings_Reset);             // Settings.h
	  COMMAND_CASE("resetflashwritecounter" , Command_RTC_resetFlashWriteCounter); // RTC.h
	  COMMAND_CASE("restart"                , Command_System_Restart);             // System.h
	  COMMAND_CASE("rules"                  , Command_Rules_UseRules);             // Rule.h
      break;
    }
    case 's': {
	  COMMAND_CASE("save"                   , Command_Settings_Save);              // Settings.h
	#if FEATURE_SD
	  COMMAND_CASE("sdcard"                 , Command_SD_LS);                      // SDCARDS.h
	  COMMAND_CASE("sdremove"               , Command_SD_Remove);                  // SDCARDS.h
	#endif
	  COMMAND_CASE("sendto"                 , Command_UPD_SendTo);                 // UDP.h
	  COMMAND_CASE("sendtohttp"             , Command_HTTP_SendToHTTP);            // HTTP.h
	  COMMAND_CASE("sendtoudp"              , Command_UDP_SendToUPD);              // UDP.h
	  COMMAND_CASE("serialfloat"            , Command_SerialFloat);                // Diagnostic.h
	  COMMAND_CASE("settings"               , Command_Settings_Print);             // Settings.h
	  COMMAND_CASE("subnet"                 , Command_Subnet);                     // Network Command
	  COMMAND_CASE("sysload"                , Command_SysLoad);                    // Diagnostic.h
      break;
    }
    case 't': {
	  COMMAND_CASE("taskclear"              , Command_Task_Clear);                 // Tasks.h
	  COMMAND_CASE("taskclearall"           , Command_Task_ClearAll);              // Tasks.h
	  COMMAND_CASE("taskrun"                , Command_Task_Run);                   // Tasks.h
	  COMMAND_CASE("taskvalueset"           , Command_Task_ValueSet);              // Tasks.h
	  COMMAND_CASE("taskvaluesetandrun"     , Command_Task_ValueSetAndRun);        // Tasks.h
	  COMMAND_CASE("timerpause"             , Command_Timer_Pause);                // Timers.h
	  COMMAND_CASE("timerresume"            , Command_Timer_Resume);               // Timers.h
	  COMMAND_CASE("timerset"               , Command_Timer_Set);                  // Timers.h
	  COMMAND_CASE("timezone"               , Command_TimeZone);                   // Time.h
      break;
    }
    case 'u': {
	  COMMAND_CASE("udpport"                , Command_UDP_Port);                   // UDP.h
	  COMMAND_CASE("udptest"                , Command_UDP_Test);                   // UDP.h
	  COMMAND_CASE("unit"                   , Command_Settings_Unit);              // Settings.h
	  COMMAND_CASE("usentp"                 , Command_useNTP);                     // Time.h
      break;
    }
    case 'w': {
	  COMMAND_CASE("wdconfig"               , Command_WD_Config);                  // WD.h
	  COMMAND_CASE("wdread"                 , Command_WD_Read);                    // WD.h
	  COMMAND_CASE("wifiapmode"             , Command_Wifi_APMode);                // WiFi.h
	  COMMAND_CASE("wificonnect"            , Command_Wifi_Connect);               // WiFi.h
	  COMMAND_CASE("wifidisconnect"         , Command_Wifi_Disconnect);            // WiFi.h
	  COMMAND_CASE("wifikey"                , Command_Wifi_Key);                   // WiFi.h
	  COMMAND_CASE("wifikey2"               , Command_Wifi_Key2);                  // WiFi.h
	  COMMAND_CASE("wifimode"               , Command_Wifi_Mode);                  // WiFi.h
	  COMMAND_CASE("wifiscan"               , Command_Wifi_Scan);                  // WiFi.h
	  COMMAND_CASE("wifissid"               , Command_Wifi_SSID);                  // WiFi.h
	  COMMAND_CASE("wifissid2"              , Command_Wifi_SSID2);                 // WiFi.h
	  COMMAND_CASE("wifistamode"            , Command_Wifi_STAMode);               // WiFi.h
      break;
    }
    default:
      break;
  }
  String errorUnknown = F("Command unknown: \"");
  errorUnknown += cmd_lc;
  errorUnknown += '\"';
  addLog(LOG_LEVEL_INFO, errorUnknown);
  return false;

  #undef COMMAND_CASE
}

void ExecuteCommand(byte source, const char *Line)
{
  checkRAM(F("ExecuteCommand"));
  String status = "";
  boolean success = false;
  char TmpStr1[INPUT_COMMAND_SIZE];
  TmpStr1[0] = 0;
  char cmd[INPUT_COMMAND_SIZE];
  cmd[0] = 0;
  struct EventStruct TempEvent;
  // FIXME TD-er: Not sure what happens now, but TaskIndex cannot be set here
  // since commands can originate from anywhere.
  TempEvent.Source = source;
  GetArgv(Line, cmd, 1);
  if (GetArgv(Line, TmpStr1, 2)) TempEvent.Par1 = str2int(TmpStr1);
  if (GetArgv(Line, TmpStr1, 3)) TempEvent.Par2 = str2int(TmpStr1);
  if (GetArgv(Line, TmpStr1, 4)) TempEvent.Par3 = str2int(TmpStr1);
  if (GetArgv(Line, TmpStr1, 5)) TempEvent.Par4 = str2int(TmpStr1);
  if (GetArgv(Line, TmpStr1, 6)) TempEvent.Par5 = str2int(TmpStr1);

  success = doExecuteCommand((char*)&cmd[0], &TempEvent, Line);
  yield();

  if (success)
    status += F("\nOk");
  else
    status += F("\nUnknown command!");
  SendStatus(source, status);
  yield();
}

#ifdef FEATURE_SD
void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
#endif
