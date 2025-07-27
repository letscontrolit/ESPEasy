/*
  PubSubClient.h - A simple client for MQTT.
  Nick O'Leary, Holger Mueller
  http://knolleary.net
  https://github.com/hmueller01/pubsubclient3
*/

#ifndef PubSubClient_h
#define PubSubClient_h

#include <Arduino.h>

#include "Client.h"
#include "IPAddress.h"
#include "Stream.h"

#define MQTT_VERSION_3_1   3
#define MQTT_VERSION_3_1_1 4

//< @note The following #define directives can be used to configure the library.

/**
 * @brief Sets the version of the MQTT protocol to use.
 * @note Default value is MQTT_VERSION_3_1_1 for MQTT 3.1.1.
 */
#ifndef MQTT_VERSION
#define MQTT_VERSION MQTT_VERSION_3_1_1
#endif

// MQTT_MAX_POSSIBLE_PACKET_SIZE : Maximum packet size defined by MQTT protocol.
#ifndef MQTT_MAX_POSSIBLE_PACKET_SIZE
#define MQTT_MAX_POSSIBLE_PACKET_SIZE 268435455
#endif

/**
 * @brief Sets the largest packet size, in bytes, the client will handle.
 * Any packet received that exceeds this size will be ignored.
 * This value can be overridden by calling setBufferSize.
 * @note Default value is 256 bytes.
 */
#ifndef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 256
#endif

/**
 * @brief Sets the keepalive interval, in seconds, the client will use.
 * This is used to maintain the connection when no other packets are being sent or received.
 * This value can be overridden by calling setKeepAlive.
 * @note Default value is 15 seconds.
 */
#ifndef MQTT_KEEPALIVE
#define MQTT_KEEPALIVE 15
#endif

/**
 * @brief Sets the timeout, in seconds, when reading from the network.
 * This also applies as the timeout for calls to connect.
 * This value can be overridden by calling setSocketTimeout.
 * @note Default value is 15 seconds.
 */
#ifndef MQTT_SOCKET_TIMEOUT
#define MQTT_SOCKET_TIMEOUT 15
#endif

/**
 * @brief Sets the maximum number of bytes passed to the network client in each write call.
 * Some hardware has a limit to how much data can be passed to them in one go,
 * such as the Arduino Wifi Shield.
 * @note Defaults to undefined, which passes the entire packet in each write call.
 */
// #define MQTT_MAX_TRANSFER_SIZE 80

// Possible values for client.state()
#define MQTT_CONNECTION_TIMEOUT      -4
#define MQTT_CONNECTION_LOST         -3
#define MQTT_CONNECT_FAILED          -2
#define MQTT_DISCONNECTED            -1
#define MQTT_CONNECTED               0
#define MQTT_CONNECT_BAD_PROTOCOL    1
#define MQTT_CONNECT_BAD_CLIENT_ID   2
#define MQTT_CONNECT_UNAVAILABLE     3
#define MQTT_CONNECT_BAD_CREDENTIALS 4
#define MQTT_CONNECT_UNAUTHORIZED    5

#define MQTTRETAINED    1        // Retained flag in the header
#define MQTTCONNECT     1 << 4   // Client request to connect to Server
#define MQTTCONNACK     2 << 4   // Connect Acknowledgment
#define MQTTPUBLISH     3 << 4   // Publish message
#define MQTTPUBACK      4 << 4   // Publish Acknowledgment
#define MQTTPUBREC      5 << 4   // Publish Received (assured delivery part 1)
#define MQTTPUBREL      6 << 4   // Publish Release (assured delivery part 2)
#define MQTTPUBCOMP     7 << 4   // Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE   8 << 4   // Client Subscribe request
#define MQTTSUBACK      9 << 4   // Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE 10 << 4  // Client Unsubscribe request
#define MQTTUNSUBACK    11 << 4  // Unsubscribe Acknowledgment
#define MQTTPINGREQ     12 << 4  // PING Request
#define MQTTPINGRESP    13 << 4  // PING Response
#define MQTTDISCONNECT  14 << 4  // Client is Disconnecting
#define MQTTRESERVED    15 << 4  // Reserved

