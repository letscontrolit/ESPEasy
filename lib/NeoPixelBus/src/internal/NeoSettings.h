/*-------------------------------------------------------------------------
NeoSettings provides settings classes to describe settings

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

class NeoNoSettings
{
};

class NeoRgbCurrentSettings
{
public:
    NeoRgbCurrentSettings(uint16_t red, uint16_t green, uint16_t blue) :
        RedTenthMilliAmpere(red),
        GreenTenthMilliAmpere(green),
        BlueTenthMilliAmpere(blue)
    {
    }

    // ------------------------------------------------------------------------
    // operator [] - readonly
    // access elements in order by index rather than member name
    // ------------------------------------------------------------------------
    uint16_t operator[](size_t idx) const
    {
        switch (idx)
        {
        case 0:
            return RedTenthMilliAmpere;
        case 1:
            return GreenTenthMilliAmpere;
        default:
            return BlueTenthMilliAmpere;
        }
    }

    const uint16_t RedTenthMilliAmpere;   // in 1/10th ma
    const uint16_t GreenTenthMilliAmpere; // in 1/10th ma
    const uint16_t BlueTenthMilliAmpere;  // in 1/10th ma
};

class NeoRgbwCurrentSettings
{
public:
    NeoRgbwCurrentSettings(uint16_t red, uint16_t green, uint16_t blue, uint16_t white) :
        RedTenthMilliAmpere(red),
        GreenTenthMilliAmpere(green),
        BlueTenthMilliAmpere(blue),
        WhiteTenthMilliAmpere(white)
    {
    }

    // ------------------------------------------------------------------------
    // operator [] - readonly
    // access elements in order by index rather than member name
    // ------------------------------------------------------------------------
    uint16_t operator[](size_t idx) const
    {
        switch (idx)
        {
        case 0:
            return RedTenthMilliAmpere;
        case 1:
            return GreenTenthMilliAmpere;
        case 2:
            return BlueTenthMilliAmpere;
        default:
            return WhiteTenthMilliAmpere;
        }
    }

    const uint16_t RedTenthMilliAmpere;   // in 1/10th ma
    const uint16_t GreenTenthMilliAmpere; // in 1/10th ma
    const uint16_t BlueTenthMilliAmpere;  // in 1/10th ma
    const uint16_t WhiteTenthMilliAmpere; // in 1/10th ma
};

class NeoRgbwwCurrentSettings
{
public:
    NeoRgbwwCurrentSettings(uint16_t red, uint16_t green, uint16_t blue, uint16_t warmWhite, uint16_t coolWhite) :
        RedTenthMilliAmpere(red),
        GreenTenthMilliAmpere(green),
        BlueTenthMilliAmpere(blue),
        WarmWhiteTenthMilliAmpere(warmWhite),
        CoolWhiteTenthMilliAmpere(coolWhite)
    {
    }

    // ------------------------------------------------------------------------
    // operator [] - readonly
    // access elements in order by index rather than member name
    // ------------------------------------------------------------------------
    uint16_t operator[](size_t idx) const
    {
        switch (idx)
        {
        case 0:
            return RedTenthMilliAmpere;
        case 1:
            return GreenTenthMilliAmpere;
        case 2:
            return BlueTenthMilliAmpere;
        case 3:
            return WarmWhiteTenthMilliAmpere;
        default:
            return CoolWhiteTenthMilliAmpere;
        }
    }

    const uint16_t RedTenthMilliAmpere;   // in 1/10th ma
    const uint16_t GreenTenthMilliAmpere; // in 1/10th ma
    const uint16_t BlueTenthMilliAmpere;  // in 1/10th ma
    const uint16_t WarmWhiteTenthMilliAmpere; // in 1/10th ma
    const uint16_t CoolWhiteTenthMilliAmpere; // in 1/10th ma
};