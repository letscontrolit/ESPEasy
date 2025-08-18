/**

AT24CX.cpp
Library for using the EEPROM AT24C32/64

Copyright (c) 2014 Christian Paul

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

 */
#include "AT24CX.h"
#include <Wire.h>

/**
 * Constructor with AT24Cx EEPROM at index 0
 */
AT24CX::AT24CX() {
	init(0, 32, 4096);
}

/**
 * Constructor with AT24Cx EEPROM at given index, size of page and max size in bytes
 */
AT24CX::AT24CX(uint8_t index, uint8_t pageSize, uint32_t maxSize) {
	init(index, pageSize, maxSize);
}

/**
 * Constructor with AT24C32 EEPROM at index 0
 */
AT24C32::AT24C32() {
	init(0, 32, 4096);
}
/**
 * Constructor with AT24Cx EEPROM at given index
 */
AT24C32::AT24C32(uint8_t index) {
	init(index, 32, 4096);
}

/**
 * Constructor with AT24C64 EEPROM at index 0
 */
AT24C64::AT24C64() {
	init(0, 32, 8192);
}
/**
 * Constructor with AT24C64 EEPROM at given index
 */
AT24C64::AT24C64(uint8_t index) {
	init(index, 32, 8192);
}

/**
 * Constructor with AT24C128 EEPROM at index 0
 */
AT24C128::AT24C128() {
	init(0, 64, 16384);
}
/**
 * Constructor with AT24C128 EEPROM at given index
 */
AT24C128::AT24C128(uint8_t index) {
	init(index, 64, 16384);
}

/**
 * Constructor with AT24C256 EEPROM at index 0
 */
AT24C256::AT24C256() {
	init(0, 64, 32768);
}
/**
 * Constructor with AT24C128 EEPROM at given index
 */
AT24C256::AT24C256(uint8_t index) {
	init(index, 64, 32768);
}

/**
 * Constructor with AT24C512 EEPROM at index 0
 */
AT24C512::AT24C512() {
	init(0, 128, 65536);
}
/**
 * Constructor with AT24C512 EEPROM at given index
 */
AT24C512::AT24C512(uint8_t index) {
	init(index, 128, 65536);
}

/**
 * Init
 */
void AT24CX::init(uint8_t index, uint8_t pageSize, uint32_t maxSize) {
	_id = AT24CX_ID | (index & 0x7);
	_pageSize = pageSize;
	_maxSize = maxSize;
	// Wire.begin();
}

/**
 * Check address not reached
 */
bool AT24CX::checkSize(uint32_t address, uint32_t size) {
  return (address + size - 1) <= _maxSize;
}

/**
 * Write byte
 */
void AT24CX::write(uint32_t address, uint8_t data) {
	if (!checkSize(address, 1)) {
		return;
	}
	Wire.beginTransmission(_id);
	if(Wire.endTransmission()==0) {
		Wire.beginTransmission(_id);
		Wire.write(address >> 8);
		Wire.write(address & 0xFF);
			Wire.write(data);
		Wire.endTransmission();
		delay(20);
	}
}

/**
 * Write integer
 */
void AT24CX::writeInt(uint32_t address, uint16_t data) {
	write(address, (uint8_t*)&data, 2);
}

/**
 * Write long
 */
void AT24CX::writeLong(uint32_t address, uint32_t data) {
	write(address, (uint8_t*)&data, 4);
}

/**
 * Write float
 */
void AT24CX::writeFloat(uint32_t address, float data) {
	write(address, (uint8_t*)&data, 4);
}

/**
 * Write double
 */
void AT24CX::writeDouble(uint32_t address, double data) {
	write(address, (uint8_t*)&data, 8);
}

/**
 * Write chars
 */
void AT24CX::writeChars(uint32_t address, char *data, int length) {
	write(address, (uint8_t*)data, length);
}

/**
 * Write bytes
 */
void AT24CX::writeBytes(uint32_t address, uint8_t *data, int length) {
	write(address, data, length);
}

