/***************************************************
 Title : Driver class for Si7020-A20
 Modified by : Kaz Wong
 Begin Date : 20 Feb 2017
 Datasheet Link : https://www.silabs.com/documents/public/data-sheets/Si7020-A20.pdf
 Description : Class for read temperature and Humidity
 ****************************************************/
#ifndef Si7020_H_
#define Si7020_H_

#include <Arduino.h>
#include "I2CSensor.h"

class Si7020 : private I2CSensor {
private:
	static float humidity;
	static float temperature;
public:
	Si7020();
	~Si7020();

  bool initWireI2C();
  uint8_t ReadUserReg1();
  uint8_t ReadHCReg();
	void UpdateRH();
	void UpdateTp();
	
	float GetRH();
	float GetTp();
};

#endif
