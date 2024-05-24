/*-------------------------------------------------------------------------
ColumnMajorLayout provides a collection of class objects that are used with NeoTopology
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


class ColumnMajorLayout;
class ColumnMajor90Layout;
class ColumnMajor180Layout;
class ColumnMajor270Layout;

class ColumnMajorTilePreference
{
public:
    typedef ColumnMajorLayout EvenRowEvenColumnLayout;
    typedef ColumnMajor270Layout EvenRowOddColumnLayout;
    typedef ColumnMajor90Layout OddRowEvenColumnLayout;
    typedef ColumnMajor180Layout OddRowOddColumnLayout;
};

// layout example of 4x4
// 00  04  08  12
// 01  05  09  13
// 02  06  10  14
// 03  07  11  15
//
class ColumnMajorLayout : public ColumnMajorTilePreference
{
public:
    static uint16_t Map(uint16_t /* width */, uint16_t height, uint16_t x, uint16_t y)
    {
        return x * height + y;
    }
};

// layout example of 4x4
// 03  02  01  00
// 07  06  05  04
// 11  10  09  08
// 15  14  13  12
//
class ColumnMajor90Layout : public ColumnMajorTilePreference
{
public:
    static uint16_t Map(uint16_t width, uint16_t /* height */, uint16_t x, uint16_t y)
    {
        return (width - 1 - x) + y * width;
    }
};

// layout example of 4x4
// 15  11  07  03
// 14  10  06  02
// 13  09  05  01
// 12  08  04  00
//
class ColumnMajor180Layout : public ColumnMajorTilePreference
{
public:
    static uint16_t Map(uint16_t width, uint16_t height, uint16_t x, uint16_t y)
    {
        return (width - 1 - x) * height + (height - 1 - y);
    }
};

// layout example of 4x4
// 12  13  14  15
// 08  09  10  11
// 04  05  06  07
// 00  01  02  03
//
class ColumnMajor270Layout : public ColumnMajorTilePreference
{
public:
    static uint16_t Map(uint16_t width, uint16_t height, uint16_t x, uint16_t y)
    {
        return x + (height - 1 - y) * width;
    }
};
