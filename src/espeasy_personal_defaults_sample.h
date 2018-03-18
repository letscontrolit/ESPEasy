#ifndef ESPEASY_PERSONAL_DEFAULTS_H_
#define ESPEASY_PERSONAL_DEFAULTS_H_
// ********************************************************************************
//   User specific configuration, not updated via GIT updates
// ********************************************************************************


// --- Basic Config Settings ------------------------------------------------------------------------
// #define DEFAULT_NAME        "ESP_Easy"          // Enter your device friendly name
// #define UNIT                0                   // Unit Number
// #define DEFAULT_DELAY       60                  // Sleep Delay in seconds

// --- Wifi AP Mode (when your Wifi Network is not reachable) ----------------------------------------
// #define DEFAULT_AP_IP       192,168,4,1         // Enter IP address (comma separated) for AP (config) mode
// #define DEFAULT_AP_KEY      "configesp"         // Enter network WPA key for AP (config) mode

// --- Wifi Client Mode -----------------------------------------------------------------------------
// #define DEFAULT_SSID        "ssid"              // Enter your Wifi network SSID
// #define DEFAULT_KEY         "wpakey"            // Enter your Wifi network WPA key

// #define DEFAULT_USE_STATIC_IP   false           // (true|false) enabled or disabled static IP
// #define DEFAULT_IP          "192.168.0.50"      // Enter your IP address
// #define DEFAULT_DNS         "192.168.0.1"       // Enter your DNS
// #define DEFAULT_GW          "192.168.0.1"       // Enter your Gateway
// #define DEFAULT_SUBNET      "255.255.255.0"     // Enter your Subnet
// #define DEFAULT_IPRANGE_LOW  "0.0.0.0"          // Allowed IP range to access webserver
// #define DEFAULT_IPRANGE_HIGH "255.255.255.255"  // Allowed IP range to access webserver
// #define DEFAULT_IP_BLOCK_LEVEL 1                // 0: ALL_ALLOWED  1: LOCAL_SUBNET_ALLOWED  2: ONLY_IP_RANGE_ALLOWED

// --- Default Controller ------------------------------------------------------------------------------
// #define DEFAULT_CONTROLLER   false              // true or false enabled or disabled, set 1st controller defaults
// using a default template, you also need to set a DEFAULT PROTOCOL to a suitable MQTT protocol !
// #define DEFAULT_PUB         "sensors/espeasy/%sysname%/%tskname%/%valname%" // Enter your pub
// #define DEFAULT_SUB         "sensors/espeasy/%sysname%/#" // Enter your sub
// #define DEFAULT_SERVER      "192.168.0.8"       // Enter your Server IP address
// #define DEFAULT_PORT        8080                // Enter your Server port value

// #define DEFAULT_PROTOCOL    1                   // Protocol used for controller communications
//   1 = Domoticz HTTP
//   2 = Domoticz MQTT
//   3 = Nodo Telnet
//   4 = ThingSpeak
//   5 = OpenHAB MQTT
//   6 = PiDome MQTT
//   7 = EmonCMS
//   8 = Generic HTTP
//   9 = FHEM HTTP


// --- Advanced Settings ---------------------------------------------------------------------------------
// #define DEFAULT_USE_RULES                       false   // (true|false) Enable Rules?

// #define DEFAULT_MQTT_RETAIN                     false   // (true|false) Retain MQTT messages?
// #define DEFAULT_MQTT_DELAY                      1000    // Time in milliseconds to retain MQTT messages

// #define DEFAULT_USE_NTP                         false   // (true|false) Use NTP Server
// #define DEFAULT_NTP_HOST                        ""              // NTP Server Hostname
// #define DEFAULT_TIME_ZONE                       0               // Time Offset (in minutes)
// #define DEFAULT_USE_DST                         false   // (true|false) Use Daily Time Saving

// #define DEFAULT_SYSLOG_IP                       ""                              // Syslog IP Address
// #define DEFAULT_SYSLOG_LEVEL            0                               // Syslog Log Level
// #define DEFAULT_SERIAL_LOG_LEVEL        LOG_LEVEL_INFO  // Serial Log Level
// #define DEFAULT_WEB_LOG_LEVEL           LOG_LEVEL_INFO  // Web Log Level
// #define DEFAULT_SD_LOG_LEVEL            0                               // SD Card Log Level
// #define DEFAULT_USE_SD_LOG                      false                   // (true|false) Enable Logging to the SD card

// #define DEFAULT_USE_SERIAL                      true    // (true|false) Enable Logging to the Serial Port
// #define DEFAULT_SERIAL_BAUD                     115200  // Serial Port Baud Rate


#endif /* ESPEASY_PERSONAL_DEFAULTS_H_ */
