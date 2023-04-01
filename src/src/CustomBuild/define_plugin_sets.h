#ifndef CUSTOMBUILD_DEFINE_PLUGIN_SETS_H
#define CUSTOMBUILD_DEFINE_PLUGIN_SETS_H

#include "../../include/ESPEasy_config.h"

/*
#################################################
 This is the place where plugins are registered
#################################################
To create/register a plugin, you have to :
- find an available number, ie 777.
- Create your own plugin, ie as "_P777_myfunction.ino"
- be sure it starts with ""#ifdef USES_P777", and ends with "#endif"
- then register it into the PLUGIN_SET_EXPERIMENTAL block (see below)
 #ifdef PLUGIN_SET_EXPERIMENTAL
     #define USES_P777   // MYsuperPlugin
 #endif
 - you can from now on test it by compiling using the PLUGIN_BUILD_DEV flag
 either by adding "-DPLUGIN_BUILD_DEV" when compiling, or by momentarly
 adding "#define PLUGIN_BUILD_DEV" at the top of the ESPEasy.ino file
 - You will then have to push a PR including your plugin + the corret line (#define USES_P777) added to this file
 When found stable enough, the maintainer (and only him) will choose to move it to COLLECTION or NORMAL
*/

//#define FEATURE_SD 1

/******************************************************************************\
 * WebServer pages   **********************************************************
\******************************************************************************/
// FIXME TD-er: Make useful selections for these pages to be included. (e.g. view only)

#ifndef WEBSERVER_CUSTOM_BUILD_DEFINED
    #ifndef WEBSERVER_TIMINGSTATS
        #define WEBSERVER_TIMINGSTATS
    #endif
    #ifndef WEBSERVER_SYSVARS
        #define WEBSERVER_SYSVARS
    #endif
    #ifndef WEBSERVER_NEW_UI
    //    #define WEBSERVER_NEW_UI
    #endif
    #ifndef WEBSERVER_I2C_SCANNER
        #define WEBSERVER_I2C_SCANNER
    #endif
    #ifndef WEBSERVER_FAVICON
        #define WEBSERVER_FAVICON
    #endif
    #ifndef WEBSERVER_CSS
        #define WEBSERVER_CSS
    #endif
    #ifndef WEBSERVER_INCLUDE_JS
        #define WEBSERVER_INCLUDE_JS
    #endif
    #ifndef WEBSERVER_LOG
        #define WEBSERVER_LOG
    #endif
    #ifndef WEBSERVER_GITHUB_COPY
        #define WEBSERVER_GITHUB_COPY
    #endif
    #ifndef WEBSERVER_ROOT
        #define WEBSERVER_ROOT
    #endif
    #ifndef WEBSERVER_ADVANCED
        #define WEBSERVER_ADVANCED
    #endif
    #ifndef WEBSERVER_CONFIG
        #define WEBSERVER_CONFIG
    #endif
    #ifndef WEBSERVER_CONTROL
        #define WEBSERVER_CONTROL
    #endif
    #ifndef WEBSERVER_CONTROLLERS
        #define WEBSERVER_CONTROLLERS
    #endif
    #ifndef WEBSERVER_CUSTOM
        #define WEBSERVER_CUSTOM
    #endif
    #ifndef WEBSERVER_DEVICES
        #define WEBSERVER_DEVICES
    #endif
    #ifndef WEBSERVER_DOWNLOAD
        #define WEBSERVER_DOWNLOAD
    #endif
    #ifndef WEBSERVER_FACTORY_RESET
        #define WEBSERVER_FACTORY_RESET
    #endif
    #ifndef WEBSERVER_FILELIST
        #define WEBSERVER_FILELIST
    #endif
    #ifndef WEBSERVER_HARDWARE
        #define WEBSERVER_HARDWARE
    #endif
    #ifndef WEBSERVER_PINSTATES
        #define WEBSERVER_PINSTATES
    #endif
    #ifndef WEBSERVER_RULES
        #define WEBSERVER_RULES
    #endif
    #ifndef WEBSERVER_SETUP
        #define WEBSERVER_SETUP
    #endif
    #ifndef WEBSERVER_SYSINFO
        #define WEBSERVER_SYSINFO
    #endif
    #ifndef WEBSERVER_METRICS
        #define WEBSERVER_METRICS
    #endif
    #ifndef WEBSERVER_TOOLS
        #define WEBSERVER_TOOLS
    #endif
    #ifndef WEBSERVER_UPLOAD
        #define WEBSERVER_UPLOAD
    #endif
    #ifndef WEBSERVER_WIFI_SCANNER
        #define WEBSERVER_WIFI_SCANNER
    #endif
    #ifndef WEBSERVER_NEW_RULES
//        #define WEBSERVER_NEW_RULES
    #endif
#endif

#ifdef WEBSERVER_CSS
  #ifndef WEBSERVER_EMBED_CUSTOM_CSS
    #ifndef EMBED_ESPEASY_DEFAULT_MIN_CSS
      #define EMBED_ESPEASY_DEFAULT_MIN_CSS
    #endif
    #ifndef EMBED_ESPEASY_DEFAULT_MIN_CSS_USE_GZ // Use gzipped minified css (saves ~3.7 kB of .bin size)
      #define EMBED_ESPEASY_DEFAULT_MIN_CSS_USE_GZ
    #endif
  #endif
#endif


#ifndef PLUGIN_BUILD_CUSTOM
    #ifndef FEATURE_SSDP
        #define FEATURE_SSDP  1
    #endif
    #ifndef FEATURE_TIMING_STATS
        #define FEATURE_TIMING_STATS  1
    #endif
    #ifndef FEATURE_I2CMULTIPLEXER
        #define FEATURE_I2CMULTIPLEXER  1
    #endif
    #ifndef FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES
        #define FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES 1
    #endif
    #ifndef FEATURE_EXT_RTC
        #define FEATURE_EXT_RTC 1
    #endif
#endif

#ifdef MEMORY_ANALYSIS
  #ifdef MQTT_ONLY
    #define USES_C002   // Domoticz MQTT
    #define USES_C005   // Home Assistant (openHAB) MQTT
    #define USES_C006   // PiDome MQTT
    #define USES_C014   // homie 3 & 4dev MQTT
    #define USES_P037   // MQTTImport
  #endif
#endif

#ifndef FEATURE_TOOLTIPS
  #define FEATURE_TOOLTIPS  1
#endif // ifndef FEATURE_TOOLTIPS

/******************************************************************************\
 * Available options **********************************************************
\******************************************************************************/
#if defined(CORE_POST_2_5_0) && !defined(MEMORY_ANALYSIS) && !defined(USE_CUSTOM_H)
    #ifndef FEATURE_SETTINGS_ARCHIVE
    // FIXME TD-er: Disabled for now, to reduce binary size
//        #define FEATURE_SETTINGS_ARCHIVE 1
    #endif // ifndef FEATURE_SETTINGS_ARCHIVE
#endif

#if defined(FEATURE_SETTINGS_ARCHIVE) && defined(FORCE_PRE_2_5_0)
  #undef FEATURE_SETTINGS_ARCHIVE
#endif

#ifndef FEATURE_NO_HTTP_CLIENT
  #define FEATURE_NO_HTTP_CLIENT  0
#endif


/******************************************************************************\
 * BUILD Configs **************************************************************
\******************************************************************************/

// IR library is large, so make a separate build including stable plugins and IR.
#ifdef PLUGIN_BUILD_DEV_IR
    #define PLUGIN_BUILD_DEV       // add dev
    #define PLUGIN_BUILD_IR
#endif

#ifdef PLUGIN_BUILD_COLLECTION_IR
    #define PLUGIN_BUILD_COLLECTION   // add collection
    #define PLUGIN_BUILD_IR
#endif

#ifdef PLUGIN_BUILD_MINIMAL_IR
    #ifndef FEATURE_DOMOTICZ
        #define FEATURE_DOMOTICZ  1
    #endif
    #ifndef FEATURE_FHEM
        #define FEATURE_FHEM  1
    #endif
    #ifndef FEATURE_HOMEASSISTANT_OPENHAB
        #define FEATURE_HOMEASSISTANT_OPENHAB 1
    #endif

    #define PLUGIN_BUILD_MINIMAL_OTA
    #define PLUGIN_DESCR  "Minimal, IR"
    #define PLUGIN_BUILD_IR
#endif

#ifdef PLUGIN_BUILD_MINIMAL_IRext
    #ifndef FEATURE_DOMOTICZ
        #define FEATURE_DOMOTICZ  1
    #endif
    #ifndef FEATURE_FHEM
        #define FEATURE_FHEM  1
    #endif
    #ifndef FEATURE_HOMEASSISTANT_OPENHAB
        #define FEATURE_HOMEASSISTANT_OPENHAB 1
    #endif

    #define PLUGIN_BUILD_MINIMAL_OTA
    #define PLUGIN_DESCR  "Minimal, IR with AC"
    #define PLUGIN_BUILD_IR_EXTENDED
#endif

#ifdef PLUGIN_BUILD_NORMAL_IR
    #define PLUGIN_BUILD_NORMAL     // add stable
    #define PLUGIN_DESCR  "Normal, IR"
    #define PLUGIN_BUILD_IR
#endif

#ifdef PLUGIN_BUILD_NORMAL_IRext
  #define PLUGIN_BUILD_NORMAL     // add stable
  #if defined(PLUGIN_SET_COLLECTION_ESP32)
    #define PLUGIN_DESCR  "Collection_A, IR with AC"
  #elif defined(PLUGIN_SET_COLLECTION_B_ESP32)
    #define PLUGIN_DESCR  "Collection_B, IR with AC"
  #elif defined(PLUGIN_SET_COLLECTION_C_ESP32)
    #define PLUGIN_DESCR  "Collection_C, IR with AC"
  #elif defined(PLUGIN_SET_COLLECTION_D_ESP32)
    #define PLUGIN_DESCR  "Collection_D, IR with AC"
  #elif defined(PLUGIN_SET_COLLECTION_E_ESP32)
    #define PLUGIN_DESCR  "Collection_E, IR with AC"
  #elif defined(PLUGIN_SET_COLLECTION_F_ESP32)
    #define PLUGIN_DESCR  "Collection_F, IR with AC"
  #else
    #define PLUGIN_DESCR  "Normal, IR with AC"
  #endif
  #define PLUGIN_BUILD_IR_EXTENDED
#endif

#ifdef PLUGIN_BUILD_DEV
  #define  PLUGIN_SET_EXPERIMENTAL
  #define  CONTROLLER_SET_EXPERIMENTAL
  #define  NOTIFIER_SET_EXPERIMENTAL
  #define  PLUGIN_BUILD_COLLECTION   // add collection
#endif

#ifdef PLUGIN_BUILD_COLLECTION
  #if !defined(PLUGIN_BUILD_COLLECTION_B) && !defined(PLUGIN_BUILD_COLLECTION_C) && !defined(PLUGIN_BUILD_COLLECTION_D) && !defined(PLUGIN_BUILD_COLLECTION_E) && !defined(PLUGIN_BUILD_COLLECTION_F)
    #define PLUGIN_DESCR  "Collection_A"
    #define PLUGIN_SET_COLLECTION_A
  #endif
  #define PLUGIN_SET_COLLECTION
  #define CONTROLLER_SET_COLLECTION
  #define NOTIFIER_SET_COLLECTION
  #define PLUGIN_BUILD_NORMAL     // add stable
#endif

#ifdef PLUGIN_BUILD_COLLECTION_B
  #define PLUGIN_DESCR  "Collection_B"
  #define PLUGIN_SET_COLLECTION
  #define PLUGIN_SET_COLLECTION_B
  #define CONTROLLER_SET_COLLECTION
  #define NOTIFIER_SET_COLLECTION
  #define PLUGIN_BUILD_NORMAL     // add stable
#endif

#ifdef PLUGIN_BUILD_COLLECTION_C
  #define PLUGIN_DESCR  "Collection_C"
  #define PLUGIN_SET_COLLECTION
  #define PLUGIN_SET_COLLECTION_C
  #define CONTROLLER_SET_COLLECTION
  #define NOTIFIER_SET_COLLECTION
  #define PLUGIN_BUILD_NORMAL     // add stable
#endif

#ifdef PLUGIN_BUILD_COLLECTION_D
  #define PLUGIN_DESCR  "Collection_D"
  #define PLUGIN_SET_COLLECTION
  #define PLUGIN_SET_COLLECTION_D
  #define CONTROLLER_SET_COLLECTION
  #define NOTIFIER_SET_COLLECTION
  #define PLUGIN_BUILD_NORMAL     // add stable
#endif

#ifdef PLUGIN_BUILD_COLLECTION_E
  #define PLUGIN_DESCR  "Collection_E"
  #define PLUGIN_SET_COLLECTION
  #define PLUGIN_SET_COLLECTION_E
  #define CONTROLLER_SET_COLLECTION
  #define NOTIFIER_SET_COLLECTION
  #define PLUGIN_BUILD_NORMAL     // add stable
#endif

