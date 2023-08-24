/*
  This is a library written for the VL53L1X I2C Distance sensor.

  Written by Andy England @ SparkFun Electronics, October 17th, 2017

  The sensor uses I2C to communicate, as well as a single (optional)
  interrupt line that is not currently supported in this driver.

  https://github.com/sparkfun/SparkFun_VL53L1X_Arduino_Library

  Do you like this library? Help support SparkFun. Buy a board!

  Development environment specifics:
  Arduino IDE 1.8.1

	==== MIT License ====	
	Copyright © 2022 SparkFun Electronics

	Permission is hereby granted, free of charge, to any person obtaining a copy of this 
	software and associated documentation files (the “Software”), to deal in the Software without restriction, 
	including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
	and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
	WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS 
	OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
	TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
	=====================	
*/

#include <stdlib.h>
#include "SparkFun_VL53L1X.h"

SFEVL53L1X::SFEVL53L1X()
{
	_i2cPort = &Wire;
	_shutdownPin = -1;
	_interruptPin = -1;
	_device = new VL53L1X(&Wire, -1, -1);
}
SFEVL53L1X::SFEVL53L1X(TwoWire &i2cPort, int shutdownPin, int interruptPin)
{
	_i2cPort = &i2cPort;
	_shutdownPin = shutdownPin;
	_interruptPin = interruptPin;
	_device = new VL53L1X(&i2cPort, shutdownPin, interruptPin);
}
SFEVL53L1X::~SFEVL53L1X()
{
	delete (VL53L1X *)_device;
}

bool SFEVL53L1X::init()
{
	return _device->VL53L1X_SensorInit();
}

bool SFEVL53L1X::begin()
{
	// if (checkID() == false)
	// 	return (VL53L1_ERROR_PLATFORM_SPECIFIC_START);

	return _device->VL53L1X_SensorInit();
}

bool SFEVL53L1X::begin(TwoWire &i2cPort)
{
	_i2cPort = &i2cPort;
	_device->dev_i2c = &i2cPort;

	return begin();
}

/*Checks the ID of the device, returns true if ID is correct*/

bool SFEVL53L1X::checkID()
{
	uint16_t sensorId;
	_device->VL53L1X_GetSensorId(&sensorId);
	if ((sensorId == 0xEACC) || (sensorId == 0xEBAA))
		return true;
	return false;
}

uint16_t SFEVL53L1X::getID() {
	uint16_t sensorId;
	_device->VL53L1X_GetSensorId(&sensorId);
	return sensorId;
}
/*Turns the sensor on if the Shutdown pin is connected*/

void SFEVL53L1X::sensorOn()
{
	if (_shutdownPin >= 0)
	{
		digitalWrite(_shutdownPin, HIGH);
	}
	delay(10);
}

/*Turns the sensor off if the Shutdown pin is connected*/

void SFEVL53L1X::sensorOff()
{
	if (_shutdownPin >= 0)
	{
		digitalWrite(_shutdownPin, LOW);
	}
	delay(10);
}

/*Gets the software version number of the current library installed.*/

VL53L1X_Version_t SFEVL53L1X::getSoftwareVersion()
{
	VL53L1X_Version_t tempVersion;
	_device->VL53L1X_GetSWVersion(&tempVersion);
	return tempVersion;
}

void SFEVL53L1X::setI2CAddress(uint8_t addr)
{
	_i2cAddress = addr;
	_device->VL53L1X_SetI2CAddress(addr);
}

int SFEVL53L1X::getI2CAddress()
{
	return _i2cAddress;
}

void SFEVL53L1X::clearInterrupt()
{
	_device->VL53L1X_ClearInterrupt();
}

void SFEVL53L1X::setInterruptPolarityHigh()
{
	_device->VL53L1X_SetInterruptPolarity(1);
}

void SFEVL53L1X::setInterruptPolarityLow()
{
	_device->VL53L1X_SetInterruptPolarity(0);
}

/**
 * This function gets the interrupt polarity\n
 * 1=active high (default), 0=active low
 */

