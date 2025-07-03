#include "BDDTest.h"
#include "Buffer.h"
#include "PubSubClient.h"
#include "ShimClient.h"
#include "trace.h"

byte server[] = {172, 16, 0, 2};

// function declarations
void callback(char* topic, uint8_t* payload, size_t length);
int test_connect_fails_no_network();
int test_connect_fails_on_no_response();
int test_connect_properly_formatted();
int test_connect_properly_formatted_hostname();
int test_connect_fails_on_bad_rc();
int test_connect_non_clean_session();
int test_connect_accepts_username_password();
int test_connect_accepts_username_no_password();
int test_connect_accepts_username_blank_password();
int test_connect_ignores_password_no_username();
int test_connect_with_will();
int test_connect_with_will_username_password();
int test_connect_disconnect_connect();
int test_connect_custom_keepalive();

void callback(_UNUSED_ char* topic, _UNUSED_ uint8_t* payload, _UNUSED_ size_t length) {
    // handle message arrived
}

int test_connect_fails_no_network() {
    IT("fails to connect if underlying client doesn't connect");
    ShimClient shimClient;
    shimClient.setAllowConnect(false);
    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1");
    IS_FALSE(rc);
    int state = client.state();
    IS_TRUE(state == MQTT_CONNECT_FAILED);
    END_IT
}

int test_connect_fails_on_no_response() {
    IT("fails to connect if no response received after 15 seconds");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);
    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1");
    IS_FALSE(rc);
    int state = client.state();
    IS_TRUE(state == MQTT_CONNECTION_TIMEOUT);
    END_IT
}

int test_connect_properly_formatted() {
    IT("sends a properly formatted connect packet and succeeds");
    ShimClient shimClient;

    shimClient.setAllowConnect(true);
    byte expectServer[] = {172, 16, 0, 2};
    shimClient.expectConnect(expectServer, 1883);
    byte connect[] = {0x10, 0x18, 0x0,  0x4,  0x4d, 0x51, 0x54, 0x54, 0x4,  0x2,  0x0,  0xf,  0x0,
                      0xc,  0x63, 0x6c, 0x69, 0x65, 0x6e, 0x74, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x31};
    byte connack[] = {0x20, 0x02, 0x00, 0x00};

    shimClient.expect(connect, 26);
    shimClient.respond(connack, 4);

    PubSubClient client(server, 1883, callback, shimClient);
    int state = client.state();
    IS_TRUE(state == MQTT_DISCONNECTED);

    bool rc = client.connect("client_test1");
    IS_TRUE(rc);
    IS_FALSE(shimClient.error());

    state = client.state();
    IS_TRUE(state == MQTT_CONNECTED);

    END_IT
}

int test_connect_properly_formatted_hostname() {
    IT("accepts a hostname");
    ShimClient shimClient;

    shimClient.setAllowConnect(true);
    shimClient.expectConnect("localhost", 1883);
    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.respond(connack, 4);

    PubSubClient client("localhost", 1883, callback, shimClient);
    bool rc = client.connect("client_test1");
    IS_TRUE(rc);
    IS_FALSE(shimClient.error());

    END_IT
}

int test_connect_fails_on_bad_rc() {
    IT("fails to connect if a bad return code is received");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);
    byte connack[] = {0x20, 0x02, 0x00, 0x01};
    shimClient.respond(connack, 4);

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1");
    IS_FALSE(rc);

    int state = client.state();
    IS_TRUE(state == 0x01);

    END_IT
}

int test_connect_non_clean_session() {
    IT("sends a properly formatted non-clean session connect packet and succeeds");
    ShimClient shimClient;

    shimClient.setAllowConnect(true);
    byte expectServer[] = {172, 16, 0, 2};
    shimClient.expectConnect(expectServer, 1883);
    byte connect[] = {0x10, 0x18, 0x0,  0x4,  0x4d, 0x51, 0x54, 0x54, 0x4,  0x0,  0x0,  0xf,  0x0,
                      0xc,  0x63, 0x6c, 0x69, 0x65, 0x6e, 0x74, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x31};
    byte connack[] = {0x20, 0x02, 0x00, 0x00};

    shimClient.expect(connect, 26);
    shimClient.respond(connack, 4);

    PubSubClient client(server, 1883, callback, shimClient);
    int state = client.state();
    IS_TRUE(state == MQTT_DISCONNECTED);

    bool rc = client.connect("client_test1", 0, 0, 0, 0, 0, 0, 0);
    IS_TRUE(rc);
    IS_FALSE(shimClient.error());

    state = client.state();
    IS_TRUE(state == MQTT_CONNECTED);

    END_IT
}

