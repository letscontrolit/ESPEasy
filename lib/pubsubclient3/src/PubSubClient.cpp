/**
 * @file PubSubClient.cpp
 * @brief A simple client for MQTT.
 * @author Nicholas O'Leary - http://knolleary.net
 * @author Holger Mueller - https://github.com/hmueller01/pubsubclient3
 * @copyright MIT License 2008-2025
 *
 * This file is part of the PubSubClient library.
 */

#include "PubSubClient.h"

/**
 * @brief Macro to check if a string 's' can be safely added to the MQTT _buffer.
 *
 * If either check fails, the client connection is stopped and the function returns false.
 * @param l current length in the _buffer
 * @param s string to check
 */
#define CHECK_STRING_LENGTH(l, s)                                            \
    if ((!s) || (l + 2 + strnlen(s, _bufferSize) > _bufferSize)) { \
        _client->stop();                                                     \
        return false;                                                        \
    }

PubSubClient::PubSubClient() {
    setBufferSize(MQTT_MAX_PACKET_SIZE);
    setKeepAlive(MQTT_KEEPALIVE);
    setSocketTimeout(MQTT_SOCKET_TIMEOUT);
}

PubSubClient::PubSubClient(Client& client) : PubSubClient() {
    setClient(client);
}

PubSubClient::PubSubClient(IPAddress addr, uint16_t port, Client& client) : PubSubClient() {
    setServer(addr, port);
    setClient(client);
}

PubSubClient::PubSubClient(IPAddress addr, uint16_t port, Client& client, Stream& stream) : PubSubClient() {
    setServer(addr, port);
    setClient(client);
    setStream(stream);
}

PubSubClient::PubSubClient(IPAddress addr, uint16_t port, MQTT_CALLBACK_SIGNATURE, Client& client) : PubSubClient() {
    setServer(addr, port);
    setCallback(callback);
    setClient(client);
}

PubSubClient::PubSubClient(IPAddress addr, uint16_t port, MQTT_CALLBACK_SIGNATURE, Client& client, Stream& stream) : PubSubClient() {
    setServer(addr, port);
    setCallback(callback);
    setClient(client);
    setStream(stream);
}

PubSubClient::PubSubClient(uint8_t* ip, uint16_t port, Client& client) : PubSubClient() {
    setServer(ip, port);
    setClient(client);
}

PubSubClient::PubSubClient(uint8_t* ip, uint16_t port, Client& client, Stream& stream) : PubSubClient() {
    setServer(ip, port);
    setClient(client);
    setStream(stream);
}

PubSubClient::PubSubClient(uint8_t* ip, uint16_t port, MQTT_CALLBACK_SIGNATURE, Client& client) : PubSubClient() {
    setServer(ip, port);
    setCallback(callback);
    setClient(client);
}

PubSubClient::PubSubClient(uint8_t* ip, uint16_t port, MQTT_CALLBACK_SIGNATURE, Client& client, Stream& stream) : PubSubClient() {
    setServer(ip, port);
    setCallback(callback);
    setClient(client);
    setStream(stream);
}

PubSubClient::PubSubClient(const char* domain, uint16_t port, Client& client) : PubSubClient() {
    setServer(domain, port);
    setClient(client);
}

PubSubClient::PubSubClient(const char* domain, uint16_t port, Client& client, Stream& stream) : PubSubClient() {
    setServer(domain, port);
    setClient(client);
    setStream(stream);
}

PubSubClient::PubSubClient(const char* domain, uint16_t port, MQTT_CALLBACK_SIGNATURE, Client& client) : PubSubClient() {
    setServer(domain, port);
    setCallback(callback);
    setClient(client);
}

PubSubClient::PubSubClient(const char* domain, uint16_t port, MQTT_CALLBACK_SIGNATURE, Client& client, Stream& stream) : PubSubClient() {
    setServer(domain, port);
    setCallback(callback);
    setClient(client);
    setStream(stream);
}

PubSubClient::~PubSubClient() {
    free(_domain);
    free(_buffer);
}