#define MQTTQOS0 (0 << 1)
#define MQTTQOS1 (1 << 1)
#define MQTTQOS2 (2 << 1)

// Maximum size of fixed header and variable length size header
#define MQTT_MAX_HEADER_SIZE 5

#if defined(__has_include) && __has_include(<functional>) && !defined(NOFUNCTIONAL)
#include <functional>
/**
 * @brief Define the signature required by any callback function.
 * @note The parameters are TOPIC, PAYLOAD, and LENGTH, respectively.
 */
#define MQTT_CALLBACK_SIGNATURE std::function<void(char*, uint8_t*, size_t)> callback
#else
#define MQTT_CALLBACK_SIGNATURE void (*callback)(char*, uint8_t*, size_t)
#endif

#define CHECK_STRING_LENGTH(l, s)                                  \
    if ((!s) || (l + 2 + strnlen(s, this->bufferSize) > this->bufferSize)) { \
        _client->stop();                                           \
        return false;                                              \
    }

#ifdef DEBUG_ESP_PORT
#ifdef DEBUG_PUBSUBCLIENT
#define DEBUG_PSC_PRINTF(fmt, ...) DEBUG_ESP_PORT.printf(("PubSubClient: " fmt), ##__VA_ARGS__)
#else
#define DEBUG_PSC_PRINTF(...)
#endif

#define ERROR_PSC_PRINTF(fmt, ...) DEBUG_ESP_PORT.printf(("PubSubClient error: " fmt), ##__VA_ARGS__)
#define ERROR_PSC_PRINTF_P(fmt, ...) DEBUG_ESP_PORT.printf_P(PSTR("PubSubClient error: " fmt), ##__VA_ARGS__)
#else  // DEBUG_ESP_PORT
#ifndef DEBUG_PSC_PRINTF
#define DEBUG_PSC_PRINTF(...)
#endif
#ifndef ERROR_PSC_PRINTF
#define ERROR_PSC_PRINTF(fmt, ...)
#endif
#ifndef ERROR_PSC_PRINTF_P
#define ERROR_PSC_PRINTF_P(fmt, ...)
#endif
#endif

/**
 * @class PubSubClient
 * @brief This class provides a client for doing simple publish and subscribe messaging with a server that supports MQTT.
 */
class PubSubClient : public Print {
   private:
    Client* _client{};
    uint8_t* buffer{};
    size_t bufferSize{};
    unsigned long keepAliveMillis{};
    unsigned long socketTimeoutMillis{};
    uint16_t nextMsgId{};
    unsigned long lastOutActivity{};
    unsigned long lastInActivity{};
    bool pingOutstanding{};
    MQTT_CALLBACK_SIGNATURE{};
    IPAddress ip{};
    char* domain{};
    uint16_t port{};
    Stream* stream{};
    int _state{MQTT_DISCONNECTED};
    size_t _bufferWritePos{};

    size_t readPacket(uint8_t* hdrLen);
    bool handlePacket(uint8_t hdrLen, size_t len);
    bool readByte(uint8_t* result);
    bool readByte(uint8_t* result, size_t* pos);
    uint8_t buildHeader(uint8_t header, uint8_t* buf, size_t length);
    bool write(uint8_t header, uint8_t* buf, size_t length);
    size_t writeString(const char* string, uint8_t* buf, size_t pos);
    size_t writeString(const char* string, uint8_t* buf, size_t pos, size_t size);

    // Add to buffer and flush if full (only to be used with beginPublish/endPublish)Add commentMore actions
    size_t appendBuffer(uint8_t data);
    size_t appendBuffer(const uint8_t *data, size_t size);
    size_t flushBuffer();

