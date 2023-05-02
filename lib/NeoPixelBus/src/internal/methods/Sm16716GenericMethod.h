/*-------------------------------------------------------------------------
NeoPixel library helper functions for SM16716 using general Pins

Written by Michael C. Miller.
Contributed by Ivo H (ivoh95)

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


template<typename T_TWOWIRE> class Sm16716MethodBase
{
public:
    typedef typename T_TWOWIRE::SettingsObject SettingsObject;

    Sm16716MethodBase(uint8_t pinClock, uint8_t pinData, uint16_t pixelCount, size_t elementSize, size_t settingsSize) :
        _sizeData(pixelCount* elementSize + settingsSize),
        _sizeFrame(6), // 48 bits
        _wire(pinClock, pinData)
    {
        _data = static_cast<uint8_t*>(malloc(_sizeData));
        memset(_data, 0, _sizeData);
    }

#if !defined(__AVR_ATtiny85__) && !defined(ARDUINO_attiny)
    Sm16716MethodBase(uint16_t pixelCount, size_t elementSize, size_t settingsSize) :
        Sm16716MethodBase(SCK, MOSI, pixelCount, elementSize, settingsSize)
    {
    }
#endif

    ~Sm16716MethodBase()
    {
        free(_data);
    }

    bool IsReadyToUpdate() const
    {
        return true; // dot stars don't have a required delay
    }

#if defined(ARDUINO_ARCH_ESP32)
    // can't support hardware SPI due to weird extra bits
    //void Initialize(int8_t sck, int8_t miso, int8_t mosi, int8_t ss)
    //{
    //    _wire.begin(sck, miso, mosi, ss);
    //}
#endif

    void Initialize()
    {
        _wire.begin();
    }

    void Update(bool)
    {
        _wire.beginTransaction();

        // start frame
        for (size_t frameBytes = 0; frameBytes < _sizeFrame; frameBytes++)
        {
            _wire.transmitByte(0x00);
        }
        _wire.transmitBit(LOW);
        _wire.transmitBit(LOW); // two extra 0s to make the 50 0 header
        _wire.transmitBit(HIGH); // one to start the led frame

        for (size_t pixel = 0; pixel < (_sizeData / 3); pixel++)
        {
            _wire.transmitByte(_data[pixel]);
            _wire.transmitByte(_data[pixel + 1]);
            _wire.transmitByte(_data[pixel + 2]);
            _wire.transmitBit(HIGH); //show the color and start the next frame
        }

        _wire.endTransaction();
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
    const size_t   _sizeData;   // Size of '_data' buffer below
    const size_t   _sizeFrame;

    T_TWOWIRE _wire;
    uint8_t* _data;       // Holds LED color values
};

// can ONLY support our bitbang for wire due to requirement for custom transmitBit method
// to handle not byte oriented data stream
//
typedef Sm16716MethodBase<TwoWireBitBangImple> Sm16716Method;
