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

#include "AM2320.h"

#include <Wire.h>

AM2320::AM2320() {
    // do nothing
}

void  AM2320::begin() {
    Wire.begin();
}

#ifdef ESP8266
void AM2320::begin(int sda, int scl) {
    Wire.begin(sda, scl);
}
#endif

float AM2320::getTemperature() {
    return _temperature;
}

float AM2320::getHumidity() {
    return _humidity;
}

bool AM2320::measure() {
    _errorCode = 0;

    if ( ! _read_registers(0x00, 4)) {
        _errorCode = 1;
        return false;
    }

    unsigned int receivedCrc = 0;       // allocate 16 bits for storing crc from sensor
    receivedCrc = ((receivedCrc | _buf[7]) << 8 | _buf[6]);   // pack high and low byte together

    if (receivedCrc == crc16(_buf, 6)) {
        int humudity = ((_buf[2] << 8) | _buf[3]);
        _humidity =  humudity / 10.0;

        int temperature = ((_buf[4] & 0x7F) << 8) | _buf[5];
        if ((_buf[2] & 0x80) >> 8 == 1) {       // negative temperature
            _temperature = (temperature / 10.0) * -1;    // devide data by 10 according to the datasheet
        }
        else {                                  // positive temperature
            _temperature = temperature / 10.0;           // devide data by 10 according to the datasheet
        }

        return true;
    }
    else {
        _errorCode = 2;
        return false;
    }

}

int AM2320::getErrorCode() {
    return _errorCode;
}

bool AM2320::_read_registers(int startAddress, int numByte) {
    _wake();                // wake up sensor

    Wire.beginTransmission(AM2320_ADDR);
    Wire.write(0x03);           // function code: 0x03 - read register data
    Wire.write(startAddress);   // begin address
    Wire.write(numByte);        // number of bytes to read

    // send and check result if not success, return error code
    if (Wire.endTransmission(true) != 0) {        
        return false;                           // return sensor not ready code
    }
    delayMicroseconds(1500);                    // as specified in datasheet
    Wire.requestFrom(AM2320_ADDR, numByte + 4); // request bytes from sensor
                                                // see function code description in datasheet    
    for ( int i = 0; i < numByte + 4; i++) {    // read
        _buf[i] = Wire.read();
    }

    return true;
}

void AM2320::_wake() {
    Wire.beginTransmission(AM2320_ADDR);
    Wire.endTransmission();
}

unsigned int crc16(byte *byte, unsigned int numByte) {
    unsigned int crc = 0xFFFF;          // 16-bit crc register

    while (numByte > 0) {               // loop until process all bytes
        crc ^= *byte;                   // exclusive-or crc with first byte

        for (int i = 0; i < 8; i++) {       // perform 8 shifts
            unsigned int lsb = crc & 0x01;  // extract LSB from crc
            crc >>= 1;                      // shift be one position to the right

            if (lsb == 0) {                 // LSB is 0
                continue;                   // repete the process
            }
            else {                          // LSB is 1
                crc ^= 0xA001;              // exclusive-or with 1010 0000 0000 0001
            }
        }

        numByte--;          // decrement number of byte left to be processed
        byte++;             // move to next byte
    }

    return crc;
}