
#ifndef DATASTRUCTS_SETTINGSSTRUCT_H
#define DATASTRUCTS_SETTINGSSTRUCT_H

#include "../../ESPEasy_common.h"

#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/ChecksumType.h"
#include "../DataStructs/DeviceStruct.h"
#include "../DataTypes/EthernetParameters.h"
#include "../DataTypes/NetworkMedium.h"
#include "../DataTypes/NPluginID.h"
#include "../DataTypes/PluginID.h"
//#include "../DataTypes/TaskEnabledState.h"
#include "../DataTypes/TimeSource.h"
#include "../Globals/Plugins.h"

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
template<unsigned int N_TASKS>
class SettingsStruct_tmpl
{
  public:

//  SettingsStruct_tmpl() = default;

  // VariousBits1 defaults to 0, keep in mind when adding bit lookups.
  bool appendUnitToHostname() const { return !VariousBits_1.appendUnitToHostname; }
  void appendUnitToHostname(bool value) { VariousBits_1.appendUnitToHostname = !value;}

  bool uniqueMQTTclientIdReconnect_unused() const { return VariousBits_1.unused_02; }
  void uniqueMQTTclientIdReconnect_unused(bool value) { VariousBits_1.unused_02 = value; }

  bool OldRulesEngine() const { 
#ifdef WEBSERVER_NEW_RULES
    return !VariousBits_1.OldRulesEngine;
#else
    return true;
#endif
  }
  void OldRulesEngine(bool value) { VariousBits_1.OldRulesEngine = !value; }

  bool ForceWiFi_bg_mode() const { return VariousBits_1.ForceWiFi_bg_mode; }
  void ForceWiFi_bg_mode(bool value) { VariousBits_1.ForceWiFi_bg_mode = value; }

  bool WiFiRestart_connection_lost() const { return VariousBits_1.WiFiRestart_connection_lost; }
  void WiFiRestart_connection_lost(bool value) { VariousBits_1.WiFiRestart_connection_lost = value; }

  bool EcoPowerMode() const { return VariousBits_1.EcoPowerMode; }
  void EcoPowerMode(bool value) { VariousBits_1.EcoPowerMode = value; }

  bool WifiNoneSleep() const { return VariousBits_1.WifiNoneSleep; }
  void WifiNoneSleep(bool value) { VariousBits_1.WifiNoneSleep = value; }

  // Enable send gratuitous ARP by default, so invert the values (default = 0)
  bool gratuitousARP() const { return !VariousBits_1.gratuitousARP; }
  void gratuitousARP(bool value) { VariousBits_1.gratuitousARP = !value; }

  // Be a bit more tolerant when parsing the last argument of a command.
  // See: https://github.com/letscontrolit/ESPEasy/issues/2724
  bool TolerantLastArgParse() const { return VariousBits_1.TolerantLastArgParse; }
  void TolerantLastArgParse(bool value) { VariousBits_1.TolerantLastArgParse = value; }

  // SendToHttp command does not wait for ack, with this flag it does wait.
  bool SendToHttp_ack() const { return VariousBits_1.SendToHttp_ack; }
  void SendToHttp_ack(bool value) { VariousBits_1.SendToHttp_ack = value; }

  // Enable/disable ESPEasyNow protocol
  bool UseESPEasyNow() const { 
#ifdef USES_ESPEASY_NOW
    return VariousBits_1.UseESPEasyNow; 
#else
    return false;
#endif
 }
  void UseESPEasyNow(bool value) { 
#ifdef USES_ESPEASY_NOW
    VariousBits_1.UseESPEasyNow = value; 
#endif
  }

  // Whether to try to connect to a hidden SSID network
  bool IncludeHiddenSSID() const { return VariousBits_1.IncludeHiddenSSID; }
  void IncludeHiddenSSID(bool value) { VariousBits_1.IncludeHiddenSSID = value; }

  // When sending, the TX power may be boosted to max TX power.
  bool UseMaxTXpowerForSending() const { return VariousBits_1.UseMaxTXpowerForSending; }
  void UseMaxTXpowerForSending(bool value) { VariousBits_1.UseMaxTXpowerForSending = value; }

  // When set you can use the Sensor in AP-Mode without beeing forced to /setup
  bool ApDontForceSetup() const { return VariousBits_1.ApDontForceSetup; }
  void ApDontForceSetup(bool value) { VariousBits_1.ApDontForceSetup = value; }

