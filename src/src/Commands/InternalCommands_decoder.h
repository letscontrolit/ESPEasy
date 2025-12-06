#ifndef COMMANDS_INTERNALCOMMANDS_DECODER_H
#define COMMANDS_INTERNALCOMMANDS_DECODER_H

#include "../../ESPEasy_common.h"


enum class ESPEasy_cmd_e : uint8_t {
  accessinfo,
  asyncevent,
  build,
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
  background,
#endif // ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
#ifdef USES_C012
  blynkget,
#endif // #ifdef USES_C012
#ifdef USES_C015
  blynkset,
#endif // #ifdef USES_C015


  clearaccessblock,
  clearpassword,
  clearrtcram,
#ifdef ESP8266
  clearsdkwifi,
  clearwifirfcal,
#endif // #ifdef ESP8266
  config,
  controllerdisable,
  controllerenable,

  datetime,
  debug,
  dec,
  deepsleep,
  delay,
#if FEATURE_PLUGIN_PRIORITY
  disableprioritytask,
#endif // #if FEATURE_PLUGIN_PRIORITY
  dns,
  dst,

  erasesdkwifi,
  event,
  executerules,
#if FEATURE_ETHERNET
  ethphyadr,
  ethpinmdc,
  ethpinmdio,
  ethpinpower,
  ethphytype,
  ethclockmode,
  ethip,
  ethgateway,
  ethsubnet,
  ethdns,
  ethdisconnect,
  ethwifimode,
#endif // FEATURE_ETHERNET

  factoryreset,

  gateway,
  gpio,
  gpiotoggle,
  hiddenssid,

  i2cscanner,
  inc,
  ip,
#if FEATURE_USE_IPV6
  ip6,
#endif
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
  jsonportstatus,
#endif // ifndef BUILD_NO_DIAGNOSTIC_COMMANDS

  #if FEATURE_LAT_LONG_VAR_CMD
  latitude,
  #endif // if FEATURE_LAT_LONG_VAR_CMD
  let,
  #if FEATURE_STRING_VARIABLES
  letstr,
  #endif // if FEATURE_STRING_VARIABLES
  load,
  logentry,
  #if FEATURE_LAT_LONG_VAR_CMD
  longitude,
  #endif // if FEATURE_LAT_LONG_VAR_CMD
  looptimerset,
  looptimerset_ms,
  looptimersetandrun,
  looptimersetandrun_ms,
  longpulse,
  longpulse_ms,
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
  logportstatus,
  lowmem,
#endif // ifndef BUILD_NO_DIAGNOSTIC_COMMANDS

  monitor,
  monitorrange,
#ifdef USES_P009
  mcpgpio,
  mcpgpiorange,
  mcpgpiopattern,
  mcpgpiotoggle,
  mcplongpulse,
  mcplongpulse_ms,
  mcpmode,
  mcpmoderange,
  mcppulse,
#endif // #ifdef USES_P009
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
  malloc,
  meminfo,
  meminfodetail,
#endif // ifndef BUILD_NO_DIAGNOSTIC_COMMANDS

  name,
  nosleep,
#if FEATURE_NOTIFIER
  notify,
#endif // #if FEATURE_NOTIFIER
  ntphost,

#if FEATURE_DALLAS_HELPER && FEATURE_COMMAND_OWSCAN
  owscan,
#endif // if FEATURE_DALLAS_HELPER && FEATURE_COMMAND_OWSCAN

  password,
#ifdef USES_P019
  pcfgpio,
  pcfgpiorange,
  pcfgpiopattern,
  pcfgpiotoggle,
  pcflongpulse,
  pcflongpulse_ms,
  pcfmode,
  pcfmoderange,
  pcfpulse,
#endif // #ifdef USES_P019
#if FEATURE_POST_TO_HTTP
  posttohttp,
#if FEATURE_HTTP_TLS
  posttohttps,
#endif // if FEATURE_HTTP_TLS
#endif // #if FEATURE_POST_TO_HTTP
#if FEATURE_CUSTOM_PROVISIONING
  provision,
# ifdef PLUGIN_BUILD_MAX_ESP32 // FIXME DEPRECATED: Fallback for temporary backward compatibility
  provisionconfig,
  provisionsecurity,
#  if FEATURE_NOTIFIER
  provisionnotification,
#  endif // #if FEATURE_NOTIFIER
  provisionprovision,
  provisionrules,
  provisionfirmware,
# endif // #ifdef PLUGIN_BUILD_MAX_ESP32
#endif  // #if FEATURE_CUSTOM_PROVISIONING
  pulse,
#if FEATURE_MQTT
  publish,
  publishr,
#endif // #if FEATURE_MQTT
#if FEATURE_PUT_TO_HTTP
  puttohttp,
#if FEATURE_HTTP_TLS
  puttohttps,
#endif // if FEATURE_HTTP_TLS
#endif // #if FEATURE_PUT_TO_HTTP
  pwm,

  reboot,
  resetflashwritecounter,
  restart,
  rtttl,
  rules,

  save,
  scheduletaskrun,
#if FEATURE_SD
  sdcard,
  sdremove,
#endif // #if FEATURE_SD
#if FEATURE_ESPEASY_P2P
  sendto,
#endif // #if FEATURE_ESPEASY_P2P
#if FEATURE_SEND_TO_HTTP
  sendtohttp,
#if FEATURE_HTTP_TLS
  sendtohttps,
#endif // if FEATURE_HTTP_TLS
#endif // FEATURE_SEND_TO_HTTP
  sendtoudp,
  sendtoudpmix,
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
  serialfloat,
#endif // ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
  settings,
#if FEATURE_SERVO
  servo,
#endif // #if FEATURE_SERVO
  status,
  subnet,
#if FEATURE_MQTT
  subscribe,
#endif // #if FEATURE_MQTT
#ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
  sysload,
#endif // ifndef BUILD_NO_DIAGNOSTIC_COMMANDS

  taskclear,
  taskclearall,
  taskdisable,
  taskenable,
  taskrun,
  taskrunat,
  taskvalueset,
  #if FEATURE_STRING_VARIABLES
  taskvaluesetderived,
  taskvaluesetpresentation,
  #endif // if FEATURE_STRING_VARIABLES
  taskvaluetoggle,
  taskvaluesetandrun,
  timerpause,
  timerresume,
  timerset,
  timerset_ms,
  timezone,
  tone,

  udpport,
#if FEATURE_ESPEASY_P2P
  udptest,
#endif // #if FEATURE_ESPEASY_P2P
  unit,
  unmonitor,
  unmonitorrange,
  usentp,

  wifiallowap,
  wifiapmode,
  wificonnect,
  wifidisconnect,
  wifikey,
  wifikey2,
  wifimode,
  wifiscan,
  wifissid,
  wifissid2,
  wifistamode,
#ifndef LIMIT_BUILD_SIZE
  wdconfig,
  wdread,
#endif // ifndef LIMIT_BUILD_SIZE


  NotMatched  // Keep as last one
};


ESPEasy_cmd_e match_ESPEasy_internal_command(const String& cmd);

#ifndef BUILD_NO_DEBUG
bool toString(ESPEasy_cmd_e cmd, String& str);

// Added for checking at runtime to see if all commands will be matched
bool checkAll_internalCommands();
#endif

#endif // ifndef COMMANDS_INTERNALCOMMANDS_DECODER_H
