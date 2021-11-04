#include "../Globals/MQTT.h"

#ifdef USES_MQTT

// MQTT client
WiFiClient mqtt;
# ifdef USE_MQTT_TLS
String  mqtt_tls_last_errorstr;
int32_t mqtt_tls_last_error = 0;

#  ifdef ESP32
WiFiClientSecure mqtt_tls;
#  endif // ifdef ESP32
#  ifdef ESP8266
BearSSL::WiFiClientSecure mqtt_tls;
BearSSL::X509List mqtt_X509List;
#  endif // ifdef ESP8266
char *mqtt_rootCA = nullptr;
# endif  // ifdef USE_MQTT_TLS

PubSubClient MQTTclient(mqtt);
bool MQTTclient_should_reconnect        = true;
bool MQTTclient_must_send_LWT_connected = false;
bool MQTTclient_connected               = false;
int  mqtt_reconnect_count               = 0;
LongTermTimer mqtt_last_connect_attempt;
#endif // USES_MQTT

#ifdef USES_P037

// mqtt import status
bool P037_MQTTImport_connected = false;
#endif // ifdef USES_P037
