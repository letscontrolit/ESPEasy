/*-------------------------------------------------------------------------
NeoRgbwFeature provides feature classes to describe color order and
color depth for NeoPixelBus template class

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

class NeoRgbwFeature :
    public Neo4ByteFeature<ColorIndexR, ColorIndexG, ColorIndexB, ColorIndexW>,
    public NeoElementsNoSettings
{
};

class NeoRbgwFeature :
    public Neo4ByteFeature<ColorIndexR, ColorIndexB, ColorIndexG, ColorIndexW>,
    public NeoElementsNoSettings
{
};

class NeoGbrwFeature :
    public Neo4ByteFeature<ColorIndexG, ColorIndexB, ColorIndexR, ColorIndexW>,
    public NeoElementsNoSettings
{
};

class NeoGrbwFeature :
    public Neo4ByteFeature<ColorIndexG, ColorIndexR, ColorIndexB, ColorIndexW>,
    public NeoElementsNoSettings
{
};

class NeoBgrwFeature :
    public Neo4ByteFeature<ColorIndexB, ColorIndexG, ColorIndexR, ColorIndexW>,
    public NeoElementsNoSettings
{
};

class NeoBrgwFeature :
    public Neo4ByteFeature<ColorIndexB, ColorIndexR, ColorIndexG, ColorIndexW>,
    public NeoElementsNoSettings
{
};