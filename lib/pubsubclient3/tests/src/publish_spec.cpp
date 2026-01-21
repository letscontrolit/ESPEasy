#include "BDDTest.h"
#include "Buffer.h"
#include "PubSubClient.h"
#include "ShimClient.h"
#include "trace.h"

byte server[] = {172, 16, 0, 2};

// function declarations
void callback(char* topic, uint8_t* payload, size_t plength);
int test_publish();
int test_publish_bytes();
int test_publish_retained();
int test_publish_retained_2();
int test_publish_not_connected();
int test_publish_too_long();
int test_publish_P();
int test_publish_P_too_long();
int test_publish_empty_topic();
int test_publish_null_payload();
int test_publish_qos1();
int test_publish_qos2();
int test_publish_P_qos1();
int test_publish_P_qos2();

void callback(_UNUSED_ char* topic, _UNUSED_ uint8_t* payload, _UNUSED_ size_t plength) {
    // handle message arrived
}

int test_publish() {
    IT("publishes a null-terminated string");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.respond(connack, sizeof(connack));

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1");
    IS_TRUE(rc);

    byte publish[] = {0x30, 0x0e, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 'p', 'a', 'y', 'l', 'o', 'a', 'd'};
    shimClient.expect(publish, sizeof(publish));

    rc = client.publish("topic", "payload");
    IS_TRUE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int test_publish_bytes() {
    IT("publishes a byte array");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte payload[] = {0x01, 0x02, 0x03, 0x00, 0x05};
    size_t length = sizeof(payload);

    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.respond(connack, sizeof(connack));

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1");
    IS_TRUE(rc);

    byte publish[] = {0x30, 0x0c, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 0x01, 0x02, 0x03, 0x00, 0x05};
    shimClient.expect(publish, sizeof(publish));

    rc = client.publish("topic", payload, length);
    IS_TRUE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int test_publish_retained() {
    IT("publishes retained - 1");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte payload[] = {0x01, 0x02, 0x03, 0x00, 0x05};
    size_t length = sizeof(payload);

    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.respond(connack, sizeof(connack));

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1");
    IS_TRUE(rc);

    byte publish[] = {0x31, 0x0c, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 0x1, 0x2, 0x3, 0x0, 0x5};
    shimClient.expect(publish, sizeof(publish));

    rc = client.publish("topic", payload, length, true);
    IS_TRUE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int test_publish_retained_2() {
    IT("publishes retained - 2");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.respond(connack, sizeof(connack));

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1");
    IS_TRUE(rc);

    byte publish[] = {0x31, 0x0c, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 'A', 'B', 'C', 'D', 'E'};
    shimClient.expect(publish, sizeof(publish));

    rc = client.publish("topic", "ABCDE", true);
    IS_TRUE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int test_publish_not_connected() {
    IT("publish fails when not connected");
    ShimClient shimClient;

    PubSubClient client(server, 1883, callback, shimClient);

    bool rc = client.publish("topic", "payload");
    IS_FALSE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int test_publish_too_long() {
    IT("publish fails when topic/payload are too long");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    //              0        1         2         3         4         5         6         7         8         9         0         1         2
    char topic[] = "1234567890123456789012345678901234567890123456789012345678901234";

    //                0        1         2         3         4         5         6         7         8         9         0         1         2
    char payload[] = "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";

    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.respond(connack, 4);

    PubSubClient client(server, 1883, callback, shimClient);
    client.setBufferSize(64);
    bool rc = client.connect("client_test1");
    IS_TRUE(rc);

    rc = client.publish(topic, payload);
    IS_FALSE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int test_publish_P() {
    IT("publishes using PROGMEM");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte payload[] = {0x01, 0x02, 0x03, 0x00, 0x05};
    size_t length = sizeof(payload);

    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.respond(connack, sizeof(connack));

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1");
    IS_TRUE(rc);

    byte publish[] = {0x31, 0x0c, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 0x01, 0x02, 0x03, 0x00, 0x05};
    shimClient.expect(publish, sizeof(publish));

    rc = client.publish_P("topic", payload, length, true);
    IS_TRUE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int test_publish_P_too_long() {
    IT("publish using PROGMEM fails when topic is too long");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    //              0        1         2         3         4         5         6         7         8         9         0         1         2
    char topic[] = "1234567890123456789012345678901234567890123456789012345678901234";

    //                0        1         2         3         4         5         6         7         8         9         0         1         2
    char payload[] = "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";

    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.respond(connack, sizeof(connack));

    PubSubClient client(server, 1883, callback, shimClient);
    client.setBufferSize(64);
    bool rc = client.connect("client_test1");
    IS_TRUE(rc);

    rc = client.publish_P(topic, payload, false);
    IS_FALSE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int test_publish_empty_topic() {
    IT("publish fails when topic is empty");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.respond(connack, 4);

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1");
    IS_TRUE(rc);

    rc = client.publish("", "payload");
    IS_FALSE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int test_publish_null_payload() {
    IT("publishes with null payload");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.respond(connack, sizeof(connack));

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1");
    IS_TRUE(rc);

    byte publish[] = {0x30, 0x07, 0x00, 0x05, 't', 'o', 'p', 'i', 'c'};
    shimClient.expect(publish, sizeof(publish));

    rc = client.publish("topic", nullptr);
    IS_TRUE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int test_publish_qos1() {
    IT("publishes with QoS 1");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.respond(connack, 4);

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1");
    IS_TRUE(rc);

    // Example publish packet for QoS 1 (0x32)
    byte publish[] = {0x32, 0x10, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 'p', 'a', 'y', 'l', 'o', 'a', 'd', 0x00, 0x02};
    shimClient.expect(publish, sizeof(publish));

    rc = client.publish("topic", "payload", MQTT_QOS1, false);
    IS_TRUE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int test_publish_qos2() {
    IT("publishes with QoS 2");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.respond(connack, 4);

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1");
    IS_TRUE(rc);

    // Example publish packet for QoS 2 (0x34)
    byte publish[] = {0x34, 0x10, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 'p', 'a', 'y', 'l', 'o', 'a', 'd', 0x00, 0x02};
    shimClient.expect(publish, sizeof(publish));

    rc = client.publish("topic", "payload", MQTT_QOS2, false);
    IS_TRUE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int test_publish_P_qos1() {
    IT("publishes using PROGMEM with QoS 1 retained");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte payload[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    size_t length = sizeof(payload);

    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.respond(connack, sizeof(connack));

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1");
    IS_TRUE(rc);

    byte publish[] = {0x33, 0x0e, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x02};
    shimClient.expect(publish, sizeof(publish));

    rc = client.publish_P("topic", payload, length, MQTT_QOS1, true);
    IS_TRUE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int test_publish_P_qos2() {
    IT("publishes using PROGMEM with QoS 2 retained");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte payload[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    size_t length = sizeof(payload);

    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.respond(connack, sizeof(connack));

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1");
    IS_TRUE(rc);

    byte publish[] = {0x35, 0x0e, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x02};
    shimClient.expect(publish, sizeof(publish));

    rc = client.publish_P("topic", payload, length, MQTT_QOS2, true);
    IS_TRUE(rc);

    IS_FALSE(shimClient.error());

    END_IT
}

int main() {
    SUITE("Publish");
    test_publish();
    test_publish_bytes();
    test_publish_retained();
    test_publish_retained_2();
    test_publish_qos1();
    test_publish_qos2();
    test_publish_null_payload();
    test_publish_not_connected();
    test_publish_empty_topic();
    test_publish_too_long();
    test_publish_P();
    test_publish_P_qos1();
    test_publish_P_qos2();
    test_publish_P_too_long();

    FINISH
}
