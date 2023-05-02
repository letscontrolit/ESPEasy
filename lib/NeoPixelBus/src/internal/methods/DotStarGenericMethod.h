/*-------------------------------------------------------------------------
NeoPixel library helper functions for DotStars using general Pins (APA102).

Written by Michael C. Miller.

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


template<typename T_TWOWIRE> class DotStarMethodBase
{
public:
    typedef typename T_TWOWIRE::SettingsObject SettingsObject;

    DotStarMethodBase(uint8_t pinClock, uint8_t pinData, uint16_t pixelCount, size_t elementSize, size_t settingsSize) :
        _sizeData(pixelCount * elementSize + settingsSize),
        _sizeEndFrame((pixelCount + 15) / 16), // 16 = div 2 (bit for every two pixels) div 8 (bits to bytes)
        _wire(pinClock, pinData)
    {
        _data = static_cast<uint8_t*>(malloc(_sizeData));
        // data cleared later in Begin()
    }

#if !defined(__AVR_ATtiny85__) && !defined(ARDUINO_attiny)
    DotStarMethodBase(uint16_t pixelCount, size_t elementSize, size_t settingsSize) :
        DotStarMethodBase(SCK, MOSI, pixelCount, elementSize, settingsSize)
    {
    }
#endif

    ~DotStarMethodBase()
    {
        free(_data);
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
    }

    void Update(bool)
    {
        const uint8_t startFrame[4] = { 0x00 };
        const uint8_t resetFrame[4] = { 0x00 };
        
        _wire.beginTransaction();

        // start frame
        _wire.transmitBytes(startFrame, sizeof(startFrame));
        
        // data
        _wire.transmitBytes(_data, _sizeData);

       // reset frame
        _wire.transmitBytes(resetFrame, sizeof(resetFrame));
        
        // end frame 
        
        // one bit for every two pixels with no less than 1 byte
        for (size_t endFrameByte = 0; endFrameByte < _sizeEndFrame; endFrameByte++)
        {
            _wire.transmitByte(0x00);
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
    const size_t   _sizeEndFrame;

    T_TWOWIRE _wire;
    uint8_t* _data;       // Holds LED color values
};

typedef DotStarMethodBase<TwoWireBitBangImple> DotStarMethod;

#if !defined(__AVR_ATtiny85__) && !defined(ARDUINO_attiny)
#include "TwoWireSpiImple.h"
typedef DotStarMethodBase<TwoWireSpiImple<SpiSpeed40Mhz>> DotStarSpi40MhzMethod;
typedef DotStarMethodBase<TwoWireSpiImple<SpiSpeed20Mhz>> DotStarSpi20MhzMethod;
typedef DotStarMethodBase<TwoWireSpiImple<SpiSpeed10Mhz>> DotStarSpi10MhzMethod;
typedef DotStarMethodBase<TwoWireSpiImple<SpiSpeed5Mhz>> DotStarSpi5MhzMethod;
typedef DotStarMethodBase<TwoWireSpiImple<SpiSpeed2Mhz>> DotStarSpi2MhzMethod;
typedef DotStarMethodBase<TwoWireSpiImple<SpiSpeed1Mhz>> DotStarSpi1MhzMethod;
typedef DotStarMethodBase<TwoWireSpiImple<SpiSpeed500Khz>> DotStarSpi500KhzMethod;
typedef DotStarMethodBase<TwoWireSpiImple<SpiSpeedHz>> DotStarSpiHzMethod;

typedef DotStarSpi10MhzMethod DotStarSpiMethod;
#endif

#if defined(ARDUINO_ARCH_ESP32)
// Give option to use Vspi alias of Spi class if wanting to specify which SPI peripheral is used on the ESP32
typedef DotStarMethodBase<TwoWireSpiImple<SpiSpeed40Mhz>> DotStarEsp32Vspi40MhzMethod;
typedef DotStarMethodBase<TwoWireSpiImple<SpiSpeed20Mhz>> DotStarEsp32Vspi20MhzMethod;
typedef DotStarMethodBase<TwoWireSpiImple<SpiSpeed10Mhz>> DotStarEsp32Vspi10MhzMethod;
typedef DotStarMethodBase<TwoWireSpiImple<SpiSpeed5Mhz>> DotStarEsp32Vspi5MhzMethod;
typedef DotStarMethodBase<TwoWireSpiImple<SpiSpeed2Mhz>> DotStarEsp32Vspi2MhzMethod;
typedef DotStarMethodBase<TwoWireSpiImple<SpiSpeed1Mhz>> DotStarEsp32Vspi1MhzMethod;
typedef DotStarMethodBase<TwoWireSpiImple<SpiSpeed500Khz>> DotStarEsp32Vspi500KhzMethod;
typedef DotStarMethodBase<TwoWireSpiImple<SpiSpeedHz>> DotStarEsp32VspiHzMethod;

typedef DotStarSpi10MhzMethod DotStarEsp32VspiMethod;

#include "TwoWireHspiImple.h"
typedef DotStarMethodBase<TwoWireHspiImple<SpiSpeed40Mhz>> DotStarEsp32Hspi40MhzMethod;
typedef DotStarMethodBase<TwoWireHspiImple<SpiSpeed20Mhz>> DotStarEsp32Hspi20MhzMethod;
typedef DotStarMethodBase<TwoWireHspiImple<SpiSpeed10Mhz>> DotStarEsp32Hspi10MhzMethod;
typedef DotStarMethodBase<TwoWireHspiImple<SpiSpeed5Mhz>> DotStarEsp32Hspi5MhzMethod;
typedef DotStarMethodBase<TwoWireHspiImple<SpiSpeed2Mhz>> DotStarEsp32Hspi2MhzMethod;
typedef DotStarMethodBase<TwoWireHspiImple<SpiSpeed1Mhz>> DotStarEsp32Hspi1MhzMethod;
typedef DotStarMethodBase<TwoWireHspiImple<SpiSpeed500Khz>> DotStarEsp32Hspi500KhzMethod;
typedef DotStarMethodBase<TwoWireHspiImple<SpiSpeedHz>> DotStarEsp32HspiHzMethod;

typedef DotStarEsp32Hspi10MhzMethod DotStarEsp32HspiMethod;
#endif
