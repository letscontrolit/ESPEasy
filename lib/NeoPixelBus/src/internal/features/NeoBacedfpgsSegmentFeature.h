/*-------------------------------------------------------------------------
NeoBacedfpgsSegmentFeature provides feature classes to describe color order and
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

// BACEDF.G+ byte order
class NeoBacedfpgsSegmentFeature :
    public NeoByteElements<9, SevenSegDigit, uint8_t>,
    public NeoElementsNoSettings
{
public:
    static void applyPixelColor(uint8_t* pPixels, uint16_t indexPixel, ColorObject color)
    {
        uint8_t* p = getPixelAddress(pPixels, indexPixel);

        // Segment Digit is Abcdefgps order
        *p++ = color.Segment[LedSegment_B];
        *p++ = color.Segment[LedSegment_A];
        *p++ = color.Segment[LedSegment_C];

        *p++ = color.Segment[LedSegment_E];
        *p++ = color.Segment[LedSegment_D];
        *p++ = color.Segment[LedSegment_F];

        *p++ = color.Segment[LedSegment_Decimal];
        *p++ = color.Segment[LedSegment_G];
        *p++ = color.Segment[LedSegment_Custom];
    }

    static ColorObject retrievePixelColor(const uint8_t* pPixels, uint16_t indexPixel)
    {
        ColorObject color;
        const uint8_t* p = getPixelAddress(pPixels, indexPixel);

        color.Segment[LedSegment_B] = *p++;
        color.Segment[LedSegment_A] = *p++;
        color.Segment[LedSegment_C] = *p++;

        color.Segment[LedSegment_E] = *p++;
        color.Segment[LedSegment_D] = *p++;
        color.Segment[LedSegment_F] = *p++;

        color.Segment[LedSegment_Decimal] = *p++;
        color.Segment[LedSegment_G] = *p++;
        color.Segment[LedSegment_Custom] = *p++;

        return color;
    }

    static ColorObject retrievePixelColor_P(PGM_VOID_P pPixels, uint16_t indexPixel)
    {
        ColorObject color;
        const uint8_t* p = getPixelAddress((const uint8_t*)pPixels, indexPixel);

        color.Segment[LedSegment_B] = pgm_read_byte(p++);
        color.Segment[LedSegment_A] = pgm_read_byte(p++);
        color.Segment[LedSegment_C] = pgm_read_byte(p++);

        color.Segment[LedSegment_E] = pgm_read_byte(p++);
        color.Segment[LedSegment_D] = pgm_read_byte(p++);
        color.Segment[LedSegment_F] = pgm_read_byte(p++);

        color.Segment[LedSegment_Decimal] = pgm_read_byte(p++);
        color.Segment[LedSegment_G] = pgm_read_byte(p++);
        color.Segment[LedSegment_Custom] = pgm_read_byte(p++);

        return color;
    }

};
