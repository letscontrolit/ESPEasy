/**
 ******************************************************************************
 * @file    vl53l0x_class.cpp
 * @author  IMG
 * @version V0.0.1
 * @date    14-December-2018
 * @brief   Implementation file for the VL53L1X driver class
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2018 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
*/

/* Includes */
#include <stdlib.h>
#include "Arduino.h"
#include "vl53l1x_class.h"

#define ALGO__PART_TO_PART_RANGE_OFFSET_MM 0x001E
#define MM_CONFIG__INNER_OFFSET_MM 0x0020
#define MM_CONFIG__OUTER_OFFSET_MM 0x0022

//#define DEBUG_MODE

const uint8_t VL51L1X_DEFAULT_CONFIGURATION[] = {
	0x00, /* 0x2d : set bit 2 and 5 to 1 for fast plus mode (1MHz I2C), else don't touch */
	0x01, /* 0x2e : bit 0 if I2C pulled up at 1.8V, else set bit 0 to 1 (pull up at AVDD) */
	0x01, /* 0x2f : bit 0 if GPIO pulled up at 1.8V, else set bit 0 to 1 (pull up at AVDD) */
	0x01, /* 0x30 : set bit 4 to 0 for active high interrupt and 1 for active low (bits 3:0 must be 0x1), use SetInterruptPolarity() */
	0x02, /* 0x31 : bit 1 = interrupt depending on the polarity, use CheckForDataReady() */
	0x00, /* 0x32 : not user-modifiable */
	0x02, /* 0x33 : not user-modifiable */
	0x08, /* 0x34 : not user-modifiable */
	0x00, /* 0x35 : not user-modifiable */
	0x08, /* 0x36 : not user-modifiable */
	0x10, /* 0x37 : not user-modifiable */
	0x01, /* 0x38 : not user-modifiable */
	0x01, /* 0x39 : not user-modifiable */
	0x00, /* 0x3a : not user-modifiable */
	0x00, /* 0x3b : not user-modifiable */
	0x00, /* 0x3c : not user-modifiable */
	0x00, /* 0x3d : not user-modifiable */
	0xff, /* 0x3e : not user-modifiable */
	0x00, /* 0x3f : not user-modifiable */
	0x0F, /* 0x40 : not user-modifiable */
	0x00, /* 0x41 : not user-modifiable */
	0x00, /* 0x42 : not user-modifiable */
	0x00, /* 0x43 : not user-modifiable */
	0x00, /* 0x44 : not user-modifiable */
	0x00, /* 0x45 : not user-modifiable */
	0x20, /* 0x46 : interrupt configuration 0->level low detection, 1-> level high, 2-> Out of window, 3->In window, 0x20-> New sample ready , TBC */
	0x0b, /* 0x47 : not user-modifiable */
	0x00, /* 0x48 : not user-modifiable */
	0x00, /* 0x49 : not user-modifiable */
	0x02, /* 0x4a : not user-modifiable */
	0x0a, /* 0x4b : not user-modifiable */
	0x21, /* 0x4c : not user-modifiable */
	0x00, /* 0x4d : not user-modifiable */
	0x00, /* 0x4e : not user-modifiable */
	0x05, /* 0x4f : not user-modifiable */
	0x00, /* 0x50 : not user-modifiable */
	0x00, /* 0x51 : not user-modifiable */
	0x00, /* 0x52 : not user-modifiable */
	0x00, /* 0x53 : not user-modifiable */
	0xc8, /* 0x54 : not user-modifiable */
	0x00, /* 0x55 : not user-modifiable */
	0x00, /* 0x56 : not user-modifiable */
	0x38, /* 0x57 : not user-modifiable */
	0xff, /* 0x58 : not user-modifiable */
	0x01, /* 0x59 : not user-modifiable */
	0x00, /* 0x5a : not user-modifiable */
	0x08, /* 0x5b : not user-modifiable */
	0x00, /* 0x5c : not user-modifiable */
	0x00, /* 0x5d : not user-modifiable */
	0x01, /* 0x5e : not user-modifiable */
	0xdb, /* 0x5f : not user-modifiable */
	0x0f, /* 0x60 : not user-modifiable */
	0x01, /* 0x61 : not user-modifiable */
	0xf1, /* 0x62 : not user-modifiable */
	0x0d, /* 0x63 : not user-modifiable */
	0x01, /* 0x64 : Sigma threshold MSB (mm in 14.2 format for MSB+LSB), use SetSigmaThreshold(), default value 90 mm  */
	0x68, /* 0x65 : Sigma threshold LSB */
	0x00, /* 0x66 : Min count Rate MSB (MCPS in 9.7 format for MSB+LSB), use SetSignalThreshold() */
	0x80, /* 0x67 : Min count Rate LSB */
	0x08, /* 0x68 : not user-modifiable */
	0xb8, /* 0x69 : not user-modifiable */
	0x00, /* 0x6a : not user-modifiable */
	0x00, /* 0x6b : not user-modifiable */
	0x00, /* 0x6c : Intermeasurement period MSB, 32 bits register, use SetIntermeasurementInMs() */
	0x00, /* 0x6d : Intermeasurement period */
	0x0f, /* 0x6e : Intermeasurement period */
	0x89, /* 0x6f : Intermeasurement period LSB */
	0x00, /* 0x70 : not user-modifiable */
	0x00, /* 0x71 : not user-modifiable */
	0x00, /* 0x72 : distance threshold high MSB (in mm, MSB+LSB), use SetD:tanceThreshold() */
	0x00, /* 0x73 : distance threshold high LSB */
	0x00, /* 0x74 : distance threshold low MSB ( in mm, MSB+LSB), use SetD:tanceThreshold() */
	0x00, /* 0x75 : distance threshold low LSB */
	0x00, /* 0x76 : not user-modifiable */
	0x01, /* 0x77 : not user-modifiable */
	0x0f, /* 0x78 : not user-modifiable */
	0x0d, /* 0x79 : not user-modifiable */
	0x0e, /* 0x7a : not user-modifiable */
	0x0e, /* 0x7b : not user-modifiable */
	0x00, /* 0x7c : not user-modifiable */
	0x00, /* 0x7d : not user-modifiable */
	0x02, /* 0x7e : not user-modifiable */
	0xc7, /* 0x7f : ROI center, use SetROI() */
	0xff, /* 0x80 : XY ROI (X=Width, Y=Height), use SetROI() */
	0x9B, /* 0x81 : not user-modifiable */
	0x00, /* 0x82 : not user-modifiable */
	0x00, /* 0x83 : not user-modifiable */
	0x00, /* 0x84 : not user-modifiable */
	0x01, /* 0x85 : not user-modifiable */
	0x00, /* 0x86 : clear interrupt, use ClearInterrupt() */
	0x00  /* 0x87 : start ranging, use StartRanging() or StopRanging(), If you want an automatic start after VL53L1X_init() call, put 0x40 in location 0x87 */
};

