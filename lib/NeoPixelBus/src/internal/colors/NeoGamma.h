/*-------------------------------------------------------------------------
NeoGamma class is used to correct RGB colors for human eye gamma levels equally
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

// use one of the gamma method classes as a converter for this template class
// T_METHOD - 
//    NeoGammaEquationMethod 
//    NeoGammaCieLabEquationMethod
//    NeoGammaTableMethod
//    NeoGammaNullMethod
//    NeoGammaInvert<one of the above>
//
template<typename T_METHOD> class NeoGamma
{
public:
    static RgbColor Correct(const RgbColor& original)
    {
        return RgbColor(T_METHOD::Correct(original.R),
            T_METHOD::Correct(original.G),
            T_METHOD::Correct(original.B));
    }

    static RgbwColor Correct(const RgbwColor& original)
    {
        return RgbwColor(T_METHOD::Correct(original.R),
            T_METHOD::Correct(original.G),
            T_METHOD::Correct(original.B),
            T_METHOD::Correct(original.W) );
    }

    static Rgb48Color Correct(const Rgb48Color& original)
    {
        return Rgb48Color(T_METHOD::Correct(original.R),
            T_METHOD::Correct(original.G),
            T_METHOD::Correct(original.B));
    }

    static Rgbw64Color Correct(const Rgbw64Color& original)
    {
        return Rgbw64Color(T_METHOD::Correct(original.R),
            T_METHOD::Correct(original.G),
            T_METHOD::Correct(original.B),
            T_METHOD::Correct(original.W));
    }
};



