/*************************************************** 
  This is a library for the I2C communication for Sessami
  Devided from CAP1188 breakout from Adafruit

  These sensors use I2C/SPI to communicate, 2+ pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/
 /***************************************************
 Title : I2C class for sensor
 Modified by : Kaz Wong
 Begin Date : 7 Dec 2016
 Description : I2C communication class for sensor
 ****************************************************/

#ifndef I2CSENSOR_H_
#define I2CSENSOR_H_

#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>

#define B0 B00000001 //1
#define B1 B00000010 //2
#define B2 B00000100 //4
#define B3 B00001000 //8
#define B4 B00010000 //16
#define B5 B00100000 //32
#define B6 B01000000 //64
#define B7 B10000000 //128
#define HB 0x100
#define HBMASK 0xFF

typedef bool State;

#define HI true
#define LO false

class I2CSensor {
private:
	int addr;

protected:
	int GetAddr() {
		return addr;
	}
	I2CSensor(int _addr);
	~I2CSensor();

	virtual bool initWireI2C() = 0;
	uint8_t I2CRead();
	void I2CWrite(uint8_t *reg, unsigned int size);
	
	virtual uint8_t ReadRegister(uint8_t reg);
	virtual void WriteRegister(uint8_t reg, uint8_t value);
	virtual uint8_t* ReadRegister(uint8_t reg, uint8_t read_size);
	void WriteRegBit(uint8_t reg, uint8_t xbit, State st);
	virtual void WriteRegBit(uint8_t reg, uint8_t xbit, State st, uint8_t *byte);
	
};

#endif
