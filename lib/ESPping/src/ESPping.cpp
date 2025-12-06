/*
  ESP8266Ping - Ping library for ESP8266
  Copyright (c) 2015 Daniele Colanardi. All rights reserved.

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
#include "ESPping.h"
#include <stdint.h>

#ifdef ESP8266
extern "C" void esp_schedule();
extern "C" void esp_yield();
#endif
#ifdef ESP32
extern "C" void esp_schedule(void) {};
extern "C" void esp_yield(void) {};
#endif

PingClass::PingClass() {}

bool PingClass::ping(IPAddress dest, int16_t count) {
    _expected_count = count;
    _errors = 0;
    _success = 0;

    _min_time = INT_MAX;
    _avg_time = 0.0f;
    _max_time = 0;
    

    memset(&_options, 0, sizeof(struct ping_option));

    // Repeat count (how many time send a ping message to destination)
    _options.count = count;
    // Time interval between two ping (seconds??)
    _options.coarse_time = 1;
    // Destination machine
    _options.ip = dest;

    // Callbacks
    _options.recv_function = reinterpret_cast<ping_recv_function>(&PingClass::_ping_recv_cb);
    _options.sent_function = NULL; //reinterpret_cast<ping_sent_function>(&_ping_sent_cb);

    // Let's go!
    if(ping_start(&_options)) {
        #ifdef ESP8266
        //wait until finished
        while((_success + _errors) < _expected_count){
            delay(1);
            esp_yield();
        };
        #else
        // Suspend till the process end
        esp_yield();
        #endif
    }
    return (_success > 0);
}

bool PingClass::ping(const char* host, int16_t count) {
    IPAddress remote_addr;

    if (WiFi.hostByName(host, remote_addr))
        return ping(remote_addr, count);

    return false;
}

float PingClass::averageTime() {
    return _avg_time;
}

float PingClass::minTime() {
    return _min_time;
}

float PingClass::maxTime() {
    return _max_time;
}

void PingClass::_ping_recv_cb(void *opt, void *resp) {
    // Cast the parameters to get some usable info
    ping_resp*   ping_resp = reinterpret_cast<struct ping_resp*>(resp);
    //ping_option* ping_opt  = reinterpret_cast<struct ping_option*>(opt);

    // Error or success?
    #ifdef ESP8266
    if (ping_resp->ping_err == -1)
        _errors++;
    else {
        _success++;
        _avg_time += ping_resp->resp_time;
        if(ping_resp->resp_time < _min_time)
          _min_time = ping_resp->resp_time;
        if(ping_resp->resp_time > _max_time)
          _max_time = ping_resp->resp_time;
        
    }
    #else
    _errors = ping_resp->ping_err;
    _success = _expected_count - _errors;
    _avg_time = ping_resp->resp_time;
    #endif

    // Some debug info
    DEBUG_PING(
            "DEBUG: ping reply"
            "\ttotal_count = %d"
            "\tresp_time = %d"
            "\tseqno = %d"
            "\ttimeout_count = %d"
            "\tbytes = %d"
            "\ttotal_bytes = %d"
            "\ttotal_time = %d"
            "\tping_err = %d \n",
            (int)ping_resp->total_count, (int)ping_resp->resp_time, (int)ping_resp->seqno,
            (int)ping_resp->timeout_count, (int)ping_resp->bytes, (int)ping_resp->total_bytes,
            (int)ping_resp->total_time, (int)ping_resp->ping_err
    );

    // Is it time to end?
    // Don't using seqno because it does not increase on error
    #ifdef ESP8266
    if (_success + _errors == _expected_count ) {
        _avg_time = _success > 0 ? _avg_time / _success : 0;
    #else
    if (ping_resp->total_count== _expected_count) {

    #endif
    
        DEBUG_PING("success %d errors %d\n", _success, _errors);
        DEBUG_PING("Resp times min %d, avg %.2f, max %d ms\n", _min_time, _avg_time, _max_time);

        #ifdef ESP32
        // Done, return to main functiom
        esp_schedule();
        #endif
    }
}

byte PingClass::_expected_count = 0;
byte PingClass::_errors = 0;
byte PingClass::_success = 0;
float PingClass::_avg_time = 0;
float PingClass::_min_time = std::numeric_limits<float>::max();
float PingClass::_max_time = 0;

PingClass Ping;

