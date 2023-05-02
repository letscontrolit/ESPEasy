/*-------------------------------------------------------------------------
NeoPixel library helper functions for Tlc5947 24 channel PWM controller using general Pins.

Written by Michael C. Miller.
Written by Dennis Kasprzyk.

I invest time and resources providing this open source code,
please support me by dontating (see https://github.com/Makuna/NeoPixelBus)

-------------------------------------------------------------------------
This file is part of the Makuna/NeoPixelBus library.

NeoPixelBus is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

NeoPixelBus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with NeoPixel.  If not, see
<http://www.gnu.org/licenses/>.
-------------------------------------------------------------------------*/

#pragma once

// must also check for arm due to Teensy incorrectly having ARDUINO_ARCH_AVR set
#if defined(ARDUINO_ARCH_AVR) && !defined(__arm__)
#include "TwoWireBitBangImpleAvr.h"
#else
#include "TwoWireBitBangImple.h"
#endif

#define TLC5947_MODULE_PWM_CHANNEL_COUNT 24

class Tlc5947Converter8Bit
{
public:
    static const size_t sizeChannel = 1;
    static void ConvertFrame(uint8_t* sendBufferPtr, uint8_t* channelPtr)
    {
        // Write 2 channels into 3 bytes scaling 8-bit to 12-bit per channel 
        for (int indexChannel = 0; indexChannel < TLC5947_MODULE_PWM_CHANNEL_COUNT; indexChannel += 2)
        {
            uint8_t ch1 = *channelPtr--;
            uint8_t ch2 = *channelPtr--;

            *sendBufferPtr++ = ch1;
            *sendBufferPtr++ = (ch1 & 0xf0) | (ch2 >> 4);
            *sendBufferPtr++ = ((ch2 << 4) & 0xf0) | (ch2 >> 4);
        }
    }
};

class Tlc5947Converter16Bit
{
public:
    static const size_t sizeChannel = 2;
    static void ConvertFrame(uint8_t* sendBufferPtr, uint8_t* sourceBufferPtr)
    {
        uint16_t* channelPtr = (uint16_t*)sourceBufferPtr;
        
        // Write 2 channels into 3 bytes using upper 12-bit of each channel 
        for (int indexChannel = 0; indexChannel < TLC5947_MODULE_PWM_CHANNEL_COUNT; indexChannel += 2)
        {
            uint8_t ch1 = *channelPtr--;
            uint8_t ch2 = *channelPtr--;

            *sendBufferPtr++ = ch1 >> 8;
            *sendBufferPtr++ = (ch1 & 0xf0) | (ch2 >> 12);
            *sendBufferPtr++ = ch2 >> 4;
        }
    }
};


