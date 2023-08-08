/*-------------------------------------------------------------------------
Rgbw64Color provides a color object that can be directly consumed by NeoPixelBus

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

#include <Arduino.h>
#include "../NeoSettings.h"
#include "RgbColorBase.h"
#include "RgbColor.h"
#include "RgbwColor.h"
#include "Rgb48Color.h"
#include "HslColor.h"
#include "HsbColor.h"
#include "Rgbw64Color.h"
#include "HtmlColor.h"

Rgbw64Color::Rgbw64Color(const HslColor& color)
{
    Rgb48Color rgbColor(color);
    *this = rgbColor;
}

Rgbw64Color::Rgbw64Color(const HsbColor& color)
{
    Rgb48Color rgbColor(color);
    *this = rgbColor;
}

uint16_t Rgbw64Color::CalculateBrightness() const
{
    uint16_t colorB = static_cast<uint16_t>((static_cast<uint32_t>(R) + static_cast<uint32_t>(G) + static_cast<uint32_t>(B)) / 3);
    if (W > colorB)
    {
        return W;
    }
    else
    {
        return colorB;
    }
}

Rgbw64Color Rgbw64Color::Dim(uint16_t ratio) const
{
    // specifically avoids float math
    return Rgbw64Color(_elementDim(R, ratio), _elementDim(G, ratio), _elementDim(B, ratio), _elementDim(W, ratio));
}

Rgbw64Color Rgbw64Color::Brighten(uint16_t ratio) const
{
    // specifically avoids float math
    return Rgbw64Color(_elementBrighten(R, ratio), _elementBrighten(G, ratio), _elementBrighten(B, ratio), _elementBrighten(W, ratio));
}

void Rgbw64Color::Darken(uint16_t delta)
{
    if (R > delta)
    {
        R -= delta;
    }
    else
    {
        R = 0;
    }

    if (G > delta)
    {
        G -= delta;
    }
    else
    {
        G = 0;
    }

    if (B > delta)
    {
        B -= delta;
    }
    else
    {
        B = 0;
    }

    if (W > delta)
    {
        W -= delta;
    }
    else
    {
        W = 0;
    }
}

void Rgbw64Color::Lighten(uint16_t delta)
{
    if (IsColorLess())
    {
        if (W < Max - delta)
        {
            W += delta;
        }
        else
        {
            W = Max;
        }
    }
    else
    {
        if (R < Max - delta)
        {
            R += delta;
        }
        else
        {
            R = Max;
        }

        if (G < Max - delta)
        {
            G += delta;
        }
        else
        {
            G = Max;
        }

        if (B < Max - delta)
        {
            B += delta;
        }
        else
        {
            B = Max;
        }
    }
}

Rgbw64Color Rgbw64Color::LinearBlend(const Rgbw64Color& left, const Rgbw64Color& right, float progress)
{
    return Rgbw64Color( left.R + ((static_cast<int32_t>(right.R) - left.R) * progress),
        left.G + ((static_cast<int32_t>(right.G) - left.G) * progress),
        left.B + ((static_cast<int32_t>(right.B) - left.B) * progress),
        left.W + ((static_cast<int32_t>(right.W) - left.W) * progress) );
}
Rgbw64Color Rgbw64Color::LinearBlend(const Rgbw64Color& left, const Rgbw64Color& right, uint8_t progress)
{
    return Rgbw64Color(left.R + (((static_cast<int64_t>(right.R) - left.R) * static_cast<int64_t>(progress) + 1) >> 8),
        left.G + (((static_cast<int64_t>(right.G) - left.G) * static_cast<int64_t>(progress) + 1) >> 8),
        left.B + (((static_cast<int64_t>(right.B) - left.B) * static_cast<int64_t>(progress) + 1) >> 8),
        left.W + (((static_cast<int64_t>(right.W) - left.W) * static_cast<int64_t>(progress) + 1) >> 8));
}

Rgbw64Color Rgbw64Color::BilinearBlend(const Rgbw64Color& c00, 
    const Rgbw64Color& c01, 
    const Rgbw64Color& c10, 
    const Rgbw64Color& c11, 
    float x, 
    float y)
{
    float v00 = (1.0f - x) * (1.0f - y);
    float v10 = x * (1.0f - y);
    float v01 = (1.0f - x) * y;
    float v11 = x * y;

    return Rgbw64Color(
        c00.R * v00 + c10.R * v10 + c01.R * v01 + c11.R * v11,
        c00.G * v00 + c10.G * v10 + c01.G * v01 + c11.G * v11,
        c00.B * v00 + c10.B * v10 + c01.B * v01 + c11.B * v11,
        c00.W * v00 + c10.W * v10 + c01.W * v01 + c11.W * v11 );
}