/*-------------------------------------------------------------------------
NeoTm1814Features provides feature classes to describe color order and
color depth for NeoPixelBus template class specific to the TM1814 chip

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
 
class NeoTm1814Settings : public NeoRgbwCurrentSettings
{
public:
    NeoTm1814Settings(uint16_t red, uint16_t green, uint16_t blue, uint16_t white)  :
        NeoRgbwCurrentSettings(red, green, blue, white)
    {
    }

    const static uint16_t MinCurrent = 65;
    const static uint16_t MaxCurrent = 380;

    static uint16_t LimitCurrent(uint16_t value)
    {
        if (value < MinCurrent)
        {
            value = MinCurrent;
        }
        else if (value > MaxCurrent)
        {
            value = MaxCurrent;
        }
        return value;
    }
};

template <uint8_t V_IC_1, uint8_t V_IC_2, uint8_t V_IC_3, uint8_t V_IC_4>
class NeoElementsTm1814Settings 
{
private:
    const static uint16_t EncodeDivisor = 5;

public:
    typedef NeoTm1814Settings SettingsObject;
    static const size_t SettingsSize = 8;

    static void applySettings(MAYBE_UNUSED uint8_t* pData, MAYBE_UNUSED size_t sizeData, MAYBE_UNUSED const SettingsObject& settings)
    {
        // settings are at the front of the data stream
        uint8_t* pSet = pData;

        // C1
        *pSet++ = (SettingsObject::LimitCurrent(settings[V_IC_1]) - SettingsObject::MinCurrent) / EncodeDivisor;
        *pSet++ = (SettingsObject::LimitCurrent(settings[V_IC_2]) - SettingsObject::MinCurrent) / EncodeDivisor;
        *pSet++ = (SettingsObject::LimitCurrent(settings[V_IC_3]) - SettingsObject::MinCurrent) / EncodeDivisor;
        *pSet++ = (SettingsObject::LimitCurrent(settings[V_IC_4]) - SettingsObject::MinCurrent) / EncodeDivisor;
        
        uint8_t* pC1 = pData;

        // C2
        for (uint8_t elem = 0; elem < 4; elem++)
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


class NeoWrgbTm1814Feature : 
    public Neo4ByteFeature<ColorIndexW, ColorIndexR, ColorIndexG, ColorIndexB>,
    public NeoElementsTm1814Settings<ColorIndexW, ColorIndexR, ColorIndexG, ColorIndexB>
{   
};