int test_connect_accepts_username_password() {
    IT("accepts a username and password");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte connect[] = {0x10, 0x24, 0x0,  0x4,  0x4d, 0x51, 0x54, 0x54, 0x4, 0xc2, 0x0,  0xf,  0x0,  0xc, 0x63, 0x6c, 0x69, 0x65, 0x6e,
                      0x74, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x31, 0x0,  0x4, 0x75, 0x73, 0x65, 0x72, 0x0, 0x4,  0x70, 0x61, 0x73, 0x73};
    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.expect(connect, 0x26);
    shimClient.respond(connack, 4);

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1", "user", "pass");
    IS_TRUE(rc);
    IS_FALSE(shimClient.error());

    END_IT
}

int test_connect_accepts_username_no_password() {
    IT("accepts a username but no password");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte connect[] = {0x10, 0x1e, 0x0,  0x4,  0x4d, 0x51, 0x54, 0x54, 0x4,  0x82, 0x0, 0xf, 0x0,  0xc,  0x63, 0x6c,
                      0x69, 0x65, 0x6e, 0x74, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x31, 0x0, 0x4, 0x75, 0x73, 0x65, 0x72};
    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.expect(connect, 0x20);
    shimClient.respond(connack, 4);

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1", "user", 0);
    IS_TRUE(rc);
    IS_FALSE(shimClient.error());

    END_IT
}

int test_connect_accepts_username_blank_password() {
    IT("accepts a username and blank password");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte connect[] = {0x10, 0x20, 0x0,  0x4,  0x4d, 0x51, 0x54, 0x54, 0x4,  0xc2, 0x0, 0xf,  0x0,  0xc,  0x63, 0x6c, 0x69,
                      0x65, 0x6e, 0x74, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x31, 0x0,  0x4, 0x75, 0x73, 0x65, 0x72, 0x0,  0x0};
    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.expect(connect, 0x26);
    shimClient.respond(connack, 4);

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1", "user", "pass");
    IS_TRUE(rc);
    IS_FALSE(shimClient.error());

    END_IT
}

int test_connect_ignores_password_no_username() {
    IT("ignores a password but no username");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte connect[] = {0x10, 0x18, 0x0,  0x4,  0x4d, 0x51, 0x54, 0x54, 0x4,  0x2,  0x0,  0xf,  0x0,
                      0xc,  0x63, 0x6c, 0x69, 0x65, 0x6e, 0x74, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x31};
    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.expect(connect, 26);
    shimClient.respond(connack, 4);

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1", 0, "pass");
    IS_TRUE(rc);
    IS_FALSE(shimClient.error());

    END_IT
}

int test_connect_with_will() {
    IT("accepts a will");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte connect[] = {0x10, 0x30, 0x0,  0x4,  0x4d, 0x51, 0x54, 0x54, 0x4,  0xe,  0x0,  0xf,  0x0,  0xc,  0x63, 0x6c, 0x69,
                      0x65, 0x6e, 0x74, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x31, 0x0,  0x9,  0x77, 0x69, 0x6c, 0x6c, 0x54, 0x6f,
                      0x70, 0x69, 0x63, 0x0,  0xb,  0x77, 0x69, 0x6c, 0x6c, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65};
    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.expect(connect, 0x32);
    shimClient.respond(connack, 4);

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1", "willTopic", 1, 0, "willMessage");
    IS_TRUE(rc);
    IS_FALSE(shimClient.error());

    END_IT
}

