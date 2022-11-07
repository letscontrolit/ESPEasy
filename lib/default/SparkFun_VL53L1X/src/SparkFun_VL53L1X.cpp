/*
  This is a library written for the VL53L1X I2C Distance sensor.

  Written by Andy England @ SparkFun Electronics, October 17th, 2017

  The sensor uses I2C to communicate, as well as a single (optional)
  interrupt line that is not currently supported in this driver.

  https://github.com/sparkfun/SparkFun_VL53L1X_Arduino_Library

  Do you like this library? Help support SparkFun. Buy a board!

  Development environment specifics:
  Arduino IDE 1.8.1

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include "Arduino.h"
#include "SparkFun_VL53L1X.h"
#include "vl53l1x_class.h"

SFEVL53L1X::SFEVL53L1X(TwoWire &i2cPort, int shutdownPin, int interruptPin)
{
	_i2cPort = &i2cPort;
	_shutdownPin = shutdownPin;
	_interruptPin = interruptPin;
	_device = new VL53L1X(&i2cPort, shutdownPin, interruptPin);
}

bool SFEVL53L1X::init()
{
	return _device->VL53L1X_SensorInit();
}

bool SFEVL53L1X::begin()
{
	if (checkID() == false)
		return (VL53L1_ERROR_PLATFORM_SPECIFIC_START);

	return _device->VL53L1X_SensorInit();
}

/*Checks the ID of the device, returns true if ID is correct*/

bool SFEVL53L1X::checkID()
{
	uint16_t sensorId;
	_device->VL53L1X_GetSensorId(&sensorId);
	return sensorId == 0xEACC;
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
	_device->VL53L1X_SetDistanceMode(2);
}

void SFEVL53L1X::setDistanceModeShort()
{
	_device->VL53L1X_SetDistanceMode(1);
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
