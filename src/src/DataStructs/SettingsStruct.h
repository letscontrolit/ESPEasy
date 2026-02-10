
#ifndef DATASTRUCTS_SETTINGSSTRUCT_H
#define DATASTRUCTS_SETTINGSSTRUCT_H

#include "../../ESPEasy_common.h"

#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/ChecksumType.h"
#include "../DataStructs/DeviceStruct.h"
#include "../DataTypes/ControllerIndex.h"
#include "../ESPEasy/net/DataTypes/NWPluginID.h"
#include "../ESPEasy/net/DataTypes/NetworkIndex.h"
#include "../DataTypes/NPluginID.h"
#include "../DataTypes/PluginID.h"
//#include "../DataTypes/TaskEnabledState.h"
#include "../DataTypes/TimeSource.h"
#include "../Globals/Plugins.h"
#include "../ESPEasy/net/Globals/NWPlugins.h"

#include "../../ESPEasy/net/DataTypes/NetworkMedium.h"

#if FEATURE_ETHERNET
#include "../../ESPEasy/net/DataTypes/EthernetParameters.h"
#endif

#ifdef ESP32
#include <hal/spi_types.h>
#endif

//we disable SPI if not defined
#ifndef DEFAULT_SPI
 #define DEFAULT_SPI 0
#endif


// FIXME TD-er: Move this PinBootState to DataTypes folder

// State is stored, so don't change order
enum class PinBootState {
  Default_state  = 0,
  Output_low     = 1,
  Output_high    = 2,
  Input_pullup   = 3,
  Input_pulldown = 4,  // Only on ESP32 and GPIO16 on ESP82xx
  Input          = 5,

  // Options for later:
  // ANALOG (only on ESP32)
  // WAKEUP_PULLUP (only on ESP8266)
  // WAKEUP_PULLDOWN (only on ESP8266)
  // SPECIAL
  // FUNCTION_0 (only on ESP8266)
  // FUNCTION_1
  // FUNCTION_2
  // FUNCTION_3
  // FUNCTION_4
  // FUNCTION_5 (only on ESP32)
  // FUNCTION_6 (only on ESP32)

};




/*********************************************************************************************\
 * SettingsStruct
\*********************************************************************************************/
template<uint32_t N_TASKS>
class SettingsStruct_tmpl
{
  public:

//  SettingsStruct_tmpl() = default;

  // VariousBits1 defaults to 0, keep in mind when adding bit lookups.
  inline bool appendUnitToHostname() const { return !VariousBits_1.appendUnitToHostname; }
  inline void appendUnitToHostname(bool value) { VariousBits_1.appendUnitToHostname = !value;}

  inline bool uniqueMQTTclientIdReconnect_unused() const { return VariousBits_1.unused_02; }
  inline void uniqueMQTTclientIdReconnect_unused(bool value) { VariousBits_1.unused_02 = value; }

  inline bool OldRulesEngine() const { 
#ifdef WEBSERVER_NEW_RULES
    return !VariousBits_1.OldRulesEngine;
#else
    return true;
#endif
  }
  inline void OldRulesEngine(bool value) { VariousBits_1.OldRulesEngine = !value; }

  inline bool ForceWiFi_bg_mode() const { return VariousBits_1.ForceWiFi_bg_mode; }
  inline void ForceWiFi_bg_mode(bool value) { VariousBits_1.ForceWiFi_bg_mode = value; }

  inline bool WiFiRestart_connection_lost() const { return VariousBits_1.WiFiRestart_connection_lost; }
  inline void WiFiRestart_connection_lost(bool value) { VariousBits_1.WiFiRestart_connection_lost = value; }

  inline bool EcoPowerMode() const { return VariousBits_1.EcoPowerMode; }
  inline void EcoPowerMode(bool value) { VariousBits_1.EcoPowerMode = value; }

  inline bool WifiNoneSleep() const { return VariousBits_1.WifiNoneSleep; }
  inline void WifiNoneSleep(bool value) { VariousBits_1.WifiNoneSleep = value; }

  // Enable send gratuitous ARP by default, so invert the values (default = 0)
  inline bool gratuitousARP() const { return !VariousBits_1.gratuitousARP; }
  inline void gratuitousARP(bool value) { VariousBits_1.gratuitousARP = !value; }

  // Be a bit more tolerant when parsing the last argument of a command.
  // See: https://github.com/letscontrolit/ESPEasy/issues/2724
  inline bool TolerantLastArgParse() const { return VariousBits_1.TolerantLastArgParse; }
  inline void TolerantLastArgParse(bool value) { VariousBits_1.TolerantLastArgParse = value; }

  // SendToHttp command does not wait for ack, with this flag it does wait.
  inline bool SendToHttp_ack() const { return VariousBits_1.SendToHttp_ack; }
  inline void SendToHttp_ack(bool value) { VariousBits_1.SendToHttp_ack = value; }

  // Enable/disable ESPEasyNow protocol
  inline bool UseESPEasyNow() const { 
#ifdef USES_ESPEASY_NOW
    return VariousBits_1.UseESPEasyNow; 
#else
    return false;
#endif
 }
  inline void UseESPEasyNow(bool value) { 
#ifdef USES_ESPEASY_NOW
    VariousBits_1.UseESPEasyNow = value; 
#endif
  }

