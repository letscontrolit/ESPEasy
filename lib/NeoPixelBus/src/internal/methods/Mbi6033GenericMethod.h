/*-------------------------------------------------------------------------
NeoPixel library helper functions for Mbi6033s using general Pins.

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

#include "../NeoUtil.h"
// must also check for arm due to Teensy incorrectly having ARDUINO_ARCH_AVR set
#if defined(ARDUINO_ARCH_AVR) && !defined(__arm__)
#include "TwoWireBitBangImpleAvr.h"
#else
#include "TwoWireBitBangImple.h"
#endif


template<typename T_TWOWIRE> class Mbi6033MethodBase
{
public:
    typedef typename T_TWOWIRE::SettingsObject SettingsObject;

    Mbi6033MethodBase(uint8_t pinClock, uint8_t pinData, uint16_t pixelCount, size_t elementSize, size_t settingsSize) :
        _countChips(NeoUtil::RoundUp(pixelCount * elementSize, c_countBytesPerChip) / c_countBytesPerChip),
        _sizeData(_countChips * c_countBytesPerChip + settingsSize),
        _pinClock(pinClock),
        _wire(pinClock, pinData)
    {
        _data = static_cast<uint8_t*>(malloc(_sizeData));
        // data cleared later in Begin()
    }

#if !defined(__AVR_ATtiny85__) && !defined(ARDUINO_attiny)
    Mbi6033MethodBase(uint16_t pixelCount, size_t elementSize, size_t settingsSize) :
        Mbi6033MethodBase(SCK, MOSI, pixelCount, elementSize, settingsSize)
    {
    }
#endif

    ~Mbi6033MethodBase()
    {
        free(_data);
    }

    bool IsReadyToUpdate() const
    {
        return true; // clock driven chips don't have a required delay
    }

#if defined(ARDUINO_ARCH_ESP32)
    void Initialize(int8_t sck, int8_t miso, int8_t mosi, int8_t ss)
    {
        _pinClock = sck;
        _wire.begin(sck, miso, mosi, ss);
    }
#endif

    void Initialize()
    {
        _wire.begin();
    }

    void Update(bool)
    {
        // non current header format:
        // 8 bits: command (0xf3) = non current header mode
        // 14 bits: sync (0x0000) 
        // 14 bits: length (count chips - 1)
        // 8 bits: configuration (0x02) 
        //    bits (6-4) refresh rate divider bits (0 for fastest)
        //    bit (1) on or off
        // 4 bits: header code name X1 (0x0)
        const uint16_t chipLength = _countChips - 1;
        const uint8_t headerFrame[6] = { 0xf3, 
                0x00, 
                static_cast<uint8_t>(chipLength >> 12), 
                static_cast<uint8_t>((chipLength >> 4) & 0xff),
                static_cast<uint8_t>((chipLength << 4) & 0xff),
                0x20 };
        
        // prefix protocol, >21us clock low, clock high, >21us clock low
        // expecting at least 21us since last call to show
        // but using hardware SPI won't allow messing with clock
        // directly like this...
        //delayMicroseconds(c_usResetTime);
        //digitalWrite(_pinClock, HIGH);
        //digitalWrite(_pinClock, LOW);
        //delayMicroseconds(c_usResetTime);

        _wire.beginTransaction();

        // reset by toggle of clock
        delayMicroseconds(c_usResetTime);
        _wire.transmitBit(0); // Our Two Wire BitBang supports this
        delayMicroseconds(c_usResetTime);

        // header frame
        _wire.transmitBytes(headerFrame, sizeof(headerFrame));
        
        // data:
        // 24 bytes per chip of data (12 16 bit values)
        _wire.transmitBytes(_data, _sizeData);

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

    void applySettings(MAYBE_UNUSED const SettingsObject& settings)
    {
        _wire.applySettings(settings);
    }

private:
    // while spec states 4 RGB * 16 bit values, 
    // its really 12 channels * 16 bit values, as they could
    // be wired how ever the circuit is built, like 3 RGBW
    static const uint16_t c_countBytesPerChip = 24; // twelve 16 bit values
    static const uint16_t c_usResetTime = 21; 

    const uint16_t _countChips; // not pixels, driver chips
    const size_t   _sizeData;   // Size of '_data' buffer below
    
    uint8_t _pinClock;
    T_TWOWIRE _wire;
    uint8_t* _data;       // Holds LED color values
};

typedef Mbi6033MethodBase<TwoWireBitBangImple> Mbi6033Method;

/* Due to reset model by these chips needing to control clock, we can't use hardware SPI
* as neither the normal SPI exposes a single bit send nor does it allow direct clock pulses
* using digitalWrite.  If a generalized solution could be found, then this could be enabled
* 
#if !defined(__AVR_ATtiny85__) && !defined(ARDUINO_attiny)
#include "TwoWireSpiImple.h"
typedef Mbi6033MethodBase<TwoWireSpiImple<SpiSpeed40Mhz>> Mbi6033Spi40MhzMethod;
typedef Mbi6033MethodBase<TwoWireSpiImple<SpiSpeed20Mhz>> Mbi6033Spi20MhzMethod;
typedef Mbi6033MethodBase<TwoWireSpiImple<SpiSpeed10Mhz>> Mbi6033Spi10MhzMethod;
typedef Mbi6033MethodBase<TwoWireSpiImple<SpiSpeed5Mhz>> Mbi6033Spi5MhzMethod;
typedef Mbi6033MethodBase<TwoWireSpiImple<SpiSpeed2Mhz>> Mbi6033Spi2MhzMethod;
typedef Mbi6033MethodBase<TwoWireSpiImple<SpiSpeed1Mhz>> Mbi6033Spi1MhzMethod;
typedef Mbi6033MethodBase<TwoWireSpiImple<SpiSpeed500Khz>> Mbi6033Spi500KhzMethod;
typedef Mbi6033MethodBase<TwoWireSpiImple<SpiSpeedHz>> Mbi6033SpiHzMethod;

typedef Mbi6033Spi10MhzMethod Mbi6033SpiMethod;
#endif

#if defined(ARDUINO_ARCH_ESP32)
// Give option to use Vspi alias of Spi class if wanting to specify which SPI peripheral is used on the ESP32
typedef Mbi6033MethodBase<TwoWireSpiImple<SpiSpeed40Mhz>> Mbi6033Esp32Vspi40MhzMethod;
typedef Mbi6033MethodBase<TwoWireSpiImple<SpiSpeed20Mhz>> Mbi6033Esp32Vspi20MhzMethod;
typedef Mbi6033MethodBase<TwoWireSpiImple<SpiSpeed10Mhz>> Mbi6033Esp32Vspi10MhzMethod;
typedef Mbi6033MethodBase<TwoWireSpiImple<SpiSpeed5Mhz>> Mbi6033Esp32Vspi5MhzMethod;
typedef Mbi6033MethodBase<TwoWireSpiImple<SpiSpeed2Mhz>> Mbi6033Esp32Vspi2MhzMethod;
typedef Mbi6033MethodBase<TwoWireSpiImple<SpiSpeed1Mhz>> Mbi6033Esp32Vspi1MhzMethod;
typedef Mbi6033MethodBase<TwoWireSpiImple<SpiSpeed500Khz>> Mbi6033Esp32Vspi500KhzMethod;
typedef Mbi6033MethodBase<TwoWireSpiImple<SpiSpeedHz>> Mbi6033Esp32VspiHzMethod;

typedef Mbi6033Spi10MhzMethod Mbi6033Esp32VspiMethod;

#include "TwoWireHspiImple.h"
typedef Mbi6033MethodBase<TwoWireHspiImple<SpiSpeed40Mhz>> Mbi6033Esp32Hspi40MhzMethod;
typedef Mbi6033MethodBase<TwoWireHspiImple<SpiSpeed20Mhz>> Mbi6033Esp32Hspi20MhzMethod;
typedef Mbi6033MethodBase<TwoWireHspiImple<SpiSpeed10Mhz>> Mbi6033Esp32Hspi10MhzMethod;
typedef Mbi6033MethodBase<TwoWireHspiImple<SpiSpeed5Mhz>> Mbi6033Esp32Hspi5MhzMethod;
typedef Mbi6033MethodBase<TwoWireHspiImple<SpiSpeed2Mhz>> Mbi6033Esp32Hspi2MhzMethod;
typedef Mbi6033MethodBase<TwoWireHspiImple<SpiSpeed1Mhz>> Mbi6033Esp32Hspi1MhzMethod;
typedef Mbi6033MethodBase<TwoWireHspiImple<SpiSpeed500Khz>> Mbi6033Esp32Hspi500KhzMethod;
typedef Mbi6033MethodBase<TwoWireHspiImple<SpiSpeedHz>> Mbi6033Esp32HspiHzMethod;

typedef Mbi6033Esp32Hspi10MhzMethod Mbi6033Esp32HspiMethod;
#endif
*/