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

#ifndef AS3935TWOWIRE_H_
#define AS3935TWOWIRE_H_

#include "AS3935MI.h"

#include <Arduino.h>

#include <Wire.h>

class AS3935TwoWire :
	public AS3935MI
{
public:
	enum I2C_address_t : uint8_t
	{
		AS3935I2C_A01 = 0b01,
		AS3935I2C_A10 = 0b10,
		AS3935I2C_A11 = 0b11
	};

	AS3935TwoWire(TwoWire *wire, uint8_t address, uint8_t irq);
	virtual ~AS3935TwoWire();

protected:
	TwoWire *wire_;

	uint8_t address_;

private:
	virtual bool beginInterface();

	virtual uint8_t readRegister(uint8_t reg);

	virtual void writeRegister(uint8_t reg, uint8_t value);
};

#endif /* AS3935TWOWIRE_H_ */
