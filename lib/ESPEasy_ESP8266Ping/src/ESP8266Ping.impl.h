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

extern "C" void esp_schedule();
extern "C" void esp_yield();

PingClass::PingClass() {}

bool PingClass::ping(IPAddress dest, byte count) {
    _expected_count = count;
    _errors = 0;
    _success = 0;

    _avg_time = 0;

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
        // Suspend till the process end
        esp_yield();
    }

    return (_success > 0);
}

bool PingClass::ping(const char* host, byte count) {
    IPAddress remote_addr;

    if (WiFi.hostByName(host, remote_addr))
        return ping(remote_addr, count);

    return false;
}

int PingClass::averageTime() {
    return _avg_time;
}

void PingClass::_ping_recv_cb(void *opt, void *resp) {
    // Cast the parameters to get some usable info
    ping_resp*   ping_resp = reinterpret_cast<struct ping_resp*>(resp);
    //ping_option* ping_opt  = reinterpret_cast<struct ping_option*>(opt);

    // Error or success?
    if (ping_resp->ping_err == -1)
        _errors++;
    else {
        _success++;
        _avg_time += ping_resp->resp_time;
    }

    // Some debug info
    DEBUG_PING(
            "DEBUG: ping reply\n"
            "\ttotal_count = %d \n"
            "\tresp_time = %d \n"
            "\tseqno = %d \n"
            "\ttimeout_count = %d \n"
            "\tbytes = %d \n"
            "\ttotal_bytes = %d \n"
            "\ttotal_time = %d \n"
            "\tping_err = %d \n",
            ping_resp->total_count, ping_resp->resp_time, ping_resp->seqno,
            ping_resp->timeout_count, ping_resp->bytes, ping_resp->total_bytes,
            ping_resp->total_time, ping_resp->ping_err
    );

    // Is it time to end?
    // Don't using seqno because it does not increase on error
    if (_success + _errors == _expected_count) {
        _avg_time = _avg_time / _expected_count;

        DEBUG_PING("Avg resp time %d ms\n", _avg_time);

        // Done, return to main functiom
        esp_schedule();
    }
}

byte PingClass::_expected_count = 0;
byte PingClass::_errors = 0;
byte PingClass::_success = 0;
int  PingClass::_avg_time = 0;
