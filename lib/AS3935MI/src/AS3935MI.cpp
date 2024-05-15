//Yet Another Arduino ams AS3935 'Franklin' lightning sensor library 
// Copyright (c) 2018-2019 Gregor Christandl <christandlg@yahoo.com>
// home: https://bitbucket.org/christandlg/as3935
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA	02110-1301	USA

#include "AS3935MI.h"


#ifdef ESP8266
#define getMicros64 micros64
#elif defined(ESP32)
#define getMicros64 esp_timer_get_time
#else
#define getMicros64 micros
#endif


// When we can't use attachInterruptArg to directly access volatile members,
// we must use static variables in the .cpp file
#ifndef AS3935MI_HAS_ATTACHINTERRUPTARG_FUNCTION
	AS3935MI_VOLATILE_TYPE interrupt_timestamp_ = 0;
	AS3935MI_VOLATILE_TYPE interrupt_count_     = 0;

	// Store the time micros as 32-bit int so it can be stored and comprared as an atomic operation.
	// Expected duration will be much less than 2^32 usec, thus overflow isn't an issue here
	AS3935MI_VOLATILE_TYPE calibration_start_micros_ = 0;
	AS3935MI_VOLATILE_TYPE calibration_end_micros_   = 0;
#endif

AS3935MI::AS3935MI(uint8_t irq) :
	irq_(irq),
	tuning_cap_cache_(0),
	mode_(AS3935MI::interrupt_mode_t::detached)
{
}

AS3935MI::~AS3935MI()
{
	if (mode_ != AS3935MI::interrupt_mode_t::detached) {
		detachInterrupt(irq_);
	}
}

bool AS3935MI::begin()
{
	if (!beginInterface())
		return false;

	writePowerDown(false);

	set_interruptMode(AS3935MI::interrupt_mode_t::detached);
	resetToDefaults();

	return true;
}

uint8_t AS3935MI::readStormDistance()
{
	return readRegisterValue(AS3935_REGISTER_DISTANCE, AS3935_MASK_DISTANCE);
}

uint8_t AS3935MI::readInterruptSource()
{
	interrupt_timestamp_ = 0;
	return readRegisterValue(AS3935_REGISTER_INT, AS3935_MASK_INT);
}

bool AS3935MI::readPowerDown()
{
	return (readRegisterValue(AS3935_REGISTER_PWD, AS3935_MASK_PWD) == 1 ? true : false);
}

void AS3935MI::writePowerDown(bool enabled)
{
	writeRegisterValue(AS3935_REGISTER_PWD, AS3935_MASK_PWD, enabled ? 1 : 0);
}

bool AS3935MI::readMaskDisturbers()
{
	return (readRegisterValue(AS3935_REGISTER_MASK_DIST, AS3935_MASK_MASK_DIST) == 1 ? true : false);
}

void AS3935MI::writeMaskDisturbers(bool enabled)
{
	writeRegisterValue(AS3935_REGISTER_MASK_DIST, AS3935_MASK_MASK_DIST, enabled ? 1 : 0);
}

uint8_t AS3935MI::readAFE()
{
	return readRegisterValue(AS3935_REGISTER_AFE_GB, AS3935_MASK_AFE_GB);
}

void AS3935MI::writeAFE(uint8_t afe_setting)
{
	writeRegisterValue(AS3935_REGISTER_AFE_GB, AS3935_MASK_AFE_GB, afe_setting);
}

uint8_t AS3935MI::readNoiseFloorThreshold()
{
	return readRegisterValue(AS3935_REGISTER_NF_LEV, AS3935_MASK_NF_LEV);
}

void AS3935MI::writeNoiseFloorThreshold(uint8_t threshold)
{
	if (threshold > AS3935_NFL_7)
		return;

	writeRegisterValue(AS3935_REGISTER_NF_LEV, AS3935_MASK_NF_LEV, threshold);

	delayMicroseconds(AS3935_TIMEOUT);
}

uint8_t AS3935MI::readWatchdogThreshold()
{
	return readRegisterValue(AS3935_REGISTER_WDTH, AS3935_MASK_WDTH);
}

