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

#ifndef AS3935MI_H_
#define AS3935MI_H_

#include <Arduino.h>

#if defined(ESP8266) || defined(ESP32)
// When we can't use attachInterruptArg to directly access volatile members,
// we must use static variables in the .cpp file
// This means only a single instance of this class can be used.
//
// When we can use attachInterruptArg, we can use volatile members 
// and thus have multiple instances of this class without jumping through hoops
// to avoid issues sharing volatile variables

#define AS3935MI_HAS_ATTACHINTERRUPTARG_FUNCTION
#endif

#if ESP_IDF_VERSION_MAJOR >= 5
# include <atomic>
#endif


// Allow for 3.5% deviation
# define AS3935MI_ALLOWED_DEVIATION    0.035f

// Division ratio and nr of samples chosen so we expect a
// 500 kHz LCO measurement to take about 18 msec on ESP32
// On others it will take about 32 msec.
// ESP8266 can't handle > 20 kHz interrupt calls very well, 
// therefore set to DR_32 and edge trigger to "RISING"
# ifdef ESP32

// Expected LCO frequency for DR_16 = 31250 Hz
#  define AS3935MI_LCO_DIVISION_RATIO AS3935MI::AS3935_DR_16
#  define AS3935MI_NR_CALIBRATION_SAMPLES 1000ul
#  define AS3935MI_CALIBRATION_MODE_EDGE_TRIGGER  CHANGE
# else // ifdef ESP32

// Expected LCO frequency for DR_32 = 15625 Hz
#  define AS3935MI_LCO_DIVISION_RATIO AS3935MI::AS3935_DR_32
#  define AS3935MI_NR_CALIBRATION_SAMPLES  500ul
#  define AS3935MI_CALIBRATION_MODE_EDGE_TRIGGER  RISING
# endif // ifdef ESP32

class AS3935MI
{
public:
	enum afe_setting_t : uint8_t
	{
		AS3935_INDOORS = 0b10010,
		AS3935_OUTDOORS = 0b01110
	};

	enum interrupt_name_t : uint8_t
	{
		AS3935_INT_DUPDATE = 0b0000,//distance estimation has changed due to purging of old events in the statistics, based on the lightning distance estimation algorithm.
		AS3935_INT_NH = 0b0001,		//noise level too high
		AS3935_INT_D = 0b0100,		//disturber detected
		AS3935_INT_L = 0b1000		//lightning interrupt
	};

	enum wdth_setting_t : uint8_t
	{
		AS3935_WDTH_0 = 0b0000,
		AS3935_WDTH_1 = 0b0001,
		AS3935_WDTH_2 = 0b0010,
		AS3935_WDTH_3 = 0b0011,
		AS3935_WDTH_4 = 0b0100,
		AS3935_WDTH_5 = 0b0101,
		AS3935_WDTH_6 = 0b0110,
		AS3935_WDTH_7 = 0b0111,
		AS3935_WDTH_8 = 0b1000,
		AS3935_WDTH_9 = 0b1001,
		AS3935_WDTH_10 = 0b1010,
		AS3935_WDTH_11 = 0b1011,
		AS3935_WDTH_12 = 0b1100,
		AS3935_WDTH_13 = 0b1101,
		AS3935_WDTH_14 = 0b1110,
		AS3935_WDTH_15 = 0b1111
	};

	enum srej_setting_t : uint8_t
	{
		AS3935_SREJ_0 = 0b0000,
		AS3935_SREJ_1 = 0b0001,
		AS3935_SREJ_2 = 0b0010,
		AS3935_SREJ_3 = 0b0011,
		AS3935_SREJ_4 = 0b0100,
		AS3935_SREJ_5 = 0b0101,
		AS3935_SREJ_6 = 0b0110,
		AS3935_SREJ_7 = 0b0111,
		AS3935_SREJ_8 = 0b1000,
		AS3935_SREJ_9 = 0b1001,
		AS3935_SREJ_10 = 0b1010, 
		AS3935_SREJ_11 = 0b1011, 
		AS3935_SREJ_12 = 0b1100, 
		AS3935_SREJ_13 = 0b1101, 
		AS3935_SREJ_14 = 0b1110, 
		AS3935_SREJ_15 = 0b1111
	};

	enum noise_floor_threshold_t : uint8_t
	{
		AS3935_NFL_0 = 0b000,
		AS3935_NFL_1 = 0b001,
		AS3935_NFL_2 = 0b010,		//default
		AS3935_NFL_3 = 0b011,
		AS3935_NFL_4 = 0b100,
		AS3935_NFL_5 = 0b101,
		AS3935_NFL_6 = 0b110,
		AS3935_NFL_7 = 0b111,
	};