  // Whether to try to connect to a hidden SSID network
  inline bool IncludeHiddenSSID() const { return VariousBits_1.IncludeHiddenSSID; }
  inline void IncludeHiddenSSID(bool value) { VariousBits_1.IncludeHiddenSSID = value; }

  // When sending, the TX power may be boosted to max TX power.
  inline bool UseMaxTXpowerForSending() const { return VariousBits_1.UseMaxTXpowerForSending; }
  inline void UseMaxTXpowerForSending(bool value) { VariousBits_1.UseMaxTXpowerForSending = value; }

  // When set, user will be redirected to /setup or root page when connecting to this AP
  inline bool ApCaptivePortal() const { return !VariousBits_1.ApCaptivePortal; }
  inline void ApCaptivePortal(bool value) { VariousBits_1.ApCaptivePortal = !value; }

  // When outputting JSON bools use quoted values (on, backward compatible) or use official JSON true/false unquoted
  inline bool JSONBoolWithoutQuotes() const { return VariousBits_1.JSONBoolWithoutQuotes; }
  inline void JSONBoolWithoutQuotes(bool value) { VariousBits_1.JSONBoolWithoutQuotes = value; }
  
  // Enable timing statistics (may consume a few kB of RAM)
  inline bool EnableTimingStats() const { return VariousBits_1.EnableTimingStats; }
  inline void EnableTimingStats(bool value) { VariousBits_1.EnableTimingStats = value; }

  // Allow to actively reset I2C bus if it appears to be hanging.
  inline bool EnableClearHangingI2Cbus() const { 
#if FEATURE_CLEAR_I2C_STUCK
    return VariousBits_1.EnableClearHangingI2Cbus; 
#else
    return false;
#endif
}
  inline void EnableClearHangingI2Cbus(bool value) { VariousBits_1.EnableClearHangingI2Cbus = value; }

  // Enable RAM Tracking (may consume a few kB of RAM and cause some performance hit)
  inline bool EnableRAMTracking() const { return VariousBits_1.EnableRAMTracking; }
  inline void EnableRAMTracking(bool value) { VariousBits_1.EnableRAMTracking = value; }

  // Enable caching of rules, to speed up rules processing
  inline bool EnableRulesCaching() const { return !VariousBits_1.EnableRulesCaching; }
  inline void EnableRulesCaching(bool value) { VariousBits_1.EnableRulesCaching = !value; }

  // Allow the cached event entries to be sorted based on how frequent they occur.
  // This may speed up rules processing, especially on large rule sets with lots of rules blocks.
  inline bool EnableRulesEventReorder() const { return !VariousBits_1.EnableRulesEventReorder; }
  inline void EnableRulesEventReorder(bool value) { VariousBits_1.EnableRulesEventReorder = !value; }

  // Allow OTA to use 'unlimited' bin sized files, possibly overwriting the file-system, and trashing files
  // Can be used if the configuration is later retrieved/restored manually
  inline bool AllowOTAUnlimited() const { return VariousBits_1.AllowOTAUnlimited; }
  inline void AllowOTAUnlimited(bool value) { VariousBits_1.AllowOTAUnlimited = value; }

  // Default behavior is to not allow following redirects  
  inline bool SendToHTTP_follow_redirects() const { return VariousBits_1.SendToHTTP_follow_redirects; }
  inline void SendToHTTP_follow_redirects(bool value) { VariousBits_1.SendToHTTP_follow_redirects = value; }

  #if FEATURE_I2C_DEVICE_CHECK
  // Check if an I2C device is found at configured address at plugin_INIT and plugin_READ
  inline bool CheckI2Cdevice() const { return !VariousBits_1.CheckI2Cdevice; }
  inline void CheckI2Cdevice(bool value) { VariousBits_1.CheckI2Cdevice = !value; }
  #endif // if FEATURE_I2C_DEVICE_CHECK

  // Wait for a second after calling WiFi.begin()
  // Especially useful for some FritzBox routers.
  inline bool WaitWiFiConnect() const { return VariousBits_2.WaitWiFiConnect; }
  inline void WaitWiFiConnect(bool value) { VariousBits_2.WaitWiFiConnect = value; }

#ifdef ESP32
  // Toggle between passive/active WiFi scan.
  inline bool PassiveWiFiScan() const { return !VariousBits_2.PassiveWiFiScan; }
  inline void PassiveWiFiScan(bool value) { VariousBits_2.PassiveWiFiScan = !value; }
#endif

  // Connect to Hidden SSID using channel and BSSID
  // This is much slower, but appears to be needed for some access points 
  // like MikroTik.
  inline bool HiddenSSID_SlowConnectPerBSSID() const { return !VariousBits_2.HiddenSSID_SlowConnectPerBSSID; }
  inline void HiddenSSID_SlowConnectPerBSSID(bool value) { VariousBits_2.HiddenSSID_SlowConnectPerBSSID = !value; }

  inline bool EnableIPv6() const { return !VariousBits_2.EnableIPv6; }
  inline void EnableIPv6(bool value) { VariousBits_2.EnableIPv6 = !value; }

