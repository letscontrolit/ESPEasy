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

#pragma once

#include "Arduino.h"
#include "Wire.h"
#include "RangeSensor.h"
#include "vl53l1_error_codes.h"
#include "vl53l1x_class.h"

class SFEVL53L1X
{
	public:
	SFEVL53L1X(TwoWire &i2cPort = Wire, int shutdownPin = -1, int interruptPin = -1); //Constructs our Distance sensor without an interrupt or shutdown pin
	bool init(); //Deprecated version of begin
	bool begin(); //Initialization of sensor
	bool checkID(); //Check the ID of the sensor, returns true if ID is correct
	void sensorOn(); //Toggles shutdown pin to turn sensor on and off
    void sensorOff(); //Toggles shutdown pin to turn sensor on and off
	VL53L1X_Version_t getSoftwareVersion(); //Get's the current ST software version
	void setI2CAddress(uint8_t addr); //Set the I2C address
	int getI2CAddress(); //Get the I2C address
	void clearInterrupt(); // Clear the interrupt flag
	void setInterruptPolarityHigh(); //Set the polarity of an active interrupt to High
	void setInterruptPolarityLow(); //Set the polarity of an active interrupt to Low
	uint8_t getInterruptPolarity(); //get the current interrupt polarity
	void startRanging(); //Begins taking measurements
	void stopRanging(); //Stops taking measurements
	bool checkForDataReady(); //Checks the to see if data is ready
	void setTimingBudgetInMs(uint16_t timingBudget); //Set the timing budget for a measurement
	uint16_t getTimingBudgetInMs(); //Get the timing budget for a measurement
	void setDistanceModeLong(); //Set to 4M range
	void setDistanceModeShort(); //Set to 1.3M range
	uint8_t getDistanceMode(); //Get the distance mode, returns 1 for short and 2 for long
	void setIntermeasurementPeriod(uint16_t intermeasurement); //Set time between measurements in ms
	uint16_t getIntermeasurementPeriod(); //Get time between measurements in ms
	bool checkBootState(); //Check if the VL53L1X has been initialized
	uint16_t getSensorID(); //Get the sensor ID
	uint16_t getDistance(); //Returns distance
	uint16_t getSignalPerSpad(); //Returns the average signal rate per SPAD (The sensitive pads that detect light, the VL53L1X has a 16x16 array of these) in kcps/SPAD, or kilo counts per second per SPAD.
	uint16_t getAmbientPerSpad(); //Returns the ambient noise when not measuring a signal in kcps/SPAD.
	uint16_t getSignalRate(); //Returns the signal rate in kcps. All SPADs combined.
	uint16_t getSpadNb(); //Returns the current number of enabled SPADs
	uint16_t getAmbientRate(); // Returns the total ambinet rate in kcps. All SPADs combined.
	uint8_t getRangeStatus(); //Returns the range status, which can be any of the following. 0 = no error, 1 = signal fail, 2 = sigma fail, 7 = wrapped target fail
	void setOffset(int16_t offset); //Manually set an offset in mm
	int16_t getOffset(); //Get the current offset in mm
	void setXTalk(uint16_t xTalk); //Manually set the value of crosstalk in counts per second (cps), which is interference from any sort of window in front of your sensor.
	uint16_t getXTalk(); //Returns the current crosstalk value in cps.
	void setDistanceThreshold(uint16_t lowThresh, uint16_t hiThresh, uint8_t window);//Set bounds for the interrupt. lowThresh and hiThresh are the bounds of your interrupt while window decides when the interrupt should fire. The options for window are shown below.
	//0: Interrupt triggered on measured distance below lowThresh.
	//1: Interrupt triggered on measured distance above hiThresh.
	//2: Interrupt triggered on measured distance outside of bounds.
	//3: Interrupt triggered on measured distance inside of bounds.
	uint16_t getDistanceThresholdWindow(); //Returns distance threshold window option
	uint16_t getDistanceThresholdLow(); //Returns lower bound in mm.
	uint16_t getDistanceThresholdHigh(); //Returns upper bound in mm
	/**Table of Optical Centers**
	*
	* 128,136,144,152,160,168,176,184,  192,200,208,216,224,232,240,248
	* 129,137,145,153,161,169,177,185,  193,201,209,217,225,233,241,249
	* 130,138,146,154,162,170,178,186,  194,202,210,218,226,234,242,250
	* 131,139,147,155,163,171,179,187,  195,203,211,219,227,235,243,251
	* 132,140,148,156,164,172,180,188,  196,204,212,220,228,236,244,252
	* 133,141,149,157,165,173,181,189,  197,205,213,221,229,237,245,253
	* 134,142,150,158,166,174,182,190,  198,206,214,222,230,238,246,254
	* 135,143,151,159,167,175,183,191,  199,207,215,223,231,239,247,255
	
	* 127,119,111,103, 95, 87, 79, 71,  63, 55, 47, 39, 31, 23, 15, 7
	* 126,118,110,102, 94, 86, 78, 70,  62, 54, 46, 38, 30, 22, 14, 6
	* 125,117,109,101, 93, 85, 77, 69,  61, 53, 45, 37, 29, 21, 13, 5
	* 124,116,108,100, 92, 84, 76, 68,  60, 52, 44, 36, 28, 20, 12, 4
	* 123,115,107, 99, 91, 83, 75, 67,  59, 51, 43, 35, 27, 19, 11, 3
	* 122,114,106, 98, 90, 82, 74, 66,  58, 50, 42, 34, 26, 18, 10, 2
	* 121,113,105, 97, 89, 81, 73, 65,  57, 49, 41, 33, 25, 17, 9, 1
	* 120,112,104, 96, 88, 80, 72, 64,  56, 48, 40, 32, 24, 16, 8, 0 Pin 1
	*
	* To set the center, set the pad that is to the right and above the exact center of the region you'd like to measure as your opticalCenter*/
	void setROI(uint8_t x, uint8_t y, uint8_t opticalCenter); //Set the height and width of the ROI(region of interest) in SPADs, lowest possible option is 4. Set optical center based on above table
	uint16_t getROIX(); //Returns the width of the ROI in SPADs
	uint16_t getROIY(); //Returns the height of the ROI in SPADs
	void setSignalThreshold(uint16_t signalThreshold); //Programs the necessary threshold to trigger a measurement. Default is 1024 kcps.
	uint16_t getSignalThreshold(); //Returns the signal threshold in kcps
	void setSigmaThreshold(uint16_t sigmaThreshold); //Programs a new sigma threshold in mm. (default=15 mm)
	uint16_t getSigmaThreshold(); //Returns the current sigma threshold.
	void startTemperatureUpdate(); //Recalibrates the sensor for temperature changes. Run this any time the temperature has changed by more than 8°C
	void calibrateOffset(uint16_t targetDistanceInMm); //Autocalibrate the offset by placing a target a known distance away from the sensor and passing this known distance into the function.
	void calibrateXTalk(uint16_t targetDistanceInMm); //Autocalibrate the crosstalk by placing a target a known distance away from the sensor and passing this known distance into the function.
	private:
	TwoWire *_i2cPort;
	int _shutdownPin;
	int _interruptPin;
	int _i2cAddress = 0x52;
	VL53L1X* _device;
};