#ifdef PLUGIN_BUILD_COLLECTION_F
  #define PLUGIN_DESCR  "Collection_F"
  #define PLUGIN_SET_COLLECTION
  #define PLUGIN_SET_COLLECTION_F
  #define CONTROLLER_SET_COLLECTION
  #define NOTIFIER_SET_COLLECTION
  #define PLUGIN_BUILD_NORMAL     // add stable
#endif

#ifndef PLUGIN_BUILD_CUSTOM
  #ifndef PLUGIN_BUILD_NORMAL
    #define PLUGIN_BUILD_NORMAL // defaults to stable, if not custom
  #endif
#endif

#ifdef PLUGIN_CLIMATE_COLLECTION
  #ifdef PLUGIN_BUILD_NORMAL
    #undef PLUGIN_BUILD_NORMAL
  #endif
  #define PLUGIN_SET_NONE // Specifically configured below
  #define CONTROLLER_SET_STABLE
  #define NOTIFIER_SET_STABLE
  #ifndef FEATURE_ESPEASY_P2P
    #define FEATURE_ESPEASY_P2P 1
  #endif

  #ifndef FEATURE_I2CMULTIPLEXER
    #define FEATURE_I2CMULTIPLEXER  1
  #endif
  #ifndef FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES
    #define FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES 1
  #endif
  #define KEEP_TRIGONOMETRIC_FUNCTIONS_RULES
  #ifndef FEATURE_PLUGIN_STATS
    #define FEATURE_PLUGIN_STATS  1
  #endif
  #ifndef FEATURE_CHART_JS
    #define FEATURE_CHART_JS  1
  #endif
  #ifndef FEATURE_RULES_EASY_COLOR_CODE
    #define FEATURE_RULES_EASY_COLOR_CODE 1
  #endif
#endif

#ifdef PLUGIN_BUILD_NORMAL
    #define  PLUGIN_SET_STABLE
    #define  CONTROLLER_SET_STABLE
    #define  NOTIFIER_SET_STABLE
    #ifndef FEATURE_ESPEASY_P2P
      #define FEATURE_ESPEASY_P2P 1
    #endif

    #ifndef FEATURE_I2CMULTIPLEXER
        #define FEATURE_I2CMULTIPLEXER  1
    #endif
    #ifndef FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES
        #define FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES 1
    #endif
    #define KEEP_TRIGONOMETRIC_FUNCTIONS_RULES
    #ifndef FEATURE_PLUGIN_STATS
        #define FEATURE_PLUGIN_STATS  1
    #endif
    #ifndef FEATURE_CHART_JS
        #define FEATURE_CHART_JS  1
    #endif
    #ifndef FEATURE_RULES_EASY_COLOR_CODE
        #define FEATURE_RULES_EASY_COLOR_CODE 1
    #endif
#endif

#if FEATURE_FHEM
    #define USES_C009   // FHEM HTTP
#endif

#if FEATURE_HOMEASSISTANT_OPENHAB
    #define USES_C005   // Home Assistant (openHAB) MQTT
#endif

#ifdef PLUGIN_BUILD_MINIMAL_OTA
    // Disable ESPEasy p2p for minimal OTA builds.
    #ifdef FEATURE_ESPEASY_P2P
      #undef FEATURE_ESPEASY_P2P
    #endif
    #define FEATURE_ESPEASY_P2P 0

    #ifdef FEATURE_MDNS
      #undef FEATURE_MDNS
    #endif
    #define FEATURE_MDNS 0

    #ifndef DISABLE_SC16IS752_Serial
      #define DISABLE_SC16IS752_Serial
    #endif

    #ifdef FEATURE_ARDUINO_OTA
      #undef FEATURE_ARDUINO_OTA
    #endif
    #define FEATURE_ARDUINO_OTA 0

    #ifndef PLUGIN_DESCR
      #define PLUGIN_DESCR  "Minimal 1M OTA"
    #endif


    #ifndef CONTROLLER_SET_NONE
      #define CONTROLLER_SET_NONE
    #endif

    #define BUILD_MINIMAL_OTA
    #ifndef BUILD_NO_DEBUG
      #define BUILD_NO_DEBUG
    #endif

//    #define USES_C001   // Domoticz HTTP
//    #define USES_C002   // Domoticz MQTT
//    #define USES_C005   // Home Assistant (openHAB) MQTT
//    #define USES_C006   // PiDome MQTT
  #if !FEATURE_NO_HTTP_CLIENT
    #define USES_C008   // Generic HTTP
  #endif
//    #define USES_C009   // FHEM HTTP
//    #define USES_C010   // Generic UDP
//    #define USES_C013   // ESPEasy P2P network

//    #define NOTIFIER_SET_STABLE
    #ifndef NOTIFIER_SET_NONE
      #define NOTIFIER_SET_NONE
    #endif

    #ifdef FEATURE_POST_TO_HTTP
      #undef FEATURE_POST_TO_HTTP
    #endif
    #define FEATURE_POST_TO_HTTP  0 // Disabled

    #ifndef PLUGIN_SET_NONE
      #define PLUGIN_SET_NONE
    #endif

    #ifdef FEATURE_SETTINGS_ARCHIVE
        #undef FEATURE_SETTINGS_ARCHIVE
    #endif // if FEATURE_SETTINGS_ARCHIVE
    #define FEATURE_SETTINGS_ARCHIVE  0

    #ifdef FEATURE_TIMING_STATS
        #undef FEATURE_TIMING_STATS
    #endif
    #define FEATURE_TIMING_STATS  0

    #ifdef FEATURE_ZEROFILLED_UNITNUMBER
        #undef FEATURE_ZEROFILLED_UNITNUMBER
    #endif
    #define FEATURE_ZEROFILLED_UNITNUMBER  0

    #if defined(FEATURE_I2C_DEVICE_CHECK)
      #undef FEATURE_I2C_DEVICE_CHECK
    #endif
    #define FEATURE_I2C_DEVICE_CHECK 0 // Disable I2C device check code

    #if defined(FEATURE_I2C_GET_ADDRESS)
      #undef FEATURE_I2C_GET_ADDRESS
    #endif
    #define FEATURE_I2C_GET_ADDRESS 0 // Disable fetching I2C device address

    #ifndef USES_P001
        #define USES_P001   // switch
    #endif
    #ifndef USES_P026
      #define USES_P026   // SysInfo
    #endif
    #ifndef USES_P033
      #define USES_P033   // Dummy
    #endif
    #ifndef USES_P037
//        #define USES_P037   // MQTTImport
    #endif

    #ifndef USES_P004
//        #define USES_P004   // Dallas
    #endif
    #ifndef USES_P005
//        #define USES_P005   // DHT
    #endif

    #ifdef FEATURE_SERVO
      #undef FEATURE_SERVO
    #endif
    #define FEATURE_SERVO 0
    #ifdef FEATURE_RTTTL
      #undef FEATURE_RTTTL
    #endif
    #define FEATURE_RTTTL 0
#endif


// Strip out parts not needed for either MINIMAL_OTA and MEMORY_ANALYSIS
#if defined(BUILD_MINIMAL_OTA) || defined(MEMORY_ANALYSIS)
    #ifndef WEBSERVER_CUSTOM_BUILD_DEFINED
        #ifdef WEBSERVER_TIMINGSTATS
            #undef WEBSERVER_TIMINGSTATS
        #endif
        #ifdef WEBSERVER_SYSVARS
            #undef WEBSERVER_SYSVARS
        #endif
        #ifdef WEBSERVER_NEW_UI
            #undef WEBSERVER_NEW_UI
        #endif
        #ifdef WEBSERVER_I2C_SCANNER
            #undef WEBSERVER_I2C_SCANNER
        #endif
        #ifdef WEBSERVER_FAVICON
            #undef WEBSERVER_FAVICON
        #endif
        #ifdef WEBSERVER_CSS
            #undef WEBSERVER_CSS
        #endif
        #ifndef WEBSERVER_EMBED_CUSTOM_CSS
          #ifdef EMBED_ESPEASY_DEFAULT_MIN_CSS
            #undef EMBED_ESPEASY_DEFAULT_MIN_CSS
          #endif
        #endif
        #ifdef WEBSERVER_INCLUDE_JS
            #undef WEBSERVER_INCLUDE_JS
        #endif
        #ifdef WEBSERVER_LOG
            #undef WEBSERVER_LOG
        #endif
        #ifdef WEBSERVER_GITHUB_COPY
            #undef WEBSERVER_GITHUB_COPY
        #endif
        #ifdef WEBSERVER_PINSTATES
            #undef WEBSERVER_PINSTATES
        #endif
        #ifdef WEBSERVER_WIFI_SCANNER
            #undef WEBSERVER_WIFI_SCANNER
        #endif
        #ifdef WEBSERVER_CUSTOM
            #undef WEBSERVER_CUSTOM
        #endif
        #ifdef WEBSERVER_NEW_RULES
            #undef WEBSERVER_NEW_RULES
        #endif
        #ifdef SHOW_SYSINFO_JSON
            #undef SHOW_SYSINFO_JSON
        #endif
        #ifndef WEBSERVER_SYSINFO_MINIMAL
            #define WEBSERVER_SYSINFO_MINIMAL
        #endif

    #endif // WEBSERVER_CUSTOM_BUILD_DEFINED


    // FEATURE_GPIO_USE_ESP8266_WAVEFORM needs about 200 bytes
    //#define FEATURE_GPIO_USE_ESP8266_WAVEFORM 0


    #ifndef LIMIT_BUILD_SIZE
        #define LIMIT_BUILD_SIZE
    #endif
    #ifdef FEATURE_I2C_DEVICE_SCAN
        #undef FEATURE_I2C_DEVICE_SCAN
    #endif // if FEATURE_I2C_DEVICE_SCAN
    #define FEATURE_I2C_DEVICE_SCAN     0   // turn feature off in OTA builds
    #ifdef KEEP_TRIGONOMETRIC_FUNCTIONS_RULES
        #undef KEEP_TRIGONOMETRIC_FUNCTIONS_RULES
    #endif
    #ifndef NOTIFIER_SET_NONE
        #define NOTIFIER_SET_NONE
    #endif
    #ifdef FEATURE_EXT_RTC
        #undef FEATURE_EXT_RTC
    #endif
    #define FEATURE_EXT_RTC 0
#endif



#ifdef BUILD_NO_DEBUG
    #ifdef WEBSERVER_RULES_DEBUG
        #undef WEBSERVER_RULES_DEBUG
    #endif
#endif


/******************************************************************************\
 * IR plugins *****************************************************************
\******************************************************************************/
// See lib\IRremoteESP8266\src\IRremoteESP8266.h
// Disable all settings like these when not needed:
// #define DECODE_TOSHIBA_AC      true
// #define SEND_TOSHIBA_AC        true
#ifdef PLUGIN_BUILD_IR
    #if !defined(PLUGIN_DESCR) && !defined(PLUGIN_BUILD_MAX_ESP32)
      #define PLUGIN_DESCR  "IR"
    #endif
    #ifndef USES_P016    
      #define USES_P016      // IR
    #endif
    #define P016_SEND_IR_TO_CONTROLLER false //IF true then the JSON replay solution is transmited back to the condroller.
    #ifndef USES_P035    
      #define USES_P035      // IRTX
    #endif
    #define P016_P035_USE_RAW_RAW2 //Use the RAW and RAW2 encodings, disabling it saves 3.7Kb
#endif

#ifdef PLUGIN_BUILD_IR_EXTENDED
    #if !defined(PLUGIN_DESCR) && !defined(PLUGIN_BUILD_MAX_ESP32)
        #define PLUGIN_DESCR  "IR Extended"
    #endif // PLUGIN_DESCR
    #ifndef USES_P016    
      #define USES_P016      // IR
    #endif
    #define P016_SEND_IR_TO_CONTROLLER false //IF true then the JSON replay solution is transmited back to the condroller.
    #ifndef USES_P035    
      #define USES_P035      // IRTX
    #endif
    // The following define is needed for extended decoding of A/C Messages and or using standardised common arguments for controlling all deeply supported A/C units
    #define P016_P035_Extended_AC
    #define P016_P035_USE_RAW_RAW2 //Use the RAW and RAW2 encodings, disabling it saves 3.7Kb
    #ifndef ESP8266_1M       // Leaving out Heatpump IR for 1M builds because it won't fit after upgrading IRremoteESP8266 library to v2.8.1
      #define USES_P088      // ToniA IR plugin
    #endif
    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
    #define USES_P029      // Output - Domoticz MQTT Helper
    #define PLUGIN_SET_ONLY_TEMP_HUM
#endif

#ifdef PLUGIN_BUILD_IR_EXTENDED_NO_RX
    #if !defined(PLUGIN_DESCR) && !defined(PLUGIN_BUILD_MAX_ESP32)
        #define PLUGIN_DESCR  "IR Extended, no IR RX"
    #endif // PLUGIN_DESCR
    #ifndef USES_P035    
      #define USES_P035      // IRTX
    #endif
    // The following define is needed for extended decoding of A/C Messages and or using standardised common arguments for controlling all deeply supported A/C units
    #define P016_P035_Extended_AC
    #define P016_P035_USE_RAW_RAW2 //Use the RAW and RAW2 encodings, disabling it saves 3.7Kb
    #define USES_P088      //ToniA IR plugin