  // Use Espressif's auto reconnect.
  inline bool SDK_WiFi_autoreconnect() const { return VariousBits_2.SDK_WiFi_autoreconnect; }
  inline void SDK_WiFi_autoreconnect(bool value) { VariousBits_2.SDK_WiFi_autoreconnect = value; }

  #if FEATURE_RULES_EASY_COLOR_CODE
  // Inhibit RulesCodeCompletion
  inline bool DisableRulesCodeCompletion() const { return VariousBits_2.DisableRulesCodeCompletion; }
  inline void DisableRulesCodeCompletion(bool value) { VariousBits_2.DisableRulesCodeCompletion = value; }
  #endif // if FEATURE_RULES_EASY_COLOR_CODE

  #if FEATURE_TARSTREAM_SUPPORT
  inline bool DisableSaveConfigAsTar() const { return VariousBits_2.DisableSaveConfigAsTar; }
  inline void DisableSaveConfigAsTar(bool value) { VariousBits_2.DisableSaveConfigAsTar = value; }
  #endif // if FEATURE_TARSTREAM_SUPPORT

  #if FEATURE_TASKVALUE_UNIT_OF_MEASURE
  inline bool ShowUnitOfMeasureOnDevicesPage() const { return !VariousBits_2.ShowUnitOfMeasureOnDevicesPage; }
  inline void ShowUnitOfMeasureOnDevicesPage(bool value) { VariousBits_2.ShowUnitOfMeasureOnDevicesPage = !value; }
  #endif // if FEATURE_TASKVALUE_UNIT_OF_MEASURE

#if CONFIG_SOC_WIFI_SUPPORT_5G
  wifi_band_mode_t WiFi_band_mode() const { 
    if (VariousBits_2.WiFi_band_mode == 0) return WIFI_BAND_MODE_AUTO;
    return static_cast<wifi_band_mode_t>(VariousBits_2.WiFi_band_mode); 
  }
  inline void WiFi_band_mode(wifi_band_mode_t value) { VariousBits_2.WiFi_band_mode = value; }
#endif

#ifdef ESP32
  inline bool WiFi_AP_enable_NAPT() const { return !VariousBits_2.WiFi_AP_enable_NAPT; }
  inline void WiFi_AP_enable_NAPT(bool enable) { VariousBits_2.WiFi_AP_enable_NAPT = !enable; }
#endif


  #if FEATURE_MQTT_CONNECT_BACKGROUND
  inline bool MQTTConnectInBackground() const { return !VariousBits_2.MQTTConnectInBackground; } // Inverted!
  inline void MQTTConnectInBackground(bool value) { VariousBits_2.MQTTConnectInBackground = !value; }
  #endif // if FEATURE_MQTT_CONNECT_BACKGROUND

  // Flag indicating whether all task values should be sent in a single event or one event per task value (default behavior)
  bool CombineTaskValues_SingleEvent(taskIndex_t taskIndex) const;
  void CombineTaskValues_SingleEvent(taskIndex_t taskIndex, bool value);

  #if FEATURE_STRING_VARIABLES
  bool ShowDerivedTaskValues(taskIndex_t taskIndex) const;
  void ShowDerivedTaskValues(taskIndex_t taskIndex, bool value);
  bool EventAndLogDerivedTaskValues(taskIndex_t taskIndex) const;
  void EventAndLogDerivedTaskValues(taskIndex_t taskIndex, bool value);
  bool SendDerivedTaskValues(taskIndex_t taskIndex, controllerIndex_t controllerIndex) const;
  void SendDerivedTaskValues(taskIndex_t taskIndex, controllerIndex_t controllerIndex, bool value);
  #endif // if FEATURE_STRING_VARIABLES

  inline bool StartAPfallback_NoCredentials() const  { return !VariousBits_2.StartAPfallback_NoCredentials; }
  inline void StartAPfallback_NoCredentials(bool value) { VariousBits_2.StartAPfallback_NoCredentials = !value; }

  inline bool StartAP_on_NW002_init() const  { return VariousBits_2.StartAP_on_NW002_init; }
  inline void StartAP_on_NW002_init(bool value) { VariousBits_2.StartAP_on_NW002_init = value; }

  inline bool DoNotStartAPfallback_ConnectFail() const  { return VariousBits_1.DoNotStartAPfallback_ConnectFail; }
  inline void DoNotStartAPfallback_ConnectFail(bool value) { VariousBits_1.DoNotStartAPfallback_ConnectFail = value; }

  inline uint8_t APfallback_autostart_max_uptime_m() const { return VariousBits_2.APfallback_autostart_max_uptime_m; }
  inline void    APfallback_autostart_max_uptime_m(uint8_t count) { VariousBits_2.APfallback_autostart_max_uptime_m = count; }

  inline uint8_t APfallback_minimal_on_time_sec() const { 
    if (VariousBits_2.APfallback_minimal_on_time_sec == 0) {
      return DEFAULT_AP_FALLBACK_MINIMAL_ON_TIME_SEC;
    }
    return VariousBits_2.APfallback_minimal_on_time_sec;
  }
  inline void    APfallback_minimal_on_time_sec(uint8_t count) { 
    VariousBits_2.APfallback_minimal_on_time_sec = 
      (count == DEFAULT_AP_FALLBACK_MINIMAL_ON_TIME_SEC)
      ? 0 : count;
  }

