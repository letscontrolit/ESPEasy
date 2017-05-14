/**
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2016 Ratthanan Nalintasnai
**/

#ifndef AM2303_H
#define AM2303_H

#include <Arduino.h>

// address of AM2320
#define AM2320_ADDR 0x5C

// maximum number of bytes that can be read consequtively before
// sensor splits out error
#define MAX_BYTES_READ 10

class AM2320 {
    public:
        AM2320();

        // initialize AM2320
        void begin();

        // initialize AM2320 for ESP8266 (ESP-01) which requires manual
        // specification of sda and scl pins
        #ifdef ESP8266
        void begin(int sda, int scl);
        #endif

        bool measure();

        float getTemperature();
        float getHumidity();

        int getErrorCode();
        
    private:
        byte _buf[ MAX_BYTES_READ ];
        float _temperature;
        float _humidity;
        int _errorCode;        
        void _wake();
        bool _read_registers(int startAddress, int numByte);

};

// compute CRC16
unsigned int crc16(byte *byte, unsigned int numByte);

#endif