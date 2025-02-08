#include "../Helpers/ESPEasy_FactoryDefault.h"

#include "../../ESPEasy_common.h"
#include "../../_Plugin_Helper.h"

#include "../CustomBuild/CompiletimeDefines.h"
#include "../CustomBuild/StorageLayout.h"

#include "../DataStructs/ControllerSettingsStruct.h"
#include "../DataStructs/FactoryDefaultPref.h"
#include "../DataStructs/GpioFactorySettingsStruct.h"

#include "../ESPEasyCore/ESPEasy_backgroundtasks.h"
#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../ESPEasyCore/Serial.h"

#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/RTC.h"
#include "../Globals/ResetFactoryDefaultPref.h"
#include "../Globals/SecuritySettings.h"

#include "../Helpers/_CPlugin_Helper.h"
#include "../Helpers/ESPEasyRTC.h"
#include "../Helpers/FS_Helper.h"
#include "../Helpers/Misc.h"

#ifdef ESP32

// Store in NVS partition
# include "../Helpers/ESPEasy_NVS_Helper.h"


// Max. 15 char namespace for ESPEasy Factory Default settings
# define FACTORY_DEFAULT_NVS_NAMESPACE      "ESPEasyFacDef"

# include "../Helpers/StringConverter.h"
# include "../DataStructs/FactoryDefaultPref.h"
# include "../DataStructs/FactoryDefault_UnitName_NVS.h"
# include "../DataStructs/FactoryDefault_WiFi_NVS.h"
# include "../DataStructs/FactoryDefault_Network_NVS.h"
# include "../DataStructs/FactoryDefault_LogConsoleSettings_NVS.h"
# if FEATURE_ALTERNATIVE_CDN_URL
#  include "../DataStructs/FactoryDefault_CDN_customurl_NVS.h"
# endif // if FEATURE_ALTERNATIVE_CDN_URL


#endif // ifdef ESP32


/********************************************************************************************\
   Reset all settings to factory defaults
 \*********************************************************************************************/