	enum min_num_lightnings_t : uint8_t
	{
		AS3935_MNL_1 = 0b00,		//minimum number of lightnings: 1
		AS3935_MNL_5 = 0b01,		//minimum number of lightnings: 5
		AS3935_MNL_9 = 0b10,		//minimum number of lightnings: 9
		AS3935_MNL_16 = 0b11,		//minimum number of lightnings: 16
	};

	enum division_ratio_t : uint8_t
	{
		AS3935_DR_16 = 0b00,
		AS3935_DR_32 = 0b01,
		AS3935_DR_64 = 0b10,
		AS3935_DR_128 = 0b11
	};

	
	enum class display_frequency_source_t {
		LCO, // 500 kHz resonance freq
		SRCO, // 1.1 MHz signal
		TRCO // 32768 Hz signal
	};

	static const uint8_t AS3935_DST_OOR = 0b111111;		//detected lightning was out of range

	AS3935MI(uint8_t irq);
	virtual ~AS3935MI();

	bool begin();

	/*
	@return storm distance in km. */
	uint8_t readStormDistance();

	/*
	@return interrupt source as AS9395::interrupt_name_t. */
	uint8_t readInterruptSource();

	/*
	@return true: powered down, false: powered up. */
	bool readPowerDown();

	/*
	@param enabled: true to power down, false to power up. */
	void writePowerDown(bool enabled);

	/*
	@return true if disturbers are masked, false otherwise. */
	bool readMaskDisturbers();

	/*
	@param enabled true to mask disturbers, false otherwise. */
	void writeMaskDisturbers(bool enabled);

	/*
	@return AFE setting as afe_setting_t. */
	uint8_t readAFE();

	/*
	@param afe_setting AFE setting as one if afe_setting_t. */
	void writeAFE(uint8_t afe_setting);

	/*
	@return current noise floor. */
	uint8_t readNoiseFloorThreshold();

	/*
	writes a noise floor threshold setting to the sensor. 
	@param threshold as noise_floor_threshold_t*/
	void writeNoiseFloorThreshold(uint8_t threshold);

	/*
	@return current noise floor threshold. */
	uint8_t readWatchdogThreshold();

	/*
	@param noise floor threshold setting. */
	void writeWatchdogThreshold(uint8_t noise_floor);

	/*
	@return current spike rejection setting as srej_setting_t. */
	uint8_t readSpikeRejection();

	/*
	@param spike rejection setting as srej_setting_t. */
	void writeSpikeRejection(uint8_t threshold);

	/*
	@return lightning energy. no physical meaning. */
	uint32_t readEnergy();

	/*
	@return antenna tuning*/
	uint8_t readAntennaTuning();

	/*
	writes an antenna tuning setting to the sensor. */
	bool writeAntennaTuning(uint8_t tuning);

	/*
	read the currently set antenna tuning division ratio from the sensor. */
	uint8_t readDivisionRatio();

	/*
	writes an antenna tuning division ratio setting to the sensor. */
	void writeDivisionRatio(uint8_t ratio);

	/*
	get the currently set minimum number of lightnings in the last 15 minues before lightning interrupts are issued, as min_num_lightnings_t. */
	uint8_t readMinLightnings();

	/*
	@param minimum number of lightnings in the last 15 minues before lightning interrupts are issued, as min_num_lightnings_t. */
	void writeMinLightnings(uint8_t number);

	/*
	resets all registers to default values. */
	void resetToDefaults();

	/*
	calibrates the AS3935 TCRO accordingto procedure in AS3935 datasheet p36. must be done *after* calibrating the resonance frequency.
	@return true on success, false otherwise. */
	bool calibrateRCO();

    // Set the number of samples counted during frequency measurements.
	void setFrequencyMeasureNrSamples(uint32_t nrSamples);

	// Set the edge mode trigger for any frequency measurement to either RISING or CHANGE
	void setFrequencyMeasureEdgeChange(bool triggerRisingAndFalling);

    // Set the division ratio, only used when measuring LCO (thus only during calibration)
	void setCalibrationDivisionRatio(uint8_t division_ratio);