/* VL53L1X_api.h functions */

VL53L1X_ERROR VL53L1X::VL53L1X_GetSWVersion(VL53L1X_Version_t *pVersion)
{
	VL53L1X_ERROR Status = 0;

	pVersion->major = VL53L1X_IMPLEMENTATION_VER_MAJOR;
	pVersion->minor = VL53L1X_IMPLEMENTATION_VER_MINOR;
	pVersion->build = VL53L1X_IMPLEMENTATION_VER_SUB;
	pVersion->revision = VL53L1X_IMPLEMENTATION_VER_REVISION;
	return Status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_SetI2CAddress(uint8_t new_address)
{
	VL53L1X_ERROR status = 0;

	status = VL53L1_WrByte(Device, VL53L1_I2C_SLAVE__DEVICE_ADDRESS, new_address >> 1);
	Device->I2cDevAddr = new_address;

	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_SensorInit()
{
	VL53L1X_ERROR status = 0;
	uint8_t Addr = 0x00, dataReady = 0, timeout = 0;

	for (Addr = 0x2D; Addr <= 0x87; Addr++)
	{
		status = VL53L1_WrByte(Device, Addr, VL51L1X_DEFAULT_CONFIGURATION[Addr - 0x2D]);
	}
	status = VL53L1X_StartRanging();

	//We need to wait at least the default intermeasurement period of 103ms before dataready will occur
	//But if a unit has already been powered and polling, it may happen much faster
	while (dataReady == 0)
	{
		status = VL53L1X_CheckForDataReady(&dataReady);
		if (timeout++ > 150)
			return VL53L1_ERROR_TIME_OUT;
		delay(1);
	}
	status = VL53L1X_ClearInterrupt();
	status = VL53L1X_StopRanging();
	status = VL53L1_WrByte(Device, VL53L1_VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND, 0x09); /* two bounds VHV */
	status = VL53L1_WrByte(Device, 0x0B, 0);											/* start VHV from the previous temperature */
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_ClearInterrupt()
{
	VL53L1X_ERROR status = 0;

	status = VL53L1_WrByte(Device, SYSTEM__INTERRUPT_CLEAR, 0x01);
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_SetInterruptPolarity(uint8_t NewPolarity)
{
	uint8_t Temp;
	VL53L1X_ERROR status = 0;

	status = VL53L1_RdByte(Device, GPIO_HV_MUX__CTRL, &Temp);
	Temp = Temp & 0xEF;
	status = VL53L1_WrByte(Device, GPIO_HV_MUX__CTRL, Temp | (!(NewPolarity & 1)) << 4);
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_GetInterruptPolarity(uint8_t *pInterruptPolarity)
{
	uint8_t Temp;
	VL53L1X_ERROR status = 0;

	status = VL53L1_RdByte(Device, GPIO_HV_MUX__CTRL, &Temp);
	Temp = Temp & 0x10;
	*pInterruptPolarity = !(Temp >> 4);
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_StartRanging()
{
	VL53L1X_ERROR status = 0;

	status = VL53L1_WrByte(Device, SYSTEM__MODE_START, 0x40); /* Enable VL53L1X */
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_StopRanging()
{
	VL53L1X_ERROR status = 0;

	status = VL53L1_WrByte(Device, SYSTEM__MODE_START, 0x00); /* Disable VL53L1X */
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_CheckForDataReady(uint8_t *isDataReady)
{
	uint8_t Temp;
	uint8_t IntPol;
	VL53L1X_ERROR status = 0;

	status = VL53L1X_GetInterruptPolarity(&IntPol);
	status = VL53L1_RdByte(Device, GPIO__TIO_HV_STATUS, &Temp);
	/* Read in the register to check if a new value is available */
	if (status == 0)
	{
		if ((Temp & 1) == IntPol)
			*isDataReady = 1;
		else
			*isDataReady = 0;
	}
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_SetTimingBudgetInMs(uint16_t TimingBudgetInMs)
{
	uint16_t DM;
	VL53L1X_ERROR status = 0;

	status = VL53L1X_GetDistanceMode(&DM);
	if (DM == 0)
		return 1;
	else if (DM == 1)
	{ /* Short DistanceMode */
		switch (TimingBudgetInMs)
		{
		case 15: /* only available in short distance mode */
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,
						  0x01D);
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,
						  0x0027);
			break;
		case 20:
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,
						  0x0051);
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,
						  0x006E);
			break;
		case 33:
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,
						  0x00D6);
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,
						  0x006E);
			break;
		case 50:
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,
						  0x1AE);
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,
						  0x01E8);
			break;
		case 100:
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,
						  0x02E1);
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,
						  0x0388);
			break;
		case 200:
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,
						  0x03E1);
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,
						  0x0496);
			break;
		case 500:
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,
						  0x0591);
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,
						  0x05C1);
			break;
		default:
			status = 1;
			break;
		}
	}
	else
	{
		switch (TimingBudgetInMs)
		{
		case 20:
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,
						  0x001E);
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,
						  0x0022);
			break;
		case 33:
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,
						  0x0060);
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,
						  0x006E);
			break;
		case 50:
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,
						  0x00AD);
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,
						  0x00C6);
			break;
		case 100:
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,
						  0x01CC);
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,
						  0x01EA);
			break;
		case 200:
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,
						  0x02D9);
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,
						  0x02F8);
			break;
		case 500:
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_A_HI,
						  0x048F);
			VL53L1_WrWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_B_HI,
						  0x04A4);
			break;
		default:
			status = 1;
			break;
		}
	}
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_GetTimingBudgetInMs(uint16_t *pTimingBudget)
{
	uint16_t Temp;
	VL53L1X_ERROR status = 0;

	status = VL53L1_RdWord(Device, RANGE_CONFIG__TIMEOUT_MACROP_A_HI, &Temp);
	switch (Temp)
	{
	case 0x001D:
		*pTimingBudget = 15;
		break;
	case 0x0051:
	case 0x001E:
		*pTimingBudget = 20;
		break;
	case 0x00D6:
	case 0x0060:
		*pTimingBudget = 33;
		break;
	case 0x1AE:
	case 0x00AD:
		*pTimingBudget = 50;
		break;
	case 0x02E1:
	case 0x01CC:
		*pTimingBudget = 100;
		break;
	case 0x03E1:
	case 0x02D9:
		*pTimingBudget = 200;
		break;
	case 0x0591:
	case 0x048F:
		*pTimingBudget = 500;
		break;
	default:
		*pTimingBudget = 0;
		break;
	}
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_SetDistanceMode(uint16_t DM)
{
	uint16_t TB;
	VL53L1X_ERROR status = 0;

	status = VL53L1X_GetTimingBudgetInMs(&TB);
	switch (DM)
	{
	case 1:
		status = VL53L1_WrByte(Device, PHASECAL_CONFIG__TIMEOUT_MACROP, 0x14);
		status = VL53L1_WrByte(Device, RANGE_CONFIG__VCSEL_PERIOD_A, 0x07);
		status = VL53L1_WrByte(Device, RANGE_CONFIG__VCSEL_PERIOD_B, 0x05);
		status = VL53L1_WrByte(Device, RANGE_CONFIG__VALID_PHASE_HIGH, 0x38);
		status = VL53L1_WrWord(Device, SD_CONFIG__WOI_SD0, 0x0705);
		status = VL53L1_WrWord(Device, SD_CONFIG__INITIAL_PHASE_SD0, 0x0606);
		break;
	case 2:
		status = VL53L1_WrByte(Device, PHASECAL_CONFIG__TIMEOUT_MACROP, 0x0A);
		status = VL53L1_WrByte(Device, RANGE_CONFIG__VCSEL_PERIOD_A, 0x0F);
		status = VL53L1_WrByte(Device, RANGE_CONFIG__VCSEL_PERIOD_B, 0x0D);
		status = VL53L1_WrByte(Device, RANGE_CONFIG__VALID_PHASE_HIGH, 0xB8);
		status = VL53L1_WrWord(Device, SD_CONFIG__WOI_SD0, 0x0F0D);
		status = VL53L1_WrWord(Device, SD_CONFIG__INITIAL_PHASE_SD0, 0x0E0E);
		break;
	default:
		break;
	}
	status = VL53L1X_SetTimingBudgetInMs(TB);
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_GetDistanceMode(uint16_t *DM)
{
	uint8_t TempDM, status = 0;

	status = VL53L1_RdByte(Device, PHASECAL_CONFIG__TIMEOUT_MACROP, &TempDM);
	if (TempDM == 0x14)
		*DM = 1;
	if (TempDM == 0x0A)
		*DM = 2;
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_SetInterMeasurementInMs(uint16_t InterMeasMs)
{
	uint16_t ClockPLL;
	VL53L1X_ERROR status = 0;

	status = VL53L1_RdWord(Device, VL53L1_RESULT__OSC_CALIBRATE_VAL, &ClockPLL);
	ClockPLL = ClockPLL & 0x3FF;
	VL53L1_WrDWord(Device, VL53L1_SYSTEM__INTERMEASUREMENT_PERIOD,
				   (uint32_t)(ClockPLL * InterMeasMs * 1.075));
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_GetInterMeasurementInMs(uint16_t *pIM)
{
	uint16_t ClockPLL;
	VL53L1X_ERROR status = 0;
	uint32_t tmp;

	status = VL53L1_RdDWord(Device, VL53L1_SYSTEM__INTERMEASUREMENT_PERIOD, &tmp);
	*pIM = (uint16_t)tmp;
	status = VL53L1_RdWord(Device, VL53L1_RESULT__OSC_CALIBRATE_VAL, &ClockPLL);
	ClockPLL = ClockPLL & 0x3FF;
	*pIM = (uint16_t)(*pIM / (ClockPLL * 1.065));
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_BootState(uint8_t *state)
{
	VL53L1X_ERROR status = 0;
	uint8_t tmp = 0;

	status = VL53L1_RdByte(Device, VL53L1_FIRMWARE__SYSTEM_STATUS, &tmp);
	*state = tmp;
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_GetSensorId(uint16_t *sensorId)
{
	VL53L1X_ERROR status = 0;
	uint16_t tmp = 0;

	status = VL53L1_RdWord(Device, VL53L1_IDENTIFICATION__MODEL_ID, &tmp);
	*sensorId = tmp;
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_GetDistance(uint16_t *distance)
{
	VL53L1X_ERROR status = 0;
	uint16_t tmp;

	status = (VL53L1_RdWord(Device,
							VL53L1_RESULT__FINAL_CROSSTALK_CORRECTED_RANGE_MM_SD0, &tmp));
	*distance = tmp;
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_GetSignalPerSpad(uint16_t *signalRate)
{
	VL53L1X_ERROR status = 0;
	uint16_t SpNb = 1, signal;

	status = VL53L1_RdWord(Device,
						   VL53L1_RESULT__PEAK_SIGNAL_COUNT_RATE_CROSSTALK_CORRECTED_MCPS_SD0, &signal);
	status = VL53L1_RdWord(Device,
						   VL53L1_RESULT__DSS_ACTUAL_EFFECTIVE_SPADS_SD0, &SpNb);
	*signalRate = (uint16_t)(2000.0 * signal / SpNb);
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_GetAmbientPerSpad(uint16_t *ambPerSp)
{
	VL53L1X_ERROR status = 0;
	uint16_t AmbientRate, SpNb = 1;

	status = VL53L1_RdWord(Device, RESULT__AMBIENT_COUNT_RATE_MCPS_SD, &AmbientRate);
	status = VL53L1_RdWord(Device, VL53L1_RESULT__DSS_ACTUAL_EFFECTIVE_SPADS_SD0, &SpNb);
	*ambPerSp = (uint16_t)(2000.0 * AmbientRate / SpNb);
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_GetSignalRate(uint16_t *signal)
{
	VL53L1X_ERROR status = 0;
	uint16_t tmp;

	status = VL53L1_RdWord(Device,
						   VL53L1_RESULT__PEAK_SIGNAL_COUNT_RATE_CROSSTALK_CORRECTED_MCPS_SD0, &tmp);
	*signal = tmp * 8;
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_GetSpadNb(uint16_t *spNb)
{
	VL53L1X_ERROR status = 0;
	uint16_t tmp;

	status = VL53L1_RdWord(Device,
						   VL53L1_RESULT__DSS_ACTUAL_EFFECTIVE_SPADS_SD0, &tmp);
	*spNb = tmp >> 8;
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_GetAmbientRate(uint16_t *ambRate)
{
	VL53L1X_ERROR status = 0;
	uint16_t tmp;

	status = VL53L1_RdWord(Device, RESULT__AMBIENT_COUNT_RATE_MCPS_SD, &tmp);
	*ambRate = tmp * 8;
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_GetRangeStatus(uint8_t *rangeStatus)
{
	VL53L1X_ERROR status = 0;
	uint8_t RgSt;

	status = VL53L1_RdByte(Device, VL53L1_RESULT__RANGE_STATUS, &RgSt);
	RgSt = RgSt & 0x1F;
	switch (RgSt)
	{
	case 9:
		RgSt = 0;
		break;
	case 6:
		RgSt = 1;
		break;
	case 4:
		RgSt = 2;
		break;
	case 8:
		RgSt = 3;
		break;
	case 5:
		RgSt = 4;
		break;
	case 3:
		RgSt = 5;
		break;
	case 19:
		RgSt = 6;
		break;
	case 7:
		RgSt = 7;
		break;
	case 12:
		RgSt = 9;
		break;
	case 18:
		RgSt = 10;
		break;
	case 22:
		RgSt = 11;
		break;
	case 23:
		RgSt = 12;
		break;
	case 13:
		RgSt = 13;
		break;
	default:
		RgSt = 255;
		break;
	}
	*rangeStatus = RgSt;
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_SetOffset(int16_t OffsetValue)
{
	VL53L1X_ERROR status = 0;
	int16_t Temp;

	Temp = (OffsetValue * 4);
	VL53L1_WrWord(Device, ALGO__PART_TO_PART_RANGE_OFFSET_MM,
				  (uint16_t)Temp);
	VL53L1_WrWord(Device, MM_CONFIG__INNER_OFFSET_MM, 0x0);
	VL53L1_WrWord(Device, MM_CONFIG__OUTER_OFFSET_MM, 0x0);
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_GetOffset(int16_t *offset)
{
	VL53L1X_ERROR status = 0;
	uint16_t Temp;

	status = VL53L1_RdWord(Device, ALGO__PART_TO_PART_RANGE_OFFSET_MM, &Temp);
	Temp = Temp << 3;
	Temp = Temp >> 5;
	*offset = (int16_t)(Temp);
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_SetXtalk(uint16_t XtalkValue)
{
	/* XTalkValue in count per second to avoid float type */
	VL53L1X_ERROR status = 0;

	status = VL53L1_WrWord(Device,
						   ALGO__CROSSTALK_COMPENSATION_X_PLANE_GRADIENT_KCPS,
						   0x0000);
	status = VL53L1_WrWord(Device, ALGO__CROSSTALK_COMPENSATION_Y_PLANE_GRADIENT_KCPS,
						   0x0000);
	status = VL53L1_WrWord(Device, ALGO__CROSSTALK_COMPENSATION_PLANE_OFFSET_KCPS,
						   (XtalkValue << 9) / 1000); /* * << 9 (7.9 format) and /1000 to convert cps to kpcs */
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_GetXtalk(uint16_t *xtalk)
{
	VL53L1X_ERROR status = 0;
	uint16_t tmp;

	status = VL53L1_RdWord(Device, ALGO__CROSSTALK_COMPENSATION_PLANE_OFFSET_KCPS, &tmp);
	*xtalk = (tmp * 1000) >> 9; /* * 1000 to convert kcps to cps and >> 9 (7.9 format) */
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_SetDistanceThreshold(uint16_t ThreshLow,
													uint16_t ThreshHigh, uint8_t Window,
													uint8_t IntOnNoTarget)
{
	VL53L1X_ERROR status = 0;
	uint8_t Temp = 0;

	status = VL53L1_RdByte(Device, SYSTEM__INTERRUPT_CONFIG_GPIO, &Temp);
	Temp = Temp & 0x47;
	if (IntOnNoTarget == 0)
	{
		status = VL53L1_WrByte(Device, SYSTEM__INTERRUPT_CONFIG_GPIO,
							   (Temp | (Window & 0x07)));
	}
	else
	{
		status = VL53L1_WrByte(Device, SYSTEM__INTERRUPT_CONFIG_GPIO,
							   ((Temp | (Window & 0x07)) | 0x40));
	}
	status = VL53L1_WrWord(Device, SYSTEM__THRESH_HIGH, ThreshHigh);
	status = VL53L1_WrWord(Device, SYSTEM__THRESH_LOW, ThreshLow);
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_GetDistanceThresholdWindow(uint16_t *window)
{
	VL53L1X_ERROR status = 0;
	uint8_t tmp;
	status = VL53L1_RdByte(Device, SYSTEM__INTERRUPT_CONFIG_GPIO, &tmp);
	*window = (uint16_t)(tmp & 0x7);
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_GetDistanceThresholdLow(uint16_t *low)
{
	VL53L1X_ERROR status = 0;
	uint16_t tmp;

	status = VL53L1_RdWord(Device, SYSTEM__THRESH_LOW, &tmp);
	*low = tmp;
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_GetDistanceThresholdHigh(uint16_t *high)
{
	VL53L1X_ERROR status = 0;
	uint16_t tmp;

	status = VL53L1_RdWord(Device, SYSTEM__THRESH_HIGH, &tmp);
	*high = tmp;
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_SetROI(uint8_t X, uint8_t Y, uint8_t opticalCenter)
{
	VL53L1X_ERROR status = 0;

	if (X > 16)
		X = 16;
	if (Y > 16)
		Y = 16;
	if (X > 10 || Y > 10)
	{
		opticalCenter = 199;
	}
	status = VL53L1_WrByte(Device, ROI_CONFIG__USER_ROI_CENTRE_SPAD, opticalCenter);
	status = VL53L1_WrByte(Device, ROI_CONFIG__USER_ROI_REQUESTED_GLOBAL_XY_SIZE,
						   (Y - 1) << 4 | (X - 1));
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_GetROI_XY(uint16_t *ROI_X, uint16_t *ROI_Y)
{
	VL53L1X_ERROR status = 0;
	uint8_t tmp;

	status = VL53L1_RdByte(Device, ROI_CONFIG__USER_ROI_REQUESTED_GLOBAL_XY_SIZE, &tmp);
	*ROI_X = ((uint16_t)tmp & 0x0F) + 1;
	*ROI_Y = (((uint16_t)tmp & 0xF0) >> 4) + 1;
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_SetSignalThreshold(uint16_t Signal)
{
	VL53L1X_ERROR status = 0;

	VL53L1_WrWord(Device, RANGE_CONFIG__MIN_COUNT_RATE_RTN_LIMIT_MCPS, Signal >> 3);
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_GetSignalThreshold(uint16_t *signal)
{
	VL53L1X_ERROR status = 0;
	uint16_t tmp;

	status = VL53L1_RdWord(Device,
						   RANGE_CONFIG__MIN_COUNT_RATE_RTN_LIMIT_MCPS, &tmp);
	*signal = tmp << 3;
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_SetSigmaThreshold(uint16_t Sigma)
{
	VL53L1X_ERROR status = 0;

	if (Sigma > (0xFFFF >> 2))
	{
		return 1;
	}
	/* 16 bits register 14.2 format */
	status = VL53L1_WrWord(Device, RANGE_CONFIG__SIGMA_THRESH, Sigma << 2);
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_GetSigmaThreshold(uint16_t *sigma)
{
	VL53L1X_ERROR status = 0;
	uint16_t tmp;

	status = VL53L1_RdWord(Device, RANGE_CONFIG__SIGMA_THRESH, &tmp);
	*sigma = tmp >> 2;
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1X_StartTemperatureUpdate()
{
	VL53L1X_ERROR status = 0;
	uint8_t tmp = 0;

	status = VL53L1_WrByte(Device, VL53L1_VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND, 0x81); /* full VHV */
	status = VL53L1_WrByte(Device, 0x0B, 0x92);
	status = VL53L1X_StartRanging();
	while (tmp == 0)
	{
		status = VL53L1X_CheckForDataReady(&tmp);
	}
	tmp = 0;
	status = VL53L1X_ClearInterrupt();
	status = VL53L1X_StopRanging();
	status = VL53L1_WrByte(Device, VL53L1_VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND, 0x09); /* two bounds VHV */
	status = VL53L1_WrByte(Device, 0x0B, 0);											/* start VHV from the previous temperature */
	return status;
}

/* VL53L1X_calibration.h functions */

int8_t VL53L1X::VL53L1X_CalibrateOffset(uint16_t TargetDistInMm, int16_t *offset)
{
	uint8_t i = 0, tmp;
	int16_t AverageDistance = 0;
	uint16_t distance;
	VL53L1X_ERROR status = 0;

	status = VL53L1_WrWord(Device, ALGO__PART_TO_PART_RANGE_OFFSET_MM, 0x0);
	status = VL53L1_WrWord(Device, MM_CONFIG__INNER_OFFSET_MM, 0x0);
	status = VL53L1_WrWord(Device, MM_CONFIG__OUTER_OFFSET_MM, 0x0);
	status = VL53L1X_StartRanging(); /* Enable VL53L1X sensor */
	for (i = 0; i < 50; i++)
	{
		while (tmp == 0)
		{
			status = VL53L1X_CheckForDataReady(&tmp);
		}
		tmp = 0;
		status = VL53L1X_GetDistance(&distance);
		status = VL53L1X_ClearInterrupt();
		AverageDistance = AverageDistance + distance;
	}
	status = VL53L1X_StopRanging();
	AverageDistance = AverageDistance / 50;
	*offset = TargetDistInMm - AverageDistance;
	status = VL53L1_WrWord(Device, ALGO__PART_TO_PART_RANGE_OFFSET_MM, *offset * 4);
	return status;
}

int8_t VL53L1X::VL53L1X_CalibrateXtalk(uint16_t TargetDistInMm, uint16_t *xtalk)
{
	uint8_t i, tmp = 0;
	float AverageSignalRate = 0;
	float AverageDistance = 0;
	float AverageSpadNb = 0;
	uint16_t distance = 0, spadNum;
	uint16_t sr;
	VL53L1X_ERROR status = 0;

	status = VL53L1_WrWord(Device, 0x0016, 0);
	status = VL53L1X_StartRanging();
	for (i = 0; i < 50; i++)
	{
		while (tmp == 0)
		{
			status = VL53L1X_CheckForDataReady(&tmp);
		}
		tmp = 0;
		status = VL53L1X_GetSignalRate(&sr);
		status = VL53L1X_GetDistance(&distance);
		status = VL53L1X_ClearInterrupt();
		AverageDistance = AverageDistance + distance;
		status = VL53L1X_GetSpadNb(&spadNum);
		AverageSpadNb = AverageSpadNb + spadNum;
		AverageSignalRate =
			AverageSignalRate + sr;
	}
	status = VL53L1X_StopRanging();
	AverageDistance = AverageDistance / 50;
	AverageSpadNb = AverageSpadNb / 50;
	AverageSignalRate = AverageSignalRate / 50;
	/* Calculate Xtalk value */
	*xtalk = (uint16_t)(512 * (AverageSignalRate * (1 - (AverageDistance / TargetDistInMm))) / AverageSpadNb);
	status = VL53L1_WrWord(Device, 0x0016, *xtalk);
	return status;
}

/* Write and read functions from I2C */

VL53L1X_ERROR VL53L1X::VL53L1_WriteMulti(VL53L1_DEV Dev, uint16_t index, uint8_t *pdata, uint32_t count)
{
	int status;

	status = VL53L1_I2CWrite(Dev->I2cDevAddr, index, pdata, (uint16_t)count);
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1_ReadMulti(VL53L1_DEV Dev, uint16_t index, uint8_t *pdata, uint32_t count)
{
	int status;

	status = VL53L1_I2CRead(Dev->I2cDevAddr, index, pdata, (uint16_t)count);

	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1_WrByte(VL53L1_DEV Dev, uint16_t index, uint8_t data)
{
	int status;

	status = VL53L1_I2CWrite(Dev->I2cDevAddr, index, &data, 1);
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1_WrWord(VL53L1_DEV Dev, uint16_t index, uint16_t data)
{
	int status;
	uint8_t buffer[2];

	buffer[0] = data >> 8;
	buffer[1] = data & 0x00FF;
	status = VL53L1_I2CWrite(Dev->I2cDevAddr, index, (uint8_t *)buffer, 2);
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1_WrDWord(VL53L1_DEV Dev, uint16_t index, uint32_t data)
{
	int status;
	uint8_t buffer[4];

	buffer[0] = (data >> 24) & 0xFF;
	buffer[1] = (data >> 16) & 0xFF;
	buffer[2] = (data >> 8) & 0xFF;
	buffer[3] = (data >> 0) & 0xFF;
	status = VL53L1_I2CWrite(Dev->I2cDevAddr, index, (uint8_t *)buffer, 4);
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1_RdByte(VL53L1_DEV Dev, uint16_t index, uint8_t *data)
{
	int status;

	status = VL53L1_I2CRead(Dev->I2cDevAddr, index, data, 1);

	if (status)
		return -1;

	return 0;
}

VL53L1X_ERROR VL53L1X::VL53L1_RdWord(VL53L1_DEV Dev, uint16_t index, uint16_t *data)
{
	int status;
	uint8_t buffer[2] = {0, 0};

	status = VL53L1_I2CRead(Dev->I2cDevAddr, index, buffer, 2);
	if (!status)
	{
		*data = (buffer[0] << 8) + buffer[1];
	}
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1_RdDWord(VL53L1_DEV Dev, uint16_t index, uint32_t *data)
{
	int status;
	uint8_t buffer[4] = {0, 0, 0, 0};

	status = VL53L1_I2CRead(Dev->I2cDevAddr, index, buffer, 4);
	if (!status)
	{
		*data = (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
	}
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1_UpdateByte(VL53L1_DEV Dev, uint16_t index, uint8_t AndData, uint8_t OrData)
{
	int status;
	uint8_t buffer = 0;

	/* read data direct onto buffer */
	status = VL53L1_I2CRead(Dev->I2cDevAddr, index, &buffer, 1);
	if (!status)
	{
		buffer = (buffer & AndData) | OrData;
		status = VL53L1_I2CWrite(Dev->I2cDevAddr, index, &buffer, (uint16_t)1);
	}
	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1_I2CWrite(uint8_t DeviceAddr, uint16_t RegisterAddr, uint8_t *pBuffer, uint16_t NumByteToWrite)
{
#ifdef DEBUG_MODE
	Serial.print("Beginning transmission to ");
	Serial.println(((DeviceAddr) >> 1) & 0x7F);
#endif
	dev_i2c->beginTransmission(((uint8_t)(((DeviceAddr) >> 1) & 0x7F)));
#ifdef DEBUG_MODE
	Serial.print("Writing port number ");
	Serial.println(RegisterAddr);
#endif
	uint8_t buffer[2];
	buffer[0] = RegisterAddr >> 8;
	buffer[1] = RegisterAddr & 0xFF;
	dev_i2c->write(buffer, 2);
	for (int i = 0; i < NumByteToWrite; i++)
		dev_i2c->write(pBuffer[i]);

	dev_i2c->endTransmission(true);
	return 0;
}

VL53L1X_ERROR VL53L1X::VL53L1_I2CRead(uint8_t DeviceAddr, uint16_t RegisterAddr, uint8_t *pBuffer, uint16_t NumByteToRead)
{
	int status = 0;

	//Loop until the port is transmitted correctly
	uint8_t maxAttempts = 5;
	for (uint8_t x = 0; x < maxAttempts; x++)
	{
#ifdef DEBUG_MODE
		Serial.print("Beginning transmission to ");
		Serial.println(((DeviceAddr) >> 1) & 0x7F);
#endif
		dev_i2c->beginTransmission(((uint8_t)(((DeviceAddr) >> 1) & 0x7F)));
#ifdef DEBUG_MODE
		Serial.print("Writing port number ");
		Serial.println(RegisterAddr);
#endif
		uint8_t buffer[2];
		buffer[0] = RegisterAddr >> 8;
		buffer[1] = RegisterAddr & 0xFF;
		dev_i2c->write(buffer, 2);
		status = dev_i2c->endTransmission(false);

		if (status == 0)
			break;

//Fix for some STM32 boards
//Reinitialize th i2c bus with the default parameters
#ifdef ARDUINO_ARCH_STM32
		if (status)
		{
			dev_i2c->end();
			dev_i2c->begin();
		}
#endif
		//End of fix
	}

	dev_i2c->requestFrom(((uint8_t)(((DeviceAddr) >> 1) & 0x7F)), (byte)NumByteToRead);

	int i = 0;
	while (dev_i2c->available())
	{
		pBuffer[i] = dev_i2c->read();
		i++;
	}

	return 0;
}

VL53L1X_ERROR VL53L1X::VL53L1_GetTickCount(
	uint32_t *ptick_count_ms)
{

	/* Returns current tick count in [ms] */

	VL53L1X_ERROR status = VL53L1_ERROR_NONE;

	//*ptick_count_ms = timeGetTime();
	*ptick_count_ms = 0;

	return status;
}

VL53L1X_ERROR VL53L1X::VL53L1_WaitUs(VL53L1_Dev_t *pdev, int32_t wait_us)
{
	(void)pdev;
	delay(wait_us / 1000);
	return VL53L1_ERROR_NONE;
}

VL53L1X_ERROR VL53L1X::VL53L1_WaitMs(VL53L1_Dev_t *pdev, int32_t wait_ms)
{
	(void)pdev;
	delay(wait_ms);
	return VL53L1_ERROR_NONE;
}

VL53L1X_ERROR VL53L1X::VL53L1_WaitValueMaskEx(
	VL53L1_Dev_t *pdev,
	uint32_t timeout_ms,
	uint16_t index,
	uint8_t value,
	uint8_t mask,
	uint32_t poll_delay_ms)
{

	/*
	 * Platform implementation of WaitValueMaskEx V2WReg script command
	 *
	 * WaitValueMaskEx(
	 *          duration_ms,
	 *          index,
	 *          value,
	 *          mask,
	 *          poll_delay_ms);
	 */

	VL53L1_Error status = VL53L1_ERROR_NONE;
	uint32_t start_time_ms = 0;
	uint32_t current_time_ms = 0;
	uint32_t polling_time_ms = 0;
	uint8_t byte_value = 0;
	uint8_t found = 0;

	/* calculate time limit in absolute time */

	VL53L1_GetTickCount(&start_time_ms);

	/* remember current trace functions and temporarily disable
	 * function logging
	 */

	/* wait until value is found, timeout reached on error occurred */

	while ((status == VL53L1_ERROR_NONE) &&
		   (polling_time_ms < timeout_ms) &&
		   (found == 0))
	{

		if (status == VL53L1_ERROR_NONE)
			status = VL53L1_RdByte(
				pdev,
				index,
				&byte_value);

		if ((byte_value & mask) == value)
			found = 1;

		if (status == VL53L1_ERROR_NONE &&
			found == 0 &&
			poll_delay_ms > 0)
			status = VL53L1_WaitMs(
				pdev,
				poll_delay_ms);

		/* Update polling time (Compare difference rather than absolute to
		negate 32bit wrap around issue) */
		VL53L1_GetTickCount(&current_time_ms);
		polling_time_ms = current_time_ms - start_time_ms;
	}

	if (found == 0 && status == VL53L1_ERROR_NONE)
		status = VL53L1_ERROR_TIME_OUT;

	return status;
}
