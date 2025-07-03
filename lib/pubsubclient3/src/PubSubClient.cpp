/*
  PubSubClient.cpp - A simple client for MQTT.
  Nick O'Leary, Holger Mueller
  http://knolleary.net
  https://github.com/hmueller01/pubsubclient3
*/

#include "PubSubClient.h"

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
    free(this->domain);
    free(this->buffer);
}

bool PubSubClient::connect(const char* id) {
    return connect(id, nullptr, nullptr, 0, 0, 0, 0, 1);
}

bool PubSubClient::connect(const char* id, const char* user, const char* pass) {
    return connect(id, user, pass, 0, 0, 0, 0, 1);
}

bool PubSubClient::connect(const char* id, const char* willTopic, uint8_t willQos, bool willRetain, const char* willMessage) {
    return connect(id, nullptr, nullptr, willTopic, willQos, willRetain, willMessage, 1);
}

bool PubSubClient::connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, bool willRetain,
                           const char* willMessage) {
    return connect(id, user, pass, willTopic, willQos, willRetain, willMessage, 1);
}

bool PubSubClient::connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, bool willRetain,
                           const char* willMessage, bool cleanSession) {
    if (!connected()) {
        int result = 0;

        if (_client->connected()) {
            result = 1;
        } else if (this->port != 0) {
            if (this->domain) {
                result = _client->connect(this->domain, this->port);
            } else {
                result = _client->connect(this->ip, this->port);
            }
        }

        if (result == 1) {
            nextMsgId = 1;  // init msgId (packet identifier)

#if MQTT_VERSION == MQTT_VERSION_3_1
            const uint8_t protocol[9] = {0x00, 0x06, 'M', 'Q', 'I', 's', 'd', 'p', MQTT_VERSION};
#elif MQTT_VERSION == MQTT_VERSION_3_1_1
            const uint8_t protocol[7] = {0x00, 0x04, 'M', 'Q', 'T', 'T', MQTT_VERSION};
#endif
            // Leave room in the buffer for header and variable length field
            memcpy(this->buffer + MQTT_MAX_HEADER_SIZE, protocol, sizeof(protocol));

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
            const uint16_t keepAlive = this->keepAliveMillis / 1000;
            this->buffer[length++] = flags;
            this->buffer[length++] = keepAlive >> 8;
            this->buffer[length++] = keepAlive & 0xFF;

            CHECK_STRING_LENGTH(length, id)
            length = writeString(id, this->buffer, length);
            if (willTopic) {
                CHECK_STRING_LENGTH(length, willTopic)
                length = writeString(willTopic, this->buffer, length);
                CHECK_STRING_LENGTH(length, willMessage)
                length = writeString(willMessage, this->buffer, length);
            }

            if (user) {
                CHECK_STRING_LENGTH(length, user)
                length = writeString(user, this->buffer, length);
                if (pass) {
                    CHECK_STRING_LENGTH(length, pass)
                    length = writeString(pass, this->buffer, length);
                }
            }

            write(MQTTCONNECT, this->buffer, length - MQTT_MAX_HEADER_SIZE);

            lastInActivity = lastOutActivity = millis();
            pingOutstanding = false;

            while (!_client->available()) {
                yield();
                unsigned long t = millis();
                if (t - lastInActivity >= this->socketTimeoutMillis) {
                    DEBUG_PSC_PRINTF("connect aborting due to timeout\n");
                    _state = MQTT_CONNECTION_TIMEOUT;
                    _client->stop();
                    return false;
                }
            }
            uint8_t hdrLen;
            size_t len = readPacket(&hdrLen);

            if (len == 4) {
                if (buffer[3] == 0) {
                    lastInActivity = millis();
                    _state = MQTT_CONNECTED;
                    return true;
                } else {
                    _state = buffer[3];
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
        pingOutstanding = false;
    }
    return false;
}

void PubSubClient::disconnect() {
    DEBUG_PSC_PRINTF("disconnect called\n");
    this->buffer[0] = MQTTDISCONNECT;
    this->buffer[1] = 0;
    _client->write(this->buffer, 2);
    _state = MQTT_DISCONNECTED;
    _client->flush();
    _client->stop();
    lastInActivity = lastOutActivity = millis();
    pingOutstanding = false;
}

/**
 * @brief  Reads a byte into result.
 *
 * @param  result Pointer to result buffer.
 * @return true if byte was read, false if socketTimeout occurred.
 */
bool PubSubClient::readByte(uint8_t* result) {
    unsigned long previousMillis = millis();
    while (!_client->available()) {
        yield();
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= this->socketTimeoutMillis) {
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
 * @brief  Reads a complete packet (header, topic, payload) into this->buffer.
 *
 * @param  *hdrLen Returns the variable header length send by MQTT broker (1 .. MQTT_MAX_HEADER_SIZE - 1)
 * @return Number of read bytes, 0 in case of an error (socketTimeout, buffer overflow)
 */
size_t PubSubClient::readPacket(uint8_t* hdrLen) {
    size_t len = 0;
    if (!readByte(this->buffer, &len)) return 0;
    bool isPublish = (this->buffer[0] & 0xF0) == MQTTPUBLISH;
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
        this->buffer[len++] = digit;
        length += (digit & 0x7F) * multiplier;  // length is coded in the lower 7 bits
        multiplier <<= 7;                       // multiplier *= 128
    } while ((digit & 0x80) != 0);  // do while 8th continuation bit is set
    *hdrLen = (uint8_t)(len - 1);

    DEBUG_PSC_PRINTF("readPacket received packet of length %zu (isPublish = %u)\n", length, isPublish);

    if (isPublish) {
        // Read in topic length to calculate bytes to skip over for Stream writing
        if (!readByte(this->buffer, &len)) return 0;
        if (!readByte(this->buffer, &len)) return 0;
        skip = (this->buffer[*hdrLen + 1] << 8) + this->buffer[*hdrLen + 2];
        start = 2;
        if (this->buffer[0] & MQTTQOS1) {
            // skip message id
            skip += 2;
        }
    }
    size_t idx = len;

    for (size_t i = start; i < length; i++) {
        if (!readByte(&digit)) return 0;
        if (this->stream) {
            if (isPublish && idx - *hdrLen - 2 > skip) {
                this->stream->write(digit);
            }
        }

        if (len < this->bufferSize) {
            this->buffer[len++] = digit;
        }
        idx++;
    }

    if (!this->stream && idx > this->bufferSize) {
        DEBUG_PSC_PRINTF("readPacket ignoring packet of size %zu exceeding buffer of size %zu\n", length, this->bufferSize);
        len = 0;  // This will cause the packet to be ignored.
    }
    return len;
}

/**
 * @brief  After a packet is read handle the content here (call the callback, handle pings).
 *
 * @param  hdrLen Variable header length send by MQTT broker (1 .. MQTT_MAX_HEADER_SIZE - 1).
 * @param  length Number of read bytes in this->buffer.
 * @return true if packet was successfully processed, false if a buffer over or underflow occurred.
 */
bool PubSubClient::handlePacket(uint8_t hdrLen, size_t length) {
    uint8_t type = this->buffer[0] & 0xF0;
    DEBUG_PSC_PRINTF("received message of type %u\n", type);
    switch (type) {
        case MQTTPUBLISH:
            if (callback) {
                // MQTT Publish packet: See section 3.3 MQTT v3.1 protocol specification:
                // - Header: 1 byte
                // - Remaining header length: hdrLen bytes, multibyte field (1 .. MQTT_MAX_HEADER_SIZE - 1)
                // - Topic length: 2 bytes (starts at buffer[hdrLen + 1])
                // - Topic: topicLen bytes (starts at buffer[hdrLen + 3])
                // - Packet Identifier (msgId): 0 bytes for QoS 0, 2 bytes for QoS 1 and 2 (starts at buffer[hdrLen + 3 + topicLen])
                // - Payload (for QoS = 0): length - (hdrLen + 3 + topicLen) bytes (starts at buffer[hdrLen + 3 + topicLen])
                // - Payload (for QoS > 0): length - (hdrLen + 5 + topicLen) bytes (starts at buffer[hdrLen + 5 + topicLen])
                // To get a null reminated 'C' topic string we move the topic 1 byte to the front (overwriting the LSB of the topic lenght)
                uint16_t topicLen = (this->buffer[hdrLen + 1] << 8) + this->buffer[hdrLen + 2];  // topic length in bytes
                char* topic = (char*)(this->buffer + hdrLen + 3 - 1);  // set the topic in the LSB of the topic lenght, as we move it there
                uint16_t payloadOffset = hdrLen + 3 + topicLen;        // payload starts after header and topic (if there is no packet identifier)
                size_t payloadLen = length - payloadOffset;            // this might change by 2 if we have a QoS 1 or 2 message
                uint8_t* payload = this->buffer + payloadOffset;

                if (length < payloadOffset) {  // do not move outside the max bufferSize
                    ERROR_PSC_PRINTF_P("handlePacket(): Suspicious topicLen (%u) points outside of received buffer length (%zu)\n", topicLen, length);
                    return false;
                }
                memmove(topic, topic + 1, topicLen);  // move topic inside buffer 1 byte to front
                topic[topicLen] = '\0';               // end the topic as a 'C' string with \x00

                if ((this->buffer[0] & 0x06) == MQTTQOS0) {
                    // No msgId for QOS == 0
                    callback(topic, payload, payloadLen);
                } else {
                    // For QOS 1 and 2 we have a msgId (packet identifier) after the topic at the current payloadOffset
                    if (payloadLen < 2) {  // payload must be >= 2, as we have the msgId before
                        ERROR_PSC_PRINTF_P("handlePacket(): Missing msgId in QoS 1/2 message\n");
                        return false;
                    }
                    uint16_t msgId = (this->buffer[payloadOffset] << 8) + this->buffer[payloadOffset + 1];
                    callback(topic, payload + 2, payloadLen - 2);  // remove the msgId from the callback payload

                    this->buffer[0] = MQTTPUBACK;
                    this->buffer[1] = 2;
                    this->buffer[2] = (msgId >> 8);
                    this->buffer[3] = (msgId & 0xFF);
                    if (_client->write(this->buffer, 4) == 4) {
                        lastOutActivity = millis();
                    }
                }
            }
            break;
        case MQTTPINGREQ:
            this->buffer[0] = MQTTPINGRESP;
            this->buffer[1] = 0;
            if (_client->write(this->buffer, 2) == 2) {
                lastOutActivity = millis();
            }
            break;
        case MQTTPINGRESP:
            pingOutstanding = false;
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
    if (keepAliveMillis && ((t - lastInActivity > this->keepAliveMillis) || (t - lastOutActivity > this->keepAliveMillis))) {
        if (pingOutstanding) {
            DEBUG_PSC_PRINTF("loop aborting due to timeout\n");
            _state = MQTT_CONNECTION_TIMEOUT;
            _client->stop();
            pingOutstanding = false;
            return false;
        } else {
            this->buffer[0] = MQTTPINGREQ;
            this->buffer[1] = 0;
            if (_client->write(this->buffer, 2) == 2) {
                lastInActivity = lastOutActivity = t;
                pingOutstanding = true;
            }
        }
    }
    if (_client->available()) {
        uint8_t hdrLen;
        size_t len = readPacket(&hdrLen);
        if (len > 0) {
            lastInActivity = t;
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

bool PubSubClient::publish(const char* topic, const char* payload) {
    return publish(topic, (const uint8_t*)payload, payload ? strnlen(payload, MQTT_MAX_POSSIBLE_PACKET_SIZE) : 0, false);
}

bool PubSubClient::publish(const char* topic, const char* payload, bool retained) {
    return publish(topic, (const uint8_t*)payload, payload ? strnlen(payload, MQTT_MAX_POSSIBLE_PACKET_SIZE) : 0, retained);
}

bool PubSubClient::publish(const char* topic, const uint8_t* payload, size_t plength) {
    return publish(topic, payload, plength, false);
}

bool PubSubClient::publish(const char* topic, const uint8_t* payload, size_t plength, bool retained) {
    if (beginPublish(topic, plength, retained)) {
        size_t rc = write(payload, plength);
        lastOutActivity = millis();
        return endPublish() && (rc == plength);
    }
    return false;
}

bool PubSubClient::publish_P(const char* topic, const char* payload, bool retained) {
    return publish_P(topic, (const uint8_t*)payload, payload ? strnlen_P(payload, MQTT_MAX_POSSIBLE_PACKET_SIZE) : 0, retained);
}

bool PubSubClient::publish_P(const char* topic, const uint8_t* payload, size_t plength, bool retained) {
    if (beginPublish(topic, plength, retained)) {
        size_t rc = 0;
        for (size_t i = 0; i < plength; i++) {
            rc += _client->write((uint8_t)pgm_read_byte_near(payload + i));
        }
        lastOutActivity = millis();
        return endPublish() && (rc == plength);
    }
    return false;
}

bool PubSubClient::beginPublish(const char* topic, size_t plength, bool retained) {
    if (!topic) return false;
    // check if the header and the topic (including 2 length bytes) fit into the buffer
    if (connected() && MQTT_MAX_HEADER_SIZE + strlen(topic) + 2 <= this->bufferSize) {
        // first write the topic at the end of the maximal variable header (MQTT_MAX_HEADER_SIZE) to the buffer
        size_t topicLen = writeString(topic, this->buffer, MQTT_MAX_HEADER_SIZE) - MQTT_MAX_HEADER_SIZE;
        // we now know the length of the topic string (lenght + 2 bytes signalling the length) and can build the variable header information
        const uint8_t header = MQTTPUBLISH | (retained ? MQTTRETAINED : 0);
        uint8_t hdrLen = buildHeader(header, this->buffer, topicLen + plength);
        if (hdrLen == 0) return false;  // exit here in case of header generation failure
        // as the header length is variable, it starts at MQTT_MAX_HEADER_SIZE - hdrLen (see buildHeader() documentation)
        size_t rc = _client->write(this->buffer + (MQTT_MAX_HEADER_SIZE - hdrLen), hdrLen + topicLen);
        lastOutActivity = millis();
        return (rc == (hdrLen + topicLen));
    }
    return false;
}

bool PubSubClient::endPublish() {
    flushBuffer();
    return connected();
}

/**
 * @brief  Build up the header ready to send.
 * Note: the header is built at the end of the first MQTT_MAX_HEADER_SIZE bytes, so will start
 * (MQTT_MAX_HEADER_SIZE - <returned size>) bytes into the buffer.
 *
 * @param  header Header byte, e.g. MQTTCONNECT, MQTTPUBLISH, MQTTSUBSCRIBE, MQTTUNSUBSCRIBE.
 * @param  buf Buffer to write header to.
 * @param  length Length to encode in the header.
 * @return Returns the size of the header (1 .. MQTT_MAX_HEADER_SIZE), or 0 in case of a failure (e.g. length to big).
 */
uint8_t PubSubClient::buildHeader(uint8_t header, uint8_t* buf, size_t length) {
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
    } while (len > 0 && hdrLen < MQTT_MAX_HEADER_SIZE - 1);

    if (len > 0) {
        ERROR_PSC_PRINTF_P("buildHeader() length too big %zu, left %zu\n", length, len);
        return 0;
    }

    buf[MQTT_MAX_HEADER_SIZE - 1 - hdrLen] = header;
    memcpy(buf + MQTT_MAX_HEADER_SIZE - hdrLen, hdrBuf, hdrLen);
    return hdrLen + 1;  // Full header size is variable length bit plus the 1-byte fixed header
}

size_t PubSubClient::write(uint8_t data) {
    const size_t rc = appendBuffer(data);
    if (rc != 0) {
        lastOutActivity = millis();
    }
    return rc;
}

size_t PubSubClient::write(const uint8_t* buffer, size_t size) {
    const size_t rc = appendBuffer(buffer, size);
    if (rc != 0) {
        lastOutActivity = millis();
    }
    return rc;
}

/**
 * @brief  Send the header and the prepared data to the client / MQTT broker.
 *
 * @param  header Header byte, e.g. MQTTCONNECT, MQTTPUBLISH, MQTTSUBSCRIBE, MQTTUNSUBSCRIBE.
 * @param  buf Buffer of data to write.
 * @param  length Length of buf to write.
 * @return True if successfully sent, otherwise false if buildHeader() failed or buf could not be written.
 */
bool PubSubClient::write(uint8_t header, uint8_t* buf, size_t length) {
    bool result = true;
    size_t rc;
    uint8_t hdrLen = buildHeader(header, buf, length);
    if (hdrLen == 0) return false;  // exit here in case of header generation failure

#ifdef MQTT_MAX_TRANSFER_SIZE
    uint8_t* writeBuf = buf + (MQTT_MAX_HEADER_SIZE - hdrLen);
    size_t bytesRemaining = length + hdrLen;  // Match the length type
    size_t bytesToWrite;
    while ((bytesRemaining > 0) && result) {
        yield();
        bytesToWrite = (bytesRemaining > MQTT_MAX_TRANSFER_SIZE) ? MQTT_MAX_TRANSFER_SIZE : bytesRemaining;
        rc = _client->write(writeBuf, bytesToWrite);
        result = (rc == bytesToWrite);
        bytesRemaining -= rc;
        writeBuf += rc;
        if (result) {
            lastOutActivity = millis();
        }
    }
#else
    rc = _client->write(buf + (MQTT_MAX_HEADER_SIZE - hdrLen), length + hdrLen);
    result = (rc == length + hdrLen);
    if (result) {
        lastOutActivity = millis();
    }
#endif
    return result;
}

/**
 * @brief  Write an UTF-8 encoded string to the give buffer and position. The string can have a length of 0 to 65535 bytes. The buffer is prefixed with two
 * bytes representing the length of the string. See section 1.5.3 of MQTT v3.1.1 protocol specification.
 * @note   If the string does not fit in the buffer (bufferSize) or is longer than 65535 bytes nothing is written to the buffer and the returned position
 * is unchanged.
 *
 * @param  string 'C' string of the data that shall be written in the buffer.
 * @param  buf Buffer to write the string into.
 * @param  pos Position in the buffer to write the string.
 * @return New position in the buffer (pos + 2 + string length), or pos if a buffer overrun would occur.
 */
size_t PubSubClient::writeString(const char* string, uint8_t* buf, size_t pos) {
    return writeString(string, buf, pos, this->bufferSize);
}

/**
 * @brief  Write an UTF-8 encoded string to the give buffer and position. The string can have a length of 0 to 65535 bytes. The buffer is prefixed with two
 * bytes representing the length of the string. See section 1.5.3 of MQTT v3.1.1 protocol specification.
 * @note   If the string does not fit in the buffer or is longer than 65535 bytes nothing is written to the buffer and the returned position is unchanged.
 *
 * @param  string 'C' string of the data that shall be written in the buffer.
 * @param  buf Buffer to write the string into.
 * @param  pos Position in the buffer to write the string.
 * @param  size Maximal size of the buffer.
 * @return New position in the buffer (pos + 2 + string length), or pos if a buffer overrun would occur or the string is a nullptr.
 */
size_t PubSubClient::writeString(const char* string, uint8_t* buf, size_t pos, size_t size) {
    if (!string) return pos;

    size_t sLen = strlen(string);
    if (pos + 2 + sLen <= size && sLen <= 0xFFFF) {
        buf[pos++] = (uint8_t)(sLen >> 8);
        buf[pos++] = (uint8_t)(sLen & 0xFF);
        memcpy(buf + pos, string, sLen);
        pos += sLen;
    } else {
        ERROR_PSC_PRINTF_P("writeString(): string (%zu) does not fit into buf (%zu)\n", pos + 2 + sLen, size);
    }
    return pos;
}

size_t PubSubClient::appendBuffer(uint8_t data) {
    buffer[_bufferWritePos] = data;
    ++_bufferWritePos;
    if (_bufferWritePos >= bufferSize) {
        if (flushBuffer() == 0) return 0;
    }
    return 1;
}

size_t PubSubClient::appendBuffer(const uint8_t *data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        if (appendBuffer(data[i]) == 0) return i;
    }
    return size;
}

size_t PubSubClient::flushBuffer() {
    size_t rc = 0;
    if (_bufferWritePos > 0) {
        if (connected()) {
            rc = _client->write(buffer, _bufferWritePos);
            if (rc != 0) {
                lastOutActivity = millis();
            }
        }
        _bufferWritePos = 0;
    }
    return rc;
}

bool PubSubClient::subscribe(const char* topic) {
    return subscribe(topic, 0);
}

bool PubSubClient::subscribe(const char* topic, uint8_t qos) {
    if (!topic) return false;
    if (qos > 1) return false;  // only QoS 0 and 1 supported

    size_t topicLen = strnlen(topic, this->bufferSize);
    if (this->bufferSize < MQTT_MAX_HEADER_SIZE + 2 + 2 + topicLen) {
        // Too long: header + nextMsgId (2) + topic length bytes (2) + topicLen
        return false;
    }
    if (connected()) {
        // Leave room in the buffer for header and variable length field
        uint16_t length = MQTT_MAX_HEADER_SIZE;
        nextMsgId++;
        if (nextMsgId == 0) {
            nextMsgId = 1;
        }
        this->buffer[length++] = (nextMsgId >> 8);
        this->buffer[length++] = (nextMsgId & 0xFF);
        length = writeString(topic, this->buffer, length);
        this->buffer[length++] = qos;
        return write(MQTTSUBSCRIBE | MQTTQOS1, this->buffer, length - MQTT_MAX_HEADER_SIZE);
    }
    return false;
}

bool PubSubClient::unsubscribe(const char* topic) {
    if (!topic) return false;

    size_t topicLen = strnlen(topic, this->bufferSize);
    if (this->bufferSize < MQTT_MAX_HEADER_SIZE + 2 + 2 + topicLen) {
        // Too long: header + nextMsgId (2) + topic length bytes (2) + topicLen
        return false;
    }
    if (connected()) {
        uint16_t length = MQTT_MAX_HEADER_SIZE;
        nextMsgId++;
        if (nextMsgId == 0) {
            nextMsgId = 1;
        }
        this->buffer[length++] = (nextMsgId >> 8);
        this->buffer[length++] = (nextMsgId & 0xFF);
        length = writeString(topic, this->buffer, length);
        return write(MQTTUNSUBSCRIBE | MQTTQOS1, this->buffer, length - MQTT_MAX_HEADER_SIZE);
    }
    return false;
}

PubSubClient& PubSubClient::setServer(uint8_t* ip, uint16_t port) {
    IPAddress addr(ip[0], ip[1], ip[2], ip[3]);
    return setServer(addr, port);
}

PubSubClient& PubSubClient::setServer(IPAddress ip, uint16_t port) {
    this->ip = ip;
    this->port = port;
    free(this->domain);
    this->domain = nullptr;
    return *this;
}

PubSubClient& PubSubClient::setServer(const char* domain, uint16_t port) {
    char* newDomain = nullptr;
    if (domain) {
        newDomain = (char*)realloc(this->domain, strlen(domain) + 1);
    }
    if (newDomain) {
        strcpy(newDomain, domain);
        this->domain = newDomain;
        this->port = port;
    } else {
        free(this->domain);
        this->domain = nullptr;
        this->port = 0;
    }
    return *this;
}

PubSubClient& PubSubClient::setCallback(MQTT_CALLBACK_SIGNATURE) {
    this->callback = callback;
    return *this;
}

PubSubClient& PubSubClient::setClient(Client& client) {
    this->_client = &client;
    return *this;
}

PubSubClient& PubSubClient::setStream(Stream& stream) {
    this->stream = &stream;
    return *this;
}

bool PubSubClient::setBufferSize(size_t size) {
    if (size == 0) {
        // Cannot set it back to 0
        return false;
    }
    if (this->bufferSize == 0) {
        this->buffer = (uint8_t*)malloc(size);
    } else {
        uint8_t* newBuffer = (uint8_t*)realloc(this->buffer, size);
        if (newBuffer) {
            this->buffer = newBuffer;
        } else {
            return false;
        }
    }
    this->bufferSize = size;
    return (this->buffer != nullptr);
}

size_t PubSubClient::getBufferSize() {
    return this->bufferSize;
}

PubSubClient& PubSubClient::setKeepAlive(uint16_t keepAlive) {
    this->keepAliveMillis = keepAlive * 1000UL;
    return *this;
}

PubSubClient& PubSubClient::setSocketTimeout(uint16_t timeout) {
    this->socketTimeoutMillis = timeout * 1000UL;
    return *this;
}

int PubSubClient::state() {
    return this->_state;
}