bool PubSubClient::connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, bool willRetain,
                           const char* willMessage, bool cleanSession) {
    if (!_client) return false;  // do not crash if client not set
    if (!connected()) {
        int result = 0;

        if (_client->connected()) {
            result = 1;
        } else if (_port != 0) {
            if (_domain) {
                result = _client->connect(_domain, _port);
            } else {
                result = _client->connect(_ip, _port);
            }
        }

        if (result == 1) {
            _nextMsgId = 1;  // init msgId (packet identifier)

#if MQTT_VERSION == MQTT_VERSION_3_1
            const uint8_t protocol[9] = {0x00, 0x06, 'M', 'Q', 'I', 's', 'd', 'p', MQTT_VERSION};
#elif MQTT_VERSION == MQTT_VERSION_3_1_1
            const uint8_t protocol[7] = {0x00, 0x04, 'M', 'Q', 'T', 'T', MQTT_VERSION};
#endif
            // Leave room in the _buffer for header and variable length field
            memcpy(_buffer + MQTT_MAX_HEADER_SIZE, protocol, sizeof(protocol));

            size_t length = MQTT_MAX_HEADER_SIZE + sizeof(protocol);
            uint8_t flags = 0x00;
            if (willTopic) {
                flags = (0x01 << 2) | (willQos << 3) | (willRetain << 5);  // set will flag bit 2, will QoS and will retain bit 5
            }
            if (cleanSession) {
                flags = flags | (0x01 << 1);  // set clean session bit 1
            }
            if (user) {
                flags = flags | (0x01 << 7);  // set user name flag bit 7
                if (pass) {
                    flags = flags | (0x01 << 6);  // set password flag bit 6
                }
            }
            const uint16_t keepAlive = _keepAliveMillis / 1000;
            _buffer[length++] = flags;
            _buffer[length++] = keepAlive >> 8;
            _buffer[length++] = keepAlive & 0xFF;

            CHECK_STRING_LENGTH(length, id)
            length = writeString(id, length);
            if (willTopic) {
                CHECK_STRING_LENGTH(length, willTopic)
                length = writeString(willTopic, length);
                CHECK_STRING_LENGTH(length, willMessage)
                length = writeString(willMessage, length);
            }

            if (user) {
                CHECK_STRING_LENGTH(length, user)
                length = writeString(user, length);
                if (pass) {
                    CHECK_STRING_LENGTH(length, pass)
                    length = writeString(pass, length);
                }
            }

            if (!writeControlPacket(MQTTCONNECT, length - MQTT_MAX_HEADER_SIZE)) {
                _state = MQTT_CONNECT_FAILED;
                _client->stop();
                return false;
            }
            _lastInActivity = _lastOutActivity = millis();
            _pingOutstanding = false;

            while (!_client->available()) {
                yield();
                unsigned long t = millis();
                if (t - _lastInActivity >= _socketTimeoutMillis) {
                    DEBUG_PSC_PRINTF("connect aborting due to timeout\n");
                    _state = MQTT_CONNECTION_TIMEOUT;
                    _client->stop();
                    return false;
                }
            }
            uint8_t hdrLen;
            size_t len = readPacket(&hdrLen);

            if (len == 4) {
                if (_buffer[3] == 0) {
                    _lastInActivity = millis();
                    _state = MQTT_CONNECTED;
                    return true;
                } else {
                    _state = _buffer[3];
                }
            }
            DEBUG_PSC_PRINTF("connect aborting due to protocol error\n");
            _client->stop();
        } else {
            _state = MQTT_CONNECT_FAILED;
        }
        return false;
    }
    return true;
}

bool PubSubClient::connected() {
    if (!_client) return false;

    if (_client->connected()) {
        return (_state == MQTT_CONNECTED);
    } else if (_state == MQTT_CONNECTED) {
        DEBUG_PSC_PRINTF("lost connection (client may have more details)\n");
        _state = MQTT_CONNECTION_LOST;
        _client->stop();
        _pingOutstanding = false;
    }
    return false;
}

void PubSubClient::disconnect() {
    DEBUG_PSC_PRINTF("disconnect called\n");
    _state = MQTT_DISCONNECTED;
    if (_client) {
        _buffer[0] = MQTTDISCONNECT;
        _buffer[1] = 0;
        _client->write(_buffer, 2);
        _client->flush();
        _client->stop();
        _lastInActivity = _lastOutActivity = millis();
    }
    _pingOutstanding = false;
}

/**
 * @brief  Reads a byte into result.
 *
 * @param  result Pointer to result buffer.
 * @return true if byte was read, false if socketTimeout occurred or _client was not set.
 */
bool PubSubClient::readByte(uint8_t* result) {
    if (!_client) return false;  // do not crash if client not set

    unsigned long previousMillis = millis();
    while (!_client->available()) {
        yield();
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= _socketTimeoutMillis) {
            return false;
        }
    }
    int rc = _client->read();
    if (rc < 0) {
        return false;
    }
    *result = (uint8_t)rc;
    return true;
}

/**
 * @brief  Reads a byte into result[*pos] and increments *pos.
 * Note: *pos may go out of bounds of result. This must be checked outside of this function!
 *
 * @return true if a byte was read, otherwise false (socketTimeout).
 */
bool PubSubClient::readByte(uint8_t* result, size_t* pos) {
    uint8_t* write_address = &(result[*pos]);
    if (readByte(write_address)) {
        (*pos)++;
        return true;
    }
    return false;
}

/**
 * @brief  Reads a complete packet (header, topic, payload) into _buffer.
 *
 * @param  *hdrLen Returns the variable header length send by MQTT broker (1 .. MQTT_MAX_HEADER_SIZE - 1)
 * @return Number of read bytes, 0 in case of an error (socketTimeout, buffer overflow, _client not set).
 */
