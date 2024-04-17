#include "../Commands/InternalCommands_decoder.h"

#include "../DataStructs/TimingStats.h"
#include "../Helpers/StringConverter.h"

// Keep the order of elements in ESPEasy_cmd_e enum
// the same as in the PROGMEM strings below
//
// The first item in the PROGMEM strings below should be an enum
// which is always included in each build as it is used to set an offset
// to compute the final enum value.
//
// Keep the offset used in match_ESPEasy_internal_command in sync
// when adding new commands


const char Internal_commands_ab[] PROGMEM =
  "accessinfo|"
  "asyncevent|"
  "build|"
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
  "background|"
#endif // ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
#ifdef USES_C012
  "blynkget|"
#endif // #ifdef USES_C012
#ifdef USES_C015
  "blynkset|"
#endif // #ifdef USES_C015
;

#define Int_cmd_c_offset ESPEasy_cmd_e::clearaccessblock
const char Internal_commands_c[] PROGMEM =
  "clearaccessblock|"
  "clearpassword|"
  "clearrtcram|"
#ifdef ESP8266
  "clearsdkwifi|"
  "clearwifirfcal|"
#endif // #ifdef ESP8266
  "config|"
  "controllerdisable|"
  "controllerenable|"
;

#define Int_cmd_d_offset ESPEasy_cmd_e::datetime
const char Internal_commands_d[] PROGMEM =
  "datetime|"
  "debug|"
  "dec|"
  "deepsleep|"
  "delay|"
#if FEATURE_PLUGIN_PRIORITY
  "disableprioritytask|"
#endif // #if FEATURE_PLUGIN_PRIORITY
  "dns|"
  "dst|"
;

#define Int_cmd_e_offset ESPEasy_cmd_e::erasesdkwifi
const char Internal_commands_e[] PROGMEM =
  "erasesdkwifi|"
  "event|"
  "executerules|"
#if FEATURE_ETHERNET
  "ethphyadr|"
  "ethpinmdc|"
  "ethpinmdio|"
  "ethpinpower|"
  "ethphytype|"
  "ethclockmode|"
  "ethip|"
  "ethgateway|"
  "ethsubnet|"
  "ethdns|"
  "ethdisconnect|"
  "ethwifimode|"
#endif // FEATURE_ETHERNET
;

#define Int_cmd_ghij_offset ESPEasy_cmd_e::gateway
const char Internal_commands_ghij[] PROGMEM =
  "gateway|"
  "gpio|"
  "gpiotoggle|"
  "hiddenssid|"
  "i2cscanner|"
  "inc|"
  "ip|"
#if FEATURE_USE_IPV6
  "ip6|"
#endif
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
  "jsonportstatus|"
#endif // ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
;

#define Int_cmd_l_offset ESPEasy_cmd_e::let
const char Internal_commands_l[] PROGMEM =
  "let|"
  "load|"
  "logentry|"
  "looptimerset|"
  "looptimerset_ms|"
  "longpulse|"
  "longpulse_ms|"
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
  "logportstatus|"
  "lowmem|"
#endif // ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
;

#define Int_cmd_m_offset ESPEasy_cmd_e::monitor
const char Internal_commands_m[] PROGMEM =
  "monitor|"
  "monitorrange|"
#ifdef USES_P009
  "mcpgpio|"
  "mcpgpiorange|"
  "mcpgpiopattern|"
  "mcpgpiotoggle|"
  "mcplongpulse|"
  "mcplongpulse_ms|"
  "mcpmode|"
  "mcpmoderange|"
  "mcppulse|"
#endif // #ifdef USES_P009
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
  "malloc|"
  "meminfo|"
  "meminfodetail|"
#endif // ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
;

#define Int_cmd_n_offset ESPEasy_cmd_e::name
const char Internal_commands_n[] PROGMEM =
  "name|"
  "nosleep|"
#if FEATURE_NOTIFIER
  "notify|"
#endif // #if FEATURE_NOTIFIER
  "ntphost|"
;

#define Int_cmd_p_offset ESPEasy_cmd_e::password
const char Internal_commands_p[] PROGMEM =
  "password|"
#ifdef USES_P019
  "pcfgpio|"
  "pcfgpiorange|"
  "pcfgpiopattern|"
  "pcfgpiotoggle|"
  "pcflongpulse|"
  "pcflongpulse_ms|"
  "pcfmode|"
  "pcfmoderange|"
  "pcfpulse|"
#endif // #ifdef USES_P019
#if FEATURE_POST_TO_HTTP
  "posttohttp|"