  inline bool UseAlternativeDeepSleep() const { return VariousBits_1.UseAlternativeDeepSleep; }
  inline void UseAlternativeDeepSleep(bool value) { VariousBits_1.UseAlternativeDeepSleep = value; }

  inline bool UseLastWiFiFromRTC() const { return VariousBits_1.UseLastWiFiFromRTC; }
  inline void UseLastWiFiFromRTC(bool value) { VariousBits_1.UseLastWiFiFromRTC = value; }

  ExtTimeSource_e ExtTimeSource() const;
  void ExtTimeSource(ExtTimeSource_e value);

  bool UseNTP() const;
  void UseNTP(bool value);

  inline bool AllowTaskValueSetAllPlugins() const { return VariousBits_1.AllowTaskValueSetAllPlugins; }
  inline void AllowTaskValueSetAllPlugins(bool value) { VariousBits_1.AllowTaskValueSetAllPlugins = value; }

  #if FEATURE_AUTO_DARK_MODE
  inline uint8_t getCssMode() const { return VariousBits_1.CssMode; }
  inline void    setCssMode(uint8_t value) { VariousBits_1.CssMode = value; }
  #endif // FEATURE_AUTO_DARK_MODE

  bool isTaskEnableReadonly(taskIndex_t taskIndex) const;
  void setTaskEnableReadonly(taskIndex_t taskIndex, bool value);

  #if FEATURE_PLUGIN_PRIORITY
  bool isPowerManagerTask(taskIndex_t taskIndex) const;
  void setPowerManagerTask(taskIndex_t taskIndex, bool value);

  bool isPriorityTask(taskIndex_t taskIndex) const;
  #endif // if FEATURE_PLUGIN_PRIORITY

  #if FEATURE_MQTT
  bool SendRetainedTaskValues(taskIndex_t taskIndex, controllerIndex_t controllerIndex) const;
  void SendRetainedTaskValues(taskIndex_t taskIndex, controllerIndex_t controllerIndex, bool value);
  #endif // if FEATURE_MQTT

  void validate();

  bool networkSettingsEmpty() const;

  void clearNetworkSettings();

  void clearTimeSettings();

  void clearNotifications();

  void clearControllers();

  void clearTasks();

  void clearLogSettings();

  void clearUnitNameSettings();

  void clearMisc();

  void clearTask(taskIndex_t task);

  // Return hostname + unit when selected to add unit.
  String getHostname() const;

  // Return hostname with explicit set append unit.
  String getHostname(bool appendUnit) const;

  // Return the name of the unit, without unitnr appended, with template parsing applied, replacement for Settings.Name in most places
  String getName() const;

private:

  // Compute the index in either 
  // - PinBootStates array (index_low) or 
  // - PinBootStates_ESP32 (index_high)
  // Returns whether it is a valid index
  bool getPinBootStateIndex(
    int8_t gpio_pin, 
    int8_t& index_low
    #ifdef ESP32
    , int8_t& index_high
    #endif
    ) const;
  
public:

  PinBootState getPinBootState(int8_t gpio_pin) const;
  void setPinBootState(int8_t gpio_pin, PinBootState state);

  bool getSPI_pins(int8_t spi_gpios[3]) const;

  #ifdef ESP32
  spi_host_device_t getSPI_host() const;
  #endif

  // Return true when pin is one of the SPI pins and SPI is enabled
  bool isSPI_pin(int8_t pin) const;

  // Return true when SPI enabled and opt. user defined pins valid.
  bool isSPI_valid() const;

  // Return true when pin is one of the configured I2C pins.
  bool isI2C_pin(int8_t pin) const;

  // Return true if I2C settings are correct
  bool isI2CEnabled(uint8_t i2cBus) const;

  uint8_t getI2CInterface(taskIndex_t TaskIndex) const;
  int8_t getI2CSdaPin(uint8_t i2cBus) const;
  int8_t getI2CSclPin(uint8_t i2cBus) const;
  uint32_t getI2CClockSpeed(uint8_t i2cBus) const;
  uint32_t getI2CClockSpeedSlow(uint8_t i2cBus) const;
  uint32_t getI2CClockStretch(uint8_t i2cBus) const;
  
  #if FEATURE_I2C_MULTIPLE
  uint8_t getI2CInterfaceRTC() const;
  uint8_t getI2CInterfaceWDT() const;
  uint8_t getI2CInterfacePCFMCP() const;
  #endif // if FEATURE_I2C_MULTIPLE

  #if FEATURE_I2CMULTIPLEXER
  int8_t getI2CMultiplexerType(uint8_t i2cBus) const;
  int8_t getI2CMultiplexerAddr(uint8_t i2cBus) const;
  int8_t getI2CMultiplexerResetPin(uint8_t i2cBus) const;
  #endif // if FEATURE_I2CMULTIPLEXER

  // Return true when pin is one of the fixed Ethernet pins and Ethernet is enabled
  bool isEthernetPin(int8_t pin) const;

  // Return true when pin is one of the optional Ethernet pins and Ethernet is enabled
  bool isEthernetPinOptional(int8_t pin) const;