int test_connect_with_will_username_password() {
    IT("accepts a will, username and password");
    ShimClient shimClient;
    shimClient.setAllowConnect(true);

    byte connect[] = {0x10, 0x40, 0x0,  0x4,  0x4d, 0x51, 0x54, 0x54, 0x4,  0xce, 0x0,  0xf,  0x0,  0xc,  0x63, 0x6c, 0x69, 0x65, 0x6e, 0x74, 0x5f, 0x74,
                      0x65, 0x73, 0x74, 0x31, 0x0,  0x9,  0x77, 0x69, 0x6c, 0x6c, 0x54, 0x6f, 0x70, 0x69, 0x63, 0x0,  0xb,  0x77, 0x69, 0x6c, 0x6c, 0x4d,
                      0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x0,  0x4,  0x75, 0x73, 0x65, 0x72, 0x0,  0x8,  0x70, 0x61, 0x73, 0x73, 0x77, 0x6f, 0x72, 0x64};
    byte connack[] = {0x20, 0x02, 0x00, 0x00};
    shimClient.expect(connect, 0x42);
    shimClient.respond(connack, 4);

    PubSubClient client(server, 1883, callback, shimClient);
    bool rc = client.connect("client_test1", "user", "password", "willTopic", 1, 0, "willMessage");
    IS_TRUE(rc);
    IS_FALSE(shimClient.error());

    END_IT
}

int test_connect_disconnect_connect() {
    IT("connects, disconnects and connects again");
    ShimClient shimClient;

    shimClient.setAllowConnect(true);
    byte expectServer[] = {172, 16, 0, 2};
    shimClient.expectConnect(expectServer, 1883);
    byte connect[] = {0x10, 0x18, 0x0,  0x4,  0x4d, 0x51, 0x54, 0x54, 0x4,  0x2,  0x0,  0xf,  0x0,
                      0xc,  0x63, 0x6c, 0x69, 0x65, 0x6e, 0x74, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x31};
    byte connack[] = {0x20, 0x02, 0x00, 0x00};

    shimClient.expect(connect, 26);
    shimClient.respond(connack, 4);

    PubSubClient client(server, 1883, callback, shimClient);

    int state = client.state();
    IS_TRUE(state == MQTT_DISCONNECTED);

    bool rc = client.connect("client_test1");
    IS_TRUE(rc);
    IS_FALSE(shimClient.error());

    state = client.state();
    IS_TRUE(state == MQTT_CONNECTED);

    byte disconnect[] = {0xE0, 0x00};
    shimClient.expect(disconnect, 2);

    client.disconnect();

    IS_FALSE(client.connected());
    IS_FALSE(shimClient.connected());
    IS_FALSE(shimClient.error());

    state = client.state();
    IS_TRUE(state == MQTT_DISCONNECTED);

    shimClient.expect(connect, 28);
    shimClient.respond(connack, 4);
    rc = client.connect("client_test1");
    IS_TRUE(rc);
    IS_FALSE(shimClient.error());
    state = client.state();
    IS_TRUE(state == MQTT_CONNECTED);

    END_IT
}

int test_connect_custom_keepalive() {
    IT("sends a properly formatted connect packet with custom keepalive value");
    ShimClient shimClient;

    shimClient.setAllowConnect(true);
    byte expectServer[] = {172, 16, 0, 2};
    shimClient.expectConnect(expectServer, 1883);

    // Set keepalive to 300secs == 0x01 0x2c
    byte connect[] = {0x10, 0x18, 0x0,  0x4,  0x4d, 0x51, 0x54, 0x54, 0x4,  0x2,  0x01, 0x2c, 0x0,
                      0xc,  0x63, 0x6c, 0x69, 0x65, 0x6e, 0x74, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x31};
    byte connack[] = {0x20, 0x02, 0x00, 0x00};

    shimClient.expect(connect, 26);
    shimClient.respond(connack, 4);

    PubSubClient client(server, 1883, callback, shimClient);
    int state = client.state();
    IS_TRUE(state == MQTT_DISCONNECTED);

    client.setKeepAlive(300);

    bool rc = client.connect("client_test1");
    IS_TRUE(rc);
    IS_FALSE(shimClient.error());

    state = client.state();
    IS_TRUE(state == MQTT_CONNECTED);

    END_IT
}

int main() {
    SUITE("Connect");

    test_connect_fails_no_network();
    test_connect_fails_on_no_response();

    test_connect_properly_formatted();
    test_connect_non_clean_session();
    test_connect_accepts_username_password();
    test_connect_fails_on_bad_rc();
    test_connect_properly_formatted_hostname();

    test_connect_accepts_username_no_password();
    test_connect_ignores_password_no_username();
    test_connect_with_will();
    test_connect_with_will_username_password();
    test_connect_disconnect_connect();

    test_connect_custom_keepalive();
    FINISH
}
