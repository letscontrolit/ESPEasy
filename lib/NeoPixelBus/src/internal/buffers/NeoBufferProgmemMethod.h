/*-------------------------------------------------------------------------
NeoBufferProgmemMethod

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

template<typename T_COLOR_FEATURE> class NeoBufferProgmemMethod
{
public:
    NeoBufferProgmemMethod(uint16_t width, uint16_t height, PGM_VOID_P pixels) :
        _width(width),
        _height(height),
        _pixels(pixels)
    {
    }

    operator NeoBufferContext<T_COLOR_FEATURE>()
    {
        return NeoBufferContext<T_COLOR_FEATURE>(Pixels(), PixelsSize());
    }

    const uint8_t* Pixels() const
    {
        return reinterpret_cast<const uint8_t*>(_pixels);
    };

    size_t PixelsSize() const
    {
        return PixelSize() * PixelCount();
    };

    size_t PixelSize() const
    {
        return T_COLOR_FEATURE::PixelSize;
    };

    uint16_t PixelCount() const
    {
        return _width * _height;
    };

    uint16_t Width() const
    {
        return _width;
    };

    uint16_t Height() const
    {
        return _height;
    };

    void SetPixelColor(uint16_t indexPixel, typename T_COLOR_FEATURE::ColorObject color)
    {
        // PROGMEM is read only, this will do nothing
    };

    void SetPixelColor(uint16_t x, uint16_t y, typename T_COLOR_FEATURE::ColorObject color)
    {
        // PROGMEM is read only, this will do nothing
    };

    typename T_COLOR_FEATURE::ColorObject GetPixelColor(uint16_t indexPixel) const
    {
        if (indexPixel >= PixelCount())
        {
            // Pixel # is out of bounds, this will get converted to a 
            // color object type initialized to 0 (black)
            return 0;
        }

        return T_COLOR_FEATURE::retrievePixelColor_P(_pixels, indexPixel);
    };

    typename T_COLOR_FEATURE::ColorObject GetPixelColor(int16_t x, int16_t y) const
    {
        if (x < 0 || x >= _width || y < 0 || y >= _height)
        {
            // Pixel # is out of bounds, this will get converted to a 
            // color object type initialized to 0 (black)
            return 0;
        }

        uint16_t indexPixel = x + y * _width;
        return T_COLOR_FEATURE::retrievePixelColor_P(_pixels, indexPixel);
    };

    void ClearTo(typename T_COLOR_FEATURE::ColorObject color)
    {
        // PROGMEM is read only, this will do nothing
    };

    void CopyPixels(uint8_t* pPixelDest, const uint8_t* pPixelSrc, uint16_t count)
    {
        T_COLOR_FEATURE::movePixelsInc_P(pPixelDest, pPixelSrc, count);
    }

    typedef typename T_COLOR_FEATURE::ColorObject ColorObject;
    typedef T_COLOR_FEATURE ColorFeature;

private:
    const uint16_t _width;
    const uint16_t _height;
    PGM_VOID_P _pixels;
};