  // Access to TaskDevicePin1 ... TaskDevicePin3
  // @param pinnr 1 = TaskDevicePin1, ..., 3 = TaskDevicePin3
  int8_t getTaskDevicePin(taskIndex_t taskIndex, uint8_t pinnr) const;

  float getWiFi_TX_power() const;
  void setWiFi_TX_power(float dBm);

  pluginID_t getPluginID_for_task(taskIndex_t taskIndex) const;

  void forceSave() { memset(md5, 0, 16); }

  uint32_t getVariousBits1() const {
    uint32_t res;
    memcpy(&res, &VariousBits_1, sizeof(VariousBits_1));
    return res;    
  }

  void setVariousBits1(uint32_t value) {
    memcpy(&VariousBits_1, &value, sizeof(VariousBits_1));
  }

  uint32_t getVariousBits2() const {
    uint32_t res;
    memcpy(&res, &VariousBits_2, sizeof(VariousBits_2));
    return res;    
  }

  void setVariousBits2(uint32_t value) {
    memcpy(&VariousBits_2, &value, sizeof(VariousBits_2));
  }

  bool getNetworkEnabled(ESPEasy::net::networkIndex_t index) const;

  void setNetworkEnabled(ESPEasy::net::networkIndex_t index, bool enabled);

  bool getNetworkInterfaceSubnetBlockClientIP(ESPEasy::net::networkIndex_t index) const;

  void setNetworkInterfaceSubnetBlockClientIP(ESPEasy::net::networkIndex_t index, bool enabled);

#if FEATURE_USE_IPV6
  bool getNetworkEnabled_IPv6(ESPEasy::net::networkIndex_t index) const;

  void setNetworkEnabled_IPv6(ESPEasy::net::networkIndex_t index, bool enabled);
#endif

  ESPEasy::net::nwpluginID_t getNWPluginID_for_network(ESPEasy::net::networkIndex_t index) const;

  void setNWPluginID_for_network(ESPEasy::net::networkIndex_t index, ESPEasy::net::nwpluginID_t id);

#ifdef ESP32
  uint8_t getRoutePrio_for_network(ESPEasy::net::networkIndex_t index) const;

  void setRoutePrio_for_network(ESPEasy::net::networkIndex_t index, uint8_t prio);
#endif

  uint32_t getNetworkInterfaceStartupDelayAtBoot(ESPEasy::net::networkIndex_t index) const;

  void setNetworkInterfaceStartupDelayAtBoot(ESPEasy::net::networkIndex_t index, uint32_t delay_ms);

  uint32_t PID = 0;
  int           Version = 0;
  int16_t       Build = 0;
  uint8_t       IP[4] = {0};
  uint8_t       Gateway[4] = {0};
  uint8_t       Subnet[4] = {0};
  uint8_t       DNS[4] = {0};
  uint8_t       IP_Octet = 0;
  uint8_t       Unit = 0;
  char          Name[26] = {0};
  char          NTPHost[64] = {0};
  // FIXME TD-er: Issue #2690
  uint32_t Delay = 0;              // Sleep time in seconds
  int8_t        Pin_i2c_sda = DEFAULT_PIN_I2C_SDA;
  int8_t        Pin_i2c_scl = DEFAULT_PIN_I2C_SCL;
  int8_t        Pin_status_led = DEFAULT_PIN_STATUS_LED;
  int8_t        Pin_sd_cs = -1;
  int8_t        PinBootStates[17] = {0};  // Only use getPinBootState and setPinBootState as multiple pins are packed for ESP32
  uint8_t       Syslog_IP[4] = {0};
  uint32_t  UDPPort = 8266;
  uint8_t       SyslogLevel = 0;
  uint8_t       SerialLogLevel = 2;
  uint8_t       WebLogLevel = 0;
  uint8_t       SDLogLevel = 0;
  uint32_t BaudRate = 115200;
  uint32_t MessageDelay_unused = 0;  // MQTT settings now moved to the controller settings.
  uint8_t       deepSleep_wakeTime = 0;   // 0 = Sleep Disabled, else time awake from sleep in seconds
  boolean       CustomCSS = false;
  boolean       DST = false;
  uint8_t       WDI2CAddress = 0;
  boolean       UseRules = false;
  boolean       UseSerial = false;
  boolean       UseSSDP = false;
  uint8_t       ExternalTimeSource = 0;
  uint32_t WireClockStretchLimit = 0;
  union {
    struct {
      uint32_t unused_00                        : 1; // Bit 0
      uint32_t unused_01                        : 1; // Bit 1
      uint32_t unused_02                        : 1; // Bit 2
      uint32_t unused_03                        : 1; // Bit 3
      uint32_t unused_04                        : 1; // Bit 4
      uint32_t unused_05                        : 1; // Bit 5
      uint32_t unused_06                        : 1; // Bit 6
      uint32_t unused_07                        : 1; // Bit 7
      uint32_t unused_08                        : 1; // Bit 8
      uint32_t unused_09                        : 1; // Bit 9
      uint32_t unused_10                        : 1; // Bit 10
      uint32_t unused_11                        : 1; // Bit 11
      uint32_t unused_12                        : 1; // Bit 12
      uint32_t unused_13                        : 1; // Bit 13
      uint32_t unused_14                        : 1; // Bit 14
      uint32_t unused_15                        : 1; // Bit 15
      uint32_t unused_16                        : 1; // Bit 16
      uint32_t unused_17                        : 1; // Bit 17
      uint32_t unused_18                        : 1; // Bit 18
      uint32_t unused_19                        : 1; // Bit 19
      uint32_t unused_20                        : 1; // Bit 20
      uint32_t unused_21                        : 1; // Bit 21
      uint32_t unused_22                        : 1; // Bit 22
      uint32_t unused_23                        : 1; // Bit 23
      uint32_t unused_24                        : 1; // Bit 24
      uint32_t unused_25                        : 1; // Bit 25
      uint32_t unused_26                        : 1; // Bit 26
      uint32_t unused_27                        : 1; // Bit 27
      uint32_t unused_28                        : 1; // Bit 28
      uint32_t unused_29                        : 1; // Bit 29
      uint32_t unused_30                        : 1; // Bit 30
      uint32_t unused_31                        : 1; // Bit 31
    };
    uint32_t _all_bits{};
  } VariousBits_3;  //-V730