   public:
    /**
     * @brief Creates an uninitialised client instance.
     * @note Before it can be used,
     * it must be configured using the property setters setClient and setServer.
     */
    PubSubClient();

    /**
     * @brief Creates a partially initialised client instance.
     * @param client The network client to use, for example WiFiClient.
     * @note Before it can be used,
     * it must be configured with the property setter setServer.
     */
    PubSubClient(Client& client);

    /**
     * @brief Creates a fully configured client instance.
     * @param domain The address of the server.
     * @param port The port to connect to.
     * @param client The network client to use, for example WiFiClient.
     */
    PubSubClient(IPAddress, uint16_t, Client& client);

    /**
     * @brief Creates a fully configured client instance.
     * @param domain The address of the server.
     * @param port The port to connect to.
     * @param client The network client to use, for example WiFiClient.
     * @param stream A stream to write received messages to.
     */
    PubSubClient(IPAddress, uint16_t, Client& client, Stream&);

    /**
     * @brief Creates a fully configured client instance.
     * @param domain The address of the server.
     * @param port The port to connect to.
     * @param callback Pointer to a message callback function.
     * Called when a message arrives for a subscription created by this client.
     * @param client The network client to use, for example WiFiClient.
     */
    PubSubClient(IPAddress, uint16_t, MQTT_CALLBACK_SIGNATURE, Client& client);

    /**
     * @brief Creates a fully configured client instance.
     * @param domain The address of the server.
     * @param port The port to connect to.
     * @param callback Pointer to a message callback function.
     * Called when a message arrives for a subscription created by this client.
     * @param client The network client to use, for example WiFiClient.
     * @param stream A stream to write received messages to.
     */
    PubSubClient(IPAddress, uint16_t, MQTT_CALLBACK_SIGNATURE, Client& client, Stream&);

    /**
     * @brief Creates a fully configured client instance.
     * @param domain The address of the server.
     * @param port The port to connect to.
     * @param client The network client to use, for example WiFiClient.
     */
    PubSubClient(uint8_t*, uint16_t, Client& client);

    /**
     * @brief Creates a fully configured client instance.
     * @param domain The address of the server.
     * @param port The port to connect to.
     * @param client The network client to use, for example WiFiClient.
     * @param stream A stream to write received messages to.
     */
    PubSubClient(uint8_t*, uint16_t, Client& client, Stream&);

    /**
     * @brief Creates a fully configured client instance.
     * @param domain The address of the server.
     * @param port The port to connect to.
     * @param callback Pointer to a message callback function.
     * Called when a message arrives for a subscription created by this client.
     * @param client The network client to use, for example WiFiClient.
     */
    PubSubClient(uint8_t*, uint16_t, MQTT_CALLBACK_SIGNATURE, Client& client);

    /**
     * @brief Creates a fully configured client instance.
     * @param domain The address of the server.
     * @param port The port to connect to.
     * @param callback Pointer to a message callback function.
     * Called when a message arrives for a subscription created by this client.
     * @param client The network client to use, for example WiFiClient.
     * @param stream A stream to write received messages to.
     */
    PubSubClient(uint8_t*, uint16_t, MQTT_CALLBACK_SIGNATURE, Client& client, Stream&);

    /**
     * @brief Creates a fully configured client instance.
     * @param domain The address of the server.
     * @param port The port to connect to.
     * @param client The network client to use, for example WiFiClient.
     */
    PubSubClient(const char*, uint16_t, Client& client);

    /**
     * @brief Creates a fully configured client instance.
     * @param domain The address of the server.
     * @param port The port to connect to.
     * @param client The network client to use, for example WiFiClient.
     * @param stream A stream to write received messages to.
     */
    PubSubClient(const char*, uint16_t, Client& client, Stream&);

    /**
     * @brief Creates a fully configured client instance.
     * @param domain The address of the server.
     * @param port The port to connect to.
     * @param callback Pointer to a message callback function.
     * Called when a message arrives for a subscription created by this client.
     * @param client The network client to use, for example WiFiClient.
     */
    PubSubClient(const char*, uint16_t, MQTT_CALLBACK_SIGNATURE, Client& client);

