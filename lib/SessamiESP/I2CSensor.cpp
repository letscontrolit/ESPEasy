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
 
#include "I2CSensor.h"

/**************************************************************************/
/*!
 @brief  Abstract away platform differences in Arduino wire library
 */
/**************************************************************************/
uint8_t I2CSensor::I2CRead() {
#if ARDUINO >= 100
	return Wire.read();
#else
	return Wire.receive();
#endif
}

/**************************************************************************/
/*!
 @brief  Abstract away platform differences in Arduino wire library
 */
/**************************************************************************/
void I2CSensor::I2CWrite(uint8_t *reg, unsigned int size) {
	Wire.beginTransmission(addr);
	for (int i=0; i<size; i++)
#if ARDUINO >= 100
	Wire.write((uint8_t) reg[i]);
#else
	Wire.send(reg[i]);
#endif
	Wire.endTransmission();
}



uint8_t I2CSensor::ReadRegister(uint8_t reg) {
	Wire.beginTransmission(addr);
	I2CWrite(&reg, 1);
	Wire.endTransmission();
	Wire.requestFrom(addr, 1);
  return (I2CRead());
}

//TODO - Modify I2CSensor - change functions to protected and virual, and BlockRead, BlockWrite, WriteBit.
uint8_t* I2CSensor::ReadRegister(uint8_t reg, uint8_t read_size) {
	uint8_t *data = new uint8_t[read_size];

	Wire.beginTransmission(addr);
	I2CWrite(&reg, 1);
	Wire.endTransmission();
 
  delay(100);
 
	Wire.requestFrom((uint8_t)addr, read_size, false);

  int counter = 0;
	while(Wire.available() < read_size) {
    delay(1);
    counter ++;
    if (counter >100)
      return data;
  }
  
  data[0] = I2CRead();
  data[1] = I2CRead(); 

	return data;
}

void I2CSensor::WriteRegister(uint8_t reg, uint8_t value) {
	uint8_t tmp[2];
	tmp[0] = reg;
	tmp[1] = value;
	I2CWrite(tmp, 2);
}

void I2CSensor::WriteRegBit(uint8_t reg, uint8_t xbit, State st) {
	uint8_t tmp = ReadRegister(reg);

	if (st)
		tmp = tmp | xbit;
	else
		tmp = tmp & ~xbit;
	
	WriteRegister(reg, tmp);
}

void I2CSensor::WriteRegBit(uint8_t reg, uint8_t xbit, State st, uint8_t *byte) {
	//uint8_t tmp = ReadRegister(reg);

	if (st)
		*byte = *byte | xbit;
	else
		*byte = *byte & ~xbit;
	
	WriteRegister(reg, *byte);
}

I2CSensor::I2CSensor(int _addr) :
		addr(_addr) {
}

I2CSensor::~I2CSensor() {
}


