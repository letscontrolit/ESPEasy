/*-------------------------------------------------------------------------
NeoSegmentFeatures provides feature classes to describe seven segment display
elements for NeoPixelBus template class

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

class Neo9Elements
{
public:
    static const size_t PixelSize = 9; // three 3 element

    static uint8_t* getPixelAddress(uint8_t* pPixels, uint16_t indexPixel) 
    {
        return pPixels + indexPixel * PixelSize;
    }
    static const uint8_t* getPixelAddress(const uint8_t* pPixels, uint16_t indexPixel)
    {
        return pPixels + indexPixel * PixelSize;
    }

    static void replicatePixel(uint8_t* pPixelDest, const uint8_t* pPixelSrc, uint16_t count)
    {
        uint8_t* pEnd = pPixelDest + (count * PixelSize);
        while (pPixelDest < pEnd)
        {
            for (uint8_t iElement = 0; iElement < PixelSize; iElement++)
            {
                *pPixelDest++ = pPixelSrc[iElement];
            }
        }
    }

    static void movePixelsInc(uint8_t* pPixelDest, const uint8_t* pPixelSrc, uint16_t count)
    {
        uint8_t* pEnd = pPixelDest + (count * PixelSize);
        while (pPixelDest < pEnd)
        {
            *pPixelDest++ = *pPixelSrc++;
        }
    }

    static void movePixelsInc_P(uint8_t* pPixelDest, PGM_VOID_P pPixelSrc, uint16_t count)
    {
        uint8_t* pEnd = pPixelDest + (count * PixelSize);
        const uint8_t* pSrc = (const uint8_t*)pPixelSrc;
        while (pPixelDest < pEnd)
        {
            *pPixelDest++ = pgm_read_byte(pSrc++);
        }
    }

    static void movePixelsDec(uint8_t* pPixelDest, const uint8_t* pPixelSrc, uint16_t count)
    {
        uint8_t* pDestBack = pPixelDest + (count * PixelSize);
        const uint8_t* pSrcBack = pPixelSrc + (count * PixelSize);
        while (pDestBack > pPixelDest)
        {
            *--pDestBack = *--pSrcBack;
        }
    }

    typedef SevenSegDigit ColorObject;
};

class Neo9ElementsNoSettings : public Neo9Elements
{
public:
    typedef NeoNoSettings SettingsObject;
    static const size_t SettingsSize = 0;

    static void applySettings(uint8_t*, const SettingsObject&)
    {
    }

    static uint8_t* pixels(uint8_t* pData)
    {
        return pData;
    }

    static const uint8_t* pixels(const uint8_t* pData)
    {
        return pData;
    }
};

// Abcdefgps byte order
class NeoAbcdefgSegmentFeature : public Neo9ElementsNoSettings
{
public:
    static void applyPixelColor(uint8_t* pPixels, uint16_t indexPixel, ColorObject color)
    {
        uint8_t* p = getPixelAddress(pPixels, indexPixel);
        uint8_t commonSize = (PixelSize < color.SegmentCount) ? PixelSize : color.SegmentCount;
        for (uint8_t iSegment = 0; iSegment < commonSize; iSegment++)
        {
            *p++ = color.Segment[iSegment];
        }
    }

    static ColorObject retrievePixelColor(const uint8_t* pPixels, uint16_t indexPixel)
    {
        ColorObject color;
        const uint8_t* p = getPixelAddress(pPixels, indexPixel);
        uint8_t commonSize = (PixelSize < color.SegmentCount) ? PixelSize : color.SegmentCount;

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
        uint8_t commonSize = (PixelSize < color.SegmentCount) ? PixelSize : color.SegmentCount;

        for (uint8_t iSegment = 0; iSegment < commonSize; iSegment++)
        {
            color.Segment[iSegment] = pgm_read_byte(p++);
        }

        return color;
    }
    
};


// BACEDF.G+ byte order
class NeoBacedfpgsSegmentFeature : public Neo9ElementsNoSettings
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

typedef NeoBacedfpgsSegmentFeature SevenSegmentFeature; // Abcdefg order is default