	/*
	calibrates the AS3935 antenna's resonance frequency. 
	@param (by reference, write only) frequency: after return, will hold the frequency the AS3935 
	has been calibrated to. 
	@return true on success, false on failure or if the resonance frequency could not be tuned
	to within +-3.5% of 500kHz. */
	bool calibrateResonanceFrequency(
		int32_t& frequency, 
	    uint8_t division_ratio);
	bool calibrateResonanceFrequency(
		int32_t& frequency);
	bool calibrateResonanceFrequency();

	/*
	checks if the sensor is connected by attempting to read the AFE gain boost setting. 
	@return true if the AFE gain boost setting is 0b10010 or 0b01110, false otherwise. */
	bool checkConnection();

	/*
	checks the IRQ pin by instructing the AS3935 to display the antenna's resonance frequency on the IRQ pin 
	and monitoring the pin for changing levels. IRQ pin interrupt must not be enabled during this test. 
	The test takes approximately 14ms. the test is considered successful if more than 100 transitions have 
	been detected (to prevent false positives). 
	@return true if more than 100 changes in IRQ pin logic level were detected, false otherwise. */
	bool checkIRQ();
	
	/*
	 * clears lightning distance estimation statistics
	 */
	void clearStatistics();

	/*
	increases the noise floor threshold setting, if possible.
	@return true on success, false otherwise. */
	bool decreaseNoiseFloorThreshold();

	/*
	increases the noise floor threshold setting, if possible.
	@return new value on success, 0 otherwise. */
	uint8_t increaseNoiseFloorThreshold();

	/*
	increases the watchdog threshold setting, if possible.
	@return true on success, false otherwise. */
	bool decreaseWatchdogThreshold();

	/*
	increases the watchdog threshold setting, if possible.
	@return true on success, false otherwise. */
	bool increaseWatchdogThreshold();

	/*
	increases the spike rejection setting, if possible.
	@return true on success, false otherwise. */
	bool decreaseSpikeRejection();

	/*
	increases the spike rejection setting, if possible.
	@return true on success, false otherwise. */
	bool increaseSpikeRejection();

    // Ideally 500 kHz signal divided by the set division ratio
	void displayLcoOnIrq(bool enable);

    // Ideally 1.1 MHz signal
	void displaySrcoOnIrq(bool enable);

    // Ideally 32.768 kHz signal
	void displayTrcoOnIrq(bool enable);


	bool validateCurrentResonanceFrequency(int32_t& frequency);

	int32_t measureResonanceFrequency(display_frequency_source_t source);


private:
	enum AS3935_registers_t : uint8_t
	{
		AS3935_REGISTER_AFE_GB = 0x00,			//Analog Frontend Gain Boost
		AS3935_REGISTER_PWD = 0x00,				//Power Down
		AS3935_REGISTER_NF_LEV = 0x01,			//Noise Floor Level
		AS3935_REGISTER_WDTH = 0x01,			//Watchdog threshold
		AS3935_REGISTER_CL_STAT = 0x02,			//Clear statistics
		AS3935_REGISTER_MIN_NUM_LIGH = 0x02,	//Minimum number of lightnings
		AS3935_REGISTER_SREJ = 0x02,			//Spike rejection
		AS3935_REGISTER_LCO_FDIV = 0x03,		//Frequency division ratio for antenna tuning
		AS3935_REGISTER_MASK_DIST = 0x03,		//Mask Disturber
		AS3935_REGISTER_INT = 0x03,				//Interrupt
		AS3935_REGISTER_S_LIG_L = 0x04,			//Energy of the Single Lightning LSBYTE
		AS3935_REGISTER_S_LIG_M = 0x05,			//Energy of the Single Lightning MSBYTE
		AS3935_REGISTER_S_LIG_MM = 0x06,		//Energy of the Single Lightning MMSBYTE
		AS3935_REGISTER_DISTANCE = 0x07,		//Distance estimation
		AS3935_REGISTER_DISP_XXX = 0x08,		//Display XXX on IRQ pin
		AS3935_REGISTER_DISP_LCO = 0x08,		//Display LCO on IRQ pin
		AS3935_REGISTER_DISP_SRCO = 0x08,		//Display SRCO on IRQ pin
		AS3935_REGISTER_DISP_TRCO = 0x08,		//Display TRCO on IRQ pin
		AS3935_REGISTER_TUN_CAP = 0x08,			//Internal Tuning Capacitors (from 0 to	120pF in steps of 8pF)
		AS3935_REGISTER_TRCO_CALIB_DONE = 0x3A, //Calibration of TRCO done (1=successful)
		AS3935_REGISTER_TRCO_CALIB_NOK = 0x3A,	//Calibration of TRCO unsuccessful (1 = not successful)
		AS3935_REGISTER_SRCO_CALIB_DONE = 0x3B,	//Calibration of SRCO done (1=successful)
		AS3935_REGISTER_SRCO_CALIB_NOK = 0x3B,	//Calibration of SRCO unsuccessful (1 = not successful)
		AS3935_REGISTER_PRESET_DEFAULT = 0x3C,	//Sets all registers in default mode
		AS3935_REGISTER_CALIB_RCO = 0x3D		//Sets all registers in default mode
	};

