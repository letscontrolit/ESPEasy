/*
    multi_channel_relay.h
    Seeed multi channel relay Arduino library

    Copyright (c) 2018 Seeed Technology Co., Ltd.
    Author        :   lambor
    Create Time   :   June 2018
    Change Log    :

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

#include <multi_channel_relay.h>

Multi_Channel_Relay::Multi_Channel_Relay() {

}

void Multi_Channel_Relay::begin(int address) {
    // Wire.begin(); Handled by ESPEasy already
    channel_state = 0;
    _i2cAddr = address;

}

uint8_t Multi_Channel_Relay::getFirmwareVersion(void) {
    Wire.beginTransmission(_i2cAddr);
    Wire.write(CMD_READ_FIRMWARE_VER);
    Wire.endTransmission();

    Wire.requestFrom(_i2cAddr, 1);
    //while(!Wire.available());
    return Wire.read();
}

void Multi_Channel_Relay::changeI2CAddress(uint8_t old_addr, uint8_t new_addr) {
    Wire.beginTransmission(old_addr);
    Wire.write(CMD_SAVE_I2C_ADDR);
    Wire.write(new_addr);
    Wire.endTransmission();

    _i2cAddr = new_addr;
}

uint8_t Multi_Channel_Relay::getChannelState(void) {
    return channel_state;
}

void Multi_Channel_Relay::channelCtrl(uint8_t state) {
    channel_state = state;

    Wire.beginTransmission(_i2cAddr);
    Wire.write(CMD_CHANNEL_CTRL);
    Wire.write(channel_state);
    Wire.endTransmission();
}

void Multi_Channel_Relay::turn_on_channel(uint8_t channel) {
    channel_state |= (1 << (channel - 1));

    Wire.beginTransmission(_i2cAddr);
    Wire.write(CMD_CHANNEL_CTRL);
    Wire.write(channel_state);
    Wire.endTransmission();
}

void Multi_Channel_Relay::turn_off_channel(uint8_t channel) {
    channel_state &= ~(1 << (channel - 1));

    Wire.beginTransmission(_i2cAddr);
    Wire.write(CMD_CHANNEL_CTRL);
    Wire.write(channel_state);
    Wire.endTransmission();
}

// uint8_t Multi_Channel_Relay::scanI2CDevice(void) {
    // byte error = 0, address = 0, result = 0;
    // int nDevices;

    // DEBUG_PRINT.println("Scanning...");

    // nDevices = 0;
    // for (address = 1; address <= 127; address++) {
    //     // The i2c_scanner uses the return value of
    //     // the Write.endTransmisstion to see if
    //     // a device did acknowledge to the address.
    //     Wire.beginTransmission(address);
    //     error = Wire.endTransmission();

    //     if (error == 0) {
    //         result = address;
    //         DEBUG_PRINT.print("I2C device found at address 0x");
    //         if (address < 16) {
    //             DEBUG_PRINT.print("0");
    //         }
    //         DEBUG_PRINT.print(address, HEX);
    //         DEBUG_PRINT.println("  !");

    //         nDevices++;
    //     } else if (error == 4) {
    //         DEBUG_PRINT.print("Unknown error at address 0x");
    //         if (address < 16) {
    //             DEBUG_PRINT.print("0");
    //         }
    //         DEBUG_PRINT.println(address, HEX);
    //     }
    // }
    // if (nDevices == 0) {
    //     DEBUG_PRINT.println("No I2C devices found\n");
    //     result = 0x00;
    // } else {
    //     DEBUG_PRINT.print("Found ");
    //     DEBUG_PRINT.print(nDevices);
    //     DEBUG_PRINT.print(" devices\n");
    //     if (nDevices != 1) {
    //         result = 0x00;
    //     }
    // }

    // return result;
//     return 0;
// }