    /**
     * @brief Creates a fully configured client instance.
     * @param domain The address of the server.
     * @param port The port to connect to.
     * @param callback Pointer to a message callback function.
     * Called when a message arrives for a subscription created by this client.
     * @param client The network client to use, for example WiFiClient.
     * @param stream A stream to write received messages to.
     */
    PubSubClient(const char*, uint16_t, MQTT_CALLBACK_SIGNATURE, Client& client, Stream&);

    /**
     * @brief Destructor for the PubSubClient class.
     */
    ~PubSubClient();

    /**
     * @brief Sets the server details.
     * @param ip The address of the server.
     * @param port  The port to connect to.
     * @return The client instance, allowing the function to be chained.
     */
    PubSubClient& setServer(IPAddress ip, uint16_t port);

    /**
     * @brief Sets the server details.
     * @param ip The address of the server.
     * @param port  The port to connect to.
     * @return The client instance, allowing the function to be chained.
     */
    PubSubClient& setServer(uint8_t* ip, uint16_t port);

    /**
     * @brief Sets the server details.
     * @param ip The address of the server.
     * @param port  The port to connect to.
     * @return The client instance, allowing the function to be chained.
     */
    PubSubClient& setServer(const char* domain, uint16_t port);

    /**
     * @brief Sets the message callback function.
     * @param callback Pointer to a message callback function.
     * Called when a message arrives for a subscription created by this client.
     * @return The client instance, allowing the function to be chained.
     */
    PubSubClient& setCallback(MQTT_CALLBACK_SIGNATURE);

    /**
     * @brief Sets the network client instance to use.
     * @param client The network client to use, for example WiFiClient.
     * @return The client instance, allowing the function to be chained.
     */
    PubSubClient& setClient(Client& client);

    /**
     * @brief Sets the stream to write received messages to.
     * @param stream A stream to write received messages to.
     * @return The client instance, allowing the function to be chained.
     */
    PubSubClient& setStream(Stream& stream);

    /**
     * @brief Sets the keep alive interval used by the client.
     * This value should only be changed when the client is not connected.
     * Set keepAlive to zero (0) to turn off the keep alive mechanism.
     * @param keepAlive The keep alive interval, in seconds.
     * @return The client instance, allowing the function to be chained.
     */
    PubSubClient& setKeepAlive(uint16_t keepAlive);

    /**
     * @brief Sets the socket timeout used by the client.
     * This determines how long the client will wait for incoming data when it expects data to arrive - for example,
     * whilst it is in the middle of reading a MQTT packet.
     * @param timeout The socket timeout, in seconds.
     * @return The client instance, allowing the function to be chained.
     */
    PubSubClient& setSocketTimeout(uint16_t timeout);

    /**
     * @brief Sets the size, in bytes, of the internal send and receive buffer.
     * This must be large enough to contain the full MQTT packet.
     * When sending or receiving messages, the packet will contain the full topic string,
     * the payload data, and a small number of header bytes.
     * @param size The size, in bytes, for the internal buffer.
     * @return true If the buffer was resized.
     * false If the buffer could not be resized.
     */
    bool setBufferSize(size_t size);

    /**
     * @brief Gets the current size of the internal buffer.
     * @return The size of the internal buffer.
     */
    size_t getBufferSize();

    /**
     * @brief Connects the client.
     * @param id The client ID to use when connecting to the server.
     * @return true If client succeeded in establishing a connection to the broker.
     * false If client failed to establish a connection to the broker.
     */
    bool connect(const char* id);

    /**
     * @brief Connects the client.
     * @param id The client ID to use when connecting to the server.
     * @param user The username to use. If NULL, no username or password is used.
     * @param pass The password to use. If NULL, no password is used.
     * @return true If client succeeded in establishing a connection to the broker.
     * false If client failed to establish a connection to the broker.
     */
    bool connect(const char* id, const char* user, const char* pass);