/**
 * Read integer
 */
uint16_t AT24CX::readInt(uint32_t address) {
	memset(_b, 0, sizeof(_b));
	read(address, _b, 2);
	return *(uint32_t*)&_b[0];
}

/**
 * Read long
 */
uint32_t AT24CX::readLong(uint32_t address) {
	read(address, _b, 4);
	return *(unsigned long*)&_b[0];
}

/**
 * Read float
 */
float AT24CX::readFloat(uint32_t address) {
	read(address, _b, 4);
	return *(float*)&_b[0];
}

/**
 * Read double
 */
double AT24CX::readDouble(uint32_t address) {
	read(address, _b, 8);
	return *(double*)&_b[0];
}

/**
 * Read chars
 */
void AT24CX::readChars(uint32_t address, char *data, int n) {
	read(address, (uint8_t*)data, n);
}

/**
 * Read bytes
 */
void AT24CX::readBytes(uint32_t address, uint8_t *data, int n) {
	read(address, data, n);
}

/**
 * Write sequence of n bytes
 */
void AT24CX::write(uint32_t address, uint8_t *data, int n) {
	if (!checkSize(address, n)) {
		return;
	}
	// status quo
	int c = n;						// bytes left to write
	int offD = 0;					// current offset in data pointer
	int offP;						// current offset in page
	int nc = 0;						// next n bytes to write

	// write alle bytes in multiple steps
	while (c > 0) {
		// calc offset in page
		offP = address % _pageSize;
		// maximal 30 bytes to write
		nc = min(min(c, 30), _pageSize - offP);
		write(address, data, offD, nc);
		c-=nc;
		offD+=nc;
		address+=nc;
	}
}

/**
 * Write sequence of n bytes from offset
 */
void AT24CX::write(uint32_t address, uint8_t *data, int offset, int n) {
	if (!checkSize(address, n)) {
		return;
	}
    Wire.beginTransmission(_id);
    if (Wire.endTransmission()==0) {
     	Wire.beginTransmission(_id);
    	Wire.write(address >> 8);
    	Wire.write(address & 0xFF);
    	uint8_t *adr = data+offset;
    	Wire.write(adr, n);
    	Wire.endTransmission();
    	delay(20);
    }
}

/**
 * Read byte
 */
uint8_t AT24CX::read(uint32_t address) {
	if (!checkSize(address, 1)) {
		return 0;
	}
	uint8_t b = 0;
	int r = 0;
	Wire.beginTransmission(_id);
	if (Wire.endTransmission()==0) {
		Wire.beginTransmission(_id);
		Wire.write(address >> 8);
		Wire.write(address & 0xFF);
		if (Wire.endTransmission()==0) {
		Wire.requestFrom(_id, 1);
		while (Wire.available() > 0 && r<1) {
			b = (uint8_t)Wire.read();
			r++;
		}
		}
	}
	return b;
}

/**
 * Read sequence of n bytes
 */
void AT24CX::read(uint32_t address, uint8_t *data, int n) {
	if (!checkSize(address, n)) {
		return;
	}
	int c = n;
	int offD = 0;
	// read until are n bytes read
	while (c > 0) {
		// read maximal 32 bytes
		int nc = c;
		if (nc > 32)
			nc = 32;
		read(address, data, offD, nc);
		address+=nc;
		offD+=nc;
		c-=nc;
	}
}


/**
 * Read sequence of n bytes to offset
 */
void AT24CX::read(uint32_t address, uint8_t *data, int offset, int n) {
	Wire.beginTransmission(_id);
	if (Wire.endTransmission()==0) {
		Wire.beginTransmission(_id);
		Wire.write(address >> 8);
		Wire.write(address & 0xFF);
		if (Wire.endTransmission()==0) {
		int r = 0;
			Wire.requestFrom(_id, n);
		while (Wire.available() > 0 && r<n) {
			data[offset+r] = (uint8_t)Wire.read();
			r++;
		}
		}
	}
}

