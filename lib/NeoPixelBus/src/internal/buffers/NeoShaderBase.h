/*-------------------------------------------------------------------------
NeoShaderBase

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

class NeoShaderBase
{
public:
    NeoShaderBase() :
        _state(0)
    {
    }

    bool IsDirty() const
    {
        return  (_state & NEO_DIRTY);
    };

    void Dirty()
    {
        _state |= NEO_DIRTY;
    };

    void ResetDirty()
    {
        _state &= ~NEO_DIRTY;
    };

protected:
    uint8_t _state;     // internal state
};