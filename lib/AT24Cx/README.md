# AT24CX Library

Library for using the Atmels EEPROM AT24C32/AT24C64/AT24C128/AT24C256/AT24C512 in Arduino projects.
See <https://oberguru.net/elektronik/eeprom/at24cx-at24c32-at24c64-at24c128-at24c256-at24c512.html> for definitons and differences.

Written by Christian Paul, 2014 - 2015.
This software is released under the terms of the MIT license.
See the file LICENSE or LIZENZ for details, please.

You can use any of the eight possibles EEPROM devices on the I2C bus.

Constructor 	

	AT24CX(byte pageSize);

uses the device with index 0 and given page size. You can select a device with given index between 0 and 8 with constructor

	AT24CX(byte index, byte pageSize);

Than, you can single write or read single bytes from the EEPROM with

	void write(unsigned int address, byte data);
	byte read(unsigned int address);

or write and read an array of bytes with

	void write(unsigned int address, byte *data, int n);
	void read(unsigned int address, byte *data, int n);

For writing integers, long, float, double or sequences of chars you can use the comfort functions

	void writeInt(unsigned int address, unsigned int data);
	void writeLong(unsigned int address, unsigned long data);
	void writeFloat(unsigned int address, float data);
	void writeDouble(unsigned int address, double data);
	void writeChars(unsigned int address, char *data, int length);
	
Reading the values is done by using

	unsigned int readInt(unsigned int address);
	unsigned long readLong(unsigned int address);
	float readFloat(unsigned int address);
	double readDouble(unsigned int address);
	void readChars(unsigned int address, char *data, int n);
	
Alternative you can use the individual classes with predefined page sizes:

	AT24C32();
	AT24C64();
	AT24C128();
	AT24C256();
	AT24C512();
	
or with different index than 0:

	AT24C32(byte index);
	AT24C64(byte index);
	AT24C128(byte index);
	AT24C256(byte index);
	AT24C512(byte index);

