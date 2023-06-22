/*-------------------------------------------------------------------------
Neo8ByteElements provides feature base classes to describe color elements
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

#include "../NeoUtil.h"

class Neo8ByteElements
{
public:
    static const size_t PixelSize = 8;

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
        uint32_t* pDest = reinterpret_cast<uint32_t*>(pPixelDest);
        const uint32_t* pSrc = reinterpret_cast<const uint32_t*>(pPixelSrc);
        const uint32_t* pEnd = pDest + (count * PixelSize / sizeof(*pDest));

        while (pDest < pEnd)
        {
            *pDest++ = *pSrc;
        }
    }

    static void movePixelsInc(uint8_t* pPixelDest, const uint8_t* pPixelSrc, uint16_t count)
    {
        uint32_t* pDest = reinterpret_cast<uint32_t*>(pPixelDest);
        const uint32_t* pSrc = reinterpret_cast<const uint32_t*>(pPixelSrc);
        const uint32_t* pEnd = pDest + (count * PixelSize / sizeof(*pDest));

        while (pDest < pEnd)
        {
            *pDest++ = *pSrc++;
        }
    }

    static void movePixelsInc_P(uint8_t* pPixelDest, PGM_VOID_P pPixelSrc, uint16_t count)
    {
        uint32_t* pDest = reinterpret_cast<uint32_t*>(pPixelDest);
        const uint32_t* pSrc = reinterpret_cast<const uint32_t*>(pPixelSrc);
        const uint32_t* pEnd = pDest + (count * PixelSize / sizeof(*pDest));

        while (pDest < pEnd)
        {
            *pDest++ = pgm_read_dword(pSrc++);
        }
    }

    static void movePixelsDec(uint8_t* pPixelDest, const uint8_t* pPixelSrc, uint16_t count)
    {
        uint32_t* pDest = reinterpret_cast<uint32_t*>(pPixelDest);
        const uint32_t* pSrc = reinterpret_cast<const uint32_t*>(pPixelSrc);
        uint32_t* pDestBack = pDest + (count * PixelSize / sizeof(*pDest));
        const uint32_t* pSrcBack = pSrc + (count * PixelSize / sizeof(*pSrc));

        while (pDestBack > pDest)
        {
            *--pDestBack = *--pSrcBack;
        }
    }

    typedef Rgbw64Color ColorObject;
};

class Neo8ByteElementsNoSettings : public Neo8ByteElements
{
public:
    typedef NeoNoSettings SettingsObject;
    static const size_t SettingsSize = 0;

    static void applySettings(MAYBE_UNUSED uint8_t* pData, MAYBE_UNUSED size_t sizeData, MAYBE_UNUSED const SettingsObject& settings)
    {
    }

    static uint8_t* pixels(MAYBE_UNUSED uint8_t* pData, MAYBE_UNUSED size_t sizeData)
    {
        return pData;
    }

    static const uint8_t* pixels(MAYBE_UNUSED const uint8_t* pData, MAYBE_UNUSED size_t sizeData)
    {
        return pData;
    }
};