#endif

/******************************************************************************\
 * Devices ********************************************************************
\******************************************************************************/

// Itead ----------------------------
#ifdef PLUGIN_SET_SONOFF_BASIC
    #define PLUGIN_DESCR  "Sonoff Basic"

    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
#endif

#ifdef PLUGIN_SET_SONOFF_TH1x
    #define PLUGIN_DESCR  "Sonoff TH10/TH16"

    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
    #define PLUGIN_SET_ONLY_TEMP_HUM
#endif

#ifdef PLUGIN_SET_SONOFF_POW
    #ifndef PLUGIN_DESCR
        #define PLUGIN_DESCR  "Sonoff POW R1/R2"
    #endif

    #define CONTROLLER_SET_STABLE
    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
    #define USES_P076   // HWL8012   in POW r1
    // Needs CSE7766 Energy sensor, via Serial RXD 4800 baud 8E1 (GPIO1), TXD (GPIO3)
    #define USES_P077	  // CSE7766   in POW R2
    #define USES_P081   // Cron
    #ifdef ESP8266_4M
      #define FEATURE_ADC_VCC 1
      #define USES_P002   // ADC with FEATURE_ADC_VCC=1 to measure ESP3v3
      #define CONTROLLER_SET_ALL
      #ifndef FEATURE_PLUGIN_STATS
          #define FEATURE_PLUGIN_STATS  1
      #endif
      #ifndef FEATURE_CHART_JS
          #define FEATURE_CHART_JS  1
      #endif
      #ifndef FEATURE_RULES_EASY_COLOR_CODE
          #define FEATURE_RULES_EASY_COLOR_CODE 1
      #endif
      #ifndef FEATURE_SETTINGS_ARCHIVE
        #define FEATURE_SETTINGS_ARCHIVE  1
      #endif
      #ifndef SHOW_SYSINFO_JSON
        #define SHOW_SYSINFO_JSON 1
      #endif
      #ifndef FEATURE_TIMING_STATS                  
        #define FEATURE_TIMING_STATS 1
      #endif
      #ifndef FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES 
        #define FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES 1
      #endif
      #ifdef BUILD_NO_DEBUG
        #undef BUILD_NO_DEBUG
      #endif
      
      #define FEATURE_MDNS  1
      #define FEATURE_CUSTOM_PROVISIONING 1
      #define FEATURE_DOWNLOAD 1
    #endif
#endif

#ifdef PLUGIN_SET_SONOFF_S2x
    #define PLUGIN_DESCR  "Sonoff S20/22/26"

    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
#endif

#ifdef PLUGIN_SET_SONOFF_4CH
    #define PLUGIN_DESCR  "Sonoff 4CH"
    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
#endif

#ifdef PLUGIN_SET_SONOFF_TOUCH
    #define PLUGIN_DESCR  "Sonoff Touch"
    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
#endif

// Shelly ----------------------------
#ifdef PLUGIN_SET_SHELLY_1
    #define PLUGIN_DESCR  "Shelly 1"

    #define PLUGIN_SET_ONLY_SWITCH
    #define CONTROLLER_SET_STABLE
    #define NOTIFIER_SET_STABLE
    #define USES_P004   // DS18B20
#endif

#ifdef PLUGIN_SET_SHELLY_PLUG_S
    #define PLUGIN_DESCR  "Shelly PLUG-S"

    #define PLUGIN_SET_ONLY_SWITCH
    #define CONTROLLER_SET_STABLE
    #define NOTIFIER_SET_STABLE
    #define USES_P076   // HWL8012   in POW r1
    #define USES_P077	  // CSE7766   in POW R2
    #define USES_P081   // Cron
#endif

// Easy ----------------------------
#ifdef PLUGIN_SET_EASY_TEMP
    #define PLUGIN_DESCR  "Temp Hum"
    #define PLUGIN_SET_ONLY_TEMP_HUM
#endif

#ifdef PLUGIN_SET_EASY_CARBON
    #define PLUGIN_DESCR  "Carbon"
    #define PLUGIN_SET_NONE
    #define USES_P052   // SenseAir
#endif

/*
#ifdef PLUGIN_SET_EASY_NEXTION
    #define PLUGIN_SET_ONLY_SWITCH
    //#define USES_Pxxx   // Nextion
#endif
*/

#ifdef PLUGIN_SET_EASY_OLED1
    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
    #define USES_P036   // FrameOLED
#endif

#ifdef PLUGIN_SET_EASY_OLED2
    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
    #define USES_P023   // OLED
#endif

#ifdef PLUGIN_SET_EASY_RELAY
    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
#endif

// LedStrips ----------------------------
#ifdef PLUGIN_SET_H801
    #define PLUGIN_SET_ONLY_LEDSTRIP
#endif

#ifdef PLUGIN_SET_MAGICHOME
    #define PLUGIN_SET_ONLY_LEDSTRIP
#endif

#ifdef PLUGIN_SET_MAGICHOME_IR
    #define PLUGIN_SET_ONLY_LEDSTRIP
    #ifndef USES_P016    
      #define USES_P016      // IR
    #endif

#endif


// Generic ESP32 -----------------------------
#ifdef PLUGIN_SET_GENERIC_ESP32
    #define PLUGIN_DESCR  "Generic ESP32"

    #ifndef ESP32
        #define ESP32
    #endif
    #ifdef ESP8266
        #undef ESP8266
    #endif
    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
    #define USES_P036   // FrameOLED
    #define USES_P027   // INA219
    #define USES_P028   // BME280
#endif

#ifdef PLUGIN_SET_COLLECTION_ESP32
  #if !defined(PLUGIN_SET_COLLECTION_B_ESP32) && !defined(PLUGIN_SET_COLLECTION_C_ESP32) && !defined(PLUGIN_SET_COLLECTION_D_ESP32) && !defined(PLUGIN_SET_COLLECTION_E_ESP32) && !defined(PLUGIN_SET_COLLECTION_F_ESP32)
    #ifndef PLUGIN_DESCR // COLLECTION_A_ESP32_IRExt also passes here
      #define PLUGIN_DESCR  "Collection_A ESP32"
    #endif
    #define  PLUGIN_SET_COLLECTION_A
  #endif
  #ifndef ESP32
    #define ESP32
  #endif
  #ifdef ESP8266
    #undef ESP8266
  #endif
  // Undefine contradictionary defines
  #ifdef PLUGIN_SET_NONE
    #undef PLUGIN_SET_NONE
  #endif
  #ifdef PLUGIN_SET_ONLY_SWITCH
    #undef PLUGIN_SET_ONLY_SWITCH
  #endif
  #ifdef PLUGIN_SET_ONLY_TEMP_HUM
    #undef PLUGIN_SET_ONLY_TEMP_HUM
  #endif
  #define  PLUGIN_SET_COLLECTION
  #define  CONTROLLER_SET_STABLE
  #define  CONTROLLER_SET_COLLECTION
  #define  NOTIFIER_SET_STABLE
  #define  PLUGIN_SET_STABLE     // add stable
  // See also PLUGIN_SET_COLLECTION_ESP32 section at end,
  // where incompatible plugins will be disabled.
  // TODO : Check compatibility of plugins for ESP32 board.
#endif

#ifdef PLUGIN_SET_COLLECTION_B_ESP32
  #ifndef PLUGIN_DESCR // COLLECTION_B_ESP32_IRExt also passes here
    #define PLUGIN_DESCR  "Collection_B ESP32"
  #endif
  #ifndef ESP32
    #define ESP32
  #endif
  #ifdef ESP8266
    #undef ESP8266
  #endif
  // Undefine contradictionary defines
  #ifdef PLUGIN_SET_NONE
    #undef PLUGIN_SET_NONE
  #endif
  #ifdef PLUGIN_SET_ONLY_SWITCH
    #undef PLUGIN_SET_ONLY_SWITCH
  #endif
  #ifdef PLUGIN_SET_ONLY_TEMP_HUM
    #undef PLUGIN_SET_ONLY_TEMP_HUM
  #endif
  #define  PLUGIN_SET_COLLECTION
  #define  PLUGIN_SET_COLLECTION_B
  #define  CONTROLLER_SET_STABLE
  #define  CONTROLLER_SET_COLLECTION
  #define  NOTIFIER_SET_STABLE
  #define  PLUGIN_SET_STABLE     // add stable
  // See also PLUGIN_SET_COLLECTION_ESP32 section at end,
  // where incompatible plugins will be disabled.
  // TODO : Check compatibility of plugins for ESP32 board.
#endif

#ifdef PLUGIN_SET_COLLECTION_C_ESP32
  #ifndef PLUGIN_DESCR // COLLECTION_C_ESP32_IRExt also passes here
    #define PLUGIN_DESCR  "Collection_C ESP32"
  #endif
  #ifndef ESP32
    #define ESP32
  #endif
  #ifdef ESP8266
    #undef ESP8266
  #endif
  // Undefine contradictionary defines
  #ifdef PLUGIN_SET_NONE
    #undef PLUGIN_SET_NONE
  #endif
  #ifdef PLUGIN_SET_ONLY_SWITCH
    #undef PLUGIN_SET_ONLY_SWITCH
  #endif
  #ifdef PLUGIN_SET_ONLY_TEMP_HUM
    #undef PLUGIN_SET_ONLY_TEMP_HUM
  #endif
  #define  PLUGIN_SET_COLLECTION
  #define  PLUGIN_SET_COLLECTION_C
  #define  CONTROLLER_SET_STABLE
  #define  CONTROLLER_SET_COLLECTION
  #define  NOTIFIER_SET_STABLE
  #define  PLUGIN_SET_STABLE     // add stable
  // See also PLUGIN_SET_COLLECTION_ESP32 section at end,
  // where incompatible plugins will be disabled.
  // TODO : Check compatibility of plugins for ESP32 board.
#endif

#ifdef PLUGIN_SET_COLLECTION_D_ESP32
  #ifndef PLUGIN_DESCR // COLLECTION_D_ESP32_IRExt also passes here
    #define PLUGIN_DESCR  "Collection_D ESP32"
  #endif
  #ifndef ESP32
    #define ESP32
  #endif
  #ifdef ESP8266
    #undef ESP8266
  #endif
  // Undefine contradictionary defines
  #ifdef PLUGIN_SET_NONE
    #undef PLUGIN_SET_NONE
  #endif
  #ifdef PLUGIN_SET_ONLY_SWITCH
    #undef PLUGIN_SET_ONLY_SWITCH
  #endif
  #ifdef PLUGIN_SET_ONLY_TEMP_HUM
    #undef PLUGIN_SET_ONLY_TEMP_HUM
  #endif
  #define  PLUGIN_SET_COLLECTION
  #define  PLUGIN_SET_COLLECTION_D
  #define  CONTROLLER_SET_STABLE
  #define  CONTROLLER_SET_COLLECTION
  #define  NOTIFIER_SET_STABLE
  #define  PLUGIN_SET_STABLE     // add stable
  // See also PLUGIN_SET_COLLECTION_ESP32 section at end,
  // where incompatible plugins will be disabled.
  // TODO : Check compatibility of plugins for ESP32 board.
#endif

#ifdef PLUGIN_SET_COLLECTION_E_ESP32
  #ifndef PLUGIN_DESCR // COLLECTION_E_ESP32_IRExt also passes here
    #define PLUGIN_DESCR  "Collection_E ESP32"
  #endif
  #ifndef ESP32
    #define ESP32
  #endif
  #ifdef ESP8266
    #undef ESP8266
  #endif
  // Undefine contradictionary defines
  #ifdef PLUGIN_SET_NONE
    #undef PLUGIN_SET_NONE
  #endif
  #ifdef PLUGIN_SET_ONLY_SWITCH
    #undef PLUGIN_SET_ONLY_SWITCH
  #endif
  #ifdef PLUGIN_SET_ONLY_TEMP_HUM
    #undef PLUGIN_SET_ONLY_TEMP_HUM
  #endif
  #define  PLUGIN_SET_COLLECTION
  #define  PLUGIN_SET_COLLECTION_E
  #define  CONTROLLER_SET_STABLE
  #define  CONTROLLER_SET_COLLECTION
  #define  NOTIFIER_SET_STABLE
  #define  PLUGIN_SET_STABLE     // add stable
  // See also PLUGIN_SET_COLLECTION_ESP32 section at end,
  // where incompatible plugins will be disabled.
  // TODO : Check compatibility of plugins for ESP32 board.
#endif

