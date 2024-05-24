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
#pragma once

struct RgbColor;
struct HslColor;
struct HsbColor;

// ------------------------------------------------------------------------
// Rgbw64Color represents a color object that is represented by Red, Green, Blue
// component values and an extra White component.  It contains helpful color 
// routines to manipulate the color.
// ------------------------------------------------------------------------
struct Rgbw64Color : RgbColorBase
{
    typedef NeoRgbwCurrentSettings SettingsObject;

    // ------------------------------------------------------------------------
    // Construct a Rgbw64Color using R, G, B, W values (0-65535)
    // ------------------------------------------------------------------------
    Rgbw64Color(uint16_t r, uint16_t g, uint16_t b, uint16_t w = 0) :
        R(r), G(g), B(b), W(w)
    {
    };

    // ------------------------------------------------------------------------
    // Construct a RgbColor using a single brightness value (0-65535)
    // This works well for creating gray tone colors
    // (0) = black, (65535) = white, (32768) = gray
    // ------------------------------------------------------------------------
    Rgbw64Color(uint16_t brightness) :
        R(0), G(0), B(0), W(brightness)
    {
    };

    // ------------------------------------------------------------------------
    // Construct a Rgbw64Color using RgbColor
    // ------------------------------------------------------------------------
    Rgbw64Color(const RgbColor& color) 
    {
        *this = Rgb48Color(color);
    };

    // ------------------------------------------------------------------------
    // Construct a Rgbw64Color using Rgb48Color
    // ------------------------------------------------------------------------
    Rgbw64Color(const Rgb48Color& color) :
        R(color.R),
        G(color.G),
        B(color.B),
        W(0)
    {
    };

    // ------------------------------------------------------------------------
    // Construct a Rgbw64Color using RgbwColor
    // ------------------------------------------------------------------------
    Rgbw64Color(const RgbwColor& color)
    {
        // x16 = map(x8, 0, 255, 0, 65535); // refactors to just * 257 
        R = (uint16_t)color.R * 257; // 257 = MAXUINT16/MAXUINT8 = 65535/255
        G = (uint16_t)color.G * 257;
        B = (uint16_t)color.B * 257;
        W = (uint16_t)color.W * 257;
    };


    // ------------------------------------------------------------------------
    // Construct a Rgbw64Color using HtmlColor
    // ------------------------------------------------------------------------
    Rgbw64Color(const HtmlColor& color)
    {
        *this = RgbwColor(color);
    }

    // ------------------------------------------------------------------------
    // Construct a Rgbw64Color using HslColor
    // ------------------------------------------------------------------------
    Rgbw64Color(const HslColor& color);

    // ------------------------------------------------------------------------
    // Construct a Rgbw64Color using HsbColor
    // ------------------------------------------------------------------------
    Rgbw64Color(const HsbColor& color);

    // ------------------------------------------------------------------------
    // Construct a Rgbw64Color that will have its values set in latter operations
    // CAUTION:  The R,G,B, W members are not initialized and may not be consistent
    // ------------------------------------------------------------------------
    Rgbw64Color()
    {
    };

    // ------------------------------------------------------------------------
    // Comparison operators
    // ------------------------------------------------------------------------
    bool operator==(const Rgbw64Color& other) const
    {
        return (R == other.R && G == other.G && B == other.B && W == other.W);
    };

    bool operator!=(const Rgbw64Color& other) const
    {
        return !(*this == other);
    };

    // ------------------------------------------------------------------------
   // CompareTo method
   // compares against another color with the given epsilon (delta allowed)
   // returns the greatest difference of a set of elements, 
   //   0 = equal within epsilon delta
   //   negative - this is less than other
   //   positive - this is greater than other
   // ------------------------------------------------------------------------
    int32_t CompareTo(const Rgbw64Color& other, uint16_t epsilon = 256)
    {
        return _Compare<Rgbw64Color, int32_t>(*this, other, epsilon);
    }

    // ------------------------------------------------------------------------
    // Compare method
    // compares two colors with the given epsilon (delta allowed)
    // returns the greatest difference of a set of elements, 
    //   0 = equal within epsilon delta
    //   negative - left is less than right
    //   positive - left is greater than right
    // ------------------------------------------------------------------------
    static int32_t Compare(const Rgbw64Color& left, const Rgbw64Color& right, uint16_t epsilon = 256)
    {
        return _Compare<Rgbw64Color, int32_t>(left, right, epsilon);
    }

    // ------------------------------------------------------------------------
    // operator [] - readonly
    // access elements in order by index rather than R,G,B
    // see static Count for the number of elements
    // ------------------------------------------------------------------------
    uint16_t operator[](size_t idx) const
    {
        switch (idx)
        {
        case 0:
            return R;
        case 1:
            return G;
        case 2:
            return B;
        default:
            return W;
        }
    }

    // ------------------------------------------------------------------------
    // operator [] - read write
    // access elements in order by index rather than R,G,B
    // see static Count for the number of elements
    // ------------------------------------------------------------------------
    uint16_t& operator[](size_t idx)
    {
        switch (idx)
        {
        case 0:
            return R;
        case 1:
            return G;
        case 2:
            return B;
        default:
            return W;
        }
    }

    // ------------------------------------------------------------------------
    // Returns if the color is grey, all values are equal other than white
    // ------------------------------------------------------------------------
    bool IsMonotone() const
    {
        return (R == B && R == G);
    };

