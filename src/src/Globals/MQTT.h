#ifndef GLOBALS_MQTT_H
#define GLOBALS_MQTT_H

#include "../../ESPEasy_common.h"


#if FEATURE_MQTT
# include "../Helpers/LongTermTimer.h"

# include <WiFiClient.h>
# include <PubSubClient.h>

# if FEATURE_MQTT_TLS
  #include <WiFiClientSecureLightBearSSL.h>
# endif // if FEATURE_MQTT_TLS

// MQTT client
extern WiFiClient mqtt;
# if FEATURE_MQTT_TLS
extern String  mqtt_tls_last_errorstr;
extern int32_t mqtt_tls_last_error;
extern BearSSL::WiFiClientSecure_light* mqtt_tls;
//extern BearSSL::X509List mqtt_X509List;

extern String mqtt_rootCA;
extern String mqtt_fingerprint;

# endif  // if FEATURE_MQTT_TLS
extern PubSubClient MQTTclient;
extern bool MQTTclient_should_reconnect;
extern bool MQTTclient_must_send_LWT_connected;
extern bool MQTTclient_connected;
extern int  mqtt_reconnect_count;
extern LongTermTimer MQTTclient_next_connect_attempt;
#endif // if FEATURE_MQTT

#ifdef USES_P037

// mqtt import status
extern bool P037_MQTTImport_connected;
#endif // ifdef USES_P037


#endif // GLOBALS_MQTT_H
