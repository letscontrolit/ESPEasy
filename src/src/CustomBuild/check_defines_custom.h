#ifndef CUSTOMBUILD_CHECK_DEFINES_CUSTOM_H
#define CUSTOMBUILD_CHECK_DEFINES_CUSTOM_H

/**
 * Check if any of the now renamed #define variables is still used and output a warning during compilation
 *
 * Changelog:
 * ----------
 * 2022-07-27 tonhuisman: Move file to CustomBuild folder, and also check for any regular build (like when using pre_custom*.py scripts)
 * 2022-07-26 tonhuisman: Initial checks, disable formatting by Uncrustify as it messes up the url's
 */

/* *INDENT-OFF* */

#ifdef USE_DOWNLOAD
# warning "Custom.h has '#define USE_DOWNLOAD' to be replaced with '#define FEATURE_DOWNLOAD 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_DOWNLOAD  1 // Define correct alternative
# undef USE_DOWNLOAD         // remove
#endif // ifdef USE_DOWNLOAD

#ifdef USE_CUSTOM_PROVISIONING
# warning "Custom.h has '#define USE_CUSTOM_PROVISIONING' to be replaced with '#define FEATURE_CUSTOM_PROVISIONING 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_CUSTOM_PROVISIONING  1
# undef USE_CUSTOM_PROVISIONING
#endif // ifdef USE_CUSTOM_PROVISIONING

#ifdef USE_SETTINGS_ARCHIVE
# warning "Custom.h has '#define USE_SETTINGS_ARCHIVE' to be replaced with '#define FEATURE_SETTINGS_ARCHIVE 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_SETTINGS_ARCHIVE  1
# undef USE_SETTINGS_ARCHIVE
#endif // ifdef USE_SETTINGS_ARCHIVE

#ifdef USE_TRIGONOMETRIC_FUNCTIONS_RULES
# warning "Custom.h has '#define USE_TRIGONOMETRIC_FUNCTIONS_RULES' to be replaced with '#define FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_TRIGONOMETRIC_FUNCTIONS_RULES  1
# undef USE_TRIGONOMETRIC_FUNCTIONS_RULES
#endif // ifdef USE_TRIGONOMETRIC_FUNCTIONS_RULES

#if defined(FEATURE_I2CMULTIPLEXER) && (2 - FEATURE_I2CMULTIPLEXER - 2 == 4) // 'Defined but empty' check
# warning "Custom.h has '#define FEATURE_I2CMULTIPLEXER' to be replaced with '#define FEATURE_I2CMULTIPLEXER 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# undef FEATURE_I2CMULTIPLEXER
# define FEATURE_I2CMULTIPLEXER  1
#endif // if defined(FEATURE_I2CMULTIPLEXER) && (2-FEATURE_I2CMULTIPLEXER-2 == 4)

#ifdef USE_SSDP
# warning "Custom.h has '#define USE_SSDP' to be replaced with '#define FEATURE_SSDP 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_SSDP  1
# undef USE_SSDP
#endif // ifdef USE_SSDP

#ifdef USE_TIMING_STATS
# warning "Custom.h has '#define USE_TIMING_STATS' to be replaced with '#define FEATURE_TIMING_STATS 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_TIMING_STATS  1
# undef USE_TIMING_STATS
#endif // ifdef USE_TIMING_STATS

#ifdef USE_EXT_RTC
# warning "Custom.h has '#define USE_EXT_RTC' to be replaced with '#define FEATURE_EXT_RTC 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_EXT_RTC  1
# undef USE_EXT_RTC
#endif // ifdef USE_EXT_RTC

#ifdef ENABLE_TOOLTIPS
# warning "Custom.h has '#define ENABLE_TOOLTIPS' to be replaced with '#define FEATURE_TOOLTIPS 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_TOOLTIPS  1
# undef ENABLE_TOOLTIPS
#endif // ifdef ENABLE_TOOLTIPS

#ifdef HAS_ETHERNET
# warning "Custom.h has '#define HAS_ETHERNET' to be replaced with '#define FEATURE_ETHERNET 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_ETHERNET  1
# undef HAS_ETHERNET
#endif // ifdef HAS_ETHERNET

#ifdef USES_DOMOTICZ
# warning "Custom.h has '#define USES_DOMOTICZ' to be replaced with '#define FEATURE_DOMOTICZ 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_DOMOTICZ  1
# undef USES_DOMOTICZ
#endif // ifdef USES_DOMOTICZ

#ifdef USES_FHEM
# warning "Custom.h has '#define USES_FHEM' to be replaced with '#define FEATURE_FHEM 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_FHEM  1
# undef USES_FHEM
#endif // ifdef USES_FHEM

