#ifndef GLOBALS_MQTT_H
#define GLOBALS_MQTT_H

#include "../../ESPEasy_common.h"


#ifdef USES_MQTT
# include "../Helpers/LongTermTimer.h"

# include <WiFiClient.h>
# include <PubSubClient.h>

# ifdef USE_MQTT_TLS
# ifdef ESP32
#  include "../Helpers/ESPEasy_WiFiClientSecure.h"
# else
#  include <WiFiClientSecure.h>
# endif
# endif // ifdef USE_MQTT_TLS

// MQTT client
extern WiFiClient mqtt;
# ifdef USE_MQTT_TLS
extern String  mqtt_tls_last_errorstr;
extern int32_t mqtt_tls_last_error;
#  ifdef ESP32
extern ESPEasy_WiFiClientSecure* mqtt_tls;
#  endif // ifdef ESP32
#  ifdef ESP8266
extern BearSSL::WiFiClientSecure* mqtt_tls;
extern BearSSL::X509List mqtt_X509List;

#  endif // ifdef ESP8266

extern String mqtt_rootCA;
extern String mqtt_fingerprint;

# endif  // ifdef USE_MQTT_TLS
extern PubSubClient MQTTclient;
extern bool MQTTclient_should_reconnect;
extern bool MQTTclient_must_send_LWT_connected;
extern bool MQTTclient_connected;
extern int  mqtt_reconnect_count;
extern LongTermTimer mqtt_last_connect_attempt;
#endif // USES_MQTT

#ifdef USES_P037

// mqtt import status
extern bool P037_MQTTImport_connected;
#endif // ifdef USES_P037


#endif // GLOBALS_MQTT_H