#ifdef PLUGIN_SET_COLLECTION_F_ESP32
  #ifndef PLUGIN_DESCR // COLLECTION_F_ESP32_IRExt also passes here
    #define PLUGIN_DESCR  "Collection_F ESP32"
  #endif
  #ifndef ESP32
    #define ESP32
  #endif
  #ifdef ESP8266
    #undef ESP8266
  #endif
  // Undefine contradictionary defines
  #ifdef PLUGIN_SET_NONE
    #undef PLUGIN_SET_NONE
  #endif
  #ifdef PLUGIN_SET_ONLY_SWITCH
    #undef PLUGIN_SET_ONLY_SWITCH
  #endif
  #ifdef PLUGIN_SET_ONLY_TEMP_HUM
    #undef PLUGIN_SET_ONLY_TEMP_HUM
  #endif
  #define  PLUGIN_SET_COLLECTION
  #define  PLUGIN_SET_COLLECTION_F
  #define  CONTROLLER_SET_STABLE
  #define  CONTROLLER_SET_COLLECTION
  #define  NOTIFIER_SET_STABLE
  #define  PLUGIN_SET_STABLE     // add stable
  // See also PLUGIN_SET_COLLECTION_ESP32 section at end,
  // where incompatible plugins will be disabled.
  // TODO : Check compatibility of plugins for ESP32 board.
#endif

#ifdef PLUGIN_BUILD_MAX_ESP32
    #ifndef PLUGIN_DESCR
      #define PLUGIN_DESCR  "MAX ESP32"
    #endif
    #ifndef ESP32
        #define ESP32
    #endif
    #ifdef ESP8266
        #undef ESP8266
    #endif

    #define PLUGIN_SET_MAX
    #define CONTROLLER_SET_ALL
    #define NOTIFIER_SET_ALL
    #ifndef PLUGIN_ENERGY_COLLECTION
        #define PLUGIN_ENERGY_COLLECTION
    #endif
    #ifndef PLUGIN_DISPLAY_COLLECTION
        #define PLUGIN_DISPLAY_COLLECTION
    #endif
    #ifndef PLUGIN_CLIMATE_COLLECTION
      #define PLUGIN_CLIMATE_COLLECTION
    #endif
    #ifndef PLUGIN_NEOPIXEL_COLLECTION
        #define PLUGIN_NEOPIXEL_COLLECTION
    #endif
    #ifndef FEATURE_PLUGIN_STATS
        #define FEATURE_PLUGIN_STATS  1
    #endif
    #ifndef FEATURE_CHART_JS
        #define FEATURE_CHART_JS  1
    #endif
    #ifndef FEATURE_RULES_EASY_COLOR_CODE
        #define FEATURE_RULES_EASY_COLOR_CODE 1
    #endif

    #ifdef FEATURE_CUSTOM_PROVISIONING
        #undef FEATURE_CUSTOM_PROVISIONING
    #endif
    #define FEATURE_CUSTOM_PROVISIONING 1


    // See also PLUGIN_SET_MAX section at end, to include any disabled plugins from other definitions
    // See also PLUGIN_SET_COLLECTION_ESP32 section at end,
    // where incompatible plugins will be disabled.
    // TODO : Check compatibility of plugins for ESP32 board.
#endif


// Generic ------------------------------------
#ifdef PLUGIN_SET_GENERIC_1M
    #define PLUGIN_SET_NONE
    // TODO : small list of common plugins to fit in 1M
#endif

// Ventus W266 --------------------------------
#ifdef PLUGIN_SET_VENTUS_W266
    #define PLUGIN_SET_ONLY_SWITCH
    #define PLUGIN_BUILD_DISABLED
    #define USES_P046      // Hardware	P046_VentusW266.ino
#endif


#ifdef PLUGIN_SET_LC_TECH_RELAY_X2
    #define CONTROLLER_SET_STABLE
    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
    #define USES_P026    // Sysinfo
    #define USES_P029    // Domoticz MQTT Helper
    #define USES_P033    // Dummy
    #define USES_P037    // MQTT import
    #define USES_P081    // Cron
    #define USES_P091    // Ser Switch
#endif



/******************************************************************************\
 * "ONLY" shorcuts ************************************************************
\******************************************************************************/
#ifdef PLUGIN_SET_ONLY_SWITCH
    #ifndef PLUGIN_SET_NONE
        #define PLUGIN_SET_NONE
    #endif
    #ifndef USES_P001
        #define USES_P001   // switch
    #endif
    #ifndef USES_P003
//        #define USES_P003   // pulse
    #endif
    #ifndef USES_P026
      #define USES_P026   // SysInfo
    #endif
    #ifndef USES_P033
      #define USES_P033   // Dummy
    #endif
    #ifndef USES_P037
        #define USES_P037   // MQTTImport
    #endif
#endif

#ifdef PLUGIN_SET_ONLY_TEMP_HUM
    #ifndef PLUGIN_SET_NONE
        #define PLUGIN_SET_NONE
    #endif
    #ifndef USES_P004
        #define USES_P004   // Dallas
    #endif
    #ifndef USES_P005
        #define USES_P005   // DHT
    #endif
    #ifndef USES_P014
        #define USES_P014   // SI7021
    #endif
    #ifndef USES_P028
        #define USES_P028   // BME280
    #endif
    #ifndef USES_P034
        #define USES_P034   // DHT12
    #endif
#endif

#ifdef PLUGIN_SET_ONLY_LEDSTRIP
    #ifndef PLUGIN_SET_NONE
        #define PLUGIN_SET_NONE
    #endif
    #ifndef USES_P141
        #define USES_P141   // LedStrip
    #endif
    #ifndef USES_P037
        #define USES_P037   // MQTTImport
    #endif
#endif


/******************************************************************************\
 * Main Families **************************************************************
\******************************************************************************/

// NONE #####################################
#ifdef PLUGIN_SET_NONE
  #ifdef PLUGIN_SET_STABLE
    #undef PLUGIN_SET_STABLE
  #endif
  #ifdef PLUGIN_SET_COLLECTION
    #undef PLUGIN_SET_COLLECTION
  #endif
  #ifdef PLUGIN_SET_COLLECTION_A
    #undef PLUGIN_SET_COLLECTION_A
  #endif
  #ifdef PLUGIN_SET_COLLECTION_B
    #undef PLUGIN_SET_COLLECTION_B
  #endif
  #ifdef PLUGIN_SET_COLLECTION_C
    #undef PLUGIN_SET_COLLECTION_C
  #endif
  #ifdef PLUGIN_SET_COLLECTION_D
    #undef PLUGIN_SET_COLLECTION_D
  #endif
  #ifdef PLUGIN_SET_COLLECTION_E
    #undef PLUGIN_SET_COLLECTION_E
  #endif
  #ifdef PLUGIN_SET_COLLECTION_F
    #undef PLUGIN_SET_COLLECTION_F
  #endif
  #ifdef PLUGIN_SET_EXPERIMENTAL
    #undef PLUGIN_SET_EXPERIMENTAL
  #endif
#endif


#ifdef CONTROLLER_SET_NONE
  #ifdef CONTROLLER_SET_STABLE
    #undef CONTROLLER_SET_STABLE
  #endif
  #ifdef CONTROLLER_SET_COLLECTION
    #undef CONTROLLER_SET_COLLECTION
  #endif
  #ifdef CONTROLLER_SET_EXPERIMENTAL
    #undef CONTROLLER_SET_EXPERIMENTAL
  #endif
#endif


#ifdef NOTIFIER_SET_NONE
  #ifdef NOTIFIER_SET_STABLE
    #undef NOTIFIER_SET_STABLE
  #endif
  #ifdef NOTIFIER_SET_COLLECTION
    #undef NOTIFIER_SET_COLLECTION
  #endif
  #ifdef NOTIFIER_SET_EXPERIMENTAL
    #undef NOTIFIER_SET_EXPERIMENTAL
  #endif
#endif

// ALL ###########################################
#ifdef PLUGIN_SET_ALL
  #ifndef PLUGIN_SET_STABLE
    #define PLUGIN_SET_STABLE
  #endif
  #ifndef PLUGIN_SET_COLLECTION
    #define PLUGIN_SET_COLLECTION
  #endif
  #ifndef PLUGIN_SET_EXPERIMENTAL
    #define PLUGIN_SET_EXPERIMENTAL
  #endif
#endif


#ifdef CONTROLLER_SET_ALL
  #ifndef CONTROLLER_SET_STABLE
    #define CONTROLLER_SET_STABLE
  #endif
  #ifndef CONTROLLER_SET_COLLECTION
    #define CONTROLLER_SET_COLLECTION
  #endif
  #ifndef CONTROLLER_SET_EXPERIMENTAL
    #define CONTROLLER_SET_EXPERIMENTAL
  #endif
#endif


#ifdef NOTIFIER_SET_ALL
  #ifndef NOTIFIER_SET_STABLE
    #define NOTIFIER_SET_STABLE
  #endif
  #ifndef NOTIFIER_SET_COLLECTION
    #define NOTIFIER_SET_COLLECTION
  #endif
  #ifndef NOTIFIER_SET_EXPERIMENTAL
    #define NOTIFIER_SET_EXPERIMENTAL
  #endif
#endif

// MAX ###########################################
#ifdef PLUGIN_SET_MAX
  #ifndef PLUGIN_SET_STABLE
    #define PLUGIN_SET_STABLE
  #endif
  #ifndef PLUGIN_SET_COLLECTION
    #define PLUGIN_SET_COLLECTION
  #endif
  #ifndef PLUGIN_SET_COLLECTION_A
    #define PLUGIN_SET_COLLECTION_A
  #endif
  #ifndef PLUGIN_SET_COLLECTION_B
    #define PLUGIN_SET_COLLECTION_B
  #endif
  #ifndef PLUGIN_SET_COLLECTION_C
    #define PLUGIN_SET_COLLECTION_C
  #endif
  #ifndef PLUGIN_SET_COLLECTION_D
    #define PLUGIN_SET_COLLECTION_D
  #endif
  #ifndef PLUGIN_SET_COLLECTION_E
    #define PLUGIN_SET_COLLECTION_E
  #endif
  #ifndef PLUGIN_SET_COLLECTION_F
    #define PLUGIN_SET_COLLECTION_F
  #endif
#endif




// STABLE #####################################
#ifdef PLUGIN_SET_STABLE
    #ifndef FEATURE_SERVO
      #define FEATURE_SERVO 1
    #endif
    #define FEATURE_RTTTL 1

    #define USES_P001   // Switch
    #define USES_P002   // ADC
    #define USES_P003   // Pulse
    #define USES_P004   // Dallas
    #define USES_P005   // DHT
    #define USES_P006   // BMP085
    #define USES_P007   // PCF8591
    #define USES_P008   // RFID
    #define USES_P009   // MCP

    #define USES_P010   // BH1750
    #define USES_P011   // PME
    #define USES_P012   // LCD
    #define USES_P013   // HCSR04
    #define USES_P014   // SI7021
    #define USES_P015   // TSL2561
//    #define USES_P016   // IR
    #define USES_P017   // PN532
    #define USES_P018   // Dust
    #define USES_P019   // PCF8574

    #define USES_P020   // Ser2Net
    #define USES_P021   // Level
    #define USES_P022   // PCA9685
    #define USES_P023   // OLED
    #define USES_P024   // MLX90614
    #define USES_P025   // ADS1115
    #define USES_P026   // SysInfo
    #define USES_P027   // INA219
    #define USES_P028   // BME280
    #define USES_P029   // Output

    #define USES_P031   // SHT1X
    #define USES_P032   // MS5611
    #define USES_P033   // Dummy
    #define USES_P034   // DHT12
//    #define USES_P035   // IRTX
    #define USES_P036   // FrameOLED
    #define USES_P037   // MQTTImport
    #define USES_P038   // NeoPixel
    #define USES_P039   // Environment - Thermocouple

    #define USES_P040   // RFID - ID12LA/RDM6300
    // FIXME TD-er: Disabled NeoClock and Candle plugin to make builds fit in max bin size.
//    #define USES_P041   // NeoClock
//    #define USES_P042   // Candle
    #define USES_P043   // ClkOutput
    #define USES_P044   // P1WifiGateway

    #define USES_P049   // MHZ19

    #define USES_P052   // SenseAir
    #define USES_P053   // PMSx003

    #define USES_P056   // SDS011-Dust
    #define USES_P059   // Encoder

    #define USES_P063   // TTP229_KeyPad
    #define USES_P073   // 7DGT
    #define USES_P079   // Wemos Motoshield
#endif


#ifdef CONTROLLER_SET_STABLE
  #if !FEATURE_NO_HTTP_CLIENT
    #define USES_C001   // Domoticz HTTP
  #endif
    #define USES_C002   // Domoticz MQTT
    #define USES_C003   // Nodo telnet
    #define USES_C004   // ThingSpeak
    #define USES_C005   // Home Assistant (openHAB) MQTT
    #define USES_C006   // PiDome MQTT
    #define USES_C007   // Emoncms
  #if !FEATURE_NO_HTTP_CLIENT
    #define USES_C008   // Generic HTTP
  #endif
    #define USES_C009   // FHEM HTTP
    #define USES_C010   // Generic UDP
    #define USES_C013   // ESPEasy P2P network
#endif


#ifdef NOTIFIER_SET_STABLE
    #define USES_N001   // Email
    #define USES_N002   // Buzzer

    #ifdef NOTIFIER_SET_NONE
      #undef NOTIFIER_SET_NONE
    #endif
#endif