  // When outputting JSON bools use quoted values (on, backward compatible) or use official JSON true/false unquoted
  bool JSONBoolWithoutQuotes() const { return VariousBits_1.JSONBoolWithoutQuotes; }
  void JSONBoolWithoutQuotes(bool value) { VariousBits_1.JSONBoolWithoutQuotes = value; }
  
  // Enable timing statistics (may consume a few kB of RAM)
  bool EnableTimingStats() const { return VariousBits_1.EnableTimingStats; }
  void EnableTimingStats(bool value) { VariousBits_1.EnableTimingStats = value; }

  // Allow to actively reset I2C bus if it appears to be hanging.
  bool EnableClearHangingI2Cbus() const { 
#if FEATURE_CLEAR_I2C_STUCK
    return VariousBits_1.EnableClearHangingI2Cbus; 
#else
    return false;
#endif
}
  void EnableClearHangingI2Cbus(bool value) { VariousBits_1.EnableClearHangingI2Cbus = value; }

  // Enable RAM Tracking (may consume a few kB of RAM and cause some performance hit)
  bool EnableRAMTracking() const { return VariousBits_1.EnableRAMTracking; }
  void EnableRAMTracking(bool value) { VariousBits_1.EnableRAMTracking = value; }

  // Enable caching of rules, to speed up rules processing
  bool EnableRulesCaching() const { return !VariousBits_1.EnableRulesCaching; }
  void EnableRulesCaching(bool value) { VariousBits_1.EnableRulesCaching = !value; }

  // Allow the cached event entries to be sorted based on how frequent they occur.
  // This may speed up rules processing, especially on large rule sets with lots of rules blocks.
  bool EnableRulesEventReorder() const { return !VariousBits_1.EnableRulesEventReorder; }
  void EnableRulesEventReorder(bool value) { VariousBits_1.EnableRulesEventReorder = !value; }

  // Allow OTA to use 'unlimited' bin sized files, possibly overwriting the file-system, and trashing files
  // Can be used if the configuration is later retrieved/restored manually
  bool AllowOTAUnlimited() const { return VariousBits_1.AllowOTAUnlimited; }
  void AllowOTAUnlimited(bool value) { VariousBits_1.AllowOTAUnlimited = value; }

  // Default behavior is to not allow following redirects  
  bool SendToHTTP_follow_redirects() const { return VariousBits_1.SendToHTTP_follow_redirects; }
  void SendToHTTP_follow_redirects(bool value) { VariousBits_1.SendToHTTP_follow_redirects = value; }

  #if FEATURE_I2C_DEVICE_CHECK
  // Check if an I2C device is found at configured address at plugin_INIT and plugin_READ
  bool CheckI2Cdevice() const { return !VariousBits_1.CheckI2Cdevice; }
  void CheckI2Cdevice(bool value) { VariousBits_1.CheckI2Cdevice = !value; }
  #endif // if FEATURE_I2C_DEVICE_CHECK

  // Wait for a second after calling WiFi.begin()
  // Especially useful for some FritzBox routers.
  bool WaitWiFiConnect() const { return VariousBits_2.WaitWiFiConnect; }
  void WaitWiFiConnect(bool value) { VariousBits_2.WaitWiFiConnect = value; }

  // Connect to Hidden SSID using channel and BSSID
  // This is much slower, but appears to be needed for some access points 
  // like MikroTik.
  bool HiddenSSID_SlowConnectPerBSSID() const { return !VariousBits_2.HiddenSSID_SlowConnectPerBSSID; }
  void HiddenSSID_SlowConnectPerBSSID(bool value) { VariousBits_2.HiddenSSID_SlowConnectPerBSSID = !value; }

  bool EnableIPv6() const { return !VariousBits_2.EnableIPv6; }
  void EnableIPv6(bool value) { VariousBits_2.EnableIPv6 = !value; }

  // Use Espressif's auto reconnect.
  bool SDK_WiFi_autoreconnect() const { return VariousBits_2.SDK_WiFi_autoreconnect; }
  void SDK_WiFi_autoreconnect(bool value) { VariousBits_2.SDK_WiFi_autoreconnect = value; }

  #if FEATURE_RULES_EASY_COLOR_CODE
  // Inhibit RulesCodeCompletion
  bool DisableRulesCodeCompletion() const { return VariousBits_2.DisableRulesCodeCompletion; }
  void DisableRulesCodeCompletion(bool value) { VariousBits_2.DisableRulesCodeCompletion = value; }
  #endif // if FEATURE_RULES_EASY_COLOR_CODE