void AS3935MI::writeWatchdogThreshold(uint8_t threshold)
{
	if (threshold > AS3935_WDTH_15)
		return;

	writeRegisterValue(AS3935_REGISTER_WDTH, AS3935_MASK_WDTH, threshold);

	delayMicroseconds(AS3935_TIMEOUT);
}

uint8_t AS3935MI::readSpikeRejection()
{
	return readRegisterValue(AS3935_REGISTER_SREJ, AS3935_MASK_SREJ);
}

void AS3935MI::writeSpikeRejection(uint8_t threshold)
{
	if (threshold > AS3935_SREJ_15)
		return;

	writeRegisterValue(AS3935_REGISTER_SREJ, AS3935_MASK_SREJ, threshold);

	delayMicroseconds(AS3935_TIMEOUT);
}

uint32_t AS3935MI::readEnergy()
{
	uint32_t energy = 0;
	//from https://www.eevblog.com/forum/microcontrollers/define-mmsbyte-for-as3935-lightning-detector/
	//Reg 0x04: Energy word, bits 0 : 7
	//Reg 0x05 : Energy word, bits 8 : 15
	//Reg 0x06 : Energy word, bits 16 : 20
	//energy |= LSB
	//energy |= (MSB << 8)
	//energy |= (MMSB << 16)
	energy |= static_cast<uint32_t>(readRegisterValue(AS3935_REGISTER_S_LIG_L, AS3935_MASK_S_LIG_L));
	energy |= (static_cast<uint32_t>(readRegisterValue(AS3935_REGISTER_S_LIG_M, AS3935_MASK_S_LIG_M)) << 8);
	energy |= (static_cast<uint32_t>(readRegisterValue(AS3935_REGISTER_S_LIG_MM, AS3935_MASK_S_LIG_MM)) << 16);

	return energy;
}

uint8_t AS3935MI::readAntennaTuning()
{
	uint8_t return_value = readRegisterValue(AS3935_REGISTER_TUN_CAP, AS3935_MASK_TUN_CAP);
	if (return_value != static_cast<uint8_t>(-1)) {
		// No read error, so update the tuning_cap_cache_
		tuning_cap_cache_ = return_value;
	} else {
		return tuning_cap_cache_;
	}

	return return_value;
}

void AS3935MI::writeAntennaTuning(uint8_t tuning)
{
	tuning_cap_cache_ = tuning;
	writeRegisterValue(AS3935_REGISTER_TUN_CAP, AS3935_MASK_TUN_CAP, tuning);
}

uint8_t AS3935MI::readDivisionRatio()
{
	return readRegisterValue(AS3935_REGISTER_LCO_FDIV, AS3935_MASK_LCO_FDIV);
}

void AS3935MI::writeDivisionRatio(uint8_t ratio)
{
	writeRegisterValue(AS3935_REGISTER_LCO_FDIV, AS3935_MASK_LCO_FDIV, ratio);
}

uint8_t AS3935MI::readMinLightnings()
{
	return readRegisterValue(AS3935_REGISTER_MIN_NUM_LIGH, AS3935_MASK_MIN_NUM_LIGH);
}

void AS3935MI::writeMinLightnings(uint8_t number)
{
	writeRegisterValue(AS3935_REGISTER_MIN_NUM_LIGH, AS3935_MASK_MIN_NUM_LIGH, number);
}

void AS3935MI::resetToDefaults()
{
	writeRegister(AS3935_REGISTER_PRESET_DEFAULT, AS3935_DIRECT_CMD);

	delayMicroseconds(AS3935_TIMEOUT);
}

bool AS3935MI::calibrateRCO()
{
	//cannot calibrate if in power down mode.
	if (readPowerDown())
		return false;

	//issue calibration command
	writeRegister(AS3935_REGISTER_CALIB_RCO, AS3935_DIRECT_CMD);

	//expose 1.1 MHz SRCO clock on IRQ pin
	displaySRCO_on_IRQ(true);

	//wait for calibration to finish...
	delayMicroseconds(AS3935_TIMEOUT);

	//stop exposing clock on IRQ pin
	displaySRCO_on_IRQ(false);

	//check calibration results. bits will be set if calibration failed.
	bool success_TRCO = !static_cast<bool>(readRegisterValue(AS3935_REGISTER_TRCO_CALIB_NOK, AS3935_MASK_TRCO_CALIB_NOK));
	bool success_SRCO = !static_cast<bool>(readRegisterValue(AS3935_REGISTER_SRCO_CALIB_NOK, AS3935_MASK_SRCO_CALIB_NOK));

	return (success_TRCO && success_SRCO);
}

