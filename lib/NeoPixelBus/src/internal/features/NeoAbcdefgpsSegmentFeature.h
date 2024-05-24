/*-------------------------------------------------------------------------
NeoAbcdefgpsSegmentFeature provides feature classes to describe color order and
color depth for NeoPixelBus template class when used with seven segment display

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

// Abcdefgps byte order
class NeoAbcdefgpsSegmentFeature : 
    public NeoByteElements<9, SevenSegDigit, uint8_t>,
    public NeoElementsNoSettings
{
public:
    static void applyPixelColor(uint8_t* pPixels, uint16_t indexPixel, ColorObject color)
    {
        uint8_t* p = getPixelAddress(pPixels, indexPixel);
        uint8_t commonSize = (PixelSize < color.Count) ? PixelSize : color.Count;
        for (uint8_t iSegment = 0; iSegment < commonSize; iSegment++)
        {
            *p++ = color.Segment[iSegment];
        }
    }

    static ColorObject retrievePixelColor(const uint8_t* pPixels, uint16_t indexPixel)
    {
        ColorObject color;
        const uint8_t* p = getPixelAddress(pPixels, indexPixel);
        uint8_t commonSize = (PixelSize < color.Count) ? PixelSize : color.Count;

        for (uint8_t iSegment = 0; iSegment < commonSize; iSegment++)
        {
            color.Segment[iSegment] = *p++;
        }
        return color;
    }

    static ColorObject retrievePixelColor_P(PGM_VOID_P pPixels, uint16_t indexPixel)
    {
        ColorObject color;
        const uint8_t* p = getPixelAddress((const uint8_t*)pPixels, indexPixel);
        uint8_t commonSize = (PixelSize < color.Count) ? PixelSize : color.Count;

        for (uint8_t iSegment = 0; iSegment < commonSize; iSegment++)
        {
            color.Segment[iSegment] = pgm_read_byte(p++);
        }

        return color;
    }

};