size_t PubSubClient::readPacket(uint8_t* hdrLen) {
    size_t len = 0;
    if (!readByte(_buffer, &len)) return 0;
    bool isPublish = (_buffer[0] & 0xF0) == MQTTPUBLISH;
    uint32_t multiplier = 1;
    size_t length = 0;
    uint8_t digit = 0;
    uint16_t skip = 0;
    uint8_t start = 0;

    do {
        if (len == MQTT_MAX_HEADER_SIZE) {
            // Invalid remaining length encoding - kill the connection
            DEBUG_PSC_PRINTF("readPacket detected packet of invalid length\n");
            _state = MQTT_DISCONNECTED;
            _client->stop();
            return 0;
        }
        if (!readByte(&digit)) return 0;
        _buffer[len++] = digit;
        length += (digit & 0x7F) * multiplier;  // length is coded in the lower 7 bits
        multiplier <<= 7;                       // multiplier *= 128
    } while ((digit & 0x80) != 0);  // do while 8th continuation bit is set
    *hdrLen = (uint8_t)(len - 1);

    DEBUG_PSC_PRINTF("readPacket received packet of length %zu (isPublish = %u)\n", length, isPublish);

    if (isPublish) {
        // Read in topic length to calculate bytes to skip over for Stream writing
        if (!readByte(_buffer, &len)) return 0;
        if (!readByte(_buffer, &len)) return 0;
        skip = (_buffer[*hdrLen + 1] << 8) + _buffer[*hdrLen + 2];
        start = 2;
        if (MQTT_HDR_GET_QOS(_buffer[0]) > MQTT_QOS0) {
            // skip msgId (packet identifier) for QoS 1 and 2 messages
            skip += 2;
        }
    }
    size_t idx = len;

    for (size_t i = start; i < length; i++) {
        if (!readByte(&digit)) return 0;
        if (_stream) {
            if (isPublish && (idx - *hdrLen - 2 > skip)) {
                _stream->write(digit);
            }
        }

        if (len < _bufferSize) {
            _buffer[len++] = digit;
        }
        idx++;
    }

    if (!_stream && (idx > _bufferSize)) {
        DEBUG_PSC_PRINTF("readPacket ignoring packet of size %zu exceeding buffer of size %zu\n", length, _bufferSize);
        len = 0;  // This will cause the packet to be ignored.
    }
    return len;
}

/**
 * @brief  After a packet is read handle the content here (call the callback, handle pings).
 *
 * @param  hdrLen Variable header length send by MQTT broker (1 .. MQTT_MAX_HEADER_SIZE - 1).
 * @param  length Number of read bytes in _buffer.
 * @return true if packet was successfully processed, false if a buffer over or underflow occurred.
 */
