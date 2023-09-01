/*-------------------------------------------------------------------------
NeoTm1914Features provides feature classes to describe color order and
color depth for NeoPixelBus template class specific to the TM1914 chip

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

enum NeoTm1914_Mode
{
    NeoTm1914_Mode_DinFdinAutoSwitch,  // Switches between DIN and FDIN on any signal pause > 300ms
    NeoTm1914_Mode_DinOnly, // DIN input pin used exclusively 
    NeoTm1914_Mode_FdinOnly  // FDIN input pin used exclusively 
};

class NeoTm1914Settings 
{
public:
    NeoTm1914Settings(NeoTm1914_Mode mode = NeoTm1914_Mode_DinOnly)  :
        Mode(mode)
    {
    }

    NeoTm1914_Mode Mode;
};

class Neo3ByteElementsTm1914Settings 
{
public:
    typedef NeoTm1914Settings SettingsObject;
    static const size_t SettingsSize = 6;

    static void applySettings(MAYBE_UNUSED uint8_t* pData, MAYBE_UNUSED size_t sizeData, MAYBE_UNUSED const SettingsObject& settings)
    {
        // settings are at the front of the data stream
        uint8_t* pSet = pData;
        uint8_t mode = 0xff;

        // C1 - the mode
        *pSet++ = 0xff;
        *pSet++ = 0xff;

        switch (settings.Mode)
        {
        case NeoTm1914_Mode_DinFdinAutoSwitch:
            mode = 0xff;
            break;

        case NeoTm1914_Mode_FdinOnly:
            mode = 0xfa;
            break;

        case NeoTm1914_Mode_DinOnly:
        default:
            mode = 0xf5;
            break;
        }
        *pSet++ = mode;

        // C2 - ones compliment of the above
        uint8_t* pC1 = pData;
        for (uint8_t elem = 0; elem < 3; elem++)
        {
            *pSet++ = ~(*pC1++);
        }
    }

    static uint8_t* pixels(MAYBE_UNUSED uint8_t* pData, MAYBE_UNUSED size_t sizeData)
    {
        // settings are at the front of the data stream
        return pData + SettingsSize;
    }

    static const uint8_t* pixels(MAYBE_UNUSED const uint8_t* pData, MAYBE_UNUSED size_t sizeData)
    {
        // settings are at the front of the data stream
        return pData + SettingsSize;
    }
};


class NeoRgbTm1914Feature : 
    public Neo3ByteFeature<ColorIndexR, ColorIndexG, ColorIndexB>,
    public Neo3ByteElementsTm1914Settings
{
};


class NeoGrbTm1914Feature : 
    public Neo3ByteFeature<ColorIndexG, ColorIndexR, ColorIndexB>,
    public Neo3ByteElementsTm1914Settings
{
};