    // ------------------------------------------------------------------------
    // Returns if the color components are all zero, the white component maybe 
    // anything
    // ------------------------------------------------------------------------
    bool IsColorLess() const
    {
        return (R == 0 && B == 0 && G == 0);
    };

    // ------------------------------------------------------------------------
    // CalculateBrightness will calculate the overall brightness
    // NOTE: This is a simple linear brightness
    // ------------------------------------------------------------------------
    uint16_t CalculateBrightness() const;

    // ------------------------------------------------------------------------
    // Dim will return a new color that is blended to black with the given ratio
    // ratio - (0-65535) where 65535 will return the original color and 0 will return black
    // 
    // NOTE: This is a simple linear blend
    // ------------------------------------------------------------------------
    Rgbw64Color Dim(uint16_t ratio) const;

    // ------------------------------------------------------------------------
    // Dim will return a new color that is blended to black with the given ratio
    // ratio - (0-255) where 255 will return the original color and 0 will return black
    // 
    // NOTE: This is a simple linear blend
    // ------------------------------------------------------------------------
    Rgbw64Color Dim(uint8_t ratio) const
    {
        uint16_t expanded = ratio << 8;
        return Dim(expanded);
    }

    // ------------------------------------------------------------------------
    // Brighten will return a new color that is blended to white with the given ratio
    // ratio - (0-65535) where 65535 will return the original color and 0 will return white
    // 
    // NOTE: This is a simple linear blend
    // ------------------------------------------------------------------------
    Rgbw64Color Brighten(uint16_t ratio) const;

    // ------------------------------------------------------------------------
    // Brighten will return a new color that is blended to white with the given ratio
    // ratio - (0-255) where 255 will return the original color and 0 will return white
    // 
    // NOTE: This is a simple linear blend
    // ------------------------------------------------------------------------
    Rgbw64Color Brighten(uint8_t ratio) const
    {
        uint16_t expanded = ratio << 8;
        return Brighten(expanded);
    }

    // ------------------------------------------------------------------------
    // Darken will adjust the color by the given delta toward black
    // NOTE: This is a simple linear change
    // delta - (0-65535) the amount to dim the color
    // ------------------------------------------------------------------------
    void Darken(uint16_t delta);

    // ------------------------------------------------------------------------
    // Lighten will adjust the color by the given delta toward white
    // NOTE: This is a simple linear change
    // delta - (0-65535) the amount to lighten the color
    // ------------------------------------------------------------------------
    void Lighten(uint16_t delta);

    // ------------------------------------------------------------------------
    // LinearBlend between two colors by the amount defined by progress variable
    // left - the color to start the blend at
    // right - the color to end the blend at
    // progress - (0.0 - 1.0) value where 0 will return left and 1.0 will return right
    //     and a value between will blend the color weighted linearly between them
    // ------------------------------------------------------------------------
    static Rgbw64Color LinearBlend(const Rgbw64Color& left, const Rgbw64Color& right, float progress);
    // progress - (0 - 255) value where 0 will return left and 255 will return right
    //     and a value between will blend the color weighted linearly between them
    // ------------------------------------------------------------------------
    static Rgbw64Color LinearBlend(const Rgbw64Color& left, const Rgbw64Color& right, uint8_t progress);

    // ------------------------------------------------------------------------
    // BilinearBlend between four colors by the amount defined by 2d variable
    // c00 - upper left quadrant color
    // c01 - upper right quadrant color
    // c10 - lower left quadrant color
    // c11 - lower right quadrant color
    // x - unit value (0.0 - 1.0) that defines the blend progress in horizontal space
    // y - unit value (0.0 - 1.0) that defines the blend progress in vertical space
    // ------------------------------------------------------------------------
    static Rgbw64Color BilinearBlend(const Rgbw64Color& c00, 
        const Rgbw64Color& c01, 
        const Rgbw64Color& c10, 
        const Rgbw64Color& c11, 
        float x, 
        float y);

    uint16_t CalcTotalTenthMilliAmpere(const SettingsObject& settings)
    {
        auto total = 0;

        total += R * settings.RedTenthMilliAmpere / Max;
        total += G * settings.GreenTenthMilliAmpere / Max;
        total += B * settings.BlueTenthMilliAmpere / Max;
        total += W * settings.WhiteTenthMilliAmpere / Max;

        return total;
    }

    // ------------------------------------------------------------------------
    // Red, Green, Blue, White color members (0-65535) where 
    // (0,0,0,0) is black and (65535,65535,65535, 0) and (0,0,0,65535) is white
    // Note (65535,65535,65535,65535) is extreme bright white
    // ------------------------------------------------------------------------
    uint16_t R;
    uint16_t G;
    uint16_t B;
    uint16_t W;

    const static uint16_t Max = 65535;
    const static size_t Count = 4; // four elements in []

private:
    inline static uint16_t _elementDim(uint16_t value, uint16_t ratio)
    {
        return (static_cast<uint32_t>(value) * (static_cast<uint32_t>(ratio) + 1)) >> 16;
    }

    inline static uint16_t _elementBrighten(uint16_t value, uint16_t ratio)
    {
        uint32_t element = ((static_cast<uint32_t>(value) + 1) << 16) / (static_cast<uint32_t>(ratio) + 1);

        if (element > Max)
        {
            element = Max;
        }
        else
        {
            element -= 1;
        }
        return element;
    }
};

