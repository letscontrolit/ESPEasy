
#ifndef DATASTRUCTS_SETTINGSSTRUCT_H
#define DATASTRUCTS_SETTINGSSTRUCT_H

#include "../../ESPEasy_common.h"

#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/DeviceStruct.h"
#include "../DataTypes/EthernetParameters.h"
#include "../DataTypes/NetworkMedium.h"
#include "../DataTypes/TimeSource.h"
#include "../Globals/Plugins.h"

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

//  SettingsStruct_tmpl(); //-V730

  // VariousBits1 defaults to 0, keep in mind when adding bit lookups.
  bool appendUnitToHostname() const;
  void appendUnitToHostname(bool value);

  bool uniqueMQTTclientIdReconnect_unused() const;
  void uniqueMQTTclientIdReconnect_unused(bool value);

  bool OldRulesEngine() const;
  void OldRulesEngine(bool value);

  bool ForceWiFi_bg_mode() const;
  void ForceWiFi_bg_mode(bool value);

  bool WiFiRestart_connection_lost() const;
  void WiFiRestart_connection_lost(bool value);

  bool EcoPowerMode() const;
  void EcoPowerMode(bool value);

  bool WifiNoneSleep() const;
  void WifiNoneSleep(bool value);

  // Enable send gratuitous ARP by default, so invert the values (default = 0)
  bool gratuitousARP() const;
  void gratuitousARP(bool value);

  // Be a bit more tolerant when parsing the last argument of a command.
  // See: https://github.com/letscontrolit/ESPEasy/issues/2724
  bool TolerantLastArgParse() const;
  void TolerantLastArgParse(bool value);

  // SendToHttp command does not wait for ack, with this flag it does wait.
  bool SendToHttp_ack() const;
  void SendToHttp_ack(bool value);

  // Enable/disable ESPEasyNow protocol
  bool UseESPEasyNow() const;
  void UseESPEasyNow(bool value);

  // Whether to try to connect to a hidden SSID network
  bool IncludeHiddenSSID() const;
  void IncludeHiddenSSID(bool value);

  // When sending, the TX power may be boosted to max TX power.
  bool UseMaxTXpowerForSending() const;
  void UseMaxTXpowerForSending(bool value);

  // When set you can use the Sensor in AP-Mode without beeing forced to /setup
  bool ApDontForceSetup() const;
  void ApDontForceSetup(bool value);

  // When outputting JSON bools use quoted values (on, backward compatible) or use official JSON true/false unquoted
  bool JSONBoolWithoutQuotes() const;
  void JSONBoolWithoutQuotes(bool value);
  
  // Enable timing statistics (may consume a few kB of RAM)
  bool EnableTimingStats() const;
  void EnableTimingStats(bool value);

  // Allow to actively reset I2C bus if it appears to be hanging.
  bool EnableClearHangingI2Cbus() const;
  void EnableClearHangingI2Cbus(bool value);

  // Enable RAM Tracking (may consume a few kB of RAM and cause some performance hit)
  bool EnableRAMTracking() const;
  void EnableRAMTracking(bool value);

  // Enable caching of rules, to speed up rules processing
  bool EnableRulesCaching() const;
  void EnableRulesCaching(bool value);

  // Allow the cached event entries to be sorted based on how frequent they occur.
  // This may speed up rules processing, especially on large rule sets with lots of rules blocks.
  bool EnableRulesEventReorder() const;
  void EnableRulesEventReorder(bool value);

  // Allow OTA to use 'unlimited' bin sized files, possibly overwriting the file-system, and trashing files
  // Can be used if the configuration is later retrieved/restored manually
  bool AllowOTAUnlimited() const;
  void AllowOTAUnlimited(bool value);

  // Default behavior is to not allow following redirects  
  bool SendToHTTP_follow_redirects() const;
  void SendToHTTP_follow_redirects(bool value);


  // Flag indicating whether all task values should be sent in a single event or one event per task value (default behavior)
  bool CombineTaskValues_SingleEvent(taskIndex_t taskIndex) const;
  void CombineTaskValues_SingleEvent(taskIndex_t taskIndex, bool value);

  bool DoNotStartAP() const;
  void DoNotStartAP(bool value);

  bool UseAlternativeDeepSleep() const;
  void UseAlternativeDeepSleep(bool value);

  bool UseLastWiFiFromRTC() const;
  void UseLastWiFiFromRTC(bool value);

  ExtTimeSource_e ExtTimeSource() const;
  void ExtTimeSource(ExtTimeSource_e value);

  bool UseNTP() const;
  void UseNTP(bool value);

  bool AllowTaskValueSetAllPlugins() const;
  void AllowTaskValueSetAllPlugins(bool value);

  #if FEATURE_AUTO_DARK_MODE
  uint8_t getCssMode() const;
  void    setCssMode(uint8_t value);
  #endif // FEATURE_AUTO_DARK_MODE


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

  PinBootState getPinBootState(uint8_t gpio_pin) const;
  void setPinBootState(uint8_t gpio_pin, PinBootState state);

  bool getSPI_pins(int8_t spi_gpios[3]) const;

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
  int8_t        Pin_i2c_sda = -1;
  int8_t        Pin_i2c_scl = -1;
  int8_t        Pin_status_led = -1;
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
  unsigned int  OLD_TaskDeviceID[N_TASKS] = {0};  //UNUSED: this can be removed
  union {
    struct {
      int8_t        TaskDevicePin1[N_TASKS];
      int8_t        TaskDevicePin2[N_TASKS];
      int8_t        TaskDevicePin3[N_TASKS];
      uint8_t       TaskDevicePort[N_TASKS];
    };
    int8_t        TaskDevicePin[4][N_TASKS];
  };
  boolean       TaskDevicePin1PullUp[N_TASKS] = {0};
  int16_t       TaskDevicePluginConfig[N_TASKS][PLUGIN_CONFIGVAR_MAX];
  boolean       TaskDevicePin1Inversed[N_TASKS] = {0};
  float         TaskDevicePluginConfigFloat[N_TASKS][PLUGIN_CONFIGFLOATVAR_MAX];
  union {
    int32_t  TaskDevicePluginConfigLong[N_TASKS][PLUGIN_CONFIGLONGVAR_MAX];
    uint32_t TaskDevicePluginConfigULong[N_TASKS][PLUGIN_CONFIGLONGVAR_MAX];
  };
  uint8_t       TaskDeviceSendDataFlags[N_TASKS] = {0};
  uint8_t       OLD_TaskDeviceGlobalSync[N_TASKS] = {0};
  uint8_t       TaskDeviceDataFeed[N_TASKS] = {0};    // When set to 0, only read local connected sensorsfeeds
  unsigned long TaskDeviceTimer[N_TASKS] = {0};
  boolean       TaskDeviceEnabled[N_TASKS] = {0};
  boolean       ControllerEnabled[CONTROLLER_MAX] = {0};
  boolean       NotificationEnabled[NOTIFICATION_MAX] = {0};
  unsigned int  TaskDeviceID[CONTROLLER_MAX][N_TASKS];        // IDX number (mainly used by Domoticz)
  boolean       TaskDeviceSendData[CONTROLLER_MAX][N_TASKS];
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
  uint32_t      VariousBits1 = 0;
  uint32_t      ResetFactoryDefaultPreference = 0; // Do not clear this one in the clearAll()
  uint32_t      I2C_clockSpeed = 400000;
  uint16_t      WebserverPort = 80;
  uint16_t      SyslogPort = 0;

  int8_t          ETH_Phy_Addr = -1;
  int8_t          ETH_Pin_mdc = -1;
  int8_t          ETH_Pin_mdio = -1;
  int8_t          ETH_Pin_power = -1;
  EthPhyType_t    ETH_Phy_Type = EthPhyType_t::LAN8710;
  EthClockMode_t  ETH_Clock_Mode = EthClockMode_t::Ext_crystal_osc;
  uint8_t         ETH_IP[4] = {0};
  uint8_t         ETH_Gateway[4] = {0};
  uint8_t         ETH_Subnet[4] = {0};
  uint8_t         ETH_DNS[4] = {0};
  NetworkMedium_t NetworkMedium = NetworkMedium_t::WIFI;
  int8_t          I2C_Multiplexer_Type = I2C_MULTIPLEXER_NONE;
  int8_t          I2C_Multiplexer_Addr = -1;
  int8_t          I2C_Multiplexer_Channel[N_TASKS];
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
  uint8_t       md5[16]; // Store checksum of the settings.
  
//  uint8_t       ProgmemMd5[16]; // crc of the binary that last saved the struct to file.


  // Try to extend settings to make the checksum 4-uint8_t aligned.
};

/*
SettingsStruct* SettingsStruct_ptr = new (std::nothrow) SettingsStruct;
SettingsStruct& Settings = *SettingsStruct_ptr;
*/



typedef SettingsStruct_tmpl<TASKS_MAX> SettingsStruct;

#endif // DATASTRUCTS_SETTINGSSTRUCT_H