bool AS3935MI::calibrateResonanceFrequency(int32_t& frequency)
{
	if (readPowerDown())
		return false;

	int32_t best_diff = 500000;
	int8_t	best_i	  = -1;

	frequency = 0;

	for (uint8_t i = 0; i < 16; i++)
	{
		const uint32_t freq = measureResonanceFrequency(
			display_frequency_source_t::LCO, i);

		if (freq == 0) {
			return false;
		}
		const int32_t freq_diff = 500000 - freq;

		if (abs(freq_diff) < abs(best_diff)) {
			best_diff = freq_diff;
			best_i	  = i;
			frequency = freq;
		}
	}

	if (best_i < 0) {
		frequency = 0;
		return false;
	}

	writeAntennaTuning(best_i);

	// Check for allowed deviation
	constexpr int allowedDeviation = 500000 * AS3935MI_ALLOWED_DEVIATION;

	return abs(best_diff) < allowedDeviation;
}

bool AS3935MI::calibrateResonanceFrequency()
{
	int32_t frequency = 0;
	return calibrateResonanceFrequency(frequency);
}

bool AS3935MI::checkConnection()
{
	uint8_t afe = readAFE();

	return ((afe == AS3935_INDOORS) || (afe == AS3935_OUTDOORS));
}

bool AS3935MI::checkIRQ()
{
	// Expected frequency is roughly 500 kHz, so we should see at the very least see 100 kHz
	return measureResonanceFrequency(display_frequency_source_t::LCO) > 100000;
}

void AS3935MI::clearStatistics()
{
	writeRegisterValue(AS3935_REGISTER_CL_STAT, AS3935_MASK_CL_STAT, 1);
	writeRegisterValue(AS3935_REGISTER_CL_STAT, AS3935_MASK_CL_STAT, 0);
	writeRegisterValue(AS3935_REGISTER_CL_STAT, AS3935_MASK_CL_STAT, 1);
}

bool AS3935MI::decreaseNoiseFloorThreshold()
{
	uint8_t nf_lev = readNoiseFloorThreshold();

	if (nf_lev == AS3935_NFL_0)
		return false;

	writeNoiseFloorThreshold(--nf_lev);

	return true;
}

bool AS3935MI::increaseNoiseFloorThreshold()
{
	uint8_t nf_lev = readNoiseFloorThreshold();

	if (nf_lev >= AS3935_NFL_7)
		return false;

	writeNoiseFloorThreshold(++nf_lev);

	return true;
}

bool AS3935MI::decreaseWatchdogThreshold()
{
	uint8_t wdth = readWatchdogThreshold();

	if (wdth == AS3935_WDTH_0)
		return false;

	writeWatchdogThreshold(--wdth);

	return true;
}

bool AS3935MI::increaseWatchdogThreshold()
{
	uint8_t wdth = readWatchdogThreshold();

	if (wdth >= AS3935_WDTH_15)
		return false;

	writeWatchdogThreshold(++wdth);

	return true;
}

bool AS3935MI::decreaseSpikeRejection()
{
	uint8_t srej = readSpikeRejection();

	if (srej == AS3935_SREJ_0)
		return false;

	writeSpikeRejection(--srej);

	return true;
}

bool AS3935MI::increaseSpikeRejection()
{
	uint8_t srej = readSpikeRejection();

	if (srej >= AS3935_SREJ_15)
		return false;

	writeSpikeRejection(++srej);

	return true;
}

void AS3935MI::displayLCO_on_IRQ(bool enable)
{
	// With display of any frequency, the device may sometimes report NAK when reading registers
	// So for this reason we're now writing directly and not try to read first, patch bits, write
	uint8_t value = tuning_cap_cache_;
	if (enable) {
		value |= 0b10000000;
	}
	writeRegister(AS3935_REGISTER_DISP_XXX, value);
}

void AS3935MI::displaySRCO_on_IRQ(bool enable)
{
	uint8_t value = tuning_cap_cache_;
	if (enable) {
		value |= 0b01000000;
	}
	writeRegister(AS3935_REGISTER_DISP_XXX, value);
}