	enum AS3935_register_mask_t : uint8_t
	{
		AS3935_MASK_AFE_GB =				0b00111110,	//Analog Frontend Gain Boost
		AS3935_MASK_PWD =					0b00000001, //Power Down
		AS3935_MASK_NF_LEV =				0b01110000,	//Noise Floor Level
		AS3935_MASK_WDTH =					0b00001111,	//Watchdog threshold
		AS3935_MASK_CL_STAT =				0b01000000,	//Clear statistics
		AS3935_MASK_MIN_NUM_LIGH =			0b00110000,	//Minimum number of lightnings
		AS3935_MASK_SREJ =					0b00001111,	//Spike rejection
		AS3935_MASK_LCO_FDIV =				0b11000000,	//Frequency division ratio for antenna tuning
		AS3935_MASK_MASK_DIST =				0b00100000,	//Mask Disturber
		AS3935_MASK_INT =					0b00001111,	//Interrupt
		AS3935_MASK_S_LIG_L =				0b11111111,	//Energy of the Single Lightning LSBYTE
		AS3935_MASK_S_LIG_M =				0b11111111,	//Energy of the Single Lightning MSBYTE
		AS3935_MASK_S_LIG_MM =				0b00001111,	//Energy of the Single Lightning MMSBYTE
		AS3935_MASK_DISTANCE =				0b00111111,	//Distance estimation
		AS3935_MASK_DISP_XXX =              0b11100000,	//Display XXX on IRQ pin
		AS3935_MASK_DISP_LCO =				0b10000000,	//Display LCO on IRQ pin
		AS3935_MASK_DISP_SRCO =				0b01000000,	//Display SRCO on IRQ pin
		AS3935_MASK_DISP_TRCO =				0b00100000,	//Display TRCO on IRQ pin
		AS3935_MASK_TUN_CAP =				0b00001111,	//Internal Tuning Capacitors (from 0 to	120pF in steps of 8pF)
		AS3935_MASK_TRCO_CALIB_DONE =		0b10000000, //Calibration of TRCO done (1=successful)
		AS3935_MASK_TRCO_CALIB_NOK =		0b01000000,	//Calibration of TRCO unsuccessful (1 = not successful)
		AS3935_MASK_TRCO_CALIB_ALL =		0b11000000,	//Calibration of TRCO done (0b10 = successful)
		AS3935_MASK_SRCO_CALIB_DONE =		0b10000000,	//Calibration of SRCO done (1=successful)
		AS3935_MASK_SRCO_CALIB_NOK =		0b01000000,	//Calibration of SRCO unsuccessful (1 = not successful)
		AS3935_MASK_SRCO_CALIB_ALL =		0b11000000,	//Calibration of SRCO done (0b10 = successful)
		AS3935_MASK_PRESET_DEFAULT =	    0b11111111,	//Sets all registers in default mode
		AS3935_MASK_CALIB_RCO =			    0b11111111	//Sets all registers in default mode
	};


	virtual bool beginInterface() = 0;

	/*
	@param mask
	@return number of bits to shift value so it fits into mask. */
	uint8_t getMaskShift(uint8_t mask);
	
	/*
	@param register value of register.
	@param mask mask of value in register
	@return value of masked bits. */
	uint8_t getMaskedBits(uint8_t reg, uint8_t mask);
	
	/*
	@param register value of register
	@param mask mask of value in register
	@param value value to write into masked area
	@param register value with masked bits set to value. */
	uint8_t setMaskedBits(uint8_t reg, uint8_t mask, uint8_t value);
	
	/*
	reads the masked value from the register. 
	@param reg register to read.
	@param mask mask of value.
	@return masked value in register. */
	uint8_t readRegisterValue(uint8_t reg, uint8_t mask);
	
