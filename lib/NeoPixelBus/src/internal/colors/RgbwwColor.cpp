/*-------------------------------------------------------------------------
RgbwwColor provides a color object that can be directly consumed by NeoPixelBus

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
#include "HtmlColor.h"

#include "RgbwwColor.h"

RgbwwColor::RgbwwColor(const HtmlColor& color)
{
    uint32_t temp = color.Color;
    B = (temp & 0xff);
    temp = temp >> 8;
    G = (temp & 0xff);
    temp = temp >> 8;
    R = (temp & 0xff);
    temp = temp >> 8;
    WW = (temp & 0xff);
    CW = WW;
};

RgbwwColor::RgbwwColor(const HslColor& color)
{
    RgbColor rgbColor(color);
    *this = rgbColor;
}

RgbwwColor::RgbwwColor(const HsbColor& color)
{
    RgbColor rgbColor(color);
    *this = rgbColor;
}

uint8_t RgbwwColor::CalculateBrightness() const
{
    uint8_t colorB = static_cast<uint8_t>((static_cast<uint16_t>(R) + static_cast<uint16_t>(G) + static_cast<uint16_t>(B)) / 3);
    uint8_t whiteB = static_cast<uint8_t>((static_cast<uint16_t>(WW) + static_cast<uint16_t>(CW)) / 2);

    return (whiteB > colorB) ? whiteB : colorB;
}

RgbwwColor RgbwwColor::Dim(uint8_t ratio) const
{
    // specifically avoids float math
    return RgbwwColor(_elementDim(R, ratio), 
        _elementDim(G, ratio), 
        _elementDim(B, ratio), 
        _elementDim(WW, ratio), 
        _elementDim(CW, ratio));
}

RgbwwColor RgbwwColor::Brighten(uint8_t ratio) const
{
    // specifically avoids float math
    return RgbwwColor(_elementBrighten(R, ratio), 
        _elementBrighten(G, ratio), 
        _elementBrighten(B, ratio), 
        _elementBrighten(WW, ratio), 
        _elementBrighten(CW, ratio));
}

void RgbwwColor::Darken(uint8_t delta)
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

    if (WW > delta)
    {
        WW -= delta;
    }
    else
    {
        WW = 0;
    }

    if (CW > delta)
    {
        CW -= delta;
    }
    else
    {
        CW = 0;
    }
}

void RgbwwColor::Lighten(uint8_t delta)
{
    if (IsColorLess())
    {
        if (WW < 255 - delta)
        {
            WW += delta;
        }
        else
        {
            WW = 255;
        }

        if (CW < 255 - delta)
        {
            CW += delta;
        }
        else
        {
            CW = 255;
        }
    }
    else
    {
        if (R < 255 - delta)
        {
            R += delta;
        }
        else
        {
            R = 255;
        }

        if (G < 255 - delta)
        {
            G += delta;
        }
        else
        {
            G = 255;
        }

        if (B < 255 - delta)
        {
            B += delta;
        }
        else
        {
            B = 255;
        }
    }
}

RgbwwColor RgbwwColor::LinearBlend(const RgbwwColor& left, const RgbwwColor& right, float progress)
{
    return RgbwwColor( left.R + ((static_cast<int16_t>(right.R) - left.R) * progress),
        left.G + ((static_cast<int16_t>(right.G) - left.G) * progress),
        left.B + ((static_cast<int16_t>(right.B) - left.B) * progress),
        left.WW + ((static_cast<int16_t>(right.WW) - left.WW) * progress),
        left.CW + ((static_cast<int16_t>(right.CW) - left.CW) * progress));
}

RgbwwColor RgbwwColor::LinearBlend(const RgbwwColor& left, const RgbwwColor& right, uint8_t progress)
{
    return RgbwwColor(left.R + (((static_cast<int32_t>(right.R) - left.R) * static_cast<int32_t>(progress) + 1) >> 8),
        left.G + (((static_cast<int32_t>(right.G) - left.G) * static_cast<int32_t>(progress) + 1) >> 8),
        left.B + (((static_cast<int32_t>(right.B) - left.B) * static_cast<int32_t>(progress) + 1) >> 8),
        left.WW + (((static_cast<int32_t>(right.WW) - left.WW) * static_cast<int32_t>(progress) + 1) >> 8),
        left.CW + (((static_cast<int32_t>(right.CW) - left.CW) * static_cast<int32_t>(progress) + 1) >> 8));
}

RgbwwColor RgbwwColor::BilinearBlend(const RgbwwColor& c00, 
    const RgbwwColor& c01, 
    const RgbwwColor& c10, 
    const RgbwwColor& c11, 
    float x, 
    float y)
{
    float v00 = (1.0f - x) * (1.0f - y);
    float v10 = x * (1.0f - y);
    float v01 = (1.0f - x) * y;
    float v11 = x * y;

    return RgbwwColor(
        c00.R * v00 + c10.R * v10 + c01.R * v01 + c11.R * v11,
        c00.G * v00 + c10.G * v10 + c01.G * v01 + c11.G * v11,
        c00.B * v00 + c10.B * v10 + c01.B * v01 + c11.B * v11,
        c00.WW * v00 + c10.WW * v10 + c01.WW * v01 + c11.WW * v11,
        c00.CW * v00 + c10.CW * v10 + c01.CW * v01 + c11.CW * v11);
}