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

#include "AS3935TwoWire.h"

AS3935TwoWire::AS3935TwoWire(TwoWire *wire, uint8_t address, uint8_t irq) :
	AS3935MI(irq),
	wire_(wire),
	address_(address)
{
}

AS3935TwoWire::~AS3935TwoWire()
{
	wire_ = nullptr;
}

bool AS3935TwoWire::beginInterface()
{
	if (!wire_)
		return false;

	switch (address_)
	{
	case AS3935I2C_A01:
	case AS3935I2C_A10:
	case AS3935I2C_A11:
		break;
	default:
		//return false if an invalid I2C address was given.
		return false;
	}

	return true;
}

uint8_t AS3935TwoWire::readRegister(uint8_t reg)
{
	if (!wire_)
		return 0;

#if defined(ARDUINO_SAM_DUE)
	//workaround for Arduino Due. The Due seems not to send a repeated start with the code below, so this 
	//undocumented feature of Wire::requestFrom() is used. can be used on other Arduinos too (tested on Mega2560)
	//see this thread for more info: https://forum.arduino.cc/index.php?topic=385377.0
	wire_->requestFrom(address_, 1, reg, 1, true);
#else
	wire_->beginTransmission(address_);
	wire_->write(reg);
	wire_->endTransmission(false);
	wire_->requestFrom(address_, static_cast<uint8_t>(1));
#endif

	return wire_->read();
}

void AS3935TwoWire::writeRegister(uint8_t reg, uint8_t value)
{
	if (!wire_)
		return;

	wire_->beginTransmission(address_);
	wire_->write(reg);
	wire_->write(value);
	wire_->endTransmission();
}