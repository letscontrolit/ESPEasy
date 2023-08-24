/*-------------------------------------------------------------------------
P9813BgrFeature provides feature classes to describe color order and
color depth for NeoPixelBus template class when used with P9813s

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


class P9813BgrFeature : 
    public NeoByteElements<4, RgbColor, uint32_t>,
    public NeoElementsNoSettings
{
public:
    static void applyPixelColor(uint8_t* pPixels, uint16_t indexPixel, ColorObject color)
    {
        uint8_t* p = getPixelAddress(pPixels, indexPixel);

        *p++ = 0xC0 | ((~color.B & 0xC0) >> 2) | ((~color.G & 0xC0) >> 4) | ((~color.R & 0xC0) >> 6);
        *p++ = color.B;
        *p++ = color.G;
        *p = color.R;
    }

    static ColorObject retrievePixelColor(const uint8_t* pPixels, uint16_t indexPixel)
    {
        ColorObject color;
        const uint8_t* p = getPixelAddress(pPixels, indexPixel);

        p++; // ignore the first byte
        color.B = *p++;
        color.G = *p++;
        color.R = *p;

        return color;
    }

    static ColorObject retrievePixelColor_P(PGM_VOID_P pPixels, uint16_t indexPixel)
    {
        ColorObject color;
        const uint8_t* p = getPixelAddress((const uint8_t*)pPixels, indexPixel);

        pgm_read_byte(p++); // ignore the first byte
        color.B = pgm_read_byte(p++);
        color.G = pgm_read_byte(p++);
        color.R = pgm_read_byte(p);

        return color;
    }

};