template<typename T_BITCONVERT, typename T_TWOWIRE> class Tlc5947MethodBase
{
public:
    typedef typename T_TWOWIRE::SettingsObject SettingsObject;

    // 24 channel * 12 bit
    static const size_t sizeSendBuffer = 36;

    Tlc5947MethodBase(uint8_t pinClock, uint8_t pinData, uint8_t pinLatch, uint8_t pinOutputEnable, uint16_t pixelCount, size_t elementSize, size_t settingsSize) :
        _countModule((pixelCount * elementSize + TLC5947_MODULE_PWM_CHANNEL_COUNT - 1) / TLC5947_MODULE_PWM_CHANNEL_COUNT),
        _sizeData(_countModule * TLC5947_MODULE_PWM_CHANNEL_COUNT + settingsSize),
        _wire(pinClock, pinData),
        _pinLatch(pinLatch),
        _pinOutputEnable(pinOutputEnable)
    {
        _data = static_cast<uint8_t*>(malloc(_sizeData));
        pinMode(pinLatch, OUTPUT);
        pinMode(pinOutputEnable, OUTPUT);
        digitalWrite(pinOutputEnable, HIGH);
    }

    Tlc5947MethodBase(uint8_t pinClock, uint8_t pinData, uint8_t pinLatch, uint16_t pixelCount, size_t elementSize, size_t settingsSize) :
        Tlc5947MethodBase(pinClock, pinData, pinLatch, -1, pixelCount, elementSize, settingsSize)
    {
    }

#if !defined(__AVR_ATtiny85__) && !defined(ARDUINO_attiny)
    Tlc5947MethodBase(uint8_t pinLatch, uint8_t pinOutputEnable, uint16_t pixelCount, size_t elementSize, size_t settingsSize) :
        Tlc5947MethodBase(SCK, MOSI, pinLatch, pinOutputEnable, pixelCount, elementSize, settingsSize)
    {
    }

    Tlc5947MethodBase(uint8_t pinLatch, uint16_t pixelCount, size_t elementSize, size_t settingsSize) :
        Tlc5947MethodBase(SCK, MOSI, pinLatch, -1, pixelCount, elementSize, settingsSize)
    {
    }
#endif

    ~Tlc5947MethodBase()
    {
        free(_data);
        pinMode(_pinLatch, INPUT);
        pinMode(_pinOutputEnable, INPUT);
    }

    bool IsReadyToUpdate() const
    {
        return true; // dot stars don't have a required delay
    }

#if defined(ARDUINO_ARCH_ESP32)
    void Initialize(int8_t sck, int8_t miso, int8_t mosi, int8_t ss)
    {
        _wire.begin(sck, miso, mosi, ss);
    }
#endif

    void Initialize()
    {
        _wire.begin();
        memset(_data, 0, _sizeData);
    }

    void Update(bool)
    { 
        
        digitalWrite(_pinOutputEnable, HIGH);
        
        digitalWrite(_pinLatch, LOW);
        _wire.beginTransaction();

        // We need to write the channels in reverse order. Get a Pointer to the last channel.
        uint8_t* lastChannelPtr = _data + ((_countModule * TLC5947_MODULE_PWM_CHANNEL_COUNT - 1) * T_BITCONVERT::sizeChannel);
        for (uint16_t countSend = 0; countSend < _countModule; countSend++)
        {
            // We pass a pointer to the last channel and ConvertFrame reads the channels backwards
            T_BITCONVERT::ConvertFrame(_sendBuffer, lastChannelPtr);
            _wire.transmitBytes(_sendBuffer, sizeSendBuffer);
            lastChannelPtr -= TLC5947_MODULE_PWM_CHANNEL_COUNT * T_BITCONVERT::sizeChannel;
        }        
      
        _wire.endTransaction();
        digitalWrite(_pinLatch, HIGH);
        digitalWrite(_pinLatch, LOW);
        digitalWrite(_pinOutputEnable, LOW);
    }

    bool AlwaysUpdate()
    {
        // this method requires update to be called only if changes to buffer
        return false;
    }

    uint8_t* getData() const
    {
        return _data;
    };

    size_t getDataSize() const
    {
        return _sizeData;
    };

    void applySettings([[maybe_unused]] const SettingsObject& settings)
    {
        _wire.applySettings(settings);
    }

private:
    const uint16_t _countModule; // Number of tlc5947 modules
    const size_t   _sizeData;    // Size of '_data' buffer below

    T_TWOWIRE _wire;
    uint8_t*  _data;                        // Holds LED color values
    uint8_t   _sendBuffer[sizeSendBuffer];  // Holds channel values for one module
    uint8_t   _pinLatch;
    uint8_t   _pinOutputEnable;
};

typedef Tlc5947MethodBase<Tlc5947Converter8Bit, TwoWireBitBangImple> Tlc5947Method;
typedef Tlc5947MethodBase<Tlc5947Converter16Bit, TwoWireBitBangImple> Tlc5947Method16Bit;

#if !defined(__AVR_ATtiny85__) && !defined(ARDUINO_attiny)
#include "TwoWireSpiImple.h"

// for standalone
typedef Tlc5947MethodBase<Tlc5947Converter8Bit, TwoWireSpiImple<SpiSpeed30Mhz>> Tlc5947Spi30MhzMethod;
typedef Tlc5947MethodBase<Tlc5947Converter16Bit, TwoWireSpiImple<SpiSpeed30Mhz>> Tlc5947Spi30MhzMethod16Bit;

// for cascaded devices
typedef Tlc5947MethodBase<Tlc5947Converter8Bit, TwoWireSpiImple<SpiSpeed15Mhz>> Tlc5947Spi15MhzMethod;
typedef Tlc5947MethodBase<Tlc5947Converter16Bit, TwoWireSpiImple<SpiSpeed15Mhz>> Tlc5947Spi15MhzMethod16Bit;

typedef Tlc5947MethodBase<Tlc5947Converter8Bit, TwoWireSpiImple<SpiSpeed15Mhz>> Tlc5947SpiMethod;
typedef Tlc5947MethodBase<Tlc5947Converter16Bit, TwoWireSpiImple<SpiSpeed15Mhz>> Tlc5947SpiMethod16Bit;


#endif