void ResetFactory(bool formatFS)
{
  bool mustApplySafebootDefaults = false;

  #ifdef ESP32
  ESPEasy_NVS_Helper preferences;

  if (!ResetFactoryDefaultPreference.init(preferences)) {
    mustApplySafebootDefaults = true;
  }
  #endif // ifdef ESP32
  #ifdef ESP8266 // Apply defaults on clean setup
  mustApplySafebootDefaults = 0 == Settings.ResetFactoryDefaultPreference;
  #endif // ifdef ESP8266

  if (ResetFactoryDefaultPreference.getPreference() == 0)
  {
#if FEATURE_CUSTOM_PROVISIONING
    ResetFactoryDefaultPreference.setDeviceModel(static_cast<DeviceModel>(DEFAULT_FACTORY_DEFAULT_DEVICE_MODEL));
    #if DEFAULT_PROVISIONING_FETCH_RULES1
    ResetFactoryDefaultPreference.fetchRulesTXT(0, true);
    #endif
    #if DEFAULT_PROVISIONING_FETCH_RULES2
    ResetFactoryDefaultPreference.fetchRulesTXT(1, true);
    #endif
    #if DEFAULT_PROVISIONING_FETCH_RULES3
    ResetFactoryDefaultPreference.fetchRulesTXT(2, true);
    #endif
    #if DEFAULT_PROVISIONING_FETCH_RULES4
    ResetFactoryDefaultPreference.fetchRulesTXT(3, true);
    #endif
    #if DEFAULT_PROVISIONING_FETCH_NOTIFICATIONS
    ResetFactoryDefaultPreference.fetchNotificationDat(true);
    #endif
    #if DEFAULT_PROVISIONING_FETCH_SECURITY
    ResetFactoryDefaultPreference.fetchSecurityDat(true);
    #endif
    #if DEFAULT_PROVISIONING_FETCH_CONFIG
    ResetFactoryDefaultPreference.fetchConfigDat(true);
    #endif
    #if DEFAULT_PROVISIONING_FETCH_PROVISIONING
    ResetFactoryDefaultPreference.fetchProvisioningDat(true);
    #endif
    #if DEFAULT_PROVISIONING_SAVE_URL
    ResetFactoryDefaultPreference.saveURL(true);
    #endif
    #if DEFAULT_PROVISIONING_SAVE_CREDENTIALS
    ResetFactoryDefaultPreference.storeCredentials(true);
    #endif
  #endif
    #if DEFAULT_FACTORY_RESET_KEEP_UNIT_NAME
    ResetFactoryDefaultPreference.keepUnitName(true);
    #endif
    #if DEFAULT_FACTORY_RESET_KEEP_WIFI
    ResetFactoryDefaultPreference.keepWiFi(true);
    #endif
    #if DEFAULT_FACTORY_RESET_KEEP_NETWORK
    ResetFactoryDefaultPreference.keepNetwork(true);
    #endif
    #if DEFAULT_FACTORY_RESET_KEEP_NTP_DST
    ResetFactoryDefaultPreference.keepNTP(true);
    #endif
    #if DEFAULT_FACTORY_RESET_KEEP_CONSOLE_LOG
    ResetFactoryDefaultPreference.keepLogConsoleSettings(true);
    #endif
#ifdef PLUGIN_BUILD_SAFEBOOT
    mustApplySafebootDefaults = true;
    ResetFactoryDefaultPreference.keepCustomCdnUrl(true);

    Settings.UseLastWiFiFromRTC(true);
#endif // ifdef PLUGIN_BUILD_SAFEBOOT
  }


  const GpioFactorySettingsStruct gpio_settings(ResetFactoryDefaultPreference.getDeviceModel());
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("ResetFactory"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

  // Direct Serial is allowed here, since this is only an emergency task.
  serialPrint(F("RESET: Resetting factory defaults... using "));
  serialPrint(getDeviceModelString(ResetFactoryDefaultPreference.getDeviceModel()));
  serialPrintln(F(" settings"));
  process_serialWriteBuffer();
  delay(1000);

  if (readFromRTC())
  {
    serialPrint(F("RESET: Warm boot, reset count: "));
    serialPrintln(String(RTC.factoryResetCounter));

    if (RTC.factoryResetCounter >= 3)
    {
      serialPrintln(F("RESET: Too many resets, protecting your flash memory (powercycle to solve this)"));
      return;
    }
  }
  else
  {
    serialPrintln(F("RESET: Cold boot"));
    initRTC();

    // TODO TD-er: Store set device model in RTC.
  }

  RTC.flashCounter = 0; // reset flashcounter, since we're already counting the number of factory-resets. we dont want to hit a flash-count
                        // limit during reset.
  RTC.factoryResetCounter++;
  saveToRTC();

  if (formatFS) {
    // always format on factory reset, in case of corrupt FS
    ESPEASY_FS.end();
    serialPrintln(F("RESET: formatting..."));
    FS_format();
    serialPrintln(F("RESET: formatting done..."));
    process_serialWriteBuffer();

    if (!ESPEASY_FS.begin())
    {
      serialPrintln(F("RESET: FORMAT FS FAILED!"));
      return;
    }
  }

#if FEATURE_CUSTOM_PROVISIONING
  {
    MakeProvisioningSettings(ProvisioningSettings);

    if (ProvisioningSettings.get()) {
      ProvisioningSettings->setUser(F(DEFAULT_PROVISIONING_USER));
      ProvisioningSettings->setPass(F(DEFAULT_PROVISIONING_PASS));
      ProvisioningSettings->setUrl(F(DEFAULT_PROVISIONING_URL));
      ProvisioningSettings->ResetFactoryDefaultPreference = ResetFactoryDefaultPreference.getPreference();
      saveProvisioningSettings(*ProvisioningSettings);
    }
  }
#endif // if FEATURE_CUSTOM_PROVISIONING

  // pad files with extra zeros for future extensions
  InitFile(SettingsType::SettingsFileEnum::FILE_CONFIG_type);
  InitFile(SettingsType::SettingsFileEnum::FILE_SECURITY_type);
  #if FEATURE_NOTIFIER
  InitFile(SettingsType::SettingsFileEnum::FILE_NOTIFICATION_type);
  #endif // if FEATURE_NOTIFIER

  InitFile(getRulesFileName(0), 0);

  Settings.clearMisc();

  if (!ResetFactoryDefaultPreference.keepNTP() || mustApplySafebootDefaults) {
    Settings.clearTimeSettings();
    Settings.UseNTP(DEFAULT_USE_NTP);
    strcpy_P(Settings.NTPHost, PSTR(DEFAULT_NTP_HOST));
    Settings.TimeZone = DEFAULT_TIME_ZONE;
    Settings.DST      = DEFAULT_USE_DST;
  }

  if (!ResetFactoryDefaultPreference.keepNetwork() || mustApplySafebootDefaults) {
    Settings.clearNetworkSettings();

    // TD-er Reset access control
    str2ip(F(DEFAULT_IPRANGE_LOW),  SecuritySettings.AllowedIPrangeLow);
    str2ip(F(DEFAULT_IPRANGE_HIGH), SecuritySettings.AllowedIPrangeHigh);
    SecuritySettings.IPblockLevel = DEFAULT_IP_BLOCK_LEVEL;

    #if DEFAULT_USE_STATIC_IP
    str2ip((char *)DEFAULT_IP,     Settings.IP);
    str2ip((char *)DEFAULT_DNS,    Settings.DNS);
    str2ip((char *)DEFAULT_GW,     Settings.Gateway);
    str2ip((char *)DEFAULT_SUBNET, Settings.Subnet);
    #endif // if DEFAULT_USE_STATIC_IP
    Settings.IncludeHiddenSSID(DEFAULT_WIFI_INCLUDE_HIDDEN_SSID);
  }

  Settings.clearNotifications();
  Settings.clearControllers();
  Settings.clearTasks();

  if (!ResetFactoryDefaultPreference.keepLogConsoleSettings() || mustApplySafebootDefaults) {
    Settings.clearLogSettings();
    str2ip((char *)DEFAULT_SYSLOG_IP, Settings.Syslog_IP);

    setLogLevelFor(LOG_TO_SYSLOG, DEFAULT_SYSLOG_LEVEL);
    setLogLevelFor(LOG_TO_SERIAL, DEFAULT_SERIAL_LOG_LEVEL);
    setLogLevelFor(LOG_TO_WEBLOG, DEFAULT_WEB_LOG_LEVEL);
    setLogLevelFor(LOG_TO_SDCARD, DEFAULT_SD_LOG_LEVEL);
    Settings.SyslogFacility = DEFAULT_SYSLOG_FACILITY;
    Settings.SyslogPort     = DEFAULT_SYSLOG_PORT;
    Settings.UseValueLogger = DEFAULT_USE_SD_LOG;

    // FIXME TD-er: Must also keep console settings.
    Settings.console_serial_port      = DEFAULT_CONSOLE_PORT;
    Settings.console_serial_rxpin     = DEFAULT_CONSOLE_PORT_RXPIN;
    Settings.console_serial_txpin     = DEFAULT_CONSOLE_PORT_TXPIN;
    Settings.console_serial0_fallback = DEFAULT_CONSOLE_SER0_FALLBACK;
    Settings.UseSerial                = DEFAULT_USE_SERIAL;
    Settings.BaudRate                 = DEFAULT_SERIAL_BAUD;
  }

  if (!ResetFactoryDefaultPreference.keepUnitName() || mustApplySafebootDefaults) {
    Settings.clearUnitNameSettings();
    Settings.Unit = UNIT;
    strcpy_P(Settings.Name, PSTR(DEFAULT_NAME));
    Settings.UDPPort = DEFAULT_SYNC_UDP_PORT;
  }

  if (!ResetFactoryDefaultPreference.keepWiFi() || mustApplySafebootDefaults) {
    strcpy_P(SecuritySettings.WifiSSID,  PSTR(DEFAULT_SSID));
    strcpy_P(SecuritySettings.WifiKey,   PSTR(DEFAULT_KEY));
    strcpy_P(SecuritySettings.WifiSSID2, PSTR(DEFAULT_SSID2));
    strcpy_P(SecuritySettings.WifiKey2,  PSTR(DEFAULT_KEY2));
    strcpy_P(SecuritySettings.WifiAPKey, PSTR(DEFAULT_AP_KEY));
  }
  strcpy_P(SecuritySettings.Password, PSTR(DEFAULT_ADMIN_PASS));

  Settings.ResetFactoryDefaultPreference = ResetFactoryDefaultPreference.getPreference();

  // now we set all parameters that need to be non-zero as default value


  Settings.PID     = ESP_PROJECT_PID;
  Settings.Version = VERSION;
  Settings.Build   = get_build_nr();

  //  Settings.IP_Octet				 = DEFAULT_IP_OCTET;
  //  Settings.Delay                   = DEFAULT_DELAY;
  Settings.Pin_i2c_sda    = gpio_settings.i2c_sda;
  Settings.Pin_i2c_scl    = gpio_settings.i2c_scl;
  Settings.Pin_status_led = gpio_settings.status_led;

  //  Settings.Pin_status_led_Inversed = DEFAULT_PIN_STATUS_LED_INVERSED;
  Settings.Pin_sd_cs   = -1;
  Settings.Pin_Reset   = DEFAULT_PIN_RESET_BUTTON;
  Settings.Protocol[0] = DEFAULT_PROTOCOL;

  //  Settings.deepSleep_wakeTime      = 0; // Sleep disabled
  //  Settings.CustomCSS               = false;
  //  Settings.InitSPI                 = DEFAULT_SPI;

  // advanced Settings
  //  Settings.UseRules                         = DEFAULT_USE_RULES;
  Settings.ControllerEnabled[0]             = DEFAULT_CONTROLLER_ENABLED;
  Settings.MQTTRetainFlag_unused            = DEFAULT_MQTT_RETAIN;
  Settings.MessageDelay_unused              = DEFAULT_MQTT_DELAY;
  Settings.MQTTUseUnitNameAsClientId_unused = DEFAULT_MQTT_USE_UNITNAME_AS_CLIENTID;

  // allow to set default latitude and longitude
  #ifdef DEFAULT_LATITUDE
  Settings.Latitude = DEFAULT_LATITUDE;
  #endif // ifdef DEFAULT_LATITUDE
  #ifdef DEFAULT_LONGITUDE
  Settings.Longitude = DEFAULT_LONGITUDE;
  #endif // ifdef DEFAULT_LONGITUDE

#ifdef ESP32

  // Ethernet related settings are never used on ESP8266
  Settings.ETH_Phy_Addr   = gpio_settings.eth_phyaddr;
  Settings.ETH_Pin_mdc_cs    = gpio_settings.eth_mdc;
  Settings.ETH_Pin_mdio_irq   = gpio_settings.eth_mdio;
  Settings.ETH_Pin_power_rst  = gpio_settings.eth_power;
  Settings.ETH_Phy_Type   = gpio_settings.eth_phytype;
  Settings.ETH_Clock_Mode = gpio_settings.eth_clock_mode;
#endif // ifdef ESP32
  Settings.NetworkMedium = gpio_settings.network_medium;

  /*
          Settings.GlobalSync						= DEFAULT_USE_GLOBAL_SYNC;

          Settings.IP_Octet						= DEFAULT_IP_OCTET;
          Settings.WDI2CAddress					= DEFAULT_WD_IC2_ADDRESS;
          Settings.UseSSDP						= DEFAULT_USE_SSDP;
          Settings.ConnectionFailuresThreshold	= DEFAULT_CON_FAIL_THRES;
          Settings.WireClockStretchLimit			= DEFAULT_I2C_CLOCK_LIMIT;
   */

  //  Settings.I2C_clockSpeed = DEFAULT_I2C_CLOCK_SPEED;

  Settings.JSONBoolWithoutQuotes(DEFAULT_JSON_BOOL_WITHOUT_QUOTES);
  Settings.EnableTimingStats(DEFAULT_ENABLE_TIMING_STATS);

#ifdef PLUGIN_DESCR
  strcpy_P(Settings.Name, PSTR(PLUGIN_DESCR));
#endif // ifdef PLUGIN_DESCR

#ifndef LIMIT_BUILD_SIZE
  addPredefinedPlugins(gpio_settings);
  addPredefinedRules(gpio_settings);
#endif // ifndef LIMIT_BUILD_SIZE

#if DEFAULT_CONTROLLER
  {
    // Place in a scope to have its memory freed ASAP
    MakeControllerSettings(ControllerSettings); // -V522

    if (AllocatedControllerSettings()) {
      safe_strncpy(ControllerSettings->Subscribe,            F(DEFAULT_SUB),            sizeof(ControllerSettings->Subscribe));
      safe_strncpy(ControllerSettings->Publish,              F(DEFAULT_PUB),            sizeof(ControllerSettings->Publish));
      safe_strncpy(ControllerSettings->MQTTLwtTopic,         F(DEFAULT_MQTT_LWT_TOPIC), sizeof(ControllerSettings->MQTTLwtTopic));
      safe_strncpy(ControllerSettings->LWTMessageConnect,    F(DEFAULT_MQTT_LWT_CONNECT_MESSAGE),
                   sizeof(ControllerSettings->LWTMessageConnect));
      safe_strncpy(ControllerSettings->LWTMessageDisconnect, F(DEFAULT_MQTT_LWT_DISCONNECT_MESSAGE),
                   sizeof(ControllerSettings->LWTMessageDisconnect));
      str2ip((char *)DEFAULT_SERVER, ControllerSettings->IP);
      ControllerSettings->setHostname(F(DEFAULT_SERVER_HOST));
      ControllerSettings->UseDNS = DEFAULT_SERVER_USEDNS;
      ControllerSettings->useExtendedCredentials(DEFAULT_USE_EXTD_CONTROLLER_CREDENTIALS);
      ControllerSettings->Port          = DEFAULT_PORT;
      ControllerSettings->ClientTimeout = DEFAULT_CONTROLLER_TIMEOUT;
      setControllerUser(0, *ControllerSettings, F(DEFAULT_CONTROLLER_USER));
      setControllerPass(0, *ControllerSettings, F(DEFAULT_CONTROLLER_PASS));

      SaveControllerSettings(0, *ControllerSettings);
    }
  }
#endif // if DEFAULT_CONTROLLER

#ifdef ESP32
  if (!mustApplySafebootDefaults)
  {
    Settings.ResetFactoryDefaultPreference = ResetFactoryDefaultPreference.getPreference();

    if (ResetFactoryDefaultPreference.keepUnitName())
    {
      FactoryDefault_UnitName_NVS unitNameNVS{};
      unitNameNVS.applyToSettings_from_NVS(preferences);
    }

    if (ResetFactoryDefaultPreference.keepWiFi())
    {
      FactoryDefault_WiFi_NVS wifiNVS{};
      wifiNVS.applyToSettings_from_NVS(preferences);
    }

    if (ResetFactoryDefaultPreference.keepNetwork())
    {
      // Restore Network IP settings
      FactoryDefault_Network_NVS network_nvs;
      network_nvs.applyToSettings_from_NVS(preferences);
    }

    if (ResetFactoryDefaultPreference.keepLogConsoleSettings())
    {
      // Restore Log and Console settings
      FactoryDefault_LogConsoleSettings_NVS log_console_nvs;
      log_console_nvs.applyToSettings_from_NVS(preferences);
    }

# if FEATURE_ALTERNATIVE_CDN_URL

    if (ResetFactoryDefaultPreference.keepCustomCdnUrl()) {
      FactoryDefault_CDN_customurl_NVS::applyToSettings_from_NVS(preferences);
    }
# endif // if FEATURE_ALTERNATIVE_CDN_URL
    preferences.end();
  }
#endif // ifdef ESP32

  const bool forFactoryReset = true;
  SaveSettings(forFactoryReset);
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("ResetFactory2"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  serialPrintln(F("RESET: Successful, rebooting. (you might need to press the reset button if you've just flashed the firmware)"));

  // NOTE: this is a known ESP8266 bug, not our fault. :)
  delay(1000);
  WiFi.persistent(true);  // use SDK storage of SSID/WPA parameters
  WiFiEventData.intent_to_reboot = true;
  WifiDisconnect();       // this will store empty ssid/wpa into sdk storage
  WiFi.persistent(false); // Do not use SDK storage of SSID/WPA parameters
  reboot(IntendedRebootReason_e::ResetFactory);
}

/*********************************************************************************************\
   Collect the stored preference for factory default
\*********************************************************************************************/
void applyFactoryDefaultPref() {
  // TODO TD-er: Store it in more places to make it more persistent
  Settings.ResetFactoryDefaultPreference = ResetFactoryDefaultPreference.getPreference();

#ifdef ESP32
  ESPEasy_NVS_Helper preferences;
  preferences.begin(F(FACTORY_DEFAULT_NVS_NAMESPACE));
  ResetFactoryDefaultPreference.to_NVS(preferences);
  {
    FactoryDefault_UnitName_NVS unitNameNVS{};

    if (ResetFactoryDefaultPreference.keepUnitName())
    {
      // Store Unit nr and hostname
      unitNameNVS.fromSettings_to_NVS(preferences);
    } else {
      unitNameNVS.clear_from_NVS(preferences);
    }
  }
  {
    FactoryDefault_WiFi_NVS wifiNVS{};

    if (ResetFactoryDefaultPreference.keepWiFi())
    {
      // Store WiFi credentials
      wifiNVS.fromSettings_to_NVS(preferences);
    } else {
      wifiNVS.clear_from_NVS(preferences);
    }
  }
  {
    FactoryDefault_Network_NVS network_nvs{};

    if (ResetFactoryDefaultPreference.keepNetwork())
    {
      // Store Network IP settings
      network_nvs.fromSettings_to_NVS(preferences);
    } else {
      network_nvs.clear_from_NVS(preferences);
    }
  }
  {
    FactoryDefault_LogConsoleSettings_NVS log_console_nvs{};

    if (ResetFactoryDefaultPreference.keepLogConsoleSettings())
    {
      // Store Log and Console settings
      log_console_nvs.fromSettings_to_NVS(preferences);
    } else {
      log_console_nvs.clear_from_NVS(preferences);
    }
  }
# if FEATURE_ALTERNATIVE_CDN_URL
  {
    if (ResetFactoryDefaultPreference.keepCustomCdnUrl())
    {
      // Store custom CDN
      FactoryDefault_CDN_customurl_NVS::fromSettings_to_NVS(preferences);
    } else {
      FactoryDefault_CDN_customurl_NVS::clear_from_NVS(preferences);
    }
  }
# endif // if FEATURE_ALTERNATIVE_CDN_URL


  preferences.end();
#endif // ifdef ESP32
}