#ifdef USES_HOMEASSISTANT_OPENHAB
# warning "Custom.h has '#define USES_HOMEASSISTANT_OPENHAB' to be replaced with '#define FEATURE_HOMEASSISTANT_OPENHAB 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_HOMEASSISTANT_OPENHAB  1
# undef USES_HOMEASSISTANT_OPENHAB
#endif // ifdef USES_HOMEASSISTANT_OPENHAB

#ifdef USES_PLUGIN_STATS
# warning "Custom.h has '#define USES_PLUGIN_STATS' to be replaced with '#define FEATURE_PLUGIN_STATS 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_PLUGIN_STATS  1
# undef USES_PLUGIN_STATS
#endif // ifdef USES_PLUGIN_STATS

#ifdef USES_CHART_JS
# warning "Custom.h has '#define USES_CHART_JS' to be replaced with '#define FEATURE_CHART_JS 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_CHART_JS  1
# undef USES_CHART_JS
#endif // ifdef USES_CHART_JS

#ifdef USE_SERVO
# warning "Custom.h has '#define USE_SERVO' to be replaced with '#define FEATURE_SERVO 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_SERVO  1
# undef USE_SERVO
#endif // ifdef USE_SERVO

#ifdef USE_RTTTL
# warning "Custom.h has '#define USE_RTTTL' to be replaced with '#define FEATURE_RTTTL 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_RTTTL  1
# undef USE_RTTTL
#endif // ifdef USE_RTTTL

#if USE_I2C_DEVICE_SCAN
# warning "Custom.h has '#define USE_I2C_DEVICE_SCAN true' to be replaced with '#define FEATURE_I2C_DEVICE_SCAN 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_I2C_DEVICE_SCAN  1
# undef USE_I2C_DEVICE_SCAN
#endif // if USE_I2C_DEVICE_SCAN

#if defined(FEATURE_MDNS) && (2 - FEATURE_MDNS - 2 == 4) // 'Defined but empty' check
# warning "Custom.h has '#define FEATURE_MDNS' to be replaced with '#define FEATURE_MDNS 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# undef FEATURE_MDNS
# define FEATURE_MDNS  1
#endif // if defined(FEATURE_MDNS) && (2-FEATURE_MDNS-2 == 4)

#ifdef USES_NOTIFIER
# warning "Custom.h has '#define USES_NOTIFIER' to be replaced with '#define FEATURE_NOTIFIER 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_NOTIFIER  1
# undef USES_NOTIFIER
#endif // ifdef USES_NOTIFIER

#ifdef USES_MODBUS
# warning "Custom.h has '#define USES_MODBUS' to be replaced with '#define FEATURE_MODBUS 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_MODBUS  1
# undef USES_MODBUS
#endif // ifdef USES_MODBUS

#ifdef USE_NON_STANDARD_24_TASKS
# warning "Custom.h has '#define USE_NON_STANDARD_24_TASKS' to be replaced with '#define FEATURE_NON_STANDARD_24_TASKS 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_NON_STANDARD_24_TASKS  1
# undef USE_NON_STANDARD_24_TASKS
#endif // ifdef USE_NON_STANDARD_24_TASKS

#ifdef USES_PACKED_RAW_DATA
# warning "Custom.h has '#define USES_PACKED_RAW_DATA' to be replaced with '#define FEATURE_PACKED_RAW_DATA 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_PACKED_RAW_DATA  1
# undef USES_PACKED_RAW_DATA
#endif // ifdef USES_PACKED_RAW_DATA

#ifdef USES_BLYNK
# warning "Custom.h has '#define USES_BLYNK' to be replaced with '#define FEATURE_BLYNK 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# define FEATURE_BLYNK  1
# undef USES_BLYNK
#endif // ifdef USES_BLYNK

#if defined(FEATURE_REPORTING) && (2 - FEATURE_REPORTING - 2 == 4) // 'Defined but empty' check
# warning "Custom.h has '#define FEATURE_REPORTING' to be replaced with '#define FEATURE_REPORTING 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# undef FEATURE_REPORTING
# define FEATURE_REPORTING  1
#endif // if defined(FEATURE_REPORTING) && (2-FEATURE_REPORTING-2 == 4)

#if defined(FEATURE_DNS_SERVER) && (2 - FEATURE_DNS_SERVER - 2 == 4) // 'Defined but empty' check
# warning "Custom.h has '#define FEATURE_DNS_SERVER' to be replaced with '#define FEATURE_DNS_SERVER 1', see https://github.com/letscontrolit/ESPEasy/pull/4153"
# undef FEATURE_DNS_SERVER
# define FEATURE_DNS_SERVER  1
#endif // if defined(FEATURE_DNS_SERVER) && (2-FEATURE_DNS_SERVER-2 == 4)

/* *INDENT-ON* */

#endif // ifndef CUSTOMBUILD_CHECK_DEFINES_CUSTOM_H
