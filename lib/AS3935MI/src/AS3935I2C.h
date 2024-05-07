//Yet Another Arduino ams AS3935 'Franklin' lightning sensor library 
// Copyright (c) 2018-2019 Gregor Christandl <christandlg@yahoo.com>
// home: https://bitbucket.org/christandlg/as3935mi
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef AS3935I2C_H_
#define AS3935I2C_H_

#include "AS3935TwoWire.h"

class AS3935I2C :
	public AS3935TwoWire
{
public:
	AS3935I2C(uint8_t address, uint8_t irq);
	virtual ~AS3935I2C();
};

#endif /* AS3935I2C_H_ */