#if defined(PLUGIN_SET_COLLECTION) || defined(PLUGIN_SET_COLLECTION_A) || defined(PLUGIN_SET_COLLECTION_B) || defined(PLUGIN_SET_COLLECTION_C) || defined(PLUGIN_SET_COLLECTION_D) || defined(PLUGIN_SET_COLLECTION_E) || defined(PLUGIN_SET_COLLECTION_F)
  #if !defined(PLUGIN_SET_MAX) && !defined(ESP32)
    #ifndef LIMIT_BUILD_SIZE
      #define LIMIT_BUILD_SIZE
    #endif
    #ifndef NOTIFIER_SET_NONE
      #define NOTIFIER_SET_NONE
    #endif
    
    // Do not include large blobs but fetch them from CDN
    #ifndef WEBSERVER_USE_CDN_JS_CSS
      #define WEBSERVER_USE_CDN_JS_CSS
    #endif
  #endif
  #define KEEP_I2C_MULTIPLEXER
#endif

// COLLECTIONS #####################################
#ifdef PLUGIN_SET_COLLECTION
    #define USES_P045   // MPU6050
    #define USES_P047   // I2C_soil_misture
    #define USES_P048   // Motoshield_v2

    #define USES_P050   // TCS34725
    #define USES_P051   // AM2320
    #define USES_P054   // DMX512
    #define USES_P055   // Chiming
    #define USES_P057   // HT16K33_LED
    #define USES_P058   // HT16K33_KeyPad

    #define USES_P060   // MCP3221
    #define USES_P061   // Keypad
    #define USES_P062   // MPR121_KeyPad

    #define USES_P064   // APDS9960
    #define USES_P065   // DRF0299
    #define USES_P066   // VEML6040

    #define USES_P075   // Nextion
    //#define USES_P076   // HWL8012   in POW r1
    // Needs CSE7766 Energy sensor, via Serial RXD 4800 baud 8E1 (GPIO1), TXD (GPIO3)
    //#define USES_P077	  // CSE7766   in POW R2
    //#define USES_P078   // Eastron Modbus Energy meters
    #define USES_P081   // Cron
    #define USES_P082   // GPS
    #define USES_P089   // Ping
    #if !defined(USES_P137) && defined(ESP32)
      #define USES_P137   // AXP192
    #endif
  #if !defined(USES_P138) && defined(ESP32)
    #define USES_P138   // IP5306
  #endif
#endif

#ifdef PLUGIN_SET_COLLECTION_A

    #define USES_P067   // HX711_Load_Cell
    #define USES_P068   // SHT3x

    #define USES_P070   // NeoPixel_Clock
    #define USES_P071   // Kamstrup401
    #define USES_P072   // HDC1000/HDC1008/HDC1010/HDC1050/HDC1080
    #define USES_P074   // TSL2561

    #define USES_P080   // iButton Sensor  DS1990A
    #define USES_P083   // SGP30
    #define USES_P084   // VEML6070
    #define USES_P086   // Receiving values according Homie convention. Works together with C014 Homie controller

    #define USES_P090   // CCS811 TVOC/eCO2 Sensor

    //#define USES_P095  // TFT ILI9341
    //#define USES_P096  // eInk   (Needs lib_deps = Adafruit GFX Library, LOLIN_EPD )
    #define USES_P097   // Touch (ESP32)
    #define USES_P098   // PWM motor  (relies on iRAM, cannot be combined with all other plugins)
    //#define USES_P099   // XPT2046 Touchscreen
    #define USES_P105   // AHT10/20/21
    #define USES_P134   // A02YYUW
#endif

#ifdef PLUGIN_SET_COLLECTION_B
    #define USES_P069   // LM75A

    #define USES_P100   // Pulse Counter - DS2423
    #define USES_P101   // Wake On Lan
    #define USES_P103   // Atlas Scientific EZO Sensors (pH, ORP, EZO, DO)
    #define USES_P106   // BME680
    #define USES_P107   // SI1145 UV index
    #define USES_P108   // DDS238-x ZN MODBUS energy meter (was P224 in the Playground)
    // FIXME TD-er: Disabled due to build size
    //#define USES_P109   // ThermoOLED
    #define USES_P110   // VL53L0X Time of Flight sensor
    #define USES_P113   // VL53L1X ToF
#endif

#ifdef PLUGIN_SET_COLLECTION_C
    #define USES_P085   // AcuDC24x
    #define USES_P087   // Serial Proxy

    #define USES_P091	// SerSwitch
    #define USES_P092   // DL-Bus

    #define USES_P111   // RC522 RFID reader
    #define USES_P143   // I2C Rotary encoders
#endif

#ifdef PLUGIN_SET_COLLECTION_D
    #define USES_P093   // Mitsubishi Heat Pump
    #define USES_P094  // CUL Reader
    #ifndef USES_P098
      #define USES_P098   // PWM motor
    #endif
    #define USES_P114  // VEML6075 UVA/UVB sensor
    #define USES_P115  // Fuel Gauge MAX1704x
    #define USES_P117  // SCD30
    #define USES_P124  // I2C MultiRelay
    #define USES_P127  // CDM7160
#endif

#ifdef PLUGIN_SET_COLLECTION_E
    #define USES_P119   // ITG3205 Gyro
    #define USES_P120   // ADXL345 I2C
    #define USES_P121   // HMC5883L 
    #define USES_P125   // ADXL345 SPI
    #define USES_P126  // 74HC595 Shift register
    #define USES_P129   // 74HC165 Input shiftregisters
    #define USES_P133   // LTR390 UV
    #define USES_P135   // SCD4x
    #define USES_P144   // Dust - PM1006(K) (Vindriktning)
#endif

#ifdef PLUGIN_SET_COLLECTION_F
  // Disable Itho when using second heap as it no longer fits.
  #if !defined(USES_P118) && !defined(USE_SECOND_HEAP)
    #define USES_P118 // Itho ventilation control
  #endif
  #ifndef USES_P145
    #define USES_P145   // gasses MQxxx (MQ135, MQ3, etc)
  #endif
#endif

// Collection of all energy related plugins.
#ifdef PLUGIN_ENERGY_COLLECTION
  #ifndef PLUGIN_DESCR
    #define PLUGIN_DESCR  "Energy"
  #endif
  #if !defined(LIMIT_BUILD_SIZE) && (defined(ESP8266) || !(ESP_IDF_VERSION_MAJOR > 3))
    // #define LIMIT_BUILD_SIZE // Reduce buildsize (on ESP8266 / pre-IDF4.x) to fit in all Energy plugins
    #ifndef P036_LIMIT_BUILD_SIZE
      #define P036_LIMIT_BUILD_SIZE // Reduce build size for P036 (FramedOLED) only
    #endif
    #ifndef P037_LIMIT_BUILD_SIZE
      #define P037_LIMIT_BUILD_SIZE // Reduce build size for P037 (MQTT Import) only
    #endif
  #endif
   #ifndef USES_P025
     #define USES_P025   // ADS1115
   #endif
   #ifndef USES_P027
     #define USES_P027   // INA219
   #endif
   #ifndef USES_P076
     #define USES_P076   // HWL8012   in POW r1
   #endif
   #ifndef USES_P077
     // Needs CSE7766 Energy sensor, via Serial RXD 4800 baud 8E1 (GPIO1), TXD (GPIO3)
     #define USES_P077	  // CSE7766   in POW R2
   #endif
   #ifndef USES_P078
     #define USES_P078   // Eastron Modbus Energy meters
   #endif
   #ifndef USES_P085
     #define USES_P085   // AcuDC24x
   #endif
   #ifndef USES_P093
     #define USES_P093   // Mitsubishi Heat Pump
   #endif
   #ifndef USES_P102
     #define USES_P102   // PZEM-004Tv30
   #endif
   #ifndef USES_P108
     #define USES_P108   // DDS238-x ZN MODBUS energy meter (was P224 in the Playground)
   #endif
   #ifndef USES_P115
     #define USES_P115   // Fuel Gauge MAX1704x
   #endif
   #ifndef USES_P132
     #define USES_P132   // INA3221
   #endif
  #if !defined(USES_P137) && defined(ESP32)
    #define USES_P137   // AXP192
  #endif
  #if !defined(USES_P138) && defined(ESP32)
    #define USES_P138   // IP5306
  #endif
   #ifndef USES_P148
     #define USES_P148   // Sonoff POWR3xxD and THR3xxD display
   #endif

#endif

// Collection of all display plugins. (also NeoPixel)
#ifdef PLUGIN_DISPLAY_COLLECTION
  #ifndef PLUGIN_DESCR
    #define PLUGIN_DESCR  "Display"
  #endif
   #if !defined(LIMIT_BUILD_SIZE) && (defined(ESP8266) || !(ESP_IDF_VERSION_MAJOR > 3))
     #ifndef PLUGIN_BUILD_MAX_ESP32
       #define LIMIT_BUILD_SIZE // Reduce buildsize (on ESP8266 / pre-IDF4.x) to fit in all Display plugins
       #define KEEP_I2C_MULTIPLEXER
     #endif
   #endif
   #if defined(ESP8266)
     #if defined(FEATURE_I2C_DEVICE_CHECK)
       #undef FEATURE_I2C_DEVICE_CHECK
     #endif
     #define FEATURE_I2C_DEVICE_CHECK 0 // Disable I2C device check code
   #endif
   #if !defined(FEATURE_SD) && !defined(ESP8266)
     #define FEATURE_SD 1
   #endif
   #ifndef USES_P012
     #define USES_P012   // LCD
   #endif
   #ifndef USES_P023
    #define USES_P023   // OLED
   #endif
   #ifndef USES_P036
    #define USES_P036   // FrameOLED
   #endif
   #ifdef USES_P038
    #undef USES_P038   // DISABLE NeoPixel
   #endif
   #ifdef USES_P041
    #undef USES_P041   // DISABLE NeoClock
   #endif
   #ifdef USES_P042
    #undef USES_P042   // DISABLE Candle
   #endif
   #ifndef USES_P057
    #define USES_P057   // HT16K33_LED
   #endif
   #ifdef USES_P070
    #undef USES_P070   // DISABLE NeoPixel_Clock
   #endif
   #ifndef USES_P075
    #define USES_P075   // Nextion
   #endif
   #ifndef USES_P095
    #define USES_P095  // TFT ILI9341
   #endif
   #ifndef USES_P096
    #define USES_P096  // eInk   (Needs lib_deps = Adafruit GFX Library, LOLIN_EPD )
   #endif
   #ifndef USES_P099
    #define USES_P099   // XPT2046 Touchscreen
   #endif
   #ifndef USES_P104
    #define USES_P104   // MAX7219 dot matrix
   #endif
   #if !defined(USES_P109) && defined(ESP32)
     #define USES_P109   // ThermoOLED
   #endif
   #ifndef USES_P116
     #define USES_P116   // ST77xx
   #endif
  #if !defined(USES_P137) && defined(ESP32)
    #define USES_P137   // AXP192
  #endif
  #if !defined(USES_P138) && defined(ESP32)
    #define USES_P138   // IP5306
  #endif
  #ifndef USES_P141
    #define USES_P141   // PCD8544 Nokia 5110
  #endif
  #ifndef USES_P143
    #define USES_P143   // I2C Rotary encoders
  #endif
  #ifndef USES_P148
    #define USES_P148   // Sonoff POWR3xxD and THR3xxD display
  #endif
#endif

// Collection of all climate plugins.
#ifdef PLUGIN_CLIMATE_COLLECTION
  #ifndef PLUGIN_DESCR
    #define PLUGIN_DESCR  "Climate"
  #endif

  // Features and plugins cherry picked from stable set
  #ifndef FEATURE_SERVO
    #define FEATURE_SERVO 1
  #endif
  #define FEATURE_RTTTL 1

  #define USES_P001   // Switch
  #define USES_P002   // ADC
  #define USES_P003   // Pulse
  #define USES_P004   // Dallas
  #define USES_P005   // DHT
  #define USES_P006   // BMP085

  #define USES_P011   // PME
  #define USES_P012   // LCD
  #define USES_P014   // SI7021
  #define USES_P018   // Dust
  #define USES_P019   // PCF8574

  #define USES_P021   // Level
  #define USES_P023   // OLED
  #define USES_P024   // MLX90614
  #define USES_P026   // SysInfo
  #define USES_P028   // BME280
  #define USES_P029   // Output

  #define USES_P031   // SHT1X
  #define USES_P032   // MS5611
  #define USES_P033   // Dummy
  #define USES_P034   // DHT12
  #define USES_P036   // FrameOLED
  #define USES_P037   // MQTTImport
  #define USES_P038   // NeoPixel
  #define USES_P039   // Environment - Thermocouple

  #define USES_P043   // ClkOutput
  #define USES_P044   // P1WifiGateway
  #define USES_P049   // MHZ19

  #define USES_P052   // SenseAir
  #define USES_P053   // PMSx003
  #define USES_P056   // SDS011-Dust
  #define USES_P059   // Encoder

  #define USES_P073   // 7DGT

  // Enable extra climate-related plugins (CO2/Temp/Hum)
  #ifndef USES_P047
    #define USES_P047 // Soil Moisture
  #endif
  #ifndef USES_P049
    #define USES_P049 // MH-Z19
  #endif
  #ifndef USES_P051
    #define USES_P051 // AM2320
  #endif
  #ifndef USES_P068
    #define USES_P068 // SHT3x
  #endif
  #ifndef USES_P069
    #define USES_P069 // LM75
  #endif
  #ifndef USES_P072
    #define USES_P072 // HCD1080
  #endif
  #ifndef USES_P081
    #define USES_P081 // Cron
  #endif
  #ifndef USES_P083
    #define USES_P083 // SGP30
  #endif
  #ifndef USES_P090
    #define USES_P090 // CCS811
  #endif
  #ifndef USES_P103
    #define USES_P103 // Atlas EZO
  #endif
  #ifndef USES_P105
    #define USES_P105 // AHT10/20/21
  #endif
  #ifndef USES_P106
    #define USES_P106 // BME680
  #endif
  #ifndef USES_P117
    #define USES_P117 // SCD30
  #endif
  // Disable Itho when using second heap as it no longer fits.
  #if !defined(USES_P118) && !defined(USE_SECOND_HEAP)
    #define USES_P118 // Itho ventilation control
  #endif
  #ifndef USES_P127
    #define USES_P127 // CDM7160
  #endif
  #ifndef USES_P135
    #define USES_P135 // SCD4x
  #endif
  #ifndef USES_P144
    #define USES_P144   // Dust - PM1006(K) (Vindriktning)
  #endif
  #ifndef USES_P148
    #define USES_P148   // Sonoff POWR3xxD and THR3xxD display
  #endif
  // Controllers
  #ifndef USES_C011
    #define USES_C011   // HTTP Advanced
  #endif