bool PubSubClient::handlePacket(uint8_t hdrLen, size_t length) {
    uint8_t type = _buffer[0] & 0xF0;
    DEBUG_PSC_PRINTF("received message of type %u\n", type);
    switch (type) {
        case MQTTPUBLISH:
            if (callback) {
                // MQTT Publish packet: See section 3.3 MQTT v3.1.1 protocol specification:
                // - Header: 1 byte
                // - Remaining header length: hdrLen bytes, multibyte field (1 .. MQTT_MAX_HEADER_SIZE - 1)
                // - Topic length: 2 bytes (starts at _buffer[hdrLen + 1])
                // - Topic: topicLen bytes (starts at _buffer[hdrLen + 3])
                // - Packet Identifier (msgId): 0 bytes for QoS 0, 2 bytes for QoS 1 and 2 (starts at _buffer[hdrLen + 3 + topicLen])
                // - Payload (for QoS = 0): length - (hdrLen + 3 + topicLen) bytes (starts at _buffer[hdrLen + 3 + topicLen])
                // - Payload (for QoS > 0): length - (hdrLen + 5 + topicLen) bytes (starts at _buffer[hdrLen + 5 + topicLen])
                // To get a null reminated 'C' topic string we move the topic 1 byte to the front (overwriting the LSB of the topic lenght)
                uint16_t topicLen = (_buffer[hdrLen + 1] << 8) + _buffer[hdrLen + 2];  // topic length in bytes
                char* topic = (char*)(_buffer + hdrLen + 3 - 1);                       // set the topic in the LSB of the topic lenght, as we move it there
                uint16_t payloadOffset = hdrLen + 3 + topicLen;  // payload starts after header and topic (if there is no packet identifier)
                size_t payloadLen = length - payloadOffset;      // this might change by 2 if we have a QoS 1 or 2 message
                uint8_t* payload = _buffer + payloadOffset;

                if (length < payloadOffset) {  // do not move outside the max bufferSize
                    ERROR_PSC_PRINTF_P("handlePacket(): Suspicious topicLen (%u) points outside of received buffer length (%zu)\n", topicLen, length);
                    return false;
                }
                memmove(topic, topic + 1, topicLen);  // move topic inside buffer 1 byte to front
                topic[topicLen] = '\0';               // end the topic as a 'C' string with \x00

                if (MQTT_HDR_GET_QOS(_buffer[0]) == MQTT_QOS0) {
                    // No msgId for QOS == 0
                    callback(topic, payload, payloadLen);
                } else {
                    // For QOS 1 and 2 we have a msgId (packet identifier) after the topic at the current payloadOffset
                    if (payloadLen < 2) {  // payload must be >= 2, as we have the msgId before
                        ERROR_PSC_PRINTF_P("handlePacket(): Missing msgId in QoS 1/2 message\n");
                        return false;
                    }
                    uint8_t publishQos = MQTT_HDR_GET_QOS(_buffer[0]);  // save QoS before _buffer[0] is overwritten
                    uint16_t msgId = (_buffer[payloadOffset] << 8) + _buffer[payloadOffset + 1];
                    callback(topic, payload + 2, payloadLen - 2);  // remove the msgId from the callback payload

                    // QoS 1: respond with PUBACK
                    // QoS 2: respond with PUBREC (first step of the QoS 2 subscriber handshake)
                    _buffer[0] = (publishQos == MQTT_QOS1) ? MQTTPUBACK : MQTTPUBREC;
                    _buffer[1] = 2;
                    _buffer[2] = (msgId >> 8);
                    _buffer[3] = (msgId & 0xFF);
                    if (_client->write(_buffer, 4) == 4) {
                        _lastOutActivity = millis();
                    }
                }
            }
            break;
        case MQTTPUBACK:
            // MQTT Publish Acknowledgment (QoS 1 publish received): See section 3.4 MQTT v3.1.1 protocol specification
            if (length < 4) {
                ERROR_PSC_PRINTF_P("handlePacket(): Received PUBACK packet with length %zu, expected at least 4 bytes\n", length);
                return false;
            }
            // No futher action here, as resending is not supported.
            break;
        case MQTTPUBREC:
            // MQTT Publish Received (QoS 2 publisher handshake, part 1): broker acknowledges our QoS 2 PUBLISH.
            // See section 3.5 MQTT v3.1.1 protocol specification.
            if (length < 4) {
                ERROR_PSC_PRINTF_P("handlePacket(): Received PUBREC packet with length %zu, expected at least 4 bytes\n", length);
                return false;
            }
            // MQTT Publish Release (QoS 2 publisher handshake, part 2): See section 3.6 MQTT v3.1.1 protocol specification
            _buffer[0] = MQTTPUBREL | 2;  // PUBREL fixed header: bit 1 must be set per spec
            // bytes 1-3 of PUBREL are the same as of PUBREC (remaining length + msgId)
            if (_client->write(_buffer, 4) == 4) {
                _lastOutActivity = millis();
            }
            break;
        case MQTTPUBREL:
            // MQTT Publish Release (QoS 2 subscriber handshake, part 2): broker releases the message to us.
            // See section 3.6 MQTT v3.1.1 protocol specification.
            if (length < 4) {
                ERROR_PSC_PRINTF_P("handlePacket(): Received PUBREL packet with length %zu, expected at least 4 bytes\n", length);
                return false;
            }
            // MQTT Publish Complete (QoS 2 subscriber handshake, part 3): See section 3.7 MQTT v3.1.1 protocol specification
            _buffer[0] = MQTTPUBCOMP;
            // bytes 1-3 of PUBCOMP are the same as of PUBREL (remaining length + msgId)
            if (_client->write(_buffer, 4) == 4) {
                _lastOutActivity = millis();
            }
            break;
        case MQTTPUBCOMP:
            // MQTT Publish Complete (QoS 2 publisher handshake, part 3): broker confirms delivery of our QoS 2 PUBLISH.
            // See section 3.7 MQTT v3.1.1 protocol specification.
            if (length < 4) {
                ERROR_PSC_PRINTF_P("handlePacket(): Received PUBCOMP packet with length %zu, expected at least 4 bytes\n", length);
                return false;
            }
            // No futher action here, as resending is not supported.
            break;
        case MQTTPINGREQ:
            // MQTT Ping Request: See section 3.12 MQTT v3.1.1 protocol specification
            _buffer[0] = MQTTPINGRESP;
            _buffer[1] = 0;
            if (_client->write(_buffer, 2) == 2) {
                _lastOutActivity = millis();
            }
            break;
        case MQTTPINGRESP:
            _pingOutstanding = false;
            break;
        default:
            break;
    }
    return true;
}