uint8_t SFEVL53L1X::getInterruptPolarity()
{
	uint8_t tmp;
	_device->VL53L1X_GetInterruptPolarity(&tmp);
	return tmp;
}

void SFEVL53L1X::startRanging()
{
	_device->VL53L1X_StartRanging();
}

void SFEVL53L1X::startOneshotRanging() 
{
  	_device->VL53L1X_StartOneshotRanging();
}

void SFEVL53L1X::stopRanging()
{
	_device->VL53L1X_StopRanging();
}

bool SFEVL53L1X::checkForDataReady()
{
	uint8_t dataReady;
	_device->VL53L1X_CheckForDataReady(&dataReady);
	return (bool)dataReady;
}

void SFEVL53L1X::setTimingBudgetInMs(uint16_t timingBudget)
{
	_device->VL53L1X_SetTimingBudgetInMs(timingBudget);
}

uint16_t SFEVL53L1X::getTimingBudgetInMs()
{
	uint16_t timingBudget;
	_device->VL53L1X_GetTimingBudgetInMs(&timingBudget);
	return timingBudget;
}

void SFEVL53L1X::setDistanceModeLong()
{
	_device->VL53L1X_SetDistanceMode(DISTANCE_LONG);
}

void SFEVL53L1X::setDistanceModeShort()
{
	_device->VL53L1X_SetDistanceMode(DISTANCE_SHORT);
}

uint8_t SFEVL53L1X::getDistanceMode()
{
	uint16_t distanceMode;
	_device->VL53L1X_GetDistanceMode(&distanceMode);
	return distanceMode;
}

void SFEVL53L1X::setIntermeasurementPeriod(uint16_t intermeasurement)
{
	_device->VL53L1X_SetInterMeasurementInMs(intermeasurement);
}

uint16_t SFEVL53L1X::getIntermeasurementPeriod()
{
	uint16_t intermeasurement;
	_device->VL53L1X_GetInterMeasurementInMs(&intermeasurement);
	return intermeasurement;
}

bool SFEVL53L1X::checkBootState()
{
	uint8_t bootState;
	_device->VL53L1X_BootState(&bootState);
	return (bool)bootState;
}

uint16_t SFEVL53L1X::getSensorID()
{
	uint16_t id;
	_device->VL53L1X_GetSensorId(&id);
	return id;
}

uint16_t SFEVL53L1X::getDistance()
{
	uint16_t distance;
	_device->VL53L1X_GetDistance(&distance);
	return (int)distance;
}

uint16_t SFEVL53L1X::getSignalPerSpad()
{
	uint16_t temp;
	_device->VL53L1X_GetSignalPerSpad(&temp);
	return temp;
}

uint16_t SFEVL53L1X::getAmbientPerSpad()
{
	uint16_t temp;
	_device->VL53L1X_GetAmbientPerSpad(&temp);
	return temp;
}

uint16_t SFEVL53L1X::getSignalRate()
{
	uint16_t temp;
	_device->VL53L1X_GetSignalRate(&temp);
	return temp;
}

uint16_t SFEVL53L1X::getSpadNb()
{
	uint16_t temp;
	_device->VL53L1X_GetSpadNb(&temp);
	return temp;
}

uint16_t SFEVL53L1X::getAmbientRate()
{
	uint16_t temp;
	_device->VL53L1X_GetAmbientRate(&temp);
	return temp;
}

uint8_t SFEVL53L1X::getRangeStatus()
{
	uint8_t temp;
	_device->VL53L1X_GetRangeStatus(&temp);
	return temp;
}

void SFEVL53L1X::setOffset(int16_t offset)
{
	_device->VL53L1X_SetOffset(offset);
}

int16_t SFEVL53L1X::getOffset()
{
	int16_t temp;
	_device->VL53L1X_GetOffset(&temp);
	return temp;
}

void SFEVL53L1X::setXTalk(uint16_t xTalk)
{
	_device->VL53L1X_SetXtalk(xTalk);
}

uint16_t SFEVL53L1X::getXTalk()
{
	uint16_t temp;
	_device->VL53L1X_GetXtalk(&temp);
	return temp;
}

