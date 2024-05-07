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

#ifndef AS3935SPICLASS_H_
#define AS3935SPICLASS_H_

#include "AS3935MI.h"

#include <Arduino.h>

#include <SPI.h>

class AS3935SPIClass :
	public AS3935MI
{
public:
	AS3935SPIClass(SPIClass *spi, uint8_t cs, uint8_t irq);
	virtual ~AS3935SPIClass();

protected:
	SPIClass *spi_;

	uint8_t cs_;

	static SPISettings spi_settings_;     //spi settings object. is the same for all AS3935 sensors

private:
	virtual bool beginInterface();

	virtual uint8_t readRegister(uint8_t reg);

	virtual void writeRegister(uint8_t reg, uint8_t value);
};

#endif /* AS3935SPICLASS_H_ */