  #if FEATURE_TARSTREAM_SUPPORT
  bool DisableSaveConfigAsTar() const { return VariousBits_2.DisableSaveConfigAsTar; }
  void DisableSaveConfigAsTar(bool value) { VariousBits_2.DisableSaveConfigAsTar = value; }
  #endif // if FEATURE_TARSTREAM_SUPPORT

  // Flag indicating whether all task values should be sent in a single event or one event per task value (default behavior)
  bool CombineTaskValues_SingleEvent(taskIndex_t taskIndex) const;
  void CombineTaskValues_SingleEvent(taskIndex_t taskIndex, bool value);

  bool DoNotStartAP() const  { return VariousBits_1.DoNotStartAP; }
  void DoNotStartAP(bool value) { VariousBits_1.DoNotStartAP = value; }

  bool UseAlternativeDeepSleep() const { return VariousBits_1.UseAlternativeDeepSleep; }
  void UseAlternativeDeepSleep(bool value) { VariousBits_1.UseAlternativeDeepSleep = value; }

  bool UseLastWiFiFromRTC() const { return VariousBits_1.UseLastWiFiFromRTC; }
  void UseLastWiFiFromRTC(bool value) { VariousBits_1.UseLastWiFiFromRTC = value; }

  ExtTimeSource_e ExtTimeSource() const;
  void ExtTimeSource(ExtTimeSource_e value);

  bool UseNTP() const;
  void UseNTP(bool value);

  bool AllowTaskValueSetAllPlugins() const { return VariousBits_1.AllowTaskValueSetAllPlugins; }
  void AllowTaskValueSetAllPlugins(bool value) { VariousBits_1.AllowTaskValueSetAllPlugins = value; }

  #if FEATURE_AUTO_DARK_MODE
  uint8_t getCssMode() const { return VariousBits_1.CssMode; }
  void    setCssMode(uint8_t value) { VariousBits_1.CssMode = value; }
  #endif // FEATURE_AUTO_DARK_MODE

  bool isTaskEnableReadonly(taskIndex_t taskIndex) const;
  void setTaskEnableReadonly(taskIndex_t taskIndex, bool value);

  #if FEATURE_PLUGIN_PRIORITY
  bool isPowerManagerTask(taskIndex_t taskIndex) const;
  void setPowerManagerTask(taskIndex_t taskIndex, bool value);

  bool isPriorityTask(taskIndex_t taskIndex) const;
  #endif // if FEATURE_PLUGIN_PRIORITY

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
  bool isI2CEnabled() const;

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