  uint32_t ConnectionFailuresThreshold = 0;
  int16_t       TimeZone = 0;
  boolean       MQTTRetainFlag_unused = false;
  uint8_t       InitSPI = 0; //0 = disabled, 1= enabled but for ESP32 there is option 2= SPI2 9 = User defined, see src/src/WebServer/HardwarePage.h enum SPI_Options_e
  // FIXME TD-er: Must change to cpluginID_t, but then also another check must be added since changing the pluginID_t will also render settings incompatible
  uint8_t       Protocol[CONTROLLER_MAX] = {0};
  uint8_t       Notification[NOTIFICATION_MAX] = {0}; //notifications, point to a NPLUGIN id
  // FIXME TD-er: Must change to pluginID_t, but then also another check must be added since changing the pluginID_t will also render settings incompatible
  uint8_t       TaskDeviceNumber[N_TASKS] = {0}; // The "plugin number" set at as task (e.g. 4 for P004_dallas)
  int8_t        Pin_i2c2_sda = DEFAULT_PIN_I2C2_SDA; // From here, storage borrowed from OLD_TaskDeviceID array
  int8_t        Pin_i2c2_scl = DEFAULT_PIN_I2C2_SCL;
  int8_t        Pin_i2c3_sda = DEFAULT_PIN_I2C3_SDA;
  int8_t        Pin_i2c3_scl = DEFAULT_PIN_I2C3_SCL;
  uint32_t      I2C2_clockSpeed = DEFAULT_I2C_CLOCK_SPEED;
  uint32_t      I2C2_clockSpeed_Slow = DEFAULT_I2C_CLOCK_SPEED_SLOW;
  uint32_t      I2C3_clockSpeed = DEFAULT_I2C_CLOCK_SPEED;
  uint32_t      I2C3_clockSpeed_Slow = DEFAULT_I2C_CLOCK_SPEED_SLOW;
  uint16_t      I2C_peripheral_bus = 0;
  int8_t        I2C2_Multiplexer_Type = I2C_MULTIPLEXER_NONE;
  int8_t        I2C2_Multiplexer_Addr = -1;
  int8_t        I2C2_Multiplexer_ResetPin = -1;
  int8_t        I2C3_Multiplexer_Type = I2C_MULTIPLEXER_NONE;
  int8_t        I2C3_Multiplexer_Addr = -1;
  int8_t        I2C3_Multiplexer_ResetPin = -1;
  uint32_t  OLD_TaskDeviceID[N_TASKS - 7] = {0};  //UNUSED: this can be reused

  // FIXME TD-er: When used on ESP8266, this conversion union may not work
  // It might work as it is 32-bit in size.
  union {
    struct {
      int8_t        TaskDevicePin1[N_TASKS];
      int8_t        TaskDevicePin2[N_TASKS];
      int8_t        TaskDevicePin3[N_TASKS];
      uint8_t       TaskDevicePort[N_TASKS];
    };
    int8_t        TaskDevicePin[4][N_TASKS]{};
  };
  boolean       TaskDevicePin1PullUp[N_TASKS] = {0};
  int16_t       TaskDevicePluginConfig[N_TASKS][PLUGIN_CONFIGVAR_MAX]{};
  boolean       TaskDevicePin1Inversed[N_TASKS] = {0};
  float         TaskDevicePluginConfigFloat[N_TASKS][PLUGIN_CONFIGFLOATVAR_MAX]{};

