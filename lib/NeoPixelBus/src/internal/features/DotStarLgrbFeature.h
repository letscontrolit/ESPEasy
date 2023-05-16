/*-------------------------------------------------------------------------
DotStarLgrbFeature provides feature classes to describe color order and
color depth for NeoPixelBus template class when used with DotStars

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


class DotStarLgrbFeature : public DotStar4ElementsNoSettings
{
public:
    static void applyPixelColor(uint8_t* pPixels, uint16_t indexPixel, ColorObject color)
    {
        uint8_t* p = getPixelAddress(pPixels, indexPixel);

        *p++ = 0xE0 | (color.W < 31 ? color.W : 31); // upper three bits are always 111
        *p++ = color.G;
        *p++ = color.R;
        *p = color.B;
    }

    static ColorObject retrievePixelColor(const uint8_t* pPixels, uint16_t indexPixel)
    {
        ColorObject color;
        const uint8_t* p = getPixelAddress(pPixels, indexPixel);

        color.W = (*p++) & 0x1F; // mask out upper three bits
        color.G = *p++;
        color.R = *p++;
        color.B = *p;

        return color;
    }

    static ColorObject retrievePixelColor_P(PGM_VOID_P pPixels, uint16_t indexPixel)
    {
        ColorObject color;
        const uint8_t* p = getPixelAddress((const uint8_t*)pPixels, indexPixel);

        color.W = pgm_read_byte(p++) & 0x1F; // mask out upper three bits
        color.G = pgm_read_byte(p++);
        color.R = pgm_read_byte(p++);
        color.B = pgm_read_byte(p);

        return color;
    }

};
