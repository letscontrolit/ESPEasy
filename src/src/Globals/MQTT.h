#ifndef GLOBALS_MQTT_H
#define GLOBALS_MQTT_H

#include "../../ESPEasy_common.h"


#if FEATURE_MQTT
# include "../DataTypes/ControllerIndex.h"
# include "../DataTypes/TaskIndex.h"

# include "../Helpers/LongTermTimer.h"

# include <WiFiClient.h>
# include <PubSubClient.h>

# if FEATURE_MQTT_TLS
  #  include <WiFiClientSecureLightBearSSL.h>
# endif // if FEATURE_MQTT_TLS

// MQTT client
extern WiFiClient mqtt;
# if FEATURE_MQTT_TLS
extern String  mqtt_tls_last_errorstr;
extern int32_t mqtt_tls_last_error;
extern BearSSL::WiFiClientSecure_light*mqtt_tls;

// extern BearSSL::X509List mqtt_X509List;

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

#if FEATURE_MQTT_DISCOVER
extern controllerIndex_t mqttDiscoveryController;
extern taskIndex_t mqttDiscoverOnlyTask;
extern uint32_t    mqttDiscoveryTimeout;
# define MQTT_DISCOVERY_MAX_DELAY_0_1_SECONDS 300 // 300 x 0.1 sec. max random delay before starting Auto Discovery, minimum fixed at 1 sec
#endif // if FEATURE_MQTT_DISCOVER

#endif // GLOBALS_MQTT_H