#endif // #if FEATURE_POST_TO_HTTP
#if FEATURE_CUSTOM_PROVISIONING
  "provision|"
 # ifdef PLUGIN_BUILD_MAX_ESP32 // FIXME DEPRECATED: Fallback for temporary backward compatibility
  "provisionconfig|"
  "provisionsecurity|"
  #  if FEATURE_NOTIFIER
  "provisionnotification|"
  #  endif // #if FEATURE_NOTIFIER
  "provisionprovision|"
  "provisionrules|"
  "provisionfirmware|"
 # endif // #ifdef PLUGIN_BUILD_MAX_ESP32
#endif   // #if FEATURE_CUSTOM_PROVISIONING
  "pulse|"
#if FEATURE_MQTT
  "publish|"
  "publishr|"
#endif // #if FEATURE_MQTT
#if FEATURE_PUT_TO_HTTP
  "puttohttp|"
#endif // #if FEATURE_PUT_TO_HTTP
  "pwm|"
;

#define Int_cmd_r_offset ESPEasy_cmd_e::reboot
const char Internal_commands_r[] PROGMEM =
  "reboot|"
  "reset|"
  "resetflashwritecounter|"
  "restart|"
  "rtttl|"
  "rules|"
;

#define Int_cmd_s_offset ESPEasy_cmd_e::save
const char Internal_commands_s[] PROGMEM =
  "save|"
  "scheduletaskrun|"
#if FEATURE_SD
  "sdcard|"
  "sdremove|"
#endif // #if FEATURE_SD
#if FEATURE_ESPEASY_P2P
  "sendto|"
#endif // #if FEATURE_ESPEASY_P2P
#if FEATURE_SEND_TO_HTTP
  "sendtohttp|"
#endif // FEATURE_SEND_TO_HTTP
  "sendtoudp|"
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
  "serialfloat|"
#endif // ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
  "settings|"
#if FEATURE_SERVO
  "servo|"
#endif // #if FEATURE_SERVO
  "status|"
  "subnet|"
#if FEATURE_MQTT
  "subscribe|"
#endif // #if FEATURE_MQTT
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
  "sysload|"
#endif // ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
;

#define Int_cmd_t_offset ESPEasy_cmd_e::taskclear
const char Internal_commands_t[] PROGMEM =
  "taskclear|"
  "taskclearall|"
  "taskdisable|"
  "taskenable|"
  "taskrun|"
  "taskrunat|"
  "taskvalueset|"
  "taskvaluetoggle|"
  "taskvaluesetandrun|"
  "timerpause|"
  "timerresume|"
  "timerset|"
  "timerset_ms|"
  "timezone|"
  "tone|"
;

#define Int_cmd_u_offset ESPEasy_cmd_e::udpport
const char Internal_commands_u[] PROGMEM =
  "udpport|"
#if FEATURE_ESPEASY_P2P
  "udptest|"
#endif // #if FEATURE_ESPEASY_P2P
  "unit|"
  "unmonitor|"
  "unmonitorrange|"
  "usentp|"
;

#define Int_cmd_w_offset ESPEasy_cmd_e::wifiallowap
const char Internal_commands_w[] PROGMEM =
  "wifiallowap|"
  "wifiapmode|"
  "wificonnect|"
  "wifidisconnect|"
  "wifikey|"
  "wifikey2|"
  "wifimode|"
  "wifiscan|"
  "wifissid|"
  "wifissid2|"
  "wifistamode|"
#ifndef LIMIT_BUILD_SIZE
  "wdconfig|"
  "wdread|"
#endif // ifndef LIMIT_BUILD_SIZE
;

const char* getInternalCommand_Haystack_Offset(const char firstLetter, int& offset)
{
  const char *haystack = nullptr;

  offset = static_cast<int>(ESPEasy_cmd_e::NotMatched);

  // Keep the offset in sync when adding new commands
  switch (firstLetter)
  {
    case 'a':
    case 'b':
      offset   = 0;
      haystack = Internal_commands_ab;
      break;
    case 'c':
      offset   = static_cast<int>(Int_cmd_c_offset);
      haystack = Internal_commands_c;
      break;
    case 'd':
      offset   = static_cast<int>(Int_cmd_d_offset);
      haystack = Internal_commands_d;
      break;
    case 'e':
      offset   = static_cast<int>(Int_cmd_e_offset);
      haystack = Internal_commands_e;
      break;
    case 'g':
    case 'h':
    case 'i':
    case 'j':
      offset   = static_cast<int>(Int_cmd_ghij_offset);
      haystack = Internal_commands_ghij;
      break;
    case 'l':
      offset   = static_cast<int>(Int_cmd_l_offset);
      haystack = Internal_commands_l;
      break;
    case 'm':
      offset   = static_cast<int>(Int_cmd_m_offset);
      haystack = Internal_commands_m;
      break;
    case 'n':
      offset   = static_cast<int>(Int_cmd_n_offset);
      haystack = Internal_commands_n;
      break;
    case 'p':
      offset   = static_cast<int>(Int_cmd_p_offset);
      haystack = Internal_commands_p;
      break;
    case 'r':
      offset   = static_cast<int>(Int_cmd_r_offset);
      haystack = Internal_commands_r;
      break;
    case 's':
      offset   = static_cast<int>(Int_cmd_s_offset);
      haystack = Internal_commands_s;
      break;
    case 't':
      offset   = static_cast<int>(Int_cmd_t_offset);
      haystack = Internal_commands_t;
      break;
    case 'u':
      offset   = static_cast<int>(Int_cmd_u_offset);
      haystack = Internal_commands_u;
      break;
    case 'w':
      offset   = static_cast<int>(Int_cmd_w_offset);
      haystack = Internal_commands_w;
      break;

    default:
      return nullptr;
  }
  return haystack;
}