#endif

// Collection of all NeoPixel plugins
#ifdef PLUGIN_NEOPIXEL_COLLECTION
  #ifndef PLUGIN_DESCR
    #define PLUGIN_DESCR  "NeoPixel"
  #endif
  #if !defined(FEATURE_SD) && !defined(ESP8266)
    #define FEATURE_SD  1
  #endif
  #ifndef USES_P038
    #define USES_P038   // NeoPixel
  #endif
  #ifndef USES_P041
    #define USES_P041   // NeoClock
  #endif
  #ifndef USES_P042
    #define USES_P042   // Candle
  #endif
  #ifndef USES_P070
    #define USES_P070   // NeoPixel_Clock
  #endif
  #ifndef USES_P128
    #define USES_P128   // NeoPixelBusFX
  #endif
  #ifndef USES_P131
    #define USES_P131   // NeoMatrix
  #endif
  #if !defined(USES_P137) && defined(ESP32)
    #define USES_P137   // AXP192
  #endif
  #if FEATURE_PLUGIN_STATS && defined(ESP8266)
    // Does not fit in build
    #undef FEATURE_PLUGIN_STATS
  #endif
  #ifdef ESP8266
    #define FEATURE_PLUGIN_STATS  0
  #endif
  #if FEATURE_CHART_JS && defined(ESP8266)
    // Does not fit in build
    #undef FEATURE_CHART_JS
  #endif
  #ifdef ESP8266
    #define FEATURE_CHART_JS  0
  #endif
  #if !defined(USES_P138) && defined(ESP32)
    #define USES_P138   // IP5306
  #endif
#endif

#ifdef CONTROLLER_SET_COLLECTION
    #define USES_C011   // Generic HTTP Advanced
    #define USES_C012   // Blynk HTTP
    #define USES_C014   // homie 3 & 4dev MQTT
    //#define USES_C015   // Blynk
    #define USES_C017   // Zabbix
    // #define USES_C018 // TTN RN2483
    // #define USES_C019   // ESPEasy-NOW
#endif


#ifdef NOTIFIER_SET_COLLECTION
  // To be defined
#endif


// EXPERIMENTAL (playground) #######################
#ifdef PLUGIN_SET_EXPERIMENTAL
    #define USES_P046   // VentusW266
    #define USES_P050   // TCS34725 RGB Color Sensor with IR filter and White LED
    #define USES_P064   // APDS9960 Gesture
    #define USES_P077	// CSE7766   Was P134 on Playground


    // [copied from Playground as of 6 March 2018]
    // It needs some cleanup as some are ALSO in the main repo,
    // thus they should have been removed from the Playground repo
    // #define USES_P100	// Was SRF01, now Pulse Counter - DS2423
	// #define USES_P101	// Was NeoClock, now Wake On Lan
	#define USES_P102	// Nodo
	#define USES_P103	// Event
	#define USES_P104	// SRF02
	#define USES_P105	// RGBW
	#define USES_P106	// IRTX
	#define USES_P107	// Email_Demo
	#define USES_P108	// WOL
	#define USES_P109	// RESOL_DeltaSol_Pro
	   #define USES_P110	// P1WifiGateway      (MERGED?)
	#define USES_P111	// RF
	   //#define USES_P111	// SenseAir     (MERGED?)
	#define USES_P112	// Power
	//#define USES_P112	// RFTX
	#define USES_P113	// SI1145
	#define USES_P114	// DSM501
	//#define USES_P115	// HeatpumpIR - P088 in the main repo.
//	#define USES_P116	// ID12
	#define USES_P117	// LW12FC
	//#define USES_P117	// Neopixels
	//#define USES_P117	// Nextion
	#define USES_P118	// CCS811
	#define USES_P119	// BME680
	#define USES_P120	// Thermocouple
	#define USES_P121	// Candle
//	   #define USES_P122	// NeoPixel       (MERGED?)
//	      #define USES_P123	// NeoPixel_Clock  (MERGED?)
	#define USES_P124	// NeoPixelBusFX
	//#define USES_P124	// Ventus_W266_RFM69
	#define USES_P125	// ArduCAM
	#define USES_P127	// Teleinfo
	#define USES_P130	// VEML6075
	#define USES_P131	// SHT3X
	#define USES_P133	// VL53L0X
	#define USES_P141	// LedStrip
	#define USES_P142	// RGB-Strip
	#define USES_P143	// AnyonePresent
	#define USES_P144	// RC-Switch-TX
	#define USES_P145	// Itho - P118 in the main repo.
	#define USES_P149	// MHZ19
	#define USES_P150	// SDM120C
	#define USES_P151	// CISA
	#define USES_P153	// MAX44009
	#define USES_P162	// MPL3115A2
	#define USES_P163	// DS1631
	#define USES_P165	// SerSwitch
	#define USES_P166	// WiFiMan
	#define USES_P167	// ADS1015
	#define USES_P170	// HLW8012
	#define USES_P171	// PZEM-004T
	#define USES_P180	// Mux
	#define USES_P181	// TempHumidity_SHT2x
	#define USES_P182	// MT681
	#define USES_P199	// RF443_KaKu
	#define USES_P202	// ADC_ACcurrentSensor
	   #define USES_P205	// FrameOLED      (MERGED?)
	#define USES_P209	// IFTTTMaker
	   #define USES_P210	// MQTTImport     (MERGED?)
	#define USES_P211	// MPU6050
	#define USES_P212	// MY9291
	#define USES_P213	// VEML6070
#endif


#ifdef CONTROLLER_SET_EXPERIMENTAL
  //#define USES_C016   // Cache controller
  //#define USES_C018   // TTN/RN2483
#endif


#ifdef NOTIFIER_SET_EXPERIMENTAL
#endif


// Maximized build definition for an ESP(32) with 16MB Flash and 4MB sketch partition
// Add all plugins, controllers and features that don't fit in the COLLECTION set
#ifdef PLUGIN_SET_MAX
  // Features
  #ifndef USES_ESPEASY_NOW
//    #define USES_ESPEASY_NOW
  #endif
  #ifndef FEATURE_SERVO
    #define FEATURE_SERVO 1
  #endif
  #ifndef FEATURE_RTTTL
    #define FEATURE_RTTTL 1
  #endif
  #ifndef FEATURE_SETTINGS_ARCHIVE
    #define FEATURE_SETTINGS_ARCHIVE  1
  #endif
  #ifndef FEATURE_SD
    #define FEATURE_SD 1
  #endif
  #ifndef SHOW_SYSINFO_JSON
    #define SHOW_SYSINFO_JSON 1
  #endif
  #ifndef FEATURE_I2C_DEVICE_SCAN
    #define FEATURE_I2C_DEVICE_SCAN   1
  #endif

  // Plugins
  #ifndef USES_P016
//    #define USES_P016   // IR
  #endif
  #ifndef USES_P035
//    #define USES_P035   // IRTX
  #endif
  #ifndef USES_P041
    #define USES_P041   // NeoClock
  #endif
  #ifndef USES_P042
    #define USES_P042   // Candle
  #endif
  #ifndef USES_P087
    #define USES_P087   // Serial Proxy
  #endif
  #ifndef USES_P094
    #define USES_P094  // CUL Reader
  #endif
  #ifndef USES_P095
    #define USES_P095  // TFT ILI9341
  #endif
  #ifndef USES_P096
    #define USES_P096  // eInk   (Needs lib_deps = Adafruit GFX Library, LOLIN_EPD )
  #endif
  #ifndef USES_P098
    #define USES_P098   // PWM motor
  #endif
  #ifndef USES_P099
    #define USES_P099   // XPT2046 Touchscreen
  #endif
  #ifndef USES_P102
    #define USES_P102   // PZEM004Tv3
  #endif
  #ifndef USES_P103
    #define USES_P103   // Atlas Scientific EZO Sensors (pH, ORP, EZO, DO)
  #endif
  #ifndef USES_P104
    #define USES_P104   //
  #endif
  #ifndef USES_P105
    #define USES_P105   // AHT10/20/21
  #endif
  #ifndef USES_P104
    #define USES_P104   //
  #endif
  #ifndef USES_P105
    #define USES_P105   // AHT10/20/21
  #endif
  #ifndef USES_P108
    #define USES_P108   // DDS238-x ZN MODBUS energy meter (was P224 in the Playground)
  #endif
  #ifndef USES_P109
    #define USES_P109   // ThermOLED
  #endif
  #ifndef USES_P110
    #define USES_P110   // VL53L0X
  #endif
  #ifndef USES_P111
    #define USES_P111   // RC522 RFID reader
  #endif
  #ifndef USES_P112
    #define USES_P112   // AS7256x
  #endif
  #ifndef USES_P113
    #define USES_P113   // VL53L1X
  #endif
  #ifndef USES_P114
    #define USES_P114   // VEML6075 UVA/UVB sensor
  #endif
  #ifndef USES_P115
    #define USES_P115   // Fuel gauge MAX1704x
  #endif
  #ifndef USES_P116
    #define USES_P116   // ST77xx
  #endif
  #ifndef USES_P117
    #define USES_P117   // SCD30
  #endif
  #ifndef USES_P118
    #define USES_P118   // Itho ventilation coontrol
  #endif
  #ifndef USES_P119
    #define USES_P119   // ITG3205 Gyro
  #endif
  #ifndef USES_P120
    #define USES_P120   // ADXL345 I2C Acceleration / Gravity
  #endif
  #ifndef USES_P121
    #define USES_P121   // HMC5883L 
  #endif
  #ifndef USES_P122
//    #define USES_P122   //
  #endif
  #ifndef USES_P123
//    #define USES_P123   //
  #endif
  #ifndef USES_P124
    #define USES_P124   //
  #endif
  #ifndef USES_P125
    #define USES_P125   // ADXL345 SPI Acceleration / Gravity
  #endif
  #ifndef USES_P126
    #define USES_P126   // 74HC595 Shift register
  #endif
  #ifndef USES_P127
    #define USES_P127   // CDM7160
  #endif
  #ifndef USES_P128
    #define USES_P128   // NeoPixelBusFX
  #endif
  #ifndef USES_P129
    #define USES_P129   // 74HC165 Input shiftregisters
  #endif
  #ifndef USES_P130
//    #define USES_P130   //
  #endif
  #ifndef USES_P131
    #define USES_P131   // NeoMatrix
  #endif
  #ifndef USES_P132
    #define USES_P132   // INA3221
  #endif
  #ifndef USES_P133
//    #define USES_P133   //
  #endif
  #ifndef USES_P134
//    #define USES_P134   //
  #endif
  #ifndef USES_P135
//    #define USES_P135   //
  #endif
  #ifndef USES_P136
//    #define USES_P136   //
  #endif
  #ifndef USES_P137
    #define USES_P137   // AXP192
  #endif
  #ifndef USES_P138
//    #define USES_P138   //
  #endif
  #ifndef USES_P139
//    #define USES_P139   //
  #endif
  #ifndef USES_P140
//    #define USES_P140   //
  #endif
  #ifndef USES_P141
    #define USES_P141   // PCD8544 Nokia 5110
  #endif
  #ifndef USES_P142
//    #define USES_P142   //
  #endif
  #ifndef USES_P143
    #define USES_P143   // I2C Rotary encoders
  #endif
  #ifndef USES_P144
    #define USES_P144   // Dust - PM1006(K) (Vindriktning)
  #endif
  #ifndef USES_P146
    #define USES_P146   // Cache Controller Reader
  #endif

  // Controllers
  #ifndef USES_C015
    #define USES_C015   // Blynk
  #endif
  #ifndef USES_C016
    #define USES_C016   // Cache controller
  #endif
  #ifndef USES_C018
    #define USES_C018 // TTN RN2483
  #endif

  // Notifiers