  // FIXME TD-er: When used on ESP8266, this conversion union may not work
  // It might work as it is 32-bit in size.
  union {
    int32_t  TaskDevicePluginConfigLong[N_TASKS][PLUGIN_CONFIGLONGVAR_MAX];
    uint32_t TaskDevicePluginConfigULong[N_TASKS][PLUGIN_CONFIGLONGVAR_MAX]{};
  };
  uint8_t       TaskDeviceSendDataFlags[N_TASKS] = {0};
  uint8_t       VariousTaskBits[N_TASKS] = {0};
  uint8_t       TaskDeviceDataFeed[N_TASKS] = {0};    // When set to 0, only read local connected sensorsfeeds
  uint32_t TaskDeviceTimer[N_TASKS] = {0};
  boolean       TaskDeviceEnabled[N_TASKS] = {0};
  boolean       ControllerEnabled[CONTROLLER_MAX] = {0};
  boolean       NotificationEnabled[NOTIFICATION_MAX] = {0};
  uint32_t  TaskDeviceID[CONTROLLER_MAX][N_TASKS]{};        // IDX number (mainly used by Domoticz)
  boolean       TaskDeviceSendData[CONTROLLER_MAX][N_TASKS]{};
  boolean       Pin_status_led_Inversed = false;
  boolean       deepSleepOnFail = false;
  boolean       UseValueLogger = false;
  boolean       ArduinoOTAEnable = false;
  uint16_t      DST_Start = 0;
  uint16_t      DST_End = 0;
  boolean       UseRTOSMultitasking = false;
  int8_t        Pin_Reset = -1;
  uint8_t       SyslogFacility = 0;
  uint32_t      StructSize = 0;  // Forced to be 32 bit, to make sure alignment is clear.
  boolean       MQTTUseUnitNameAsClientId_unused = false;

  //its safe to extend this struct, up to several bytes, default values in config are 0
  //look in misc.ino how config.dat is used because also other stuff is stored in it at different offsets.
  //TODO: document config.dat somewhere here
  float         Latitude = 0.0f;
  float         Longitude = 0.0f;

  // VariousBits_1 defaults to 0, keep in mind when adding bit lookups.
  struct {
      uint32_t unused_00                    : 1;  // Bit 00
      uint32_t appendUnitToHostname         : 1;  // Bit 01  Inverted
      uint32_t unused_02                    : 1;  // Bit 02 uniqueMQTTclientIdReconnect_unused
      uint32_t OldRulesEngine               : 1;  // Bit 03  Inverted
      uint32_t ForceWiFi_bg_mode            : 1;  // Bit 04
      uint32_t WiFiRestart_connection_lost  : 1;  // Bit 05
      uint32_t EcoPowerMode                 : 1;  // Bit 06
      uint32_t WifiNoneSleep                : 1;  // Bit 07
      uint32_t gratuitousARP                : 1;  // Bit 08  Inverted
      uint32_t TolerantLastArgParse         : 1;  // Bit 09
      uint32_t SendToHttp_ack               : 1;  // Bit 10
      uint32_t UseESPEasyNow                : 1;  // Bit 11
      uint32_t IncludeHiddenSSID            : 1;  // Bit 12
      uint32_t UseMaxTXpowerForSending      : 1;  // Bit 13
      uint32_t ApCaptivePortal              : 1;  // Bit 14  Inverted
      uint32_t unused_15                    : 1;  // Bit 15   was used by PeriodicalScanWiFi
      uint32_t JSONBoolWithoutQuotes        : 1;  // Bit 16
      uint32_t DoNotStartAPfallback_ConnectFail                 : 1;  // Bit 17
      uint32_t UseAlternativeDeepSleep      : 1;  // Bit 18
      uint32_t UseLastWiFiFromRTC           : 1;  // Bit 19
      uint32_t EnableTimingStats            : 1;  // Bit 20
      uint32_t AllowTaskValueSetAllPlugins  : 1;  // Bit 21
      uint32_t EnableClearHangingI2Cbus     : 1;  // Bit 22
      uint32_t EnableRAMTracking            : 1;  // Bit 23
      uint32_t EnableRulesCaching           : 1;  // Bit 24  Inverted
      uint32_t EnableRulesEventReorder      : 1;  // Bit 25  Inverted
      uint32_t AllowOTAUnlimited            : 1;  // Bit 26
      uint32_t SendToHTTP_follow_redirects  : 1;  // Bit 27
      uint32_t CssMode                      : 2;  // Bit 28
//       uint32_t unused_29                  : 1;  // Bit 29
      uint32_t CheckI2Cdevice               : 1;  // Bit 30  Inverted
      uint32_t DoNotUse_31                  : 1;  // Bit 31  Was used to detect whether various bits were even set

  } VariousBits_1;    //-V730

  uint32_t      ResetFactoryDefaultPreference = 0; // Do not clear this one in the clearAll()
  uint32_t      I2C_clockSpeed = 400000;
  uint16_t      WebserverPort = 80;
  uint16_t      SyslogPort = DEFAULT_SYSLOG_PORT;

