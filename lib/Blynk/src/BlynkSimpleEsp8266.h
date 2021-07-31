/**
 * @file       BlynkSimpleEsp8266.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief
 *
 */

#ifndef BlynkSimpleEsp8266_h
#define BlynkSimpleEsp8266_h

#ifndef ESP8266
#error This code is intended to run on the ESP8266 platform! Please check your Tools->Board setting.
#endif

#include <version.h>

#if ESP_SDK_VERSION_NUMBER < 0x020200
#error Please update your ESP8266 Arduino Core
#endif

#include <BlynkApiArduino.h>
#include <Blynk/BlynkProtocol.h>
#include <Adapters/BlynkArduinoClient.h>
#include <ESP8266WiFi.h>

class BlynkWifi
    : public BlynkProtocol<BlynkArduinoClient>
{
    typedef BlynkProtocol<BlynkArduinoClient> Base;
public:
    BlynkWifi(BlynkArduinoClient& transp)
        : Base(transp)
    {}

    // void connectWiFi(const char* ssid, const char* pass)
    // {
    //     BLYNK_LOG2(BLYNK_F("Connecting to "), ssid);
    //     WiFi.mode(WIFI_STA);
    //     if (WiFi.status() != WL_CONNECTED) {
    //         if (pass && strlen(pass)) {
    //             WiFi.begin(ssid, pass);
    //         } else {
    //             WiFi.begin(ssid);
    //         }
    //     }
    //     while (WiFi.status() != WL_CONNECTED) {
    //         BlynkDelay(500);
    //     }
    //     BLYNK_LOG1(BLYNK_F("Connected to WiFi"));

    //     IPAddress myip = WiFi.localIP();
    //     BLYNK_LOG_IP("IP: ", myip);
    // }

    void config(const char* auth,
                void(*handleInterruptCallback)(void),
                const char* domain = BLYNK_DEFAULT_DOMAIN,
                uint16_t    port   = BLYNK_DEFAULT_PORT)
    {
        Base::begin(auth,handleInterruptCallback);
        this->conn.begin(domain, port);
    }

    void config(const char* auth,
                void(*handleInterruptCallback)(void),
                IPAddress   ip,
                uint16_t    port = BLYNK_DEFAULT_PORT)
    {
        Base::begin(auth,handleInterruptCallback);
        this->conn.begin(ip, port);
    }

    // void begin(const char* auth,
    //            const char* ssid,
    //            const char* pass,
    //            const char* domain = BLYNK_DEFAULT_DOMAIN,
    //            uint16_t    port   = BLYNK_DEFAULT_PORT)
    // {
    //     connectWiFi(ssid, pass);
    //     config(auth, domain, port);
    //     while(this->connect() != true) {}
    // }

    // void begin(const char* auth,
    //            const char* ssid,
    //            const char* pass,
    //            IPAddress   ip,
    //            uint16_t    port   = BLYNK_DEFAULT_PORT)
    // {
    //     connectWiFi(ssid, pass);
    //     config(auth, ip, port);
    //     while(this->connect() != true) {}
    // }

};

static WiFiClient _blynkWifiClient;
static BlynkArduinoClient _blynkTransport(_blynkWifiClient);
BlynkWifi Blynk(_blynkTransport);

// #include <BlynkWidgets.h>

#endif
