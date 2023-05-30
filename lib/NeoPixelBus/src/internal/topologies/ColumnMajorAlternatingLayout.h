/*-------------------------------------------------------------------------
ColumnMajorAlternatingLayout provides a collection of class objects that are used with NeoTopology
object.
They define the specific layout of pixels and do the math to change the 2d
cordinate space to 1d cordinate space

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


class ColumnMajorAlternatingLayout;
class ColumnMajorAlternating180Layout;

class ColumnMajorAlternatingTilePreference
{
public:
    typedef ColumnMajorAlternatingLayout EvenRowEvenColumnLayout;
    typedef ColumnMajorAlternatingLayout EvenRowOddColumnLayout;
    typedef ColumnMajorAlternating180Layout OddRowEvenColumnLayout;
    typedef ColumnMajorAlternating180Layout OddRowOddColumnLayout;
};

// layout example of 4x4
// 00  07  08  15  
// 01  06  09  14
// 02  05  10  13
// 03  04  11  12
//
class ColumnMajorAlternatingLayout : public ColumnMajorAlternatingTilePreference
{
public:
    static uint16_t Map(uint16_t /* width */, uint16_t height, uint16_t x, uint16_t y)
    {
        uint16_t index = x * height;

        if (x & 0x0001)
        {
            index += ((height - 1) - y);
        }
        else
        {
            index += y;
        }
        return index;
    }
};

// layout example of 4x4
// 03  02  01  00  
// 04  05  06  07
// 11  10  09  08
// 12  13  14  15
//
class ColumnMajorAlternating90Layout : public ColumnMajorAlternatingTilePreference
{
public:
    static uint16_t Map(uint16_t width, uint16_t /* height */, uint16_t x, uint16_t y)
    {
        uint16_t index = y * width;

        if (y & 0x0001)
        {
            index += x;
        }
        else
        {
            index += ((width - 1) - x);
        }
        return index;
    }
};

// layout example of 4x4
// 12  11  04  03  
// 13  10  05  02
// 14  09  06  01
// 15  08  07  00
//
class ColumnMajorAlternating180Layout : public ColumnMajorAlternatingTilePreference
{
public:
    static uint16_t Map(uint16_t width, uint16_t height, uint16_t x, uint16_t y)
    {
        uint16_t mx = ((width - 1) - x);
        uint16_t index = mx * height;

        if (mx & 0x0001)
        {
            index += y;
        }
        else
        {
            index += ((height - 1) - y);
        }
        return index;
    }
};

// layout example of 4x4
// 15  14  13  12  
// 08  09  10  11
// 07  06  05  04
// 00  01  02  03
//
class ColumnMajorAlternating270Layout : public ColumnMajorAlternatingTilePreference
{
public:
    static uint16_t Map(uint16_t width, uint16_t height, uint16_t x, uint16_t y)
    {
        uint16_t my = ((height - 1) - y);
        uint16_t index = my * width;

        if (my & 0x0001)
        {
            index += ((width - 1) - x);
        }
        else
        {
            index += x;
        }
        return index;
    }
};
