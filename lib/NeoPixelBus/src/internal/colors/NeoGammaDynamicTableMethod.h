/*-------------------------------------------------------------------------
NeoGammaDynamicTableMethod class is used to correct RGB colors for human eye gamma levels equally
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

#if defined(NEOPIXEBUS_NO_STL)

typedef float(*GammaCalcFunction)(float unitValue);

#else

#undef max
#undef min
#include <functional>
typedef std::function<float(float unitValue)> GammaCalcFunction;

#endif

class NeoGammaDynamicTableMethod
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
        uint8_t hi = (value >> 8);
        uint16_t lo = (value & 0x00ff);
        uint8_t hiResult = _table[hi];
        uint16_t lowResult = 0;

        if (hi < _hintsCount)
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

    static void Initialize(GammaCalcFunction calc, bool optimize16Bit = false)
    {
        if (_hints)
        {
            delete [] _hints;
            _hints = nullptr;
        }

        // first, iterate and fill 8 bit table
        for (uint16_t entry = 0; entry < 256; entry++)
        {
            _table[entry] = static_cast<uint8_t>(255.0f * calc(entry / 255.0f) + 0.5f);
        }

        // no optimization, so no 16 bit hints table
        if (optimize16Bit)
        {
            NeoGamma16LowHint hints[256];

            // now walk table creating an optimized hint table for 16bit
            // approximation
            // There is an assumption that lower values have series of duplicates and
            // upper values at worst have two values the same
            //
            uint16_t entryStart = 0;
            uint16_t entryEnd = 0;
            uint16_t entryLastTriplet = 0;

            while (entryStart < 255)
            {
                uint8_t value = _table[entryStart];

                while (value == _table[entryEnd] && entryEnd < 255)
                {
                    entryEnd++;
                }

                if (entryEnd == entryStart)
                {
                    // no more duplicates, no need to continue
                    break;
                }
                
                uint8_t pos = 0;
                uint8_t count = entryEnd - entryStart;

                if (count >= 3)
                {
                    // remember the last triplet + series
                    // as we don't need hints after this
                    // there can be paired duplicates after and before this 
                    entryLastTriplet = entryEnd;
                }

                // fill hints with known duplicate value
                while (entryStart != entryEnd)
                {
                    hints[entryStart].count = count;
                    hints[entryStart].pos = pos;
                    entryStart++;
                    pos++;
                }
            }
            // create static hint table and copy temp table to it
            _hintsCount = entryLastTriplet; // only need to last triplet
            _hints = new NeoGamma16LowHint[_hintsCount];
            memcpy(_hints, hints, sizeof(NeoGamma16LowHint) * _hintsCount);
        }
    }

    // SerialDumpTables is used if you want to generate your own static gamma table class
    // rather than use this dynamically generated table.  Just capture the serial output
    // and use as your initializers for your tables
    static void SerialDumpTables()
    {
        Serial.println();
        Serial.println("8 bit:");
        for (uint16_t entry = 0; entry < 256; entry++)
        {
            if (entry % 16 == 0)
            {
                Serial.println();
            }
            Serial.print(_table[entry]);
            Serial.print(",  ");
        }

        Serial.println();
        Serial.println();
        Serial.print("16 bit: hintsCount = ");
        Serial.println(_hintsCount);
        if (_hints)
        {
            for (uint8_t hint = 0; hint < _hintsCount; hint++)
            {
                if (hint % 16 == 0)
                {
                    Serial.println();
                }
                Serial.print("{");
                Serial.print(_hints[hint].pos);
                Serial.print(",");
                Serial.print(_hints[hint].count);
                Serial.print("}, ");
            }
        }
        Serial.println();
    }

private:
    static uint8_t _table[256];
    static NeoGamma16LowHint* _hints;
    static uint8_t _hintsCount;
};