ESPEasy_cmd_e match_ESPEasy_internal_command(const String& cmd)
{
  START_TIMER;
  ESPEasy_cmd_e res = ESPEasy_cmd_e::NotMatched;

  if (cmd.length() < 2) {
    // No commands less than 2 characters
    return res;
  }

  int offset           = 0;
  const char *haystack = getInternalCommand_Haystack_Offset(cmd[0], offset);

  if (haystack == nullptr) {
/*
    addLog(LOG_LEVEL_ERROR, strformat(
             F("Internal command: No Haystack/offset '%s', offset: %d"),
             cmd.c_str(),
             offset));
*/
    return res;
  }


  if (haystack != nullptr) {
    const int command_i = GetCommandCode(cmd.c_str(), haystack);

    if (command_i != -1) {
      res = static_cast<ESPEasy_cmd_e>(command_i + offset);
    }
/*
    else {
      addLog(LOG_LEVEL_ERROR, strformat(
               F("Internal command: Not found '%s', haystack: %s"),
               cmd.c_str(),
               String(haystack).c_str()));
    }
*/
  }
  STOP_TIMER(COMMAND_DECODE_INTERNAL);
  return res;
}

#ifndef BUILD_NO_DEBUG
bool toString(ESPEasy_cmd_e cmd, String& str)
{
  if (cmd == ESPEasy_cmd_e::NotMatched) {
    return false;
  }
  char c     = 'z';
  bool found = false;
  int  offset;
  const char *haystack = nullptr;

  while (!found && c >= 'a') {
    haystack = getInternalCommand_Haystack_Offset(c, offset);

    if (haystack != nullptr) {
      if (offset <= static_cast<int>(cmd)) {
        found = true;
      }
    }
    --c;
  }

  if (found) {
    const int index = static_cast<int>(cmd) - offset;
/*
    addLog(LOG_LEVEL_INFO, strformat(
             F("Internal command: cmd=%d offset=%d index=%d"),
             static_cast<int>(cmd),
             offset,
             index));
*/

    if ((index >= 0) && (haystack != nullptr)) {
      // Likely long enough to parse any command
      char temp[32]{};
      str = GetTextIndexed(temp, sizeof(temp), index, haystack);
      return !str.isEmpty();
    }
  }
  return false;
}

bool checkAll_internalCommands()
{
  constexpr int last = static_cast<int>(ESPEasy_cmd_e::NotMatched);
  bool no_error      = true;

  for (int i = 0; i < last; ++i) {
    const ESPEasy_cmd_e cmd = static_cast<ESPEasy_cmd_e>(i);
    String cmd_str;

    if (!toString(cmd, cmd_str)) {
      no_error = false;
//      addLog(LOG_LEVEL_ERROR, concat(F("Internal command: no matching string for "), i));
    } else {
      const ESPEasy_cmd_e cmd_found = match_ESPEasy_internal_command(cmd_str);

      if (cmd_found != cmd) {
        if (cmd_str.isEmpty()) {
          addLog(LOG_LEVEL_ERROR, strformat(
                   F("Internal command: mismatch (%d)"), i));
        }
        else {
          addLog(LOG_LEVEL_ERROR, strformat(
                   F("Internal command: mismatch '%s' (%d)"),
                   cmd_str.c_str(),
                   i));
        }
        no_error = false;
      }
    }
  }

  if (no_error) {
    addLog(LOG_LEVEL_INFO, F("Internal command: All checked OK"));
  } 
/*
  else {
    const int index = static_cast<int>(match_ESPEasy_internal_command(F("build")));
    addLog(LOG_LEVEL_ERROR, concat(F("Internal command: index 'build'="), index));
  }
*/
  return no_error;
}

#endif // ifndef BUILD_NO_DEBUG