bool PubSubClient::loop() {
    if (!connected()) {
        return false;
    }
    bool ret = true;
    const unsigned long t = millis();
    if (_keepAliveMillis && ((t - _lastInActivity > _keepAliveMillis) || (t - _lastOutActivity > _keepAliveMillis))) {
        if (_pingOutstanding) {
            DEBUG_PSC_PRINTF("loop aborting due to timeout\n");
            _state = MQTT_CONNECTION_TIMEOUT;
            _client->stop();
            _pingOutstanding = false;
            return false;
        } else if (_bufferWritePos > 0) {
            // There is still data in the _buffer to be sent, so send it now instead of a ping
            if (flushBuffer() == 0) {
                _state = MQTT_CONNECTION_TIMEOUT;
                _client->stop();
                _pingOutstanding = false;
                return false;
            }
        } else {
            _buffer[0] = MQTTPINGREQ;
            _buffer[1] = 0;
            if (_client->write(_buffer, 2) == 2) {
                _lastInActivity = _lastOutActivity = t;
                _pingOutstanding = true;
            }
        }
    }
    if (_client->available()) {
        uint8_t hdrLen;
        size_t len = readPacket(&hdrLen);
        if (len > 0) {
            _lastInActivity = t;
            ret = handlePacket(hdrLen, len);
            if (!ret) {
                _state = MQTT_DISCONNECTED;
                _client->stop();
            }
        } else if (!connected()) {
            // readPacket has closed the connection
            return false;
        }
    }
    return ret;
}

bool PubSubClient::publish(const char* topic, const uint8_t* payload, size_t plength, uint8_t qos, bool retained) {
    if (beginPublish(topic, plength, qos, retained)) {
        size_t rc = write(payload, plength);
        return endPublish() && (rc == plength);
    }
    return false;
}

bool PubSubClient::publish(const __FlashStringHelper* topic, const uint8_t* payload, size_t plength, uint8_t qos, bool retained) {
    if (beginPublish(topic, plength, qos, retained)) {
        size_t rc = write(payload, plength);
        return endPublish() && (rc == plength);
    }
    return false;
}

bool PubSubClient::publish_P(const char* topic, const uint8_t* payload, size_t plength, uint8_t qos, bool retained) {
    if (beginPublish(topic, plength, qos, retained)) {
        size_t rc = write_P(payload, plength);
        return endPublish() && (rc == plength);
    }
    return false;
}

bool PubSubClient::publish_P(const __FlashStringHelper* topic, const uint8_t* payload, size_t plength, uint8_t qos, bool retained) {
    if (beginPublish(topic, plength, qos, retained)) {
        size_t rc = write_P(payload, plength);
        return endPublish() && (rc == plength);
    }
    return false;
}

/**
 * @brief Internal beginPublish implementation using topic stored in RAM or PROGMEM.
 *
 * @param progmem true if the topic is stored in PROGMEM/Flash, false if in RAM.
 * @param topic The topic to publish to.
 * @param plength The length of the payload.
 * @param qos The quality of service (\ref group_qos) to publish at. [0, 1, 2].
 * @param retained Publish the message with the retain flag.
 * @return true If the publish succeeded.
 * false If the publish failed, either connection lost or message too large.
 */
bool PubSubClient::beginPublishImpl(bool progmem, const char* topic, size_t plength, uint8_t qos, bool retained) {
    if (!topic) return false;

    // get topic length depending on storage (RAM vs PROGMEM)
    size_t topicLen = progmem ? strlen_P(topic) : strlen(topic);
    if (topicLen == 0) return false;  // empty topic is not allowed

    if (qos > MQTT_QOS2) {  // only valid QoS supported
        ERROR_PSC_PRINTF_P("beginPublish() called with invalid QoS %u\n", qos);
        return false;
    }

    const size_t nextMsgLen = (qos > MQTT_QOS0) ? 2 : 0;  // add 2 bytes for nextMsgId if QoS > 0
    // check if the header, the topic (including 2 length bytes) and nextMsgId fit into the _buffer
    if (connected() && (MQTT_MAX_HEADER_SIZE + topicLen + 2 + nextMsgLen <= _bufferSize)) {
        // first write the topic at the end of the maximal variable header (MQTT_MAX_HEADER_SIZE) to the _buffer
        topicLen = writeStringImpl(progmem, topic, MQTT_MAX_HEADER_SIZE) - MQTT_MAX_HEADER_SIZE;
        if (qos > MQTT_QOS0) {
            // if QoS 1 or 2, we need to send the nextMsgId (packet identifier) after topic
            writeNextMsgId(MQTT_MAX_HEADER_SIZE + topicLen);
        }
        // we now know the length of the topic string (length + 2 bytes signalling the length) and can build the variable header information
        const uint8_t header = MQTTPUBLISH | MQTT_QOS_GET_HDR(qos) | (retained ? MQTTRETAINED : 0);
        uint8_t hdrLen = buildHeader(header, topicLen + nextMsgLen + plength);
        if (hdrLen == 0) return false;  // exit here in case of header generation failure
        // as the header length is variable, it starts at MQTT_MAX_HEADER_SIZE - hdrLen (see buildHeader() documentation)
        size_t rc = _client->write(_buffer + (MQTT_MAX_HEADER_SIZE - hdrLen), hdrLen + topicLen + nextMsgLen);
        _lastOutActivity = millis();
        return (rc == (hdrLen + topicLen + nextMsgLen));
    }
    return false;
}

