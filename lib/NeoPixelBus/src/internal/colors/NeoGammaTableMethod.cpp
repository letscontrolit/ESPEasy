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
#include "NeoGammaTableMethod.h"

const uint8_t NeoGammaTableMethod::_table[] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   // 16
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,   // 32 
    3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   5,   5,   5,   5,   6,   6,   // 48
    6,   7,   7,   7,   7,   8,   8,   8,   9,   9,   9,   10,  10,  11,  11,  11,  // 64
    12,  12,  13,  13,  14,  14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  19,  // 80
    19,  20,  20,  21,  22,  22,  23,  23,  24,  25,  25,  26,  26,  27,  28,  28,  // 96
    29,  30,  30,  31,  32,  33,  33,  34,  35,  35,  36,  37,  38,  39,  39,  40,  // 112
    41,  42,  43,  43,  44,  45,  46,  47,  48,  49,  50,  50,  51,  52,  53,  54,  // 128
    55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  // 144 
    71,  73,  74,  75,  76,  77,  78,  79,  81,  82,  83,  84,  85,  87,  88,  89,  // 160 
    90,  91,  93,  94,  95,  97,  98,  99, 101, 102, 103, 105, 106, 107, 109, 110,  // 176 
    111, 113, 114, 116, 117, 119, 120, 122, 123, 125, 126, 128, 129, 131, 132, 134, // 192
    135, 137, 138, 140, 142, 143, 145, 146, 148, 150, 151, 153, 155, 156, 158, 160, // 208
    162, 163, 165, 167, 168, 170, 172, 174, 176, 177, 179, 181, 183, 185, 187, 189, // 224
    190, 192, 194, 196, 198, 200, 202, 203, 206, 207, 210, 212, 214, 216, 218, 220, // 240
    222, 224, 226, 228, 230, 232, 234, 237, 239, 241, 243, 245, 247, 250, 251, 255  // 256 
};