	/*
	sets values in a register. 
	@param reg register to set values in
	@param mask bits of register to set value in
	@param value value to set */
	void writeRegisterValue(uint8_t reg, uint8_t mask, uint8_t value);

	/*
	reads a register from the sensor. must be overwritten by derived classes.
	@param reg register to read. 
	@return register content*/
	virtual uint8_t readRegister(uint8_t reg) = 0;

	/*
	writes a register to the sensor. must be overwritten by derived classes. 
	this function is also used to send direct commands. 
	@param reg register to write to. 
	@param value value writeRegister write to register. */
	virtual void writeRegister(uint8_t reg, uint8_t value) = 0;	


	uint32_t              computeCalibratedFrequency(int32_t divider);

public:

	// Internal Tuning Capacitors (from 0 to 120pF in steps of 8pF)
	uint32_t              measureResonanceFrequency(display_frequency_source_t source, uint8_t tuningCapacitance);


	enum interrupt_mode_t {
		AS3935_INTERRUPT_UNINITIALIZED,
		AS3935_INTERRUPT_DETACHED,
		AS3935_INTERRUPT_NORMAL,
		AS3935_INTERRUPT_CALIBRATION
	};

	interrupt_mode_t      getInterruptMode() const { return mode_; }

	uint32_t              getInterruptTimestamp() const;

	void                  setInterruptMode(interrupt_mode_t mode);

private:
	static const uint8_t AS3935_DIRECT_CMD = 0x96;

	static const uint32_t AS3935_TIMEOUT = 2000;

	uint8_t irq_;				//interrupt pin

    // Tuning cap value is located in the same register as the display LCO/SRCO/TRCO flags
	// When those are active the device may not give an ACK when trying to read 
	// (via I2C) the register to update those display flags
	// To overcome this issue, we keep a cache of the tuning cap parameter 
	// and write directly to the register instead of read/set bits/write.
	uint8_t tuning_cap_cache_ = 0;

	AS3935MI::interrupt_mode_t mode_ = AS3935MI::AS3935_INTERRUPT_UNINITIALIZED;

	int calibration_mode_edgetrigger_trigger_ = AS3935MI_CALIBRATION_MODE_EDGE_TRIGGER;
	AS3935MI::division_ratio_t calibration_mode_division_ratio_ = AS3935MI_LCO_DIVISION_RATIO;


#if ESP_IDF_VERSION_MAJOR >= 5
#define AS3935MI_VOLATILE_TYPE std::atomic<uint32_t>
#else
#define AS3935MI_VOLATILE_TYPE volatile uint32_t
#endif


#ifdef AS3935MI_HAS_ATTACHINTERRUPTARG_FUNCTION
	static void IRAM_ATTR interruptISR(AS3935MI *self);
	static void IRAM_ATTR calibrateISR(AS3935MI *self);

	AS3935MI_VOLATILE_TYPE interrupt_timestamp_ = 0;
	AS3935MI_VOLATILE_TYPE interrupt_count_     = 0;

	// Store the time micros as 32-bit int so it can be stored and comprared as an atomic operation.
	// Expected duration will be much less than 2^32 usec, thus overflow isn't an issue here
	AS3935MI_VOLATILE_TYPE calibration_start_micros_ = 0;
	AS3935MI_VOLATILE_TYPE calibration_end_micros_   = 0;

	uint32_t nr_calibration_samples_  = AS3935MI_NR_CALIBRATION_SAMPLES;

#else
	static void IRAM_ATTR interruptISR();
	static void IRAM_ATTR calibrateISR();
#endif
    

public:
    // Return the result of the last frequency measurement of the given tuning cap index
	// @retval -1 when tuningCapacitance is out of range
	int32_t getAntCapFrequency(uint8_t tuningCapacitance) const;

	// Return the best ant_cap found during last LCO calibration
	// @retval -1 when no LCO calibration was performed
	int8_t  getCalibratedAntCap() const {
		return calibrated_ant_cap_;
	}

    // When set to calibrate all ant_cap indices, the LCO calibration is 
	// effectively set to perform a 'slow' calibration.
	// All caps will be tried and also using more samples.
	void setCalibrateAllAntCap(bool calibrate_all) {
		calibrate_all_ant_cap_ = calibrate_all;
	}

	bool getCalibrateAllAntCap() const {
		return calibrate_all_ant_cap_;
	}

private:
    int32_t calibration_frequencies_[16]{};
	int8_t calibrated_ant_cap_ = -1;
	bool calibrate_all_ant_cap_ = true;

};

#endif /* AS3935_H_ */