  unsigned long PID = 0;
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
  unsigned long Delay = 0;              // Sleep time in seconds
  int8_t        Pin_i2c_sda = DEFAULT_PIN_I2C_SDA;
  int8_t        Pin_i2c_scl = DEFAULT_PIN_I2C_SCL;
  int8_t        Pin_status_led = DEFAULT_PIN_STATUS_LED;
  int8_t        Pin_sd_cs = -1;
  int8_t        PinBootStates[17] = {0};  // Only use getPinBootState and setPinBootState as multiple pins are packed for ESP32
  uint8_t       Syslog_IP[4] = {0};
  unsigned int  UDPPort = 8266;
  uint8_t       SyslogLevel = 0;
  uint8_t       SerialLogLevel = 0;
  uint8_t       WebLogLevel = 0;
  uint8_t       SDLogLevel = 0;
  unsigned long BaudRate = 115200;
  unsigned long MessageDelay_unused = 0;  // MQTT settings now moved to the controller settings.
  uint8_t       deepSleep_wakeTime = 0;   // 0 = Sleep Disabled, else time awake from sleep in seconds
  boolean       CustomCSS = false;
  boolean       DST = false;
  uint8_t       WDI2CAddress = 0;
  boolean       UseRules = false;
  boolean       UseSerial = false;
  boolean       UseSSDP = false;
  uint8_t       ExternalTimeSource = 0;
  unsigned long WireClockStretchLimit = 0;
  boolean       GlobalSync = false;
  unsigned long ConnectionFailuresThreshold = 0;
  int16_t       TimeZone = 0;
  boolean       MQTTRetainFlag_unused = false;
  uint8_t       InitSPI = 0; //0 = disabled, 1= enabled but for ESP32 there is option 2= SPI2 9 = User defined, see src/src/WebServer/HardwarePage.h enum SPI_Options_e
  // FIXME TD-er: Must change to cpluginID_t, but then also another check must be added since changing the pluginID_t will also render settings incompatible
  uint8_t       Protocol[CONTROLLER_MAX] = {0};
  uint8_t       Notification[NOTIFICATION_MAX] = {0}; //notifications, point to a NPLUGIN id
  // FIXME TD-er: Must change to pluginID_t, but then also another check must be added since changing the pluginID_t will also render settings incompatible
  uint8_t       TaskDeviceNumber[N_TASKS] = {0}; // The "plugin number" set at as task (e.g. 4 for P004_dallas)
  unsigned int  OLD_TaskDeviceID[N_TASKS] = {0};  //UNUSED: this can be reused

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
  unsigned long TaskDeviceTimer[N_TASKS] = {0};
  boolean       TaskDeviceEnabled[N_TASKS] = {0};
  boolean       ControllerEnabled[CONTROLLER_MAX] = {0};
  boolean       NotificationEnabled[NOTIFICATION_MAX] = {0};
  unsigned int  TaskDeviceID[CONTROLLER_MAX][N_TASKS]{};        // IDX number (mainly used by Domoticz)
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
      uint32_t ApDontForceSetup             : 1;  // Bit 14
      uint32_t unused_15                    : 1;  // Bit 15   was used by PeriodicalScanWiFi
      uint32_t JSONBoolWithoutQuotes        : 1;  // Bit 16
      uint32_t DoNotStartAP                 : 1;  // Bit 17
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

  } VariousBits_1;

  uint32_t      ResetFactoryDefaultPreference = 0; // Do not clear this one in the clearAll()
  uint32_t      I2C_clockSpeed = 400000;
  uint16_t      WebserverPort = 80;
  uint16_t      SyslogPort = DEFAULT_SYSLOG_PORT;

  int8_t          ETH_Phy_Addr = -1;
  int8_t          ETH_Pin_mdc_cs = -1;
  int8_t          ETH_Pin_mdio_irq = -1;
  int8_t          ETH_Pin_power_rst = -1;
  EthPhyType_t    ETH_Phy_Type = EthPhyType_t::notSet;
  EthClockMode_t  ETH_Clock_Mode = EthClockMode_t::Ext_crystal_osc;
  uint8_t         ETH_IP[4] = {0};
  uint8_t         ETH_Gateway[4] = {0};
  uint8_t         ETH_Subnet[4] = {0};
  uint8_t         ETH_DNS[4] = {0};
  NetworkMedium_t NetworkMedium = NetworkMedium_t::WIFI;
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
  int8_t        WiFi_sensitivity_margin = 3;  // Margin in dBm on top of sensitivity.
  uint8_t       NumberExtraWiFiScans = 0;
  int8_t        SPI_SCLK_pin = -1;
  int8_t        SPI_MISO_pin = -1;
  int8_t        SPI_MOSI_pin = -1;
  int8_t        ForceESPEasyNOWchannel = 0;

  // Do not rename or move this checksum.
  // Checksum calculation will work "around" this
  uint8_t       md5[16]{}; // Store checksum of the settings.

  // VariousBits_2 defaults to 0, keep in mind when adding bit lookups.
  struct {
    uint32_t WaitWiFiConnect                  : 1; // Bit 00
    uint32_t SDK_WiFi_autoreconnect           : 1; // Bit 01
    uint32_t DisableRulesCodeCompletion       : 1; // Bit 02
    uint32_t HiddenSSID_SlowConnectPerBSSID   : 1; // Bit 03  // inverted
    uint32_t EnableIPv6                       : 1; // Bit 04  // inverted
    uint32_t DisableSaveConfigAsTar           : 1; // Bit 05
    uint32_t unused_06                        : 1; // Bit 06
    uint32_t unused_07                        : 1; // Bit 07
    uint32_t unused_08                        : 1; // Bit 08
    uint32_t unused_09                        : 1; // Bit 09
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

  } VariousBits_2;

  uint8_t       console_serial_port = DEFAULT_CONSOLE_PORT; 
  int8_t        console_serial_rxpin = DEFAULT_CONSOLE_PORT_RXPIN;
  int8_t        console_serial_txpin = DEFAULT_CONSOLE_PORT_TXPIN;
  uint8_t       console_serial0_fallback = DEFAULT_CONSOLE_SER0_FALLBACK;
  
  // Try to extend settings to make the checksum 4-uint8_t aligned.
};

/*
SettingsStruct* SettingsStruct_ptr = new (std::nothrow) SettingsStruct;
SettingsStruct& Settings = *SettingsStruct_ptr;
*/



typedef SettingsStruct_tmpl<TASKS_MAX> SettingsStruct;

#endif // DATASTRUCTS_SETTINGSSTRUCT_H
