/*-------------------------------------------------------------------------
Lpd6803RgbFeature provides feature class to describe color order and
color depth for NeoPixelBus template class when used with DotStar like chips

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

class  Lpd6803RgbFeature :
    public Neo2Byte555Feature<ColorIndexR, ColorIndexG, ColorIndexB>,
    public NeoElementsNoSettings
{
};


class  Lpd6803GrbFeature :
    public Neo2Byte555Feature<ColorIndexG, ColorIndexR, ColorIndexB>,
    public NeoElementsNoSettings
{
};

class  Lpd6803GbrFeature :
    public Neo2Byte555Feature<ColorIndexG, ColorIndexB, ColorIndexR>,
    public NeoElementsNoSettings
{
};

class  Lpd6803BrgFeature :
    public Neo2Byte555Feature<ColorIndexB, ColorIndexR, ColorIndexG>,
    public NeoElementsNoSettings
{
};
