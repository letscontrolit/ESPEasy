/*
 Basic MQTT example with Authentication

  - connects to an MQTT server, providing username
    and password
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic"
*/

#include <Ethernet.h>
#include <PubSubClient.h>
#include <SPI.h>

#define _UNUSED_ __attribute__((unused))

// Update these with values suitable for your network.
byte mac[] = {0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED};
IPAddress ip(172, 16, 0, 100);
IPAddress server(172, 16, 0, 2);
const char HELLO_WORLD_3[] PROGMEM = "hello world 3";
const char HELLO_WORLD_4[] PROGMEM = "hello world 4";
const char SUBSCRIBE_TOPIC[] PROGMEM = "inTopic1";

void callback(_UNUSED_ char* topic, _UNUSED_ uint8_t* payload, _UNUSED_ size_t plength) {
    // handle message arrived
}

EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);

void setup() {
    Ethernet.begin(mac, ip);
    // Note - the default maximum packet size is 128 bytes. If the
    // combined length of clientId, username and password exceed this use the
    // following to increase the buffer size:
    // client.setBufferSize(255);

    if (client.connect("arduinoClient", "testuser", "testpass")) {
        client.publish(F("outTopic"), "hello world 1", MQTT_QOS0, false);
        client.publish(F("outTopic"), F("hello world 2"), MQTT_QOS1, false);
        client.publish_P(F("outTopic"), HELLO_WORLD_3, MQTT_QOS2, false);

        client.beginPublish(F("outTopic"), strlen_P(HELLO_WORLD_4), MQTT_QOS1, false);
        client.write_P(HELLO_WORLD_4);
        client.endPublish();

        client.subscribe(F("inTopic"));
        client.subscribe_P(SUBSCRIBE_TOPIC, MQTT_QOS1);
    }
}

void loop() {
    client.loop();
}