    /**
     * @brief Connects the client.
     * @param id The client ID to use when connecting to the server.
     * @param willTopic The topic to be used by the will message.
     * @param willQos The quality of service to be used by the will message. [0, 1, 2].
     * @param willRetain Publish the will message with the retain flag.
     * @return true If client succeeded in establishing a connection to the broker.
     * false If client failed to establish a connection to the broker.
     */
    bool connect(const char* id, const char* willTopic, uint8_t willQos, bool willRetain, const char* willMessage);

    /**
     * @brief Connects the client.
     * @param id The client ID to use when connecting to the server.
     * @param user The username to use. If NULL, no username or password is used.
     * @param pass The password to use. If NULL, no password is used.
     * @param willTopic The topic to be used by the will message.
     * @param willQos The quality of service to be used by the will message. [0, 1, 2].
     * @param willRetain Publish the will message with the retain flag.
     * @return true If client succeeded in establishing a connection to the broker.
     * false If client failed to establish a connection to the broker.
     */
    bool connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, bool willRetain, const char* willMessage);

    /**
     * @brief Connects the client.
     * @param id The client ID to use when connecting to the server.
     * @param user The username to use. If NULL, no username or password is used.
     * @param pass The password to use. If NULL, no password is used.
     * @param willTopic The topic to be used by the will message.
     * @param willQos The quality of service to be used by the will message. [0, 1, 2].
     * @param willRetain Publish the will message with the retain flag.
     * @param cleanSession Connect with a clean session.
     * @return true If client succeeded in establishing a connection to the broker.
     * false If client failed to establish a connection to the broker.
     */
    bool connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, bool willRetain, const char* willMessage,
                 bool cleanSession);

    /**
     * @brief Disconnects the client.
     */
    void disconnect();

    /**
     * @brief Publishes a non retained message to the specified topic.
     * @param topic The topic to publish to.
     * @param payload The message to publish.
     * @return true If the publish succeeded.
     * false If the publish failed, either connection lost or message too large.
     */
    bool publish(const char* topic, const char* payload);

    /**
     * @brief Publishes a message to the specified topic.
     * @param topic The topic to publish to.
     * @param payload The message to publish.
     * @param retained Publish the message with the retain flag.
     * @return true If the publish succeeded.
     * false If the publish failed, either connection lost or message too large.
     */
    bool publish(const char* topic, const char* payload, bool retained);

    /**
     * @brief Publishes a non retained message to the specified topic.
     * @param topic The topic to publish to.
     * @param payload The message to publish.
     * @param plength The length of the payload.
     * @return true If the publish succeeded.
     * false If the publish failed, either connection lost or message too large.
     */
    bool publish(const char* topic, const uint8_t* payload, size_t plength);

    /**
     * @brief Publishes a message to the specified topic.
     * @param topic The topic to publish to.
     * @param payload The message to publish.
     * @param plength The length of the payload.
     * @param retained Publish the message with the retain flag.
     * @return true If the publish succeeded.
     * false If the publish failed, either connection lost or message too large.
     */
    bool publish(const char* topic, const uint8_t* payload, size_t plength, bool retained);

    /**
     * @brief Publishes a message stored in PROGMEM to the specified topic.
     * @param topic The topic to publish to.
     * @param payload The message to publish.
     * @param retained Publish the message with the retain flag.
     * @return true If the publish succeeded.
     * false If the publish failed, either connection lost or message too large.
     */
    bool publish_P(const char* topic, const char* payload, bool retained);

    /**
     * @brief Publishes a message stored in PROGMEM to the specified topic.
     * @param topic The topic to publish to.
     * @param payload The message to publish.
     * @param plength The length of the payload.
     * @param retained Publish the message with the retain flag.
     * @return true If the publish succeeded.
     * false If the publish failed, either connection lost or message too large.
     */
    bool publish_P(const char* topic, const uint8_t* payload, size_t plength, bool retained);

    /**
     * @brief Start to publish a message.
     * This API:
     *   beginPublish(...)
     *   one or more calls to write(...)
     *   endPublish()
     * Allows for arbitrarily large payloads to be sent without them having to be copied into
     * a new buffer and held in memory at one time.
     * @param topic The topic to publish to.
     * @param plength The length of the payload.
     * @param retained Publish the message with the retain flag.
     * @return true If the publish succeeded.
     * false If the publish failed, either connection lost or message too large.
     */
    bool beginPublish(const char* topic, size_t plength, bool retained);

    /**
     * @brief Finish sending a message that was started with a call to beginPublish.
     * @return true If the publish succeeded.
     * false If the publish failed, either connection lost or message too large.
     */
    bool endPublish();

    /**
     * @brief Writes a single byte as a component of a publish started with a call to beginPublish.
     *        For performance reasons, this will be appended to the internal buffer, 
     *        which will be flushed when full or on a call to endPublish().
     * @param byte A byte to write to the publish payload.
     * @return The number of bytes written.
     */
    virtual size_t write(uint8_t);

    /**
     * @brief Writes an array of bytes as a component of a publish started with a call to beginPublish.
     *        For performance reasons, this will be appended to the internal buffer, 
     *        which will be flushed when full or on a call to endPublish().
     * @param buffer The bytes to write.
     * @param size The length of the payload to be sent.
     * @return The number of bytes written.
     */
    virtual size_t write(const uint8_t* buffer, size_t size);

    /**
     * @brief Subscribes to messages published to the specified topic.
     * @param topic The topic to subscribe to.
     * @return true If sending the subscribe succeeded.
     * false If sending the subscribe failed, either connection lost or message too large.
     */
    bool subscribe(const char* topic);

    /**
     * @brief Subscribes to messages published to the specified topic.
     * @param topic The topic to subscribe to.
     * @param qos The qos to subscribe at. [0, 1].
     * @return true If sending the subscribe succeeded.
     * false If sending the subscribe failed, either connection lost or message too large.
     */
    bool subscribe(const char* topic, uint8_t qos);

    /**
     * @brief Unsubscribes from the specified topic.
     * @param topic The topic to unsubscribe from.
     * @return true If sending the unsubscribe succeeded.
     * false If sending the unsubscribe failed, either connection lost or message too large.
     */
    bool unsubscribe(const char* topic);

    /**
     * @brief This should be called regularly to allow the client to process incoming messages and maintain its connection to the server.
     * @return true If the client is still connected.
     * false If the client is no longer connected.
     */
    bool loop();

    /**
     * @brief Checks whether the client is connected to the server.
     * @return true If the client is connected.
     * false If the client is not connected.
     */
    bool connected();

    /**
     * @brief Returns the current state of the client.
     * If a connection attempt fails, this can be used to get more information about the failure.
     * @note All of the values have corresponding constants defined in PubSubClient.h.
     * @return -4 : MQTT_CONNECTION_TIMEOUT - The server didn't respond within the keepalive time.
     * -3 : MQTT_CONNECTION_LOST - The network connection was broken.
     * -2 : MQTT_CONNECT_FAILED - The network connection failed.
     * -1 : MQTT_DISCONNECTED - The client is disconnected cleanly.
     * 0 : MQTT_CONNECTED - The client is connected.
     * 1 : MQTT_CONNECT_BAD_PROTOCOL - The server doesn't support the requested version of MQTT.
     * 2 : MQTT_CONNECT_BAD_CLIENT_ID - The server rejected the client identifier.
     * 3 : MQTT_CONNECT_UNAVAILABLE - The server was unable to accept the connection.
     * 4 : MQTT_CONNECT_BAD_CREDENTIALS - The username/password were rejected.
     * 5 : MQTT_CONNECT_UNAUTHORIZED - The client was not authorized to connect.
     */
    int state();
};

#endif
