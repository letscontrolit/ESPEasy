/*-------------------------------------------------------------------------
DotStar3Elements provides feature base classes to describe color elements
for NeoPixelBus Color Feature template classes when used with DotStars

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

class DotStar3ElementsNoSettings : public Neo4ByteRgbElements
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