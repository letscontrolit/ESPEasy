/*-------------------------------------------------------------------------
NeoGamma classes are  used to correct RGB colors for human eye gamma levels

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
#include "../NeoUtil.h"
#include "NeoGammaDynamicTableMethod.h"


uint8_t NeoGammaDynamicTableMethod::_table[256] = { 0 };
NeoGammaDynamicTableMethod::NeoGamma16LowHint* NeoGammaDynamicTableMethod::_hints = nullptr;
uint8_t NeoGammaDynamicTableMethod::_hintsCount = 0;