void AS3935MI::displayTRCO_on_IRQ(bool enable)
{
	uint8_t value = tuning_cap_cache_;
	if (enable) {
		value |= 0b00100000;
	}
	writeRegister(AS3935_REGISTER_DISP_XXX, value);
}


bool AS3935MI::validateCurrentResonanceFrequency(int32_t& frequency)
{
	frequency = measureResonanceFrequency(
		display_frequency_source_t::LCO,
		readAntennaTuning());

	// Check for allowed deviation
	constexpr int allowedDeviation = 500000 * AS3935MI_ALLOWED_DEVIATION;

	return abs(500000 - frequency) < allowedDeviation;
}

int32_t AS3935MI::measureResonanceFrequency(display_frequency_source_t source)
{
	return measureResonanceFrequency(
		source,
		readAntennaTuning());
}


uint8_t AS3935MI::getMaskShift(uint8_t mask)
{
	uint8_t return_value = 0;

	//count how many times the mask must be shifted right until the lowest bit is set
	if (mask != 0)
	{
		while (!(mask & 1))
		{
			return_value++;
			mask >>= 1;
		}
	}

	return return_value;
}
	
uint8_t AS3935MI::getMaskedBits(uint8_t reg, uint8_t mask)
{
	//extract masked bits
	return ((reg & mask) >> getMaskShift(mask));
}

uint8_t AS3935MI::setMaskedBits(uint8_t reg, uint8_t mask, uint8_t value)
{
	//clear mask bits in register
	reg &= (~mask);
	
	//set masked bits in register according to value
	return ((value << getMaskShift(mask)) & mask) | reg;
}
	
uint8_t AS3935MI::readRegisterValue(uint8_t reg, uint8_t mask)
{
	return getMaskedBits(readRegister(reg), mask);
}

void AS3935MI::writeRegisterValue(uint8_t reg, uint8_t mask, uint8_t value)
{
	uint8_t reg_val = readRegister(reg);
	writeRegister(reg, setMaskedBits(reg_val, mask, value));
}


uint32_t AS3935MI::computeCalibratedFrequency(int32_t divider)
{
	/*
	if ((divider < 16) || (divider > 128)) {
		return 0ul;
	}
	*/

	// Need to copy the timestamps first as they are volatile
	const uint32_t start = calibration_start_micros_;
	const uint32_t end	 = calibration_end_micros_;

	if ((start == 0ul) || (end == 0ul)) {
		return 0ul;
	}

	const int32_t duration_usec = (int32_t) (end - start);

	if (duration_usec <= 0l) {
		return 0ul;
	}

	// Compute measured frequency
	// we have duration of AS3935MI_NR_CALIBRATION_SAMPLES pulses in usec, thus measured frequency is:
	// (AS3935MI_NR_CALIBRATION_SAMPLES * 1000'000) / duration in usec.
	// Actual frequency should take the division ratio into account.
	uint64_t freq = (static_cast<uint64_t>(divider) * 1000000ull * (AS3935MI_NR_CALIBRATION_SAMPLES + 1));

	freq /=	duration_usec;

	return static_cast<uint32_t>(freq);
}