bool PubSubClient::endPublish() {
    if (connected()) {
        if (_bufferWritePos > 0) {
            // still data in the _buffer to be sent
            if (flushBuffer() == 0) return false;
        }
        return true;
    }
    return false;
}

/**
 * @brief  Build up the header ready to send.
 * Note: the header is built at the end of the first MQTT_MAX_HEADER_SIZE bytes, so will start
 * (MQTT_MAX_HEADER_SIZE - <returned size>) bytes into the _buffer.
 *
 * @param  header Header byte, e.g. MQTTCONNECT, MQTTPUBLISH, MQTTSUBSCRIBE, MQTTUNSUBSCRIBE.
 * @param  length Length to encode in the header.
 * @return Returns the size of the header (1 .. MQTT_MAX_HEADER_SIZE), or 0 in case of a failure (e.g. length to big).
 */
uint8_t PubSubClient::buildHeader(uint8_t header, size_t length) {
    uint8_t hdrBuf[MQTT_MAX_HEADER_SIZE - 1];
    uint8_t hdrLen = 0;
    uint8_t digit;
    size_t len = length;
    do {
        digit = len & 0x7F;  // digit = len % 128
        len >>= 7;           // len = len / 128
        if (len > 0) {
            digit |= 0x80;
        }
        hdrBuf[hdrLen++] = digit;
    } while ((len > 0) && (hdrLen < MQTT_MAX_HEADER_SIZE - 1));

    if (len > 0) {
        ERROR_PSC_PRINTF_P("buildHeader: header=0x%02X, length too big %zu, left %zu\n", header, length, len);
        return 0;
    }

    _buffer[MQTT_MAX_HEADER_SIZE - 1 - hdrLen] = header;
    memcpy(_buffer + MQTT_MAX_HEADER_SIZE - hdrLen, hdrBuf, hdrLen);
    return hdrLen + 1;  // Full header size is variable length bit plus the 1-byte fixed header
}

size_t PubSubClient::write(uint8_t data) {
    return appendBuffer(data);
}

size_t PubSubClient::write(const uint8_t* buf, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (appendBuffer(buf[i]) == 0) return i;
    }
    return size;
}

size_t PubSubClient::write_P(const uint8_t* buf, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (appendBuffer((uint8_t)pgm_read_byte_near(buf + i)) == 0) return i;
    }
    return size;
}

/**
 * @brief  Write a MQTT Control Packet (header and the prepared data) to the client / MQTT broker. The prepared data of size length must already be in the
 * internal _buffer starting at MQTT_MAX_HEADER_SIZE (space is needed for header generation).
 *
 * @param  header Header byte, e.g. MQTTCONNECT, MQTTPUBLISH, MQTTSUBSCRIBE, MQTTUNSUBSCRIBE.
 * @param  length Length of _buffer to write.
 * @return True if successfully sent, otherwise false if build header failed or buffer could not be written.
 */
bool PubSubClient::writeControlPacket(uint8_t header, size_t length) {
    uint8_t hdrLen = buildHeader(header, length);
    if (hdrLen == 0) return false;  // exit here in case of header generation failure

    return writeBuffer(MQTT_MAX_HEADER_SIZE - hdrLen, hdrLen + length);
}

/**
 * @brief  Write the internal _buffer to the client / MQTT broker.
 *
 * @param  pos Position in the _buffer to start writing from.
 * @param  size Number of bytes to write from the _buffer.
 * @return Number of bytes written to the client / MQTT broker (0 .. bufferSize). If 0 is returned a write error occurred or buffer index error.
 */
size_t PubSubClient::writeBuffer(size_t pos, size_t size) {
    size_t rc = 0;
    if (_client && (size > 0) && (pos + size <= _bufferSize)) {
#ifdef MQTT_MAX_TRANSFER_SIZE
        uint8_t* writeBuf = _buffer + pos;
        size_t bytesRemaining = size;
        bool result = true;
        while ((bytesRemaining > 0) && result) {
            size_t bytesToWrite = (bytesRemaining > MQTT_MAX_TRANSFER_SIZE) ? MQTT_MAX_TRANSFER_SIZE : bytesRemaining;
            size_t bytesWritten = _client->write(writeBuf, bytesToWrite);
            result = (bytesWritten == bytesToWrite);
            bytesRemaining -= bytesWritten;
            writeBuf += bytesWritten;
            if (result) {
                _lastOutActivity = millis();
            }
            yield();
        }
        rc = result ? size : 0;  // if result is false indicate a write error
#else
        rc = _client->write(_buffer + pos, size);
        if (rc == size) {
            _lastOutActivity = millis();
        } else {
            rc = 0;  // indicate a write error
        }
#endif
    }
    return rc;
}

