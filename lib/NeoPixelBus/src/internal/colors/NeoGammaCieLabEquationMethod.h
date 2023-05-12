/*-------------------------------------------------------------------------
NeoGammaCieLabEquationMethod class is used to correct RGB colors for human eye gamma levels equally
across all color channels

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

// Alternative equation to provide at least one official model for specific LEDs
class NeoGammaCieLabEquationMethod
{
public:
    static uint8_t Correct(uint8_t value)
    {
        return static_cast<uint8_t>(255.0f * NeoEase::GammaCieLab(value / 255.0f) + 0.5f);
    }
    static uint16_t Correct(uint16_t value)
    {
        return static_cast<uint16_t>(65535.0f * NeoEase::GammaCieLab(value / 65535.0f) + 0.5f);
    }
};