/*-------------------------------------------------------------------------
NeoGammaTableMethod class is used to correct RGB colors for human eye gamma levels equally
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

// NeoGammaTableMethod uses 256 bytes of memory, but is significantly faster
class NeoGammaTableMethod
{
protected:
    struct NeoGamma16LowHint
    {
        uint8_t pos;
        uint8_t count;
    };

public:
    static uint8_t Correct(uint8_t value)
    {
        return _table[value];
    }

    static uint16_t Correct(uint16_t value)
    {
        // since a single monolithic table would be an unreasonable memory usage 
        // this will use a hybrid of two tables, the base 255 table for the hibyte
        // and a smaller table with hints on how to use the table for certain values
        // and the left over values some simple calculations to approximate the final
        // 16 bit value as compared to the equation
        static const NeoGamma16LowHint _hints[] = {
                {0,16}, {1,16}, {2,16}, {3,16}, {4,16}, {5,16}, {6,16}, {7,16}, {8,16}, {9,16}, {10,16}, {11,16}, {12,16}, {13,16}, {14,16}, {15,16},
                {0,10}, {1,10}, {2,10}, {3,10}, {4,10}, {5,10}, {6,10}, {7,10}, {8,10}, {9,10}, {0,6},   {1,6},   {2,6},   {3,6},   {4,6},   {5,6},
                {0,6},  {1,6},  {2,6},  {3,6},  {4,6},  {5,6},  {0,4},  {1,4},  {2,4},  {3,4},  {0,4},   {1,4},   {2,4},   {3,4},   {0,3},   {1,3},
                {2,3},  {0,4},  {1,4},  {2,4},  {3,4},  {0,3},  {1,3},  {2,3},  {0,3},  {1,3},  {2,3},   {0,2},   {1,2},   {0,3},   {1,3},   {2,3},
                {0,2},  {1,2},  {0,2},  {1,2},  {0,3},  {1,3},  {2,3},  {0,2},  {1,2}
        };

        uint8_t hi = (value >> 8);
        uint16_t lo = (value & 0x00ff);
        uint8_t hiResult = _table[hi];
        uint16_t lowResult = 0;

        if (hi < countof(_hints))
        {
            // use _hints table to calculate a reasonable lowbyte
            lowResult = (lo + _hints[hi].pos * 256) / _hints[hi].count;
        }
        else if (hi == 255)
        {
            // last entry is always linear
            lowResult = lo;
        }
        else
        {
            // check the _table for duplicate or jumps to adjust the range of lowbyte
            if (hiResult == _table[hi - 1])
            {
                // this result is an upper duplicate
                lowResult = (lo >> 1) | 0x80; // lo / 2 + 128
            }
            else
            {
                uint8_t delta = _table[hi + 1] - hiResult;

                if (delta == 0)
                {
                    // this result is a lower duplicate
                    lowResult = (lo >> 1); // lo / 2
                }
                else if (delta == 1)
                {
                    // this result is incremental and thus linear
                    lowResult = lo;
                }
                else
                {
                    // this result jumps by more than one, so need to spread across
                    lowResult = delta * lo;
                }
            }

        }

        return (static_cast<uint16_t>(hiResult) << 8) + lowResult;
    }


private:
    static const uint8_t _table[256];

};
