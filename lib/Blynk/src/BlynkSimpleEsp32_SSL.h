/**
 * @file       BlynkSimpleEsp32_SSL.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Dec 2017
 * @brief
 *
 */

#ifndef BlynkSimpleEsp32_SSL_h
#define BlynkSimpleEsp32_SSL_h

#ifndef ESP32
#error This code is intended to run on the ESP32 platform! Please check your Tools->Board setting.
#endif

static const char BLYNK_DEFAULT_ROOT_CA[] =
#include <certs/letsencrypt_pem.h>

#include <BlynkApiArduino.h>
#include <Blynk/BlynkProtocol.h>
#include <Adapters/BlynkArduinoClient.h>
#include <WiFiClientSecure.h>

template <typename Client>
class BlynkArduinoClientSecure
    : public BlynkArduinoClientGen<Client>
{
public:
    BlynkArduinoClientSecure(Client& client)
        : BlynkArduinoClientGen<Client>(client)
        , caCert(NULL)
    {}

    void setRootCA(const char* fp) { caCert = fp; }

    bool connect() {
        this->client->setCACert(caCert);
        if (BlynkArduinoClientGen<Client>::connect()) {
          BLYNK_LOG1(BLYNK_F("Certificate OK"));
          return true;
        } else {
          BLYNK_LOG1(BLYNK_F("Secure connection failed"));
        }
        return false;
    }

private:
    const char* caCert;
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
                const char* root_ca = BLYNK_DEFAULT_ROOT_CA)
    {
        Base::begin(auth,handleInterruptCallback);
        this->conn.begin(domain, port);
        this->conn.setRootCA(root_ca);
    }

    void config(const char* auth,
                void(*handleInterruptCallback)(void),
                IPAddress   ip,
                uint16_t    port = BLYNK_DEFAULT_PORT_SSL,
                const char* root_ca = BLYNK_DEFAULT_ROOT_CA)
    {
        Base::begin(auth,handleInterruptCallback);
        this->conn.begin(ip, port);
        this->conn.setRootCA(root_ca);
    }

};

static WiFiClientSecure _blynkWifiClient;
static BlynkArduinoClientSecure<WiFiClientSecure> _blynkTransport(_blynkWifiClient);
BlynkWifi<BlynkArduinoClientSecure<WiFiClientSecure> > Blynk(_blynkTransport);

// #include <BlynkWidgets.h>

#endif
