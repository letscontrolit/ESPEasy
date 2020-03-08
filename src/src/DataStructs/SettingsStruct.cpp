#include "../../ESPEasy_common.h"
#include "../DataStructs/SettingsStruct.h"
#include "../DataStructs/ESPEasyLimits.h"
#include "../Globals/Plugins.h"
#include "../Globals/CPlugins.h"

template<unsigned int N_TASKS>
SettingsStruct_tmpl<N_TASKS>::SettingsStruct_tmpl() : ResetFactoryDefaultPreference(0) {
  clearAll();
  clearNetworkSettings();
}

// VariousBits1 defaults to 0, keep in mind when adding bit lookups.
template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::appendUnitToHostname()  const {
  return !getBitFromUL(VariousBits1, 1);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::appendUnitToHostname(bool value) {
  setBitToUL(VariousBits1, 1, !value);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::uniqueMQTTclientIdReconnect() const {
  return getBitFromUL(VariousBits1, 2);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::uniqueMQTTclientIdReconnect(bool value) {
  setBitToUL(VariousBits1, 2, value);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::OldRulesEngine() const {
  return !getBitFromUL(VariousBits1, 3);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::OldRulesEngine(bool value) {
  setBitToUL(VariousBits1, 3, !value);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::ForceWiFi_bg_mode() const {
  return getBitFromUL(VariousBits1, 4);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::ForceWiFi_bg_mode(bool value) {
  setBitToUL(VariousBits1, 4, value);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::WiFiRestart_connection_lost() const {
  return getBitFromUL(VariousBits1, 5);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::WiFiRestart_connection_lost(bool value) {
  setBitToUL(VariousBits1, 5, value);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::EcoPowerMode() const {
  return getBitFromUL(VariousBits1, 6);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::EcoPowerMode(bool value) {
  setBitToUL(VariousBits1, 6, value);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::WifiNoneSleep() const {
  return getBitFromUL(VariousBits1, 7);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::WifiNoneSleep(bool value) {
  setBitToUL(VariousBits1, 7, value);
}

// Enable send gratuitous ARP by default, so invert the values (default = 0)
template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::gratuitousARP() const {
  return !getBitFromUL(VariousBits1, 8);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::gratuitousARP(bool value) {
  setBitToUL(VariousBits1, 8, !value);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::TolerantLastArgParse() const {
  return getBitFromUL(VariousBits1, 9);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::TolerantLastArgParse(bool value) {
  setBitToUL(VariousBits1, 9, value);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::SendToHttp_ack() const {
  return getBitFromUL(VariousBits1, 10);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::SendToHttp_ack(bool value) {
  setBitToUL(VariousBits1, 10, value);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::validate() {
  if (UDPPort > 65535) { UDPPort = 0; }

  if ((Latitude  < -90.0) || (Latitude > 90.0)) { Latitude = 0.0; }

  if ((Longitude < -180.0) || (Longitude > 180.0)) { Longitude = 0.0; }

  if (VariousBits1 > (1 << 30)) { VariousBits1 = 0; }
  ZERO_TERMINATE(Name);
  ZERO_TERMINATE(NTPHost);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::networkSettingsEmpty() const {
  return IP[0] == 0 && Gateway[0] == 0 && Subnet[0] == 0 && DNS[0] == 0;
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::clearNetworkSettings() {
  for (byte i = 0; i < 4; ++i) {
    IP[i]      = 0;
    Gateway[i] = 0;
    Subnet[i]  = 0;
    DNS[i]     = 0;
  }
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::clearTimeSettings() {
  UseNTP = false;
  ZERO_FILL(NTPHost);
  TimeZone  = 0;
  DST       = false;
  DST_Start = 0;
  DST_End   = 0;
  Latitude  = 0.0;
  Longitude = 0.0;
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::clearNotifications() {
  for (byte i = 0; i < NOTIFICATION_MAX; ++i) {
    Notification[i]        = 0;
    NotificationEnabled[i] = false;
  }
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::clearControllers() {
  for (controllerIndex_t i = 0; i < CONTROLLER_MAX; ++i) {
    Protocol[i]          = 0;
    ControllerEnabled[i] = false;
  }
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::clearTasks() {
  for (taskIndex_t task = 0; task < N_TASKS; ++task) {
    clearTask(task);
  }
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::clearLogSettings() {
  SyslogLevel    = 0;
  SerialLogLevel = 0;
  WebLogLevel    = 0;
  SDLogLevel     = 0;
  SyslogFacility = DEFAULT_SYSLOG_FACILITY;

  for (byte i = 0; i < 4; ++i) {  Syslog_IP[i] = 0; }
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::clearUnitNameSettings() {
  Unit = 0;
  ZERO_FILL(Name);
  UDPPort = 0;
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::clearMisc() {
  PID            = 0;
  Version        = 0;
  Build          = 0;
  IP_Octet       = 0;
  Delay          = 0;
  Pin_i2c_sda    = -1;
  Pin_i2c_scl    = -1;
  Pin_status_led = -1;
  Pin_sd_cs      = -1;

  for (byte i = 0; i < 17; ++i) { PinBootStates[i] = 0; }
  BaudRate                    = 0;
  MessageDelay                = 0;
  deepSleep_wakeTime                   = 0;
  CustomCSS                   = false;
  WDI2CAddress                = 0;
  UseRules                    = false;
  UseSerial                   = true;
  UseSSDP                     = false;
  WireClockStretchLimit       = 0;
  GlobalSync                  = false;
  ConnectionFailuresThreshold = 0;
  MQTTRetainFlag              = false;
  InitSPI                     = false;
  Pin_status_led_Inversed     = false;
  deepSleepOnFail             = false;
  UseValueLogger              = false;
  ArduinoOTAEnable            = false;
  UseRTOSMultitasking         = false;
  Pin_Reset                   = -1;
  StructSize                  = sizeof(SettingsStruct_tmpl<N_TASKS>);
  MQTTUseUnitNameAsClientId   = 0;
  VariousBits1                = 0;
  OldRulesEngine(DEFAULT_RULES_OLDENGINE);
  ForceWiFi_bg_mode(DEFAULT_WIFI_FORCE_BG_MODE);
  WiFiRestart_connection_lost(DEFAULT_WIFI_RESTART_WIFI_CONN_LOST);
  EcoPowerMode(DEFAULT_ECO_MODE);
  WifiNoneSleep(DEFAULT_WIFI_NONE_SLEEP);
  gratuitousARP(DEFAULT_GRATUITOUS_ARP);
  TolerantLastArgParse(DEFAULT_TOLERANT_LAST_ARG_PARSE);
  SendToHttp_ack(DEFAULT_SEND_TO_HTTP_ACK);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::clearAll() {
  clearMisc();
  clearTimeSettings();
  clearNetworkSettings();
  clearNotifications();
  clearControllers();
  clearTasks();
  clearLogSettings();
  clearUnitNameSettings();
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::clearTask(taskIndex_t task) {
  if (task >= N_TASKS) { return; }

  for (controllerIndex_t i = 0; i < CONTROLLER_MAX; ++i) {
    TaskDeviceID[i][task]       = 0;
    TaskDeviceSendData[i][task] = false;
  }
  TaskDeviceNumber[task]     = 0;
  OLD_TaskDeviceID[task]     = 0; // UNUSED: this can be removed
  TaskDevicePin1[task]       = -1;
  TaskDevicePin2[task]       = -1;
  TaskDevicePin3[task]       = -1;
  TaskDevicePort[task]       = 0;
  TaskDevicePin1PullUp[task] = false;

  for (byte cv = 0; cv < PLUGIN_CONFIGVAR_MAX; ++cv) {
    TaskDevicePluginConfig[task][cv] = 0;
  }
  TaskDevicePin1Inversed[task] = false;

  for (byte cv = 0; cv < PLUGIN_CONFIGFLOATVAR_MAX; ++cv) {
    TaskDevicePluginConfigFloat[task][cv] = 0.0;
  }

  for (byte cv = 0; cv < PLUGIN_CONFIGLONGVAR_MAX; ++cv) {
    TaskDevicePluginConfigLong[task][cv] = 0;
  }
  OLD_TaskDeviceSendData[task] = false;
  TaskDeviceGlobalSync[task]   = false;
  TaskDeviceDataFeed[task]     = 0;
  TaskDeviceTimer[task]        = 0;
  TaskDeviceEnabled[task]      = false;
}

template<unsigned int N_TASKS>
String SettingsStruct_tmpl<N_TASKS>::getHostname() const {
  return this->getHostname(this->appendUnitToHostname());
}

template<unsigned int N_TASKS>
String SettingsStruct_tmpl<N_TASKS>::getHostname(bool appendUnit) const {
  String hostname = this->Name;
  if (this->Unit != 0 && appendUnit) { // only append non-zero unit number
    hostname += '_';
    hostname += this->Unit;
  }
  return hostname;
}