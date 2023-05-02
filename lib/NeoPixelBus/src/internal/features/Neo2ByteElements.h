/*-------------------------------------------------------------------------
Neo2ByteElements provides feature base classes to describe color elements
for NeoPixelBus Color Feature template classes

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

class Neo2ByteElements
{
public:
    static const size_t PixelSize = 2;   // 1 bit + 555 encoded elements

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

    typedef RgbColor ColorObject;

protected:
    static void encodePixel(uint8_t c1, uint8_t c2, uint8_t c3, uint16_t* color555)
    {
        *color555 = (0x8000 |
            ((c1 & 0xf8) << 7) |
            ((c2 & 0xf8) << 2) |
            ((c3 & 0xf8) >> 3));
    }

    static void decodePixel(uint16_t color555, uint8_t* c1, uint8_t* c2, uint8_t* c3)
    {
        *c1 = (color555 >> 7) & 0xf8;
        *c2 = (color555 >> 2) & 0xf8;
        *c3 = (color555 << 3) & 0xf8;
    }
};

class Neo2ByteElementsNoSettings : public Neo2ByteElements
{
public:
    typedef NeoNoSettings SettingsObject;
    static const size_t SettingsSize = 0;

    static void applySettings([[maybe_unused]] uint8_t* pData, [[maybe_unused]] size_t sizeData, [[maybe_unused]] const SettingsObject& settings)
    {
    }

    static uint8_t* pixels([[maybe_unused]] uint8_t* pData, [[maybe_unused]] size_t sizeData)
    {
        return pData;
    }

    static const uint8_t* pixels([[maybe_unused]] const uint8_t* pData, [[maybe_unused]] size_t sizeData)
    {
        return pData;
    }
};