/**
 * @brief  Internal implementation of writeString using RAM or PROGMEM string.
 * Write an UTF-8 encoded string to the internal buffer at a given position. The string can have a length of 0 to 65535 bytes (depending on size of
 * internal buffer). The buffer is prefixed with two bytes representing the length of the string. See section 1.5.3 of MQTT v3.1.1 protocol specification.
 * @note   If the string does not fit in the buffer or is longer than 65535 bytes nothing is written to the buffer and the returned position is
 * unchanged.
 *
 * @param  progmem true if the string is stored in PROGMEM, false if in RAM.
 * @param  string 'C' string of the data that shall be written in the buffer.
 * @param  pos Position in the internal buffer to write the string.
 * @return New position in the internal buffer (pos + 2 + string length), or pos if a buffer overrun would occur or the string is a nullptr.
 */
size_t PubSubClient::writeStringImpl(bool progmem, const char* string, size_t pos) {
    if (!string) return pos;

    size_t sLen = progmem ? strlen_P(string) : strlen(string);
    if ((pos + 2 + sLen <= _bufferSize) && (sLen <= 0xFFFF)) {
        _buffer[pos++] = (uint8_t)(sLen >> 8);
        _buffer[pos++] = (uint8_t)(sLen & 0xFF);
        if (progmem) {
            memcpy_P(_buffer + pos, string, sLen);
        } else {
            memcpy(_buffer + pos, string, sLen);
        }
        pos += sLen;
    } else {
        ERROR_PSC_PRINTF_P("writeStringImpl(): string (%zu) does not fit into buf (%zu)\n", pos + 2 + sLen, _bufferSize);
    }
    return pos;
}

/**
 * @brief  Write an UTF-8 encoded string to the internal buffer at a given position. The string can have a length of 0 to 65535 bytes (depending on size of
 * internal buffer). The buffer is prefixed with two bytes representing the length of the string. See section 1.5.3 of MQTT v3.1.1 protocol specification.
 * @note   If the string does not fit in the buffer or is longer than 65535 bytes nothing is written to the buffer and the returned position is
 * unchanged.
 *
 * @param  string 'C' string of the data that shall be written in the buffer.
 * @param  pos Position in the internal buffer to write the string.
 * @return New position in the internal buffer (pos + 2 + string length), or pos if a buffer overrun would occur or the string is a nullptr.
 */
inline size_t PubSubClient::writeString(const char* string, size_t pos) {
    return writeStringImpl(false, string, pos);
}

/**
 * @brief  Write nextMsgId to the internal buffer at the given position.
 * @note   If the nextMsgId (2 bytes) does not fit in the buffer nothing is written to the buffer and the returned position is unchanged.
 *
 * @param  pos Position in the internal buffer to write the nextMsgId.
 * @return New position in the internal buffer (pos + 2), or pos if a buffer overrun would occur.
 */
size_t PubSubClient::writeNextMsgId(size_t pos) {
    if ((pos + 2) <= _bufferSize) {
        _nextMsgId = (++_nextMsgId == 0) ? 1 : _nextMsgId;  // increment msgId (must not be 0, so start at 1)
        _buffer[pos++] = (uint8_t)(_nextMsgId >> 8);
        _buffer[pos++] = (uint8_t)(_nextMsgId & 0xFF);
    } else {
        ERROR_PSC_PRINTF_P("writeNextMsgId(): buffer overrun (%zu) \n", pos + 2);
    }
    return pos;
}

/**
 * @brief  Append a byte to the internal _buffer. If the _buffer is full it is flushed to the client / MQTT broker.
 *
 * @param  data Byte to append to the _buffer.
 * @return Number of bytes appended to the _buffer (0 or 1). If 0 is returned a write error occurred.
 */
size_t PubSubClient::appendBuffer(uint8_t data) {
    _buffer[_bufferWritePos++] = data;
    if (_bufferWritePos >= _bufferSize) {
        if (flushBuffer() == 0) return 0;
    }
    return 1;
}

/**
 * @brief  Flush the internal _buffer (bytes 0 .. _bufferWritePos) to the client / MQTT broker.
 * This is used by endPublish() to flush data written by appendBuffer().
 *
 * @return Number of bytes written to the client / MQTT broker (0 .. bufferSize). If 0 is returned a write error occurred or the _buffer was empty.
 */
size_t PubSubClient::flushBuffer() {
    size_t rc = 0;
    if (connected()) {
        rc = writeBuffer(0, _bufferWritePos);
    }
    _bufferWritePos = 0;
    return rc;
}

