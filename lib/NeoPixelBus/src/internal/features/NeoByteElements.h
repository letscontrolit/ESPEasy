/*-------------------------------------------------------------------------
NeoByteElements provides feature base classes to describe color elements
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

// NeoElementsBase contains common methods used by features to map and
// copy pixel memory data in native stream format
// 
// V_PIXEL_SIZE - the size in bytes of a pixel in the data stream
// T_COLOR_OBJECT - the primary color object used to represent a pixel
// T_COPY - (uint8_t/uint16_t/uint32_t) the base type to use when copying/moving 
template<size_t V_PIXEL_SIZE, typename T_COLOR_OBJECT, typename T_COPY>
class NeoElementsBase
{
public:
    static const size_t PixelSize = V_PIXEL_SIZE;
    typedef T_COLOR_OBJECT ColorObject;

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
        T_COPY* pDest = reinterpret_cast<T_COPY*>(pPixelDest);
        T_COPY* pEnd = pDest + (count * PixelSize / sizeof(T_COPY));
        const T_COPY* pEndSrc = reinterpret_cast<const T_COPY*>(pPixelSrc) + PixelSize / sizeof(T_COPY);

        while (pDest < pEnd)
        {
            const T_COPY* pSrc = reinterpret_cast<const T_COPY*>(pPixelSrc);
            while (pSrc < pEndSrc)
            {
                *pDest++ = *pSrc++;
            }
        }
    }

    static void movePixelsInc(uint8_t* pPixelDest, const uint8_t* pPixelSrc, uint16_t count)
    {
        const T_COPY* pSrc = reinterpret_cast<const T_COPY*>(pPixelSrc);
        T_COPY* pDest = reinterpret_cast<T_COPY*>(pPixelDest);
        T_COPY* pEnd = pDest + (count * PixelSize / sizeof(T_COPY));

        while (pDest < pEnd)
        {
            *pDest++ = *pSrc++;
        }
    }

    static void movePixelsDec(uint8_t* pPixelDest, const uint8_t* pPixelSrc, uint16_t count)
    {
        const T_COPY* pSrc = reinterpret_cast<const T_COPY*>(pPixelSrc);
        const T_COPY* pSrcBack = pSrc + (count * PixelSize / sizeof(T_COPY));
        T_COPY* pDest = reinterpret_cast<T_COPY*>(pPixelDest);
        T_COPY* pDestBack = pDest + (count * PixelSize / sizeof(T_COPY));
        
        while (pDestBack > pDest)
        {
            *--pDestBack = *--pSrcBack;
        }
    }
};

// NeoByteElements is used for 8bit color element types and less
// 
// V_PIXEL_SIZE - the size in bytes of a pixel in the data stream
// T_COLOR_OBJECT - the primary color object used to represent a pixel
// T_COPY - (uint8_t/uint16_t/uint32_t) the base type to use when copying/moving 
template<size_t V_PIXEL_SIZE, typename T_COLOR_OBJECT, typename T_COPY>
class NeoByteElements : public NeoElementsBase<V_PIXEL_SIZE, T_COLOR_OBJECT, T_COPY>
{
public:
 
    static void movePixelsInc_P(uint8_t* pPixelDest, PGM_VOID_P pPixelSrc, uint16_t count)
    {
        uint8_t* pEnd = pPixelDest + (count * NeoElementsBase<V_PIXEL_SIZE, T_COLOR_OBJECT, T_COPY>::PixelSize);
        const uint8_t* pSrc = reinterpret_cast<const uint8_t*>(pPixelSrc);
    
        while (pPixelDest < pEnd)
        {
            *pPixelDest++ = pgm_read_byte(pSrc++);
        }
    }
};

// NeoWordElements is used for 16bit color element types
//
// V_PIXEL_SIZE - the size in bytes of a pixel in the data stream
// T_COLOR_OBJECT - the primary color object used to represent a pixel
// T_COPY - (uint16_t/uint32_t) the base type to use when copying/moving 
template<size_t V_PIXEL_SIZE, typename T_COLOR_OBJECT, typename T_COPY>
class NeoWordElements : public NeoElementsBase<V_PIXEL_SIZE, T_COLOR_OBJECT, T_COPY>
{
public:

    static void movePixelsInc_P(uint8_t* pPixelDest, PGM_VOID_P pPixelSrc, uint16_t count)
    {
        uint16_t* pDest = reinterpret_cast<uint16_t*>(pPixelDest);
        uint16_t* pEnd = pDest + (count * NeoElementsBase<V_PIXEL_SIZE, T_COLOR_OBJECT, T_COPY>::PixelSize / sizeof(uint16_t));
        const uint16_t* pSrc = reinterpret_cast<const uint16_t*>(pPixelSrc);

        while (pDest < pEnd)
        {
            *pDest++ = pgm_read_word(pSrc++);
        }
    }
};


