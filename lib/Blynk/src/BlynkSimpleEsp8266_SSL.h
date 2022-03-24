/**
 * @file       BlynkSimpleEsp8266_SSL.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2016
 * @brief
 *
 */

#ifndef BlynkSimpleEsp8266_SSL_h
#define BlynkSimpleEsp8266_SSL_h

#ifndef ESP8266
#error This code is intended to run on the ESP8266 platform! Please check your Tools->Board setting.
#endif

#include <version.h>

#if ESP_SDK_VERSION_NUMBER < 0x020200
#error Please update your ESP8266 Arduino Core
#endif

static const char BLYNK_DEFAULT_ROOT_CA[] PROGMEM =
#include <certs/letsencrypt_pem.h>

#include <BlynkApiArduino.h>
#include <Blynk/BlynkProtocol.h>
#include <Adapters/BlynkArduinoClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#ifndef wificlientbearssl_h
#error BearSSL is needed, please update your ESP8266 Arduino Core
#endif
#ifdef USING_AXTLS
#error BearSSL is needed, but USING_AXTLS is defined
#endif

#ifndef BLYNK_SSL_RX_BUF_SIZE
#define BLYNK_SSL_RX_BUF_SIZE 2048
#endif

#ifndef BLYNK_SSL_TX_BUF_SIZE
#define BLYNK_SSL_TX_BUF_SIZE 512
#endif


static X509List BlynkCert(BLYNK_DEFAULT_ROOT_CA);

template <typename Client>
class BlynkArduinoClientSecure
    : public BlynkArduinoClientGen<Client>
{
public:
    BlynkArduinoClientSecure(Client& client)
        : BlynkArduinoClientGen<Client>(client)
    {
        this->client->setBufferSizes(BLYNK_SSL_RX_BUF_SIZE, BLYNK_SSL_TX_BUF_SIZE);
    }

    void setFingerprint(const char* fp) {
        this->client->setFingerprint(fp);
    }

    void setTrustAnchors(X509List* certs) {
        this->client->setTrustAnchors(certs);
    }

    bool connect() {
        if (!BlynkArduinoClientGen<Client>::connect()) {
            int err = this->client->getLastSSLError();
            if (err == 0) {
                BLYNK_LOG1("Connection failed");
            } else {
                BLYNK_LOG2(BLYNK_F("SSL error: "), err);
            }
            return false;
        }
        return true;
    }

};

template <typename Transport>
class BlynkWifi
    : public BlynkProtocol<Transport>
{
    typedef BlynkProtocol<Transport> Base;
public:
    BlynkWifi(Transport& transp)
        : Base(transp)
    {}


    void config(const char* auth,
                void(*handleInterruptCallback)(void),
                const char* domain = BLYNK_DEFAULT_DOMAIN,
                uint16_t    port   = BLYNK_DEFAULT_PORT_SSL,
                const char* fingerprint = NULL)
    {
        Base::begin(auth,handleInterruptCallback);
        this->conn.begin(domain, port);

        if (fingerprint) {
          this->conn.setFingerprint(fingerprint);
        } else {
          this->conn.setTrustAnchors(&BlynkCert);
        }
    }

    void config(const char* auth,
                void(*handleInterruptCallback)(void),
                IPAddress   ip,
                uint16_t    port = BLYNK_DEFAULT_PORT_SSL,
                const char* fingerprint = NULL)
    {
        Base::begin(auth,handleInterruptCallback);
        this->conn.begin(ip, port);

        if (fingerprint) {
          this->conn.setFingerprint(fingerprint);
        } else {
          this->conn.setTrustAnchors(&BlynkCert);
        }
    }

};

static WiFiClientSecure _blynkWifiClient;
static BlynkArduinoClientSecure<WiFiClientSecure> _blynkTransport(_blynkWifiClient);
BlynkWifi<BlynkArduinoClientSecure<WiFiClientSecure> > Blynk(_blynkTransport);

//#include <BlynkWidgets.h>

#endif