/**
 * @brief Internal subscribes to messages published to the specified topic. The topic can be stored in RAM or PROGMEM.
 * @param progmem true if the topic is stored in PROGMEM/Flash, false if in RAM.
 * @param topic The topic to subscribe to.
 * @param qos The qos to subscribe at. [0, 1].
 * @return true If sending the subscribe succeeded.
 * false If sending the subscribe failed, either connection lost or message too large.
 */
bool PubSubClient::subscribeImpl(bool progmem, const char* topic, uint8_t qos) {
    if (!topic) return false;
    if (qos > MQTT_QOS1) return false;  // only QoS 0 and 1 supported

    // get topic length depending on storage (RAM vs PROGMEM)
    size_t topicLen = progmem ? strnlen_P(topic, _bufferSize) : strnlen(topic, _bufferSize);
    if (_bufferSize < MQTT_MAX_HEADER_SIZE + 2 + 2 + topicLen + 1) {
        // Too long: header + nextMsgId (2) + topic length bytes (2) + topicLen + QoS (1)
        return false;
    }
    if (connected()) {
        // Leave room in the _buffer for header and variable length field
        uint16_t length = MQTT_MAX_HEADER_SIZE;
        length = writeNextMsgId(length);  // _buffer size is checked before
        length = writeStringImpl(progmem, topic, length);
        _buffer[length++] = qos;
        return writeControlPacket(MQTTSUBSCRIBE | MQTT_QOS_GET_HDR(MQTT_QOS1), length - MQTT_MAX_HEADER_SIZE);
    }
    return false;
}

/**
 * @brief Internal unsubscribes from messages published to the specified topic. The topic can be stored in RAM or PROGMEM.
 * @param progmem true if the topic is stored in PROGMEM/Flash, false if in RAM.
 * @param topic The topic to unsubscribe from.
 * @return true If sending the unsubscribe succeeded.
 * false If sending the unsubscribe failed, either connection lost or message too large.
 */
bool PubSubClient::unsubscribeImpl(bool progmem, const char* topic) {
    if (!topic) return false;

    // get topic length depending on storage (RAM vs PROGMEM)
    size_t topicLen = progmem ? strnlen_P(topic, _bufferSize) : strnlen(topic, _bufferSize);
    if (_bufferSize < MQTT_MAX_HEADER_SIZE + 2 + 2 + topicLen) {
        // Too long: header + nextMsgId (2) + topic length bytes (2) + topicLen
        return false;
    }
    if (connected()) {
        uint16_t length = MQTT_MAX_HEADER_SIZE;
        length = writeNextMsgId(length);  // _buffer size is checked before
        length = writeStringImpl(progmem, topic, length);
        return writeControlPacket(MQTTUNSUBSCRIBE | MQTT_QOS_GET_HDR(MQTT_QOS1), length - MQTT_MAX_HEADER_SIZE);
    }
    return false;
}

PubSubClient& PubSubClient::setServer(uint8_t* ip, uint16_t port) {
    IPAddress addr(ip[0], ip[1], ip[2], ip[3]);
    return setServer(addr, port);
}

PubSubClient& PubSubClient::setServer(IPAddress ip, uint16_t port) {
    _ip = ip;
    _port = port;
    free(_domain);
    _domain = nullptr;
    return *this;
}

PubSubClient& PubSubClient::setServer(const char* domain, uint16_t port) {
    char* newDomain = nullptr;
    if (domain) {
        newDomain = (char*)realloc(_domain, strlen(domain) + 1);
    }
    if (newDomain) {
        strcpy(newDomain, domain);
        _domain = newDomain;
        _port = port;
    } else {
        free(_domain);
        _domain = nullptr;
        _port = 0;
    }
    return *this;
}

PubSubClient& PubSubClient::setCallback(MQTT_CALLBACK_SIGNATURE) {
    this->callback = callback;
    return *this;
}

PubSubClient& PubSubClient::setClient(Client& client) {
    _client = &client;
    return *this;
}

PubSubClient& PubSubClient::setStream(Stream& stream) {
    _stream = &stream;
    return *this;
}

bool PubSubClient::setBufferSize(size_t size) {
    if (size == 0) {
        // Cannot set it back to 0
        return false;
    }
    if (_bufferSize == 0) {
        _buffer = (uint8_t*)malloc(size);
    } else {
        uint8_t* newBuffer = (uint8_t*)realloc(_buffer, size);
        if (newBuffer) {
            _buffer = newBuffer;
        } else {
            return false;
        }
    }
    _bufferSize = size;
    return (_buffer != nullptr);
}

size_t PubSubClient::getBufferSize() {
    return _bufferSize;
}

PubSubClient& PubSubClient::setKeepAlive(uint16_t keepAlive) {
    _keepAliveMillis = keepAlive * 1000UL;
    return *this;
}

PubSubClient& PubSubClient::setSocketTimeout(uint16_t timeout) {
    _socketTimeoutMillis = timeout * 1000UL;
    return *this;
}

int PubSubClient::state() {
    return _state;
}
