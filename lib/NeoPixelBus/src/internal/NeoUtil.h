/*-------------------------------------------------------------------------
NeoPixel library helper functions

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

// some platforms do not come with STL or properly defined one, specifically functional
// if you see...
// undefined reference to `std::__throw_bad_function_call()'
// ...then you can either add the platform symbol to the list so NEOPIXEBUS_NO_STL gets defined or
// go to boards.txt and enable c++ by adding (teensy31.build.flags.libs=-lstdc++) and set to "smallest code" option in Arduino
//
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_MEGAAVR) || defined(STM32L432xx) || defined(STM32L476xx) || defined(ARDUINO_ARCH_SAM)
#define NEOPIXEBUS_NO_STL 1
#endif

// some platforms do not define this standard progmem type for some reason
//
#ifndef PGM_VOID_P
#define PGM_VOID_P const void *
#endif

#ifndef countof
#define countof(array) (sizeof(array)/sizeof(array[0]))
#endif

class NeoUtil
{
private:
    static constexpr uint8_t Reverse8BitsLookup[16] = {
            0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
            0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf };

public:
    inline static uint8_t Reverse8Bits(uint8_t n)
    {
        return (Reverse8BitsLookup[n & 0b1111] << 4) | Reverse8BitsLookup[n >> 4];
    }

    inline static size_t RoundUp(size_t numToRound, size_t multiple)
    {
        return ((numToRound + multiple - 1) / multiple) * multiple;
    }

    // alternatives that proved to be slower but left for more periodic testing
    /*
    // marginally slower than the table
    static uint8_t Reverse8Bits(uint8_t b)
    {
        b = (b & 0b11110000) >> 4 | (b & 0b00001111) << 4;
        b = (b & 0b11001100) >> 2 | (b & 0b00110011) << 2;
        b = (b & 0b10101010) >> 1 | (b & 0b01010101) << 1;
        return b;
    }
    */

    /*  WAY TO SLOW
    static uint8_t Reverse8Bits(uint8_t b)
    {
        return (b * 0x0202020202ULL & 0x010884422010ULL) % 1023;
    }
    */
};