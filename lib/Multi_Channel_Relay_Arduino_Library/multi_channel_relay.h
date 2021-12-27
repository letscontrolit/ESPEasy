/*
    multi_channel_relay.cpp
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

#pragma once

#ifndef MULTI_CHANNEL_RELAY_H
#define MULTI_CHANNEL_RELAY_H

// Architecture specific include
// #if defined(ARDUINO_ARCH_AVR)
//     #define DEBUG_PRINT Serial
// #elif defined(ARDUINO_ARCH_SAM)
//     #define DEBUG_PRINT SerialUSB
// #elif defined(ARDUINO_ARCH_SAMD)
//     #define DEBUG_PRINT SerialUSB
// #elif defined(ARDUINO_ARCH_STM32F4)
//     #define DEBUG_PRINT SerialUSB
// #else
//     #pragma message("Not match any architecture.")
//     #define DEBUG_PRINT Serial
// #endif

#include <Arduino.h>
#include <Wire.h>


#define CHANNLE1_BIT  0x01
#define CHANNLE2_BIT  0x02
#define CHANNLE3_BIT  0x04
#define CHANNLE4_BIT  0x08
#define CHANNLE5_BIT  0x10
#define CHANNLE6_BIT  0x20
#define CHANNLE7_BIT  0x40
#define CHANNLE8_BIT  0x80

#define CMD_CHANNEL_CTRL					0x10
#define CMD_SAVE_I2C_ADDR					0x11
#define CMD_READ_I2C_ADDR					0x12
#define CMD_READ_FIRMWARE_VER			0x13

class Multi_Channel_Relay {
  public:
    Multi_Channel_Relay();

    /**
        Begin Multi Channel Relay by a I2C address.
        @param address can be changed by changeI2CAddress. Please refer to example change_i2c_address
    */
    void begin(int address = 0x11);

    /**
        @brief Change device address from old_addr to new_addr.
        @param new_addr, the address to use.
    					old_addr, the original address
        @return None
    */
    void changeI2CAddress(uint8_t new_addr, uint8_t old_addr);

    /**
        /brief Get channel state
        /return One byte value to indicate channel state
      				 the bits range from 0 to 7 represents channel 1 to 8
    */
    uint8_t getChannelState(void);

    /**
        @brief Read firmware version from on board MCU
        @param
        @return Firmware version in byte
    */
    uint8_t getFirmwareVersion(void);

    /**
        @brief Control relay channels
        @param state, use one Byte to represent 8 channel
        @return None
    */
    void channelCtrl(uint8_t state);

    /**
        @brief Turn on one of 8 channels
        @param channel, channel to control with (range form 1 to 8)
        @return None
    */
    void turn_on_channel(uint8_t channel);

    /**
        @brief Turn off on of 8 channels
        @param channel, channel to control with (range form 1 to 8)
        @return None
    */
    void turn_off_channel(uint8_t channel);

    // /**
    //     @brief Scan I2C device, return the address if there is only one i2c device
    //     @param
    //     @return device address
    // */
    // uint8_t scanI2CDevice(void);

  private:
    int _i2cAddr;  //  This is the I2C address you want to use
    int channel_state;  // Value to save channel state
};



#endif