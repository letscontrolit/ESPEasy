#include "../Globals/MQTT.h"

#include "../../ESPEasy_common.h"

#if FEATURE_MQTT


// MQTT client
WiFiClient mqtt;
# if FEATURE_MQTT_TLS
String  mqtt_tls_last_errorstr;
int32_t mqtt_tls_last_error = 0;

#  ifdef ESP32
BearSSL::WiFiClientSecure_light*mqtt_tls;
#  endif // ifdef ESP32
#  ifdef ESP8266
BearSSL::WiFiClientSecure*mqtt_tls;
BearSSL::X509List mqtt_X509List;
#  endif // ifdef ESP8266
String mqtt_rootCA;
String mqtt_fingerprint;
# endif  // if FEATURE_MQTT_TLS

PubSubClient MQTTclient(mqtt);
bool MQTTclient_should_reconnect        = true;
bool MQTTclient_must_send_LWT_connected = false;
bool MQTTclient_connected               = false;
int  mqtt_reconnect_count               = 0;
LongTermTimer MQTTclient_next_connect_attempt;

# if FEATURE_MQTT_DISCOVER

controllerIndex_t mqttDiscoveryController = INVALID_CONTROLLER_INDEX;
taskIndex_t mqttDiscoverOnlyTask          = INVALID_TASK_INDEX;
uint32_t    mqttDiscoveryTimeout          = 0; // Decremented in 10 per second, random timeout before discovery is sent, for broker load
                                               // distribution
# endif // if FEATURE_MQTT_DISCOVER
#endif // if FEATURE_MQTT

#ifdef USES_P037

// mqtt import status
bool P037_MQTTImport_connected = false;
#endif // ifdef USES_P037
