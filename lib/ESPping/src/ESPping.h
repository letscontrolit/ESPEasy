/*
  ESP32Ping - Ping library for ESP32 or ESP8266
  Copyright (c) 2018 Marian Craciunescu. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef ESPping_H
#define ESPping_H

#include <stdint.h>
#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <ping32.h>
extern "C" {
  void esp_schedule(void);
  void esp_yield(void);
}
#endif

#ifdef ESP8266
#include <ESP8266WiFi.h>
extern "C" {
  #include <ping.h> 
}
#endif

//#define ENABLE_DEBUG_PING 0
#ifdef ENABLE_DEBUG_PING
  #define DEBUG_PING(...) Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_PING(...)
#endif

class PingClass {
  public:
    PingClass();

    bool ping(IPAddress dest,   int16_t count = 5);
    bool ping(const char* host, int16_t count = 5);

    float averageTime();
    float minTime();
    float maxTime();

  protected:
    static void _ping_sent_cb(void *opt, void *pdata);
    static void _ping_recv_cb(void *opt, void *pdata);

    IPAddress _dest;
    ping_option _options;

    static byte _expected_count, _errors, _success;
    static float _min_time, _max_time;
    static float _avg_time;
};

extern PingClass Ping;

#endif