uint32_t AS3935MI::measureResonanceFrequency(display_frequency_source_t source, uint8_t tuningCapacitance)
{
	set_interruptMode(interrupt_mode_t::detached);

	// set tuning capacitors
	writeAntennaTuning(tuningCapacitance);
//	delayMicroseconds(AS3935_TIMEOUT);


	unsigned sourceFreq_kHz = 500;
	int32_t divider = 1;

	// display LCO on IRQ
	switch (source) {
		case display_frequency_source_t::LCO:
			displayLCO_on_IRQ(true);
			writeDivisionRatio(AS3935MI_LCO_DIVISION_RATIO);
			divider = 16 << static_cast<uint32_t>(AS3935MI_LCO_DIVISION_RATIO);
			sourceFreq_kHz = 500;
			break;

			// TD-er: Do not try to measure the 1.1 MHz signal as the ESP32 will not be able to keep up with all the interrupts.
		case display_frequency_source_t::SRCO:
			displaySRCO_on_IRQ(true);
			sourceFreq_kHz = 1100;
			break;
		case display_frequency_source_t::TRCO:
			displayTRCO_on_IRQ(true);
			sourceFreq_kHz = 33;
			break;
	}

	set_interruptMode(interrupt_mode_t::calibration);

	// Need to give enough time for the sensor to set the LCO signal on the IRQ pin
	delayMicroseconds(AS3935_TIMEOUT);
	calibration_end_micros_	  = 0ul;
	interrupt_count_		  = 0ul;
	calibration_start_micros_ = static_cast<uint32_t>(getMicros64());

	// Wait for the amount of samples to be counted (or timeout)
	// Typically this takes 32 msec for the 500 kHz LCO
	const unsigned expectedDuration = (divider * AS3935MI_NR_CALIBRATION_SAMPLES) / sourceFreq_kHz;
	const uint32_t timeout			= millis() + (2 * expectedDuration);
	uint32_t freq					= 0;

	while (freq == 0 && (((int32_t)(millis() - timeout)) < 0)) {
		delay(1);
		freq = computeCalibratedFrequency(divider);
	}

	set_interruptMode(interrupt_mode_t::detached);

	// stop displaying LCO on IRQ
	displayLCO_on_IRQ(false);

	return freq;
}

uint32_t AS3935MI::get_interruptTimestamp() const { 
	return interrupt_timestamp_; 
}

void AS3935MI::set_interruptMode(interrupt_mode_t mode) {
	if (mode_ == mode) {
		return;
	}

	// set the IRQ pin as an input pin. do not use INPUT_PULLUP - the AS3935 will pull the pin
	// high if an event is registered.
	pinMode(irq_, INPUT);

	if (mode_ != interrupt_mode_t::detached) {
		detachInterrupt(irq_);
	}
	interrupt_timestamp_ = 0;
	interrupt_count_	 = 0;
	mode_				 = mode;

	switch (mode) {
		case interrupt_mode_t::detached:
			detachInterrupt(irq_);
			break;
		case interrupt_mode_t::normal:
#ifdef AS3935MI_HAS_ATTACHINTERRUPTARG_FUNCTION
			attachInterruptArg(digitalPinToInterrupt(irq_),
							   reinterpret_cast<void (*)(void *)>(interrupt_ISR),
							   this,
							   RISING);
#else
			attachInterrupt(digitalPinToInterrupt(irq_),
							interrupt_ISR,
					   		RISING);
#endif
			break;
		case interrupt_mode_t::calibration:
			calibration_start_micros_ = 0;
			calibration_end_micros_	 = 0;
#ifdef AS3935MI_HAS_ATTACHINTERRUPTARG_FUNCTION
			attachInterruptArg(digitalPinToInterrupt(irq_),
							   reinterpret_cast<void (*)(void *)>(calibrate_ISR),
							   this,
							   RISING);
#else
			attachInterrupt(digitalPinToInterrupt(irq_),
							calibrate_ISR,
					   		RISING);
#endif
			break;
	}
}

#ifdef AS3935MI_HAS_ATTACHINTERRUPTARG_FUNCTION
void IRAM_ATTR AS3935MI::interrupt_ISR(AS3935MI *self) {
	self->interrupt_timestamp_ = millis();
}

void IRAM_ATTR AS3935MI::calibrate_ISR(AS3935MI *self) {
	// interrupt_count_ is volatile, so we can miss when testing for exactly AS3935MI_NR_CALIBRATION_SAMPLES
	if (self->interrupt_count_ < AS3935MI_NR_CALIBRATION_SAMPLES) {
		++self->interrupt_count_;
	}
	else if (self->calibration_end_micros_ == 0ul) {
		self->calibration_end_micros_ = static_cast<uint32_t>(getMicros64());
	}
}
#else
void IRAM_ATTR AS3935MI::interrupt_ISR() {
	interrupt_timestamp_ = millis();
}

void IRAM_ATTR AS3935MI::calibrate_ISR() {
	// interrupt_count_ is volatile, so we can miss when testing for exactly AS3935MI_NR_CALIBRATION_SAMPLES
	if (interrupt_count_ < AS3935MI_NR_CALIBRATION_SAMPLES) {
		++interrupt_count_;
	}
	else if (calibration_end_micros_ == 0ul) {
		calibration_end_micros_ = static_cast<uint32_t>(getMicros64());
	}
}
#endif
