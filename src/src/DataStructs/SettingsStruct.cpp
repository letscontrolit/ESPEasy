#include "../DataStructs/SettingsStruct.h"

#include "../Globals/Plugins.h"
#include "../Globals/CPlugins.h"
#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/DeviceStruct.h"
#include "../../ESPEasy_common.h"

template<unsigned int N_TASKS>
SettingsStruct_tmpl<N_TASKS>::SettingsStruct_tmpl() : ResetFactoryDefaultPreference(0) {
  clearAll();
  clearNetworkSettings();
}

// VariousBits1 defaults to 0, keep in mind when adding bit lookups.
template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::appendUnitToHostname()  const {
  return !bitRead(VariousBits1, 1);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::appendUnitToHostname(bool value) {
  bitWrite(VariousBits1, 1, !value);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::uniqueMQTTclientIdReconnect_unused() const {
  return bitRead(VariousBits1, 2);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::uniqueMQTTclientIdReconnect_unused(bool value) {
  bitWrite(VariousBits1, 2, value);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::OldRulesEngine() const {
  #ifdef WEBSERVER_NEW_RULES
  return !bitRead(VariousBits1, 3);
  #else
  return true;
  #endif
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::OldRulesEngine(bool value) {
  bitWrite(VariousBits1, 3, !value);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::ForceWiFi_bg_mode() const {
  return bitRead(VariousBits1, 4);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::ForceWiFi_bg_mode(bool value) {
  bitWrite(VariousBits1, 4, value);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::WiFiRestart_connection_lost() const {
  return bitRead(VariousBits1, 5);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::WiFiRestart_connection_lost(bool value) {
  bitWrite(VariousBits1, 5, value);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::EcoPowerMode() const {
  return bitRead(VariousBits1, 6);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::EcoPowerMode(bool value) {
  bitWrite(VariousBits1, 6, value);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::WifiNoneSleep() const {
  return bitRead(VariousBits1, 7);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::WifiNoneSleep(bool value) {
  bitWrite(VariousBits1, 7, value);
}

// Enable send gratuitous ARP by default, so invert the values (default = 0)
template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::gratuitousARP() const {
  return !bitRead(VariousBits1, 8);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::gratuitousARP(bool value) {
  bitWrite(VariousBits1, 8, !value);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::TolerantLastArgParse() const {
  return bitRead(VariousBits1, 9);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::TolerantLastArgParse(bool value) {
  bitWrite(VariousBits1, 9, value);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::SendToHttp_ack() const {
  return bitRead(VariousBits1, 10);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::SendToHttp_ack(bool value) {
  bitWrite(VariousBits1, 10, value);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::UseESPEasyNow() const {
  return bitRead(VariousBits1, 11);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::UseESPEasyNow(bool value) {
  bitWrite(VariousBits1, 11, value);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::IncludeHiddenSSID() const {
  return bitRead(VariousBits1, 12);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::IncludeHiddenSSID(bool value) {
  bitWrite(VariousBits1, 12, value);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::UseMaxTXpowerForSending() const {
  return bitRead(VariousBits1, 13);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::UseMaxTXpowerForSending(bool value) {
  bitWrite(VariousBits1, 13, value);
}

template<unsigned int N_TASKS>
bool SettingsStruct_tmpl<N_TASKS>::CombineTaskValues_SingleEvent(taskIndex_t taskIndex) const {
  if (validTaskIndex(taskIndex))
    return bitRead(TaskDeviceSendDataFlags[taskIndex], 0);
  return false;
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::CombineTaskValues_SingleEvent(taskIndex_t taskIndex, bool value) {
  if (validTaskIndex(taskIndex))
    bitWrite(TaskDeviceSendDataFlags[taskIndex], 0, value);
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::validate() {
  if (UDPPort > 65535) { UDPPort = 0; }

  if ((Latitude  < -90.0f) || (Latitude > 90.0f)) { Latitude = 0.0f; }

  if ((Longitude < -180.0f) || (Longitude > 180.0f)) { Longitude = 0.0f; }

  if (VariousBits1 > (1 << 30)) { VariousBits1 = 0; }
  ZERO_TERMINATE(Name);
  ZERO_TERMINATE(NTPHost);

  if ((I2C_clockSpeed == 0) || (I2C_clockSpeed > 3400000)) { I2C_clockSpeed = DEFAULT_I2C_CLOCK_SPEED; }
  if (WebserverPort == 0) { WebserverPort = 80;}
  if (SyslogPort == 0) { SyslogPort = 514; }
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
    ETH_IP[i]       = 0;
    ETH_Gateway[i]  = 0;
    ETH_Subnet[i]   = 0;
    ETH_DNS[i]      = 0;
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
  Latitude  = 0.0f;
  Longitude = 0.0f;
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
  PID                      = 0;
  Version                  = 0;
  Build                    = 0;
  IP_Octet                 = 0;
  Delay                    = 0;
  Pin_i2c_sda              = DEFAULT_PIN_I2C_SDA;
  Pin_i2c_scl              = DEFAULT_PIN_I2C_SCL;
  Pin_status_led           = DEFAULT_PIN_STATUS_LED;
  Pin_sd_cs                = -1;
  ETH_Phy_Addr             = DEFAULT_ETH_PHY_ADDR;
  ETH_Pin_mdc              = DEFAULT_ETH_PIN_MDC;
  ETH_Pin_mdio             = DEFAULT_ETH_PIN_MDIO;
  ETH_Pin_power            = DEFAULT_ETH_PIN_POWER;
  ETH_Phy_Type             = DEFAULT_ETH_PHY_TYPE;
  ETH_Clock_Mode           = DEFAULT_ETH_CLOCK_MODE;
  NetworkMedium            = DEFAULT_NETWORK_MEDIUM;

  I2C_clockSpeed_Slow      = DEFAULT_I2C_CLOCK_SPEED_SLOW;
  I2C_Multiplexer_Type     = I2C_MULTIPLEXER_NONE;
  I2C_Multiplexer_Addr     = -1;
  for (taskIndex_t x = 0; x < TASKS_MAX; x++) {
    I2C_Multiplexer_Channel[x] = -1;
  }
  I2C_Multiplexer_ResetPin = -1;

  {
    // Here we initialize all data to 0, so this is the ONLY reason why PinBootStates 
    // can now be directly accessed.
    // In all other use cases, use the get and set functions for it.
    constexpr byte maxStates = sizeof(PinBootStates) / sizeof(PinBootStates[0]);
    for (byte i = 0; i < maxStates; ++i) { 
      PinBootStates[i] = 0; 
    }
    #ifdef ESP32
    constexpr byte maxStatesesp32 = sizeof(PinBootStates_ESP32) / sizeof(PinBootStates_ESP32[0]);
    for (byte i = 0; i < maxStatesesp32; ++i) {
      PinBootStates_ESP32[i] = 0;
    }
    #endif
  }
  BaudRate                         = 0;
  MessageDelay_unused              = 0;
  deepSleep_wakeTime               = 0;
  CustomCSS                        = false;
  WDI2CAddress                     = 0;
  UseRules                         = false;
  UseSerial                        = true;
  UseSSDP                          = false;
  WireClockStretchLimit            = 0;
  I2C_clockSpeed                   = 400000;
  WebserverPort                    = 80;
  SyslogPort                       = 514;
  GlobalSync                       = false;
  ConnectionFailuresThreshold      = 0;
  MQTTRetainFlag_unused            = false;
  InitSPI                          = DEFAULT_SPI;
  Pin_status_led_Inversed          = false;
  deepSleepOnFail                  = false;
  UseValueLogger                   = false;
  ArduinoOTAEnable                 = false;
  UseRTOSMultitasking              = false;
  Pin_Reset                        = -1;
  StructSize                       = sizeof(SettingsStruct_tmpl<N_TASKS>);
  MQTTUseUnitNameAsClientId_unused = 0;
  VariousBits1                     = 0;
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
    TaskDevicePluginConfigFloat[task][cv] = 0.0f;
  }

  for (byte cv = 0; cv < PLUGIN_CONFIGLONGVAR_MAX; ++cv) {
    TaskDevicePluginConfigLong[task][cv] = 0;
  }
  TaskDeviceSendDataFlags[task]  = 0;
  OLD_TaskDeviceGlobalSync[task]= 0;
  TaskDeviceDataFeed[task]      = 0;
  TaskDeviceTimer[task]         = 0;
  TaskDeviceEnabled[task]       = false;
  I2C_Multiplexer_Channel[task] = -1;
}

template<unsigned int N_TASKS>
String SettingsStruct_tmpl<N_TASKS>::getHostname() const {
  return this->getHostname(this->appendUnitToHostname());
}

template<unsigned int N_TASKS>
String SettingsStruct_tmpl<N_TASKS>::getHostname(bool appendUnit) const {
  String hostname = this->Name;

  if ((this->Unit != 0) && appendUnit) { // only append non-zero unit number
    hostname += '_';
    hostname += this->Unit;
  }
  return hostname;
}


template<unsigned int N_TASKS>
PinBootState SettingsStruct_tmpl<N_TASKS>::getPinBootState(uint8_t gpio_pin) const {
  constexpr byte maxStates = sizeof(PinBootStates) / sizeof(PinBootStates[0]);
  if (gpio_pin < maxStates) {
    return static_cast<PinBootState>(PinBootStates[gpio_pin]);
  }
  #ifdef ESP32
  constexpr byte maxStatesesp32 = sizeof(PinBootStates_ESP32) / sizeof(PinBootStates_ESP32[0]);
  const uint8_t addr = gpio_pin - maxStates;
  if (addr < maxStatesesp32) {
    return static_cast<PinBootState>(PinBootStates_ESP32[addr]);
  }
  #endif
  return PinBootState::Default_state;
}

template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::setPinBootState(uint8_t gpio_pin, PinBootState state) {
  constexpr byte maxStates = sizeof(PinBootStates) / sizeof(PinBootStates[0]);
  if (gpio_pin < maxStates) {
    PinBootStates[gpio_pin] = static_cast<int8_t>(state);
  }
  #ifdef ESP32
  constexpr byte maxStatesesp32 = sizeof(PinBootStates_ESP32) / sizeof(PinBootStates_ESP32[0]);
  const uint8_t addr = gpio_pin - maxStates;
  if (addr < maxStatesesp32) {
    PinBootStates_ESP32[addr] = static_cast<int8_t>(state);
  }
  #endif
}

template<unsigned int N_TASKS>
float SettingsStruct_tmpl<N_TASKS>::getWiFi_TX_power() const {
  return WiFi_TX_power / 4.0f;
}
  
template<unsigned int N_TASKS>
void SettingsStruct_tmpl<N_TASKS>::setWiFi_TX_power(float dBm) {
  WiFi_TX_power = dBm * 4.0f;
}