  int8_t          ETH_Phy_Addr = -1;
  int8_t          ETH_Pin_mdc_cs = -1;
  int8_t          ETH_Pin_mdio_irq = -1;
  int8_t          ETH_Pin_power_rst = -1;
#if FEATURE_ETHERNET
  ESPEasy::net::EthPhyType_t    ETH_Phy_Type = ESPEasy::net::EthPhyType_t::notSet;
# if CONFIG_ETH_USE_ESP32_EMAC && FEATURE_ETHERNET
  ESPEasy::net::EthClockMode_t  ETH_Clock_Mode = static_cast<ESPEasy::net::EthClockMode_t>(0);
#else
  uint8_t         ETH_Clock_Mode{};
#endif
#else
  uint8_t         ETH_Phy_Type{};
  uint8_t         ETH_Clock_Mode{};
#endif
  uint8_t         ETH_IP[4] = {0};
  uint8_t         ETH_Gateway[4] = {0};
  uint8_t         ETH_Subnet[4] = {0};
  uint8_t         ETH_DNS[4] = {0};
  ESPEasy::net::NetworkMedium_t NetworkMedium = ESPEasy::net::NetworkMedium_t::WIFI;
  int8_t          I2C_Multiplexer_Type = I2C_MULTIPLEXER_NONE;
  int8_t          I2C_Multiplexer_Addr = -1;
  int8_t          I2C_Multiplexer_Channel[N_TASKS]{};
  uint8_t         I2C_Flags[N_TASKS] = {0};
  uint32_t        I2C_clockSpeed_Slow = 100000;
  int8_t          I2C_Multiplexer_ResetPin = -1;

  #ifdef ESP32
  int8_t        PinBootStates_ESP32[24] = {0}; // pins 17 ... 39
  #endif
  uint8_t       WiFi_TX_power = 70; // 70 = 17.5dBm. unit: 0.25 dBm
  int8_t        WiFi_sensitivity_margin = 5;  // Margin in dBm on top of sensitivity.
  uint8_t       ConnectFailRetryCount = 0;
  int8_t        SPI_SCLK_pin = -1;
  int8_t        SPI_MISO_pin = -1;
  int8_t        SPI_MOSI_pin = -1;
  uint8_t       WiFiAP_channel = 0;

  // Do not rename or move this checksum.
  // Checksum calculation will work "around" this
  uint8_t       md5[16]{}; // Store checksum of the settings.

  // VariousBits_2 defaults to 0, keep in mind when adding bit lookups.
  union {
    struct {
      uint32_t WaitWiFiConnect                     : 1; // Bit 00
      uint32_t SDK_WiFi_autoreconnect              : 1; // Bit 01
      uint32_t DisableRulesCodeCompletion          : 1; // Bit 02
      uint32_t HiddenSSID_SlowConnectPerBSSID      : 1; // Bit 03  // inverted
      uint32_t EnableIPv6                          : 1; // Bit 04  // inverted
      uint32_t DisableSaveConfigAsTar              : 1; // Bit 05
      uint32_t PassiveWiFiScan                     : 1; // Bit 06  // inverted
      uint32_t ShowUnitOfMeasureOnDevicesPage      : 1; // Bit 07  // inverted
      uint32_t WiFi_band_mode                      : 2; // Bit 08 & 09
      uint32_t WiFi_AP_enable_NAPT                 : 1; // Bit 10  // inverted
      uint32_t RestoreUserVarsFromEEPROMOnColdBoot : 1; // Bit 11
      uint32_t RestoreUserVarsFromEEPROMOnWarmBoot : 1; // Bit 12
      uint32_t MQTTConnectInBackground             : 1; // Bit 13  // inverted

      uint32_t StartAPfallback_NoCredentials       : 1; // Bit 14 // inverted
      uint32_t StartAP_on_NW002_init               : 1; // Bit 15
      uint32_t APfallback_minimal_on_time_sec      : 8; // Bit 16 - 23
      uint32_t APfallback_autostart_max_uptime_m   : 8; // Bit 23 - 31  '0' == disabled
    };
    uint32_t _all_bits{};
  } VariousBits_2;  //-V730

  uint8_t       console_serial_port = DEFAULT_CONSOLE_PORT; 
  int8_t        console_serial_rxpin = DEFAULT_CONSOLE_PORT_RXPIN;
  int8_t        console_serial_txpin = DEFAULT_CONSOLE_PORT_TXPIN;
  uint8_t       console_serial0_fallback = DEFAULT_CONSOLE_SER0_FALLBACK;
  

  // ********************************************************************************
  //   NWPlugin (Network) settings
  // ********************************************************************************
  // FIXME TD-er: Must change to nwpluginID_t, but then also another check must be added since changing the pluginID_t will also render settings incompatible
  uint8_t       NWPluginID[NETWORK_MAX] = {0};
  uint8_t       NetworkEnabled_bits{};
  uint8_t       NetworkInterfaceSubnetBlockClientIP_bits{}; // Client IP Block Level. Allow from subnet of this interface
  uint8_t       NetworkEnabled_ipv6_bits{};                 // Whether or not to use IPv6 for the given interface  (Settings.EnableIPv6() is the global on/off for IPv6)
  uint8_t       NetworkUnused_3{};
#ifdef ESP32
  uint8_t       NetworkRoutePrio[NETWORK_MAX] = {0};
#endif
  // TODO TD-er: For ESP8266 we may likely ever use upto 2 or 3 network interfaces, so maybe re-use the rest later?
  uint16_t  NetworkInterfaceStartupDelayAtBoot[NETWORK_MAX]{};


  // Try to extend settings to make the checksum 4-uint8_t aligned.

};

/*
SettingsStruct* SettingsStruct_ptr = new (std::nothrow) SettingsStruct;
SettingsStruct& Settings = *SettingsStruct_ptr;
*/



typedef SettingsStruct_tmpl<TASKS_MAX> SettingsStruct;

#endif // DATASTRUCTS_SETTINGSSTRUCT_H
