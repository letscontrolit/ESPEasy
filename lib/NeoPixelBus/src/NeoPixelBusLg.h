/*-------------------------------------------------------------------------
NeoPixelBus library wrapper template class that provides luminance and gamma control

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

#include "NeoPixelBus.h"

//
// 
// T_GAMMA - 
//    NeoGammaEquationMethod 
//    NeoGammaCieLabEquationMethod
//    NeoGammaTableMethod
//    NeoGammaNullMethod
//    NeoGammaInvert<one of the above>

template<typename T_COLOR_FEATURE, typename T_METHOD, typename T_GAMMA = NeoGammaEquationMethod> class NeoPixelBusLg :
    public NeoPixelBus<T_COLOR_FEATURE, T_METHOD>
{
public:
    class LuminanceShader
    {
    public:
        LuminanceShader(uint8_t luminance = 255) :
            _luminance(luminance)
        {
        }

        // our shader is always dirty, but these are needed for standard
        // shader support
        bool IsDirty() const
        {
            return true;
        };

        void Dirty()
        {
        };

        void ResetDirty()
        {
        };

        typename T_COLOR_FEATURE::ColorObject Apply(uint16_t, const typename T_COLOR_FEATURE::ColorObject& original)
        {
            // dim and then return gamma adjusted
            typename T_COLOR_FEATURE::ColorObject color = original.Dim(_luminance);
            return NeoGamma<T_GAMMA>::Correct(color);
        }

    protected:
        uint8_t _luminance;

        bool setLuminance(uint8_t luminance)
        {
            bool different = (_luminance != luminance);

            if (different)
            {
                _luminance = luminance;
            }
            
            return different;
        }

        uint8_t getLuminance() const
        {
            return _luminance;
        }

        friend class NeoPixelBusLg;
    };

    // Exposed Shader instance for use with NeoDib.Render like
    // 
    // image.Render<NeoGrbFeature, MyBusType::LuminanceShader>(strip, strip.Shader);
    // where MyBusType is defined like
    // typedef NeoPixelBusLg<NeoGrbFeature, NeoWs2812xMethod> MyBusType;
    //
    LuminanceShader Shader;

public:
    NeoPixelBusLg(uint16_t countPixels, uint8_t pin) :
        NeoPixelBus<T_COLOR_FEATURE, T_METHOD>(countPixels, pin),
        Shader()
    {
    }

    NeoPixelBusLg(uint16_t countPixels, uint8_t pin, NeoBusChannel channel) :
        NeoPixelBus<T_COLOR_FEATURE, T_METHOD>(countPixels, pin, channel),
        Shader()
    {
    }

    NeoPixelBusLg(uint16_t countPixels, uint8_t pinClock, uint8_t pinData) :
        NeoPixelBus<T_COLOR_FEATURE, T_METHOD>(countPixels, pinClock, pinData),
        Shader()
    {
    }

    NeoPixelBusLg(uint16_t countPixels, uint8_t pinClock, uint8_t pinData, uint8_t pinLatch, uint8_t pinOutputEnable = NOT_A_PIN) :
        NeoPixelBus<T_COLOR_FEATURE, T_METHOD>(countPixels, pinClock, pinData, pinLatch, pinOutputEnable),
        Shader()
    {
    }

    NeoPixelBusLg(uint16_t countPixels) :
        NeoPixelBus<T_COLOR_FEATURE, T_METHOD>(countPixels),
        Shader()
    {
    }

    ~NeoPixelBusLg()
    {
    }

    void SetLuminance(uint8_t luminance)
    {
        // does NOT affect current pixel data as there is no safe way
        // to reconstruct the original color values after being
        // modified with both luminance and gamma without storing them
        if (Shader.setLuminance(luminance))
        {
            this->Dirty();
        }
    }

    uint8_t GetLuminance() const
    {
        return Shader.getLuminance();
    }

    void SetPixelColor(uint16_t indexPixel, typename T_COLOR_FEATURE::ColorObject color)
    {
        color = Shader.Apply(indexPixel, color);
        NeoPixelBus<T_COLOR_FEATURE, T_METHOD>::SetPixelColor(indexPixel, color);
    }

    /*
     GetPixelColor is not overloaded as the original will be used
     to just return the fully adjusted color value directly with
     no reverse conversion since it is fraught with inaccuracy
    */

    void ClearTo(typename T_COLOR_FEATURE::ColorObject color)
    {
        color = Shader.Apply(0, color);
        NeoPixelBus<T_COLOR_FEATURE, T_METHOD>::ClearTo(color);
    };

    void ClearTo(typename T_COLOR_FEATURE::ColorObject color, uint16_t first, uint16_t last)
    {
        color = Shader.Apply(0, color);
        NeoPixelBus<T_COLOR_FEATURE, T_METHOD>::ClearTo(color, first, last);
    }

    // if the Pixels buffer is manipulated directly, then this can be called 
    // to apply the luminance and gamma correction to those changes
    void ApplyPostAdjustments()
    {
        if (this->IsDirty())
        {
            for (uint16_t indexPixel = 0; indexPixel < NeoPixelBus<T_COLOR_FEATURE, T_METHOD>::PixelCount(); indexPixel++)
            {
                typename T_COLOR_FEATURE::ColorObject color = NeoPixelBus<T_COLOR_FEATURE, T_METHOD>::GetPixelColor(indexPixel);
                color = Shader.Apply(indexPixel, color);
                NeoPixelBus<T_COLOR_FEATURE, T_METHOD>::SetPixelColor(indexPixel, color);
            }
            this->Dirty();
        }
    }
};