void SFEVL53L1X::setDistanceThreshold(uint16_t lowThresh, uint16_t hiThresh, uint8_t window)
{
	_device->VL53L1X_SetDistanceThreshold(lowThresh, hiThresh, window, 1);
}

uint16_t SFEVL53L1X::getDistanceThresholdWindow()
{
	uint16_t temp;
	_device->VL53L1X_GetDistanceThresholdWindow(&temp);
	return temp;
}

uint16_t SFEVL53L1X::getDistanceThresholdLow()
{
	uint16_t temp;
	_device->VL53L1X_GetDistanceThresholdLow(&temp);
	return temp;
}

uint16_t SFEVL53L1X::getDistanceThresholdHigh()
{
	uint16_t temp;
	_device->VL53L1X_GetDistanceThresholdHigh(&temp);
	return temp;
}

void SFEVL53L1X::setROI(uint8_t x, uint8_t y, uint8_t opticalCenter)
{
	_device->VL53L1X_SetROI(x, y, opticalCenter);
}

uint16_t SFEVL53L1X::getROIX()
{
	uint16_t tempX;
	uint16_t tempY;
	_device->VL53L1X_GetROI_XY(&tempX, &tempY);
	return tempX;
}

uint16_t SFEVL53L1X::getROIY()
{
	uint16_t tempX;
	uint16_t tempY;
	_device->VL53L1X_GetROI_XY(&tempX, &tempY);
	return tempY;
}

void SFEVL53L1X::setSignalThreshold(uint16_t signalThreshold)
{
	_device->VL53L1X_SetSignalThreshold(signalThreshold);
}

uint16_t SFEVL53L1X::getSignalThreshold()
{
	uint16_t temp;
	_device->VL53L1X_GetSignalThreshold(&temp);
	return temp;
}

void SFEVL53L1X::setSigmaThreshold(uint16_t sigmaThreshold)
{
	_device->VL53L1X_SetSigmaThreshold(sigmaThreshold);
}

uint16_t SFEVL53L1X::getSigmaThreshold()
{
	uint16_t temp;
	_device->VL53L1X_GetSigmaThreshold(&temp);
	return temp;
}

void SFEVL53L1X::startTemperatureUpdate()
{
	_device->VL53L1X_StartTemperatureUpdate();
}

void SFEVL53L1X::calibrateOffset(uint16_t targetDistanceInMm)
{
	int16_t offset = getOffset();
	_device->VL53L1X_CalibrateOffset(targetDistanceInMm, &offset);
}

void SFEVL53L1X::calibrateXTalk(uint16_t targetDistanceInMm)
{
	uint16_t xTalk = getXTalk();
	_device->VL53L1X_CalibrateXtalk(targetDistanceInMm, &xTalk);
};

bool SFEVL53L1X::setThresholdConfig(DetectionConfig *config)
{
	return _device->VL53L1X_SetDistanceMode(config->distanceMode) == VL53L1_ERROR_NONE &&
	       _device->VL53L1X_SetDistanceThreshold(config->thresholdLow, config->thresholdHigh,
	       	(uint8_t)config->windowMode, (uint8_t)config->IntOnNoTarget) == VL53L1_ERROR_NONE;
}

bool SFEVL53L1X::getThresholdConfig(DetectionConfig *config)
{
	uint16_t temp16 = 0;

	VL53L1X_ERROR error = _device->VL53L1X_GetDistanceMode(&temp16);
	if (error != 0)
		return false;
	else
		config->distanceMode = temp16;

	error = _device->VL53L1X_GetDistanceThresholdWindow(&temp16);
	if (error != 0)
		return false;
	else
		config->windowMode = temp16;

	config->IntOnNoTarget = 1;

	error = _device->VL53L1X_GetDistanceThresholdLow(&temp16);
	if (error != 0)
		return false;
	else
		config->thresholdLow = temp16;

	error = _device->VL53L1X_GetDistanceThresholdHigh(&temp16);
	if (error != 0)
		return false;
	else
		config->thresholdHigh = temp16;

	return true;
}
