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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include "AS3935MI.h"

#include <GPIO_Direct_Access.h>

AS3935MI::AS3935MI(uint8_t irq) :
	irq_(irq)
{
}

AS3935MI::~AS3935MI()
{
}

bool AS3935MI::begin()
{
	if (!beginInterface())
		return false;

	resetToDefaults();

	return true;
}

uint8_t AS3935MI::readStormDistance()
{
	return readRegisterValue(AS3935_REGISTER_DISTANCE, AS3935_MASK_DISTANCE);
}

uint8_t AS3935MI::readInterruptSource()
{
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
	writeRegisterValue(AS3935_REGISTER_DISP_SRCO, AS3935_MASK_DISP_SRCO, static_cast<uint8_t>(1));

	//wait for calibration to finish...
	delayMicroseconds(AS3935_TIMEOUT);

	//stop exposing clock on IRQ pin
	writeRegisterValue(AS3935_REGISTER_DISP_SRCO, AS3935_MASK_DISP_SRCO, static_cast<uint8_t>(0));

	//check calibration results. bits will be set if calibration failed.
	bool success_TRCO = !static_cast<bool>(readRegisterValue(AS3935_REGISTER_TRCO_CALIB_NOK, AS3935_MASK_TRCO_CALIB_NOK));
	bool success_SRCO = !static_cast<bool>(readRegisterValue(AS3935_REGISTER_SRCO_CALIB_NOK, AS3935_MASK_SRCO_CALIB_NOK));

	return (success_TRCO && success_SRCO);
}

bool AS3935MI::calibrateResonanceFrequency(int32_t &frequency, uint8_t division_ratio)
{
	if (readPowerDown())
		return false;

	int32_t divider = 16;

	switch (division_ratio)
	{
		case AS3935_DR_16:
			divider = 16;
			break;
		case AS3935_DR_32:
			divider = 32;
			break;
		case AS3935_DR_64:
			divider = 64;
			break;
		case AS3935_DR_128:
			divider = 128;
			break;
		default:
			division_ratio = AS3935_DR_16;
			divider = 16;
		break;
	}

	writeDivisionRatio(division_ratio);

	delayMicroseconds(AS3935_TIMEOUT);

	int16_t target = static_cast<int16_t>(500000 * 2 / divider / 10); //500kHz * 2 (counting each high-low / low-high transition) / divider * 0.1s 
	int16_t best_diff = 32767;
	uint8_t best_i = 0;

	for (uint8_t i = 0; i < 16; i++)
	{
		//set tuning capacitors
		writeAntennaTuning(i);

		delayMicroseconds(AS3935_TIMEOUT);

		//display LCO on IRQ
		displayLCO_on_IRQ(true);

		bool irq_current = DIRECT_pinRead(irq_);
		bool irq_last = irq_current;

		int16_t counts = 0;

		uint32_t time_start = millis();

		//count transitions for 100ms
		while ((millis() - time_start) < 100)
		{
			irq_current = DIRECT_pinRead(irq_);

			if (irq_current != irq_last)
				counts++;

			irq_last = irq_current;
		}

		//stop displaying LCO on IRQ
		displayLCO_on_IRQ(false);

		//remember if the current setting was better than the previous
		if (abs(counts - target) < abs(best_diff))
		{
			best_diff = counts - target;
			best_i = i;
		}
	}

	writeAntennaTuning(best_i);

	//calculate frequency the sensor has been tuned to
	frequency = (static_cast<int32_t>(target) + static_cast<int32_t>(best_diff));
	frequency *= (divider * 10 / 2);

	//return true if the absolute difference between best value and target value is < 3.5% of target value
	return (abs(best_diff) < (static_cast<int32_t>(target) * 35 / 1000) ? true : false);
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
	writeDivisionRatio(AS3935_DR_16);

	//display LCO on IRQ
    displayLCO_on_IRQ(true);
	delayMicroseconds(AS3935_TIMEOUT);

	bool irq_current = DIRECT_pinRead(irq_);
	bool irq_last = irq_current;

	int16_t counts = 0;

	uint32_t time_start = millis();

	//count transitions for 10ms
	while ((millis() - time_start) < 10)
	{
		irq_current = DIRECT_pinRead(irq_);

		if (irq_current != irq_last)
			counts++;

		irq_last = irq_current;
	}

	//stop displaying LCO on IRQ
	displayLCO_on_IRQ(false);

	//return true if at least 100 transition was detected (to prevent false positives). 
	return (counts > 100);
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