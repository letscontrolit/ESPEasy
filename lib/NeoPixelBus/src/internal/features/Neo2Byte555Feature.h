/*-------------------------------------------------------------------------
Neo2Byte555Feature provides feature base classes to describe color elements
with 555 encoding for NeoPixelBus Color Feature template classes

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
 
template <uint8_t V_IC_1, uint8_t V_IC_2, uint8_t V_IC_3>
class Neo2Byte555Feature :
    public NeoByteElements<2, RgbColor, uint16_t>
{
public:
    static void applyPixelColor(uint8_t* pPixels, uint16_t indexPixel, ColorObject color)
    {
        uint8_t* p = getPixelAddress(pPixels, indexPixel);
        uint16_t color555;

        encodePixel(&color555, color);
        *p++ = color555 >> 8;
        *p = color555 & 0xff;
    }

    static ColorObject retrievePixelColor(const uint8_t* pPixels, uint16_t indexPixel)
    {
        ColorObject color;
        const uint8_t* p = getPixelAddress(pPixels, indexPixel);

        uint16_t color555;

        color555 = ((*p++) << 8);
        color555 |= (*p);

        decodePixel(&color, color555);

        return color;
    }

    static ColorObject retrievePixelColor_P(PGM_VOID_P pPixels, uint16_t indexPixel)
    {
        ColorObject color;
        const uint8_t* p = getPixelAddress((const uint8_t*)pPixels, indexPixel);

        uint16_t color555;

        color555 = (pgm_read_byte(p++) << 8);
        color555 |= pgm_read_byte(p);

        decodePixel(&color, color555);

        return color;
    }

protected:
    static void encodePixel(uint16_t* color555, const ColorObject& color)
    {
        *color555 = (0x8000 |
            ((color[V_IC_1] & 0xf8) << 7) |
            ((color[V_IC_2] & 0xf8) << 2) |
            ((color[V_IC_3] & 0xf8) >> 3));
    }

    static void decodePixel(ColorObject* color, uint16_t color555)
    {
        (*color)[V_IC_2] = (color555 >> 2) & 0xf8;
        (*color)[V_IC_3] = (color555 << 3) & 0xf8;
        (*color)[V_IC_1] = (color555 >> 7) & 0xf8;
    }
};