#endif // PLUGIN_SET_MAX


/******************************************************************************\
 * Remove incompatible plugins ************************************************
\******************************************************************************/
#ifdef ESP32
//  #undef USES_P010   // BH1750          (doesn't work yet on ESP32)
//  #undef USES_P049   // MHZ19           (doesn't work yet on ESP32)

//  #undef USES_P052   // SenseAir        (doesn't work yet on ESP32)
//  #undef USES_P053   // PMSx003

//  #undef USES_P056   // SDS011-Dust     (doesn't work yet on ESP32)
//  #undef USES_P065   // DRF0299
//  #undef USES_P071   // Kamstrup401
//  #undef USES_P075   // Nextion
//  #undef USES_P078   // Eastron Modbus Energy meters (doesn't work yet on ESP32)
//  #undef USES_P082   // GPS
#endif


#ifdef ARDUINO_ESP8266_RELEASE_2_3_0
  #ifdef USES_P081
    #undef USES_P081   // Cron
  #endif


#endif


/******************************************************************************\
 * Libraries dependencies *****************************************************
\******************************************************************************/
#if defined(USES_P020) || defined(USES_P049) || defined(USES_P052) || defined(USES_P053) || defined(USES_P056) || defined(USES_P071) || defined(USES_P075) || defined(USES_P077) || defined(USES_P078) || defined(USES_P082) || defined(USES_P085) || defined(USES_P087) || defined(USES_P093)|| defined(USES_P094) || defined(USES_P102) || defined(USES_P105) || defined(USES_P108) || defined(USES_P144) || defined(USES_C018)
  // At least one plugin uses serial.
  #ifndef PLUGIN_USES_SERIAL
    #define PLUGIN_USES_SERIAL
  #endif
#else
  // No plugin uses serial, so make sure software serial is not included.
  #define DISABLE_SOFTWARE_SERIAL
#endif

#if defined(USES_P095) || defined(USES_P096) || defined(USES_P116) || defined(USES_P131) || defined(USES_P141) // Add any plugin that uses AdafruitGFX_Helper
  #ifndef PLUGIN_USES_ADAFRUITGFX
    #define PLUGIN_USES_ADAFRUITGFX // Ensure AdafruitGFX_helper is available for graphics displays (only)
  #endif
#endif

/*
#if defined(USES_P00x) || defined(USES_P00y)
#include <the_required_lib.h>
#endif
*/

#ifdef USES_C013
  #ifdef FEATURE_ESPEASY_P2P
    #undef FEATURE_ESPEASY_P2P
  #endif
  #define FEATURE_ESPEASY_P2P 1
#endif

#if defined(USES_C018)
  #define FEATURE_PACKED_RAW_DATA 1
#endif

#if defined(USES_C019)
  #ifndef USES_ESPEASY_NOW
    #define USES_ESPEASY_NOW
  #endif
#endif

#if defined(USES_P085) || defined (USES_P052) || defined(USES_P078) || defined(USES_P108)
  // FIXME TD-er: Is this correct? Those plugins use Modbus_RTU.
//  #define FEATURE_MODBUS  1
#endif

#if defined(USES_C001) || defined (USES_C002) || defined(USES_P029)
  #ifndef FEATURE_DOMOTICZ
    #define FEATURE_DOMOTICZ  1
  #endif
#endif

#if FEATURE_DOMOTICZ  // Move Domoticz enabling logic together
    #if !defined(USES_C001) && !FEATURE_NO_HTTP_CLIENT
      #define USES_C001   // Domoticz HTTP
    #endif
    #ifndef USES_C002
      #define USES_C002   // Domoticz MQTT
    #endif
    #ifndef USES_P029
      #define USES_P029   // Output
    #endif
#endif

#if FEATURE_NO_HTTP_CLIENT  // Disable HTTP features
  // Disable HTTP related Controllers/features
  #ifdef FEATURE_SEND_TO_HTTP
    #undef FEATURE_SEND_TO_HTTP
  #endif
  #define FEATURE_SEND_TO_HTTP  0 // Disabled
  #ifdef FEATURE_POST_TO_HTTP
    #undef FEATURE_POST_TO_HTTP
  #endif
  #define FEATURE_POST_TO_HTTP  0 // Disabled
#endif


// Disable Homie plugin for now in the dev build to make it fit.
#if defined(PLUGIN_BUILD_DEV) && defined(USES_C014)
  #undef USES_C014
#endif

// VCC builds need a bit more, disable timing stats to make it fit.
#ifndef PLUGIN_BUILD_CUSTOM
  #if FEATURE_ADC_VCC && !(defined(PLUGIN_SET_MAX) || defined(NO_LIMIT_BUILD_SIZE))
    #ifndef LIMIT_BUILD_SIZE
      #define LIMIT_BUILD_SIZE
    #endif
    #ifndef NOTIFIER_SET_NONE
      #define NOTIFIER_SET_NONE
    #endif

  #endif
  #ifdef ESP8266_1M
    #ifndef LIMIT_BUILD_SIZE
      #define LIMIT_BUILD_SIZE
    #endif
    #ifndef NOTIFIER_SET_NONE
      #define NOTIFIER_SET_NONE
    #endif
    #ifndef DISABLE_SC16IS752_Serial
      #define DISABLE_SC16IS752_Serial
    #endif
  #endif
#endif


// Due to size restrictions, disable a few plugins/controllers for 1M builds
#ifdef ESP8266_1M
  #ifdef USES_C003
    #undef USES_C003
  #endif
  #ifdef USES_C016
    #undef USES_C016  // Cache controller
  #endif
  #ifdef FEATURE_SD
    #undef FEATURE_SD  // Unlikely on 1M units
  #endif
  #define FEATURE_SD 0
  #define NO_GLOBAL_SD
  #ifndef LIMIT_BUILD_SIZE
    #define LIMIT_BUILD_SIZE
  #endif
  #ifdef FEATURE_EXT_RTC
    #undef FEATURE_EXT_RTC
  #endif
  #define FEATURE_EXT_RTC 0
  #ifndef BUILD_NO_DEBUG
    #define BUILD_NO_DEBUG
  #endif
#endif

#if defined(PLUGIN_BUILD_MAX_ESP32) || defined(NO_LIMIT_BUILD_SIZE)
  #ifdef LIMIT_BUILD_SIZE
    #undef LIMIT_BUILD_SIZE
  #endif
#endif

// Disable some diagnostic parts to make builds fit.
#ifdef LIMIT_BUILD_SIZE
  #ifdef WEBSERVER_TIMINGSTATS
    #undef WEBSERVER_TIMINGSTATS
  #endif

  // Do not include large blobs but fetch them from CDN
  #ifndef WEBSERVER_USE_CDN_JS_CSS
    #define WEBSERVER_USE_CDN_JS_CSS
  #endif
  #ifdef WEBSERVER_CSS
      #undef WEBSERVER_CSS
  #endif
  #ifndef WEBSERVER_EMBED_CUSTOM_CSS
    #ifdef EMBED_ESPEASY_DEFAULT_MIN_CSS
      #undef EMBED_ESPEASY_DEFAULT_MIN_CSS
    #endif
  #endif
  #ifdef WEBSERVER_INCLUDE_JS
      #undef WEBSERVER_INCLUDE_JS
  #endif
  #ifdef EMBED_ESPEASY_DEFAULT_MIN_CSS
    #undef EMBED_ESPEASY_DEFAULT_MIN_CSS
  #endif

  #ifdef WEBSERVER_GITHUB_COPY
    #undef WEBSERVER_GITHUB_COPY
  #endif
  #ifdef WEBSERVER_CUSTOM
    // TD-er: Removing WEBSERVER_CUSTOM does free up another 1.7k
//    #undef WEBSERVER_CUSTOM
  #endif


  #ifndef BUILD_NO_DEBUG
    #define BUILD_NO_DEBUG
  #endif
  #ifndef BUILD_NO_SPECIAL_CHARACTERS_STRINGCONVERTER
    #define BUILD_NO_SPECIAL_CHARACTERS_STRINGCONVERTER
  #endif
  #ifndef KEEP_I2C_MULTIPLEXER
    #ifdef FEATURE_I2CMULTIPLEXER
      #undef FEATURE_I2CMULTIPLEXER
    #endif
    #define FEATURE_I2CMULTIPLEXER  0
  #endif
  #ifdef FEATURE_SETTINGS_ARCHIVE
    #undef FEATURE_SETTINGS_ARCHIVE
  #endif
  #define FEATURE_SETTINGS_ARCHIVE  0

  #ifdef FEATURE_SERVO
    #undef FEATURE_SERVO
  #endif
  #define FEATURE_SERVO 0
  #ifdef FEATURE_RTTTL
    #undef FEATURE_RTTTL
  #endif
  #define FEATURE_RTTTL 0
  #ifdef FEATURE_TOOLTIPS
    #undef FEATURE_TOOLTIPS
  #endif
  #define FEATURE_TOOLTIPS  0
  #ifdef FEATURE_BLYNK
    #undef FEATURE_BLYNK
  #endif
  #define FEATURE_BLYNK 0
  #if !defined(PLUGIN_SET_COLLECTION) && !defined(PLUGIN_SET_SONOFF_POW)
    #ifdef USES_P076
      #undef USES_P076   // HWL8012   in POW r1
    #endif
    #ifdef USES_P093
      #undef USES_P093   // Mitsubishi Heat Pump
    #endif
    #ifdef USES_P100 // Pulse Counter - DS2423
      #undef USES_P100
    #endif
    #ifdef USES_C017 // Zabbix
      #undef USES_C017
    #endif
  #endif
  #ifdef USES_C012
    #undef USES_C012 // Blynk
  #endif
  #ifdef USES_C015
    #undef USES_C015 // Blynk
  #endif
  #ifdef USES_C016
    #undef USES_C016 // Cache controller
  #endif
  #ifdef USES_C018
    #undef USES_C018 // LoRa TTN - RN2483/RN2903
  #endif
  #if defined(FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES) && !defined(KEEP_TRIGONOMETRIC_FUNCTIONS_RULES)
    #undef FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES
  #endif
  #ifndef KEEP_TRIGONOMETRIC_FUNCTIONS_RULES
    #define FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES 0
  #endif
  #ifdef FEATURE_SSDP
    #undef FEATURE_SSDP
  #endif
  #define FEATURE_SSDP  0
  #ifdef FEATURE_PLUGIN_STATS
    #undef FEATURE_PLUGIN_STATS
  #endif
  #define FEATURE_PLUGIN_STATS  0
  #ifdef FEATURE_CHART_JS
    #undef FEATURE_CHART_JS
  #endif
  #define FEATURE_CHART_JS  0
  #ifdef FEATURE_RULES_EASY_COLOR_CODE
    #undef FEATURE_RULES_EASY_COLOR_CODE
  #endif
  #define FEATURE_RULES_EASY_COLOR_CODE 0
  #if FEATURE_EXT_RTC
    #undef FEATURE_EXT_RTC
    #define FEATURE_EXT_RTC 0
  #endif

  #ifdef FEATURE_DNS_SERVER
    #undef FEATURE_DNS_SERVER
  #endif
  #define FEATURE_DNS_SERVER 0

  #ifdef FEATURE_MDNS
    #undef FEATURE_MDNS
  #endif
  #define FEATURE_MDNS 0

  #ifdef FEATURE_ARDUINO_OTA
    #undef FEATURE_ARDUINO_OTA
  #endif
  #define FEATURE_ARDUINO_OTA 0
#endif

// Timing stats page needs timing stats
#if defined(WEBSERVER_TIMINGSTATS) && !FEATURE_TIMING_STATS
  #define FEATURE_TIMING_STATS  1
#endif

// If timing stats page is not included, there is no need in collecting the stats
#if !defined(WEBSERVER_TIMINGSTATS) && FEATURE_TIMING_STATS
  #undef FEATURE_TIMING_STATS
  #define FEATURE_TIMING_STATS  0
#endif


#ifdef BUILD_NO_DEBUG
  #ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
    #define BUILD_NO_DIAGNOSTIC_COMMANDS
  #endif
  #ifndef BUILD_NO_RAM_TRACKER
    #define BUILD_NO_RAM_TRACKER
  #endif
#endif

  // Do not include large blobs but fetch them from CDN
#ifdef WEBSERVER_USE_CDN_JS_CSS
  #ifdef WEBSERVER_FAVICON
    #ifndef WEBSERVER_FAVICON_CDN
      #define WEBSERVER_FAVICON_CDN
    #endif
  #endif
  #ifdef WEBSERVER_CSS
    #undef WEBSERVER_CSS
  #endif
  #ifdef WEBSERVER_INCLUDE_JS
    #undef WEBSERVER_INCLUDE_JS
  #endif
  #ifdef EMBED_ESPEASY_DEFAULT_MIN_CSS
    #undef EMBED_ESPEASY_DEFAULT_MIN_CSS
  #endif
#endif

#if defined(USES_C002) || defined (USES_C005) || defined(USES_C006) || defined(USES_C014) || defined(USES_P037)
  #define FEATURE_MQTT  1
#endif

#if defined(USES_C012) || defined (USES_C015)
  #define FEATURE_BLYNK 1
#endif

// Specific notifier plugins may be enabled via Custom.h, regardless
// whether NOTIFIER_SET_NONE is defined
#if defined(USES_N001) || defined(USES_N002)
  #ifndef FEATURE_NOTIFIER
    #define FEATURE_NOTIFIER  1
  #endif
#endif

// Cache Controller Reader plugin needs Cache Controller
#if defined(USES_P146) && !defined(USES_C016)
  #define USES_C016
#endif

#if defined(USES_P146) || defined(USES_C016)
  #ifdef FEATURE_RTC_CACHE_STORAGE
    #undef FEATURE_RTC_CACHE_STORAGE
  #endif
  #define FEATURE_RTC_CACHE_STORAGE 1
#endif



// P098 PWM motor needs P003 pulse
#if defined(USES_P098)
  #ifndef USES_P003
    #define USES_P003
  #endif
#endif

#if FEATURE_MQTT
// MQTT_MAX_PACKET_SIZE : Maximum packet size
#ifndef MQTT_MAX_PACKET_SIZE
  #define MQTT_MAX_PACKET_SIZE 1024 // Is also used in PubSubClient
#endif
#endif //if FEATURE_MQTT


// It may have gotten undefined to fit a build. Make sure the Blynk controllers are not defined
#if !FEATURE_BLYNK
  #ifdef USES_C012
    #undef USES_C012
  #endif
  #ifdef USES_C015
    #undef USES_C015
  #endif
#endif

#if FEATURE_ARDUINO_OTA
  #ifndef LIMIT_BUILD_SIZE
    #ifndef FEATURE_MDNS
      #define FEATURE_MDNS  1
    #endif
  #endif
#endif

#if FEATURE_MDNS
  #ifndef FEATURE_DNS_SERVER
    #define FEATURE_DNS_SERVER  1
  #endif
#endif

#ifdef WEBSERVER_SETUP
  #ifndef PLUGIN_BUILD_MINIMAL_OTA
    #ifndef FEATURE_DNS_SERVER
      #define FEATURE_DNS_SERVER  1
    #endif
  #endif
#endif

#if FEATURE_SETTINGS_ARCHIVE || FEATURE_CUSTOM_PROVISIONING
  #ifndef FEATURE_DOWNLOAD
    #define FEATURE_DOWNLOAD  1
  #endif
#endif

// Here we can re-enable specific features in the COLLECTION sets as we have created some space there by splitting them up
#if defined(COLLECTION_FEATURE_RTTTL) && (defined(PLUGIN_SET_COLLECTION_A) || defined(PLUGIN_SET_COLLECTION_B) || defined(PLUGIN_SET_COLLECTION_C) || defined(PLUGIN_SET_COLLECTION_D) || defined(PLUGIN_SET_COLLECTION_E) || defined(PLUGIN_SET_COLLECTION_F))
  #ifndef FEATURE_RTTTL
    #define FEATURE_RTTTL 1
  #endif
#endif

#ifdef USES_ESPEASY_NOW
  #if defined(LIMIT_BUILD_SIZE) || defined(ESP8266_1M) || (defined(ESP8266) && defined(PLUGIN_BUILD_IR))
    // Will not fit on ESP8266 along with IR plugins included
    #undef USES_ESPEASY_NOW
  #endif
#endif

#if defined(USES_C019) && !defined(USES_ESPEASY_NOW)
  // C019 depends on ESPEASY_NOW, so don't use it if ESPEasy_NOW is excluded
  #undef USES_C019
#endif

#if defined(USES_C019) && !defined(FEATURE_PACKED_RAW_DATA)
  #define FEATURE_PACKED_RAW_DATA  1
#endif



// By default we enable the SHOW_SYSINFO_JSON when we enable the WEBSERVER_NEW_UI
#ifdef WEBSERVER_NEW_UI
  #define SHOW_SYSINFO_JSON 1
#endif

#ifdef USES_ESPEASY_NOW
  // ESPEasy-NOW needs the P2P feature
  #ifdef FEATURE_ESPEASY_P2P
    #undef FEATURE_ESPEASY_P2P
  #endif
  #define FEATURE_ESPEASY_P2P 1
#endif

#if !defined(ESP32) && defined(USES_P148)
  // This chip/display is only used on ESP32 devices made by Sonoff
  #undef USES_P148   // Sonoff POWR3xxD and THR3xxD display
#endif

#ifndef FEATURE_ZEROFILLED_UNITNUMBER
  #ifdef ESP8266_1M
    #define FEATURE_ZEROFILLED_UNITNUMBER    0
  #else
    #define FEATURE_ZEROFILLED_UNITNUMBER    1
  #endif
#endif



// Make sure all features which have not been set exclusively will be disabled.
// This should be done at the end of this file.
// Keep them alfabetically sorted so it is easier to add new ones

#ifndef FEATURE_BLYNK                         
#define FEATURE_BLYNK                         0
#endif

#ifndef FEATURE_CHART_JS                      
#define FEATURE_CHART_JS                      0
#endif

#ifndef FEATURE_RULES_EASY_COLOR_CODE
#define FEATURE_RULES_EASY_COLOR_CODE         0
#endif


#ifndef FEATURE_CUSTOM_PROVISIONING           
#define FEATURE_CUSTOM_PROVISIONING           0
#endif

#ifndef FEATURE_RTC_CACHE_STORAGE
#define FEATURE_RTC_CACHE_STORAGE             0
#endif

#ifndef FEATURE_DNS_SERVER                    
#define FEATURE_DNS_SERVER                    0
#endif

#ifndef FEATURE_DOMOTICZ                      
#define FEATURE_DOMOTICZ                      0
#endif

#ifndef FEATURE_DOWNLOAD                      
#define FEATURE_DOWNLOAD                      0
#endif

#ifndef FEATURE_ESPEASY_P2P                      
#define FEATURE_ESPEASY_P2P                   0
#endif

#ifndef FEATURE_ETHERNET                      
#define FEATURE_ETHERNET                      0
#endif

#ifndef FEATURE_EXT_RTC                       
#define FEATURE_EXT_RTC                       0
#endif

#ifndef FEATURE_FHEM                          
#define FEATURE_FHEM                          0
#endif

#ifndef FEATURE_GPIO_USE_ESP8266_WAVEFORM
 #ifdef ESP8266
  #define FEATURE_GPIO_USE_ESP8266_WAVEFORM   1
 #else
  #define FEATURE_GPIO_USE_ESP8266_WAVEFORM   0
 #endif
#endif

#ifndef FEATURE_HOMEASSISTANT_OPENHAB         
#define FEATURE_HOMEASSISTANT_OPENHAB         0
#endif

#ifndef FEATURE_I2CMULTIPLEXER                
#define FEATURE_I2CMULTIPLEXER                0
#endif

#ifndef FEATURE_I2C_DEVICE_SCAN               
#define FEATURE_I2C_DEVICE_SCAN               0
#endif

#ifndef FEATURE_MDNS                          
#define FEATURE_MDNS                          0
#endif

#ifndef FEATURE_MODBUS                        
#define FEATURE_MODBUS                        0
#endif

#ifndef FEATURE_MQTT                        
#define FEATURE_MQTT                          0
#endif

#ifndef FEATURE_NON_STANDARD_24_TASKS         
#define FEATURE_NON_STANDARD_24_TASKS         0
#endif

#ifndef FEATURE_NOTIFIER                      
#define FEATURE_NOTIFIER                      0
#endif

#ifndef FEATURE_PACKED_RAW_DATA               
#define FEATURE_PACKED_RAW_DATA               0
#endif

#ifndef FEATURE_PLUGIN_STATS                  
#define FEATURE_PLUGIN_STATS                  0
#endif

#ifndef FEATURE_REPORTING                     
#define FEATURE_REPORTING                     0
#endif

#ifndef FEATURE_RTTTL                         
#define FEATURE_RTTTL                         0
#endif
#if defined(FEATURE_RTTTL) && !FEATURE_RTTTL && defined(KEEP_RTTTL)
  #undef FEATURE_RTTTL
  #define FEATURE_RTTTL 1
#endif

#ifndef FEATURE_SD                         
#define FEATURE_SD                            0
#endif

#ifndef FEATURE_SERVO                         
#define FEATURE_SERVO                         0
#endif

#ifndef FEATURE_SETTINGS_ARCHIVE              
#define FEATURE_SETTINGS_ARCHIVE              0
#endif

#ifndef FEATURE_SSDP                          
#define FEATURE_SSDP                          0
#endif

#ifndef FEATURE_TIMING_STATS                  
#define FEATURE_TIMING_STATS                  0
#endif

#ifndef FEATURE_TOOLTIPS                      
#define FEATURE_TOOLTIPS                      0
#endif

#ifndef FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES 
#define FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES 0
#endif


#ifndef SHOW_SYSINFO_JSON
#define SHOW_SYSINFO_JSON 0
#endif


#ifndef FEATURE_SEND_TO_HTTP
  #define FEATURE_SEND_TO_HTTP  1 // Enabled by default
#endif

#ifndef FEATURE_POST_TO_HTTP
  #define FEATURE_POST_TO_HTTP  1 // Enabled by default
#endif

#ifndef FEATURE_HTTP_CLIENT
  #define FEATURE_HTTP_CLIENT   0 // Disable by default
#endif

#ifndef FEATURE_PLUGIN_PRIORITY
  #define FEATURE_PLUGIN_PRIORITY   0 // Disable by default
#endif

#ifndef FEATURE_I2C_DEVICE_CHECK
  #ifdef ESP8266_1M
    #define FEATURE_I2C_DEVICE_CHECK  0 // Disabled by default for 1M units
  #else
    #define FEATURE_I2C_DEVICE_CHECK  1 // Enabled by default
  #endif
#endif

#ifndef FEATURE_I2C_GET_ADDRESS
  #ifdef ESP8266_1M
    #define FEATURE_I2C_GET_ADDRESS   0 // Disabled by default for 1M units
  #else
    #define FEATURE_I2C_GET_ADDRESS   1 // Enabled by default
  #endif
#endif

#if FEATURE_I2C_DEVICE_CHECK && !FEATURE_I2C_GET_ADDRESS
  #undef FEATURE_I2C_GET_ADDRESS
  #define FEATURE_I2C_GET_ADDRESS     1 // Needed by FEATURE_I2C_DEVICE_CHECK
#endif

#if !FEATURE_HTTP_CLIENT && (defined(USES_C001) || defined(USES_C008) || defined(USES_C009) || defined(USES_C011) || (defined(FEATURE_SEND_TO_HTTP) && FEATURE_SEND_TO_HTTP) || (defined(FEATURE_POST_TO_HTTP) && FEATURE_POST_TO_HTTP) || (defined(FEATURE_DOWNLOAD) && FEATURE_DOWNLOAD) || (defined(FEATURE_SETTINGS_ARCHIVE) && FEATURE_SETTINGS_ARCHIVE))
  #undef FEATURE_HTTP_CLIENT
  #define FEATURE_HTTP_CLIENT   1 // Enable because required for these controllers/features
#endif

#ifndef FEATURE_AUTO_DARK_MODE
  #ifdef LIMIT_BUILD_SIZE
    #define FEATURE_AUTO_DARK_MODE            0
  #else
    #define FEATURE_AUTO_DARK_MODE            1
  #endif
#endif

#ifndef FEATURE_ESP8266_DIRECT_WIFI_SCAN
  // Feature still in development, do not yet use.
  #define FEATURE_ESP8266_DIRECT_WIFI_SCAN    0
#endif

#if FEATURE_ESP8266_DIRECT_WIFI_SCAN
  #ifdef ESP32
    // ESP8266 only feature
    #undef FEATURE_ESP8266_DIRECT_WIFI_SCAN
    #define FEATURE_ESP8266_DIRECT_WIFI_SCAN    0
  #endif
#endif

#ifndef FEATURE_PINSTATE_EXTENDED
  #ifdef ESP8266_1M
    #define FEATURE_PINSTATE_EXTENDED           0 // Don't use extended pinstate feature on 1M builds
  #else
    #define FEATURE_PINSTATE_EXTENDED           1 // Enable by default for all other builds
  #endif
#endif

#if !FEATURE_PLUGIN_PRIORITY && (defined(USES_P137) /*|| defined(USES_Pxxx)*/)
  #undef FEATURE_PLUGIN_PRIORITY
  #define FEATURE_PLUGIN_PRIORITY   1
#endif

// Enable FEATURE_ADC_VCC to measure supply voltage using the analog pin
// Please note that the TOUT pin has to be disconnected in this mode
// Use the "System Info" device to read the VCC value
#ifndef FEATURE_ADC_VCC
  #define FEATURE_ADC_VCC                  0
#endif

#endif // CUSTOMBUILD_DEFINE_PLUGIN_SETS_H
