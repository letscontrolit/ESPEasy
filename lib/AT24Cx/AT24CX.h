/**

AT24CX.h
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

 /**
	* Changelog:
	* 2025-08-18 tonhuisman: Add init() argument for maxSize
	*												 Check maxSize before reading or writing
	*												 Use modern uint types instead of byte, unsigned int and unsigned long
	*												 Disable Wire.begin() call, I2C should be initialized before creating this object
	* 2025-08-18 Forked from https://github.com/cyberp/AT24Cx
  */
 
#ifndef AT24CX_h
#define AT24CX_h

// includes
#include <Arduino.h>
#include <stdint.h>

// // byte
// typedef uint8_t byte;

// AT24Cx I2C adress
// 80
// 0x50
#define AT24CX_ID 0b01010000

// general class definition
class AT24CX {
public:
	AT24CX();
	AT24CX(uint8_t index, uint8_t pageSize, uint32_t maxSize);
	void write(uint32_t address, uint8_t data);
	void write(uint32_t address, uint8_t *data, int n);
	void writeInt(uint32_t address, uint16_t data);
	void writeLong(uint32_t address, uint32_t data);
	void writeFloat(uint32_t address, float data);
	void writeDouble(uint32_t address, double data);
	void writeChars(uint32_t address, char *data, int length);
	void writeBytes(uint32_t address, uint8_t *data, int length);
	uint8_t read(uint32_t address);
	void read(uint32_t address, uint8_t *data, int n);
	uint16_t readInt(uint32_t address);
	uint32_t readLong(uint32_t address);
	float readFloat(uint32_t address);
	double readDouble(uint32_t address);
	void readChars(uint32_t address, char *data, int n);
	void readBytes(uint32_t address, uint8_t *data, int n);
protected:
	void init(uint8_t index, uint8_t pageSize, uint32_t maxSize);
private:
	void read(uint32_t address, uint8_t *data, int offset, int n);
	void write(uint32_t address, uint8_t *data, int offset, int n);
	bool checkSize(uint32_t address, uint32_t size);
	int _id;
	uint8_t _b[8];
	uint8_t _pageSize;
	uint32_t _maxSize{};
};

// AT24C32 class definiton
class AT24C32 : public AT24CX {
public:
	AT24C32();
	AT24C32(uint8_t index);
};

// AT24C64 class definiton
class AT24C64 : public AT24CX {
public:
	AT24C64();
	AT24C64(uint8_t index);
};

// AT24C128 class definiton
class AT24C128 : public AT24CX {
public:
	AT24C128();
	AT24C128(uint8_t index);
};

// AT24C256 class definiton
class AT24C256 : public AT24CX {
public:
	AT24C256();
	AT24C256(uint8_t index);
};

// AT24C512 class definiton
class AT24C512 : public AT24CX {
public:
	AT24C512();
	AT24C512(uint8_t index);
};



#endif
