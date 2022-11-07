/*******************************************************************************
 Copyright Ã‚Â© 2018, STMicroelectronics International N.V.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of STMicroelectronics nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 NON-INFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS ARE DISCLAIMED.
 IN NO EVENT SHALL STMICROELECTRONICS INTERNATIONAL N.V. BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#ifndef __VL53L1X_CLASS_H
#define __VL53L1X_CLASS_H


#ifdef _MSC_VER
#   ifdef VL53L1X_API_EXPORTS
#       define VL53L1X_API  __declspec(dllexport)
#   else
#       define VL53L1X_API
#   endif
#else
#   define VL53L1X_API
#endif

/* Includes ------------------------------------------------------------------*/
#include "Arduino.h"
#include "Wire.h"
#include "RangeSensor.h"
#include "vl53l1_error_codes.h"


#define VL53L1X_IMPLEMENTATION_VER_MAJOR       1
#define VL53L1X_IMPLEMENTATION_VER_MINOR       0
#define VL53L1X_IMPLEMENTATION_VER_SUB         1
#define VL53L1X_IMPLEMENTATION_VER_REVISION  0000

typedef int8_t VL53L1X_ERROR;

#define SOFT_RESET											0x0000
#define VL53L1_I2C_SLAVE__DEVICE_ADDRESS					0x0001
#define VL53L1_VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND        0x0008
#define ALGO__CROSSTALK_COMPENSATION_PLANE_OFFSET_KCPS 		0x0016
#define ALGO__CROSSTALK_COMPENSATION_X_PLANE_GRADIENT_KCPS 	0x0018
#define ALGO__CROSSTALK_COMPENSATION_Y_PLANE_GRADIENT_KCPS 	0x001A
#define ALGO__PART_TO_PART_RANGE_OFFSET_MM					0x001E
#define MM_CONFIG__INNER_OFFSET_MM							0x0020
#define MM_CONFIG__OUTER_OFFSET_MM 							0x0022
#define GPIO_HV_MUX__CTRL									0x0030
#define GPIO__TIO_HV_STATUS       							0x0031
#define SYSTEM__INTERRUPT_CONFIG_GPIO 						0x0046
#define PHASECAL_CONFIG__TIMEOUT_MACROP     				0x004B
#define RANGE_CONFIG__TIMEOUT_MACROP_A_HI   				0x005E
#define RANGE_CONFIG__VCSEL_PERIOD_A        				0x0060
#define RANGE_CONFIG__VCSEL_PERIOD_B						0x0063
#define RANGE_CONFIG__TIMEOUT_MACROP_B_HI  					0x0061
#define RANGE_CONFIG__TIMEOUT_MACROP_B_LO  					0x0062
#define RANGE_CONFIG__SIGMA_THRESH 							0x0064
#define RANGE_CONFIG__MIN_COUNT_RATE_RTN_LIMIT_MCPS			0x0066
#define RANGE_CONFIG__VALID_PHASE_HIGH      				0x0069
#define VL53L1_SYSTEM__INTERMEASUREMENT_PERIOD				0x006C
#define SYSTEM__THRESH_HIGH 								0x0072
#define SYSTEM__THRESH_LOW 									0x0074
#define SD_CONFIG__WOI_SD0                  				0x0078
#define SD_CONFIG__INITIAL_PHASE_SD0        				0x007A
#define ROI_CONFIG__USER_ROI_CENTRE_SPAD					0x007F
#define ROI_CONFIG__USER_ROI_REQUESTED_GLOBAL_XY_SIZE		0x0080
#define SYSTEM__SEQUENCE_CONFIG								0x0081
#define VL53L1_SYSTEM__GROUPED_PARAMETER_HOLD 				0x0082
#define SYSTEM__INTERRUPT_CLEAR       						0x0086
#define SYSTEM__MODE_START                 					0x0087
#define VL53L1_RESULT__RANGE_STATUS							0x0089
#define VL53L1_RESULT__DSS_ACTUAL_EFFECTIVE_SPADS_SD0		0x008C
#define RESULT__AMBIENT_COUNT_RATE_MCPS_SD					0x0090
#define VL53L1_RESULT__FINAL_CROSSTALK_CORRECTED_RANGE_MM_SD0				0x0096
#define VL53L1_RESULT__PEAK_SIGNAL_COUNT_RATE_CROSSTALK_CORRECTED_MCPS_SD0 	0x0098
#define VL53L1_RESULT__OSC_CALIBRATE_VAL					0x00DE
#define VL53L1_FIRMWARE__SYSTEM_STATUS                      0x00E5
#define VL53L1_IDENTIFICATION__MODEL_ID                     0x010F
#define VL53L1_ROI_CONFIG__MODE_ROI_CENTRE_SPAD				0x013E


#define VL53L1X_DEFAULT_DEVICE_ADDRESS						0x52

/****************************************
 * PRIVATE define do not edit
 ****************************************/

/**
 *  @brief defines SW Version
 */
typedef struct {
	uint8_t      major;    /*!< major number */
	uint8_t      minor;    /*!< minor number */
	uint8_t      build;    /*!< build number */
	uint32_t     revision; /*!< revision number */
} VL53L1X_Version_t;


typedef struct {

	uint8_t   I2cDevAddr;
	TwoWire *I2cHandle;

} VL53L1_Dev_t;

typedef VL53L1_Dev_t *VL53L1_DEV;


/* Classes -------------------------------------------------------------------*/
/** Class representing a VL53L1 sensor component
 */
class VL53L1X : public RangeSensor
{
 public:
    /** Constructor
     * @param[in] &i2c device I2C to be used for communication
     * @param[in] &pin_gpio1 pin Mbed InterruptIn PinName to be used as component GPIO_1 INT
     * @param[in] DevAddr device address, 0x52 by default
     */
    VL53L1X(TwoWire *i2c, int pin, int pin_gpio1) : RangeSensor(), dev_i2c(i2c), gpio0(pin), gpio1Int(pin_gpio1)
    {
       MyDevice.I2cDevAddr=VL53L1X_DEFAULT_DEVICE_ADDRESS;
       MyDevice.I2cHandle = i2c;
       Device = &MyDevice;
       if(gpio0 >= 0)
       {
         pinMode(gpio0, OUTPUT);
       }
    }
    
   /** Destructor
    */
    virtual ~VL53L1X(){}
    /* warning: VL53L1 class inherits from GenericSensor, RangeSensor and LightSensor, that haven`t a destructor.
       The warning should request to introduce a virtual destructor to make sure to delete the object */

	/*** Interface Methods ***/
	/*** High level API ***/
	/**
	 * @brief       PowerOn the sensor
	 * @return      void
	 */
    /* turns on the sensor */
    virtual void VL53L1_On(void)
    {
       if(gpio0 >= 0)
       {
         digitalWrite(gpio0, HIGH);
       }
       delay(10);
    }

	/**
	 * @brief       PowerOff the sensor
	 * @return      void
	 */
    /* turns off the sensor */
    virtual void VL53L1_Off(void)
    {
       if(gpio0 >= 0)
       {
         digitalWrite(gpio0, LOW);
       }
       delay(10);
    }

	/**
	 * @brief       Initialize the sensor with default values
	 * @return      0 on Success
	 */
	 
	 VL53L1X_ERROR InitSensor(uint8_t address){
		VL53L1X_ERROR status = 0;
		uint8_t sensorState = 0;
		VL53L1_Off();
		VL53L1_On();
		status = VL53L1X_SetI2CAddress(address);
		
#ifdef DEBUG_MODE
		uint8_t byteData;
		uint16_t wordData;
		status = VL53L1_RdByte(Device, 0x010F, &byteData);
		Serial.println("VL53L1X Model_ID: " + String(byteData));
		status = VL53L1_RdByte(Device, 0x0110, &byteData);
		Serial.println("VL53L1X Module_Type: " + String(byteData));
		status = VL53L1_RdWord(Device, 0x010F, &wordData);
		Serial.println("VL53L1X: " + String(wordData));
#endif
		
		
		while (!sensorState && !status){
			status = VL53L1X_BootState(&sensorState);
			delay(2);
		}
		if(!status){
			status = VL53L1X_SensorInit();
		}
		return status;
	 }



/**
 *
 * @brief One time device initialization
 * @param void
 * @return     0 on success,  @a #CALIBRATION_WARNING if failed
 */
    virtual int Init()
    {
       return VL53L1X_SensorInit();
    }



    /* Read function of the ID device */
    virtual int ReadID(){
		uint16_t sensorId;
		VL53L1X_GetSensorId(&sensorId);
		if (sensorId == 0xEEAC)
			return 0;
		return -1;
	}



/**
 * @brief Get ranging result and only that
 * @param pRange_mm  Pointer to range distance
 * @return           0 on success
 */
	int GetDistance(uint32_t *piData)
    {
	int status;
	uint16_t distance;
	status = VL53L1X_GetDistance(&distance);
	*piData = (uint32_t) distance;
	return status;
    }


/* VL53L1X_api.h functions */


	/**
	 * @brief This function returns the SW driver version
	 */
	VL53L1X_ERROR VL53L1X_GetSWVersion(VL53L1X_Version_t *pVersion);

	/**
	 * @brief This function sets the sensor I2C address used in case multiple devices application, default address 0x52
	 */
	VL53L1X_ERROR VL53L1X_SetI2CAddress(uint8_t new_address);

	/**
	 * @brief This function loads the 135 bytes default values to initialize the sensor.
	 * @param dev Device address
	 * @return 0:success, != 0:failed
	 */
	VL53L1X_ERROR VL53L1X_SensorInit();

	/**
	 * @brief This function clears the interrupt, to be called after a ranging data reading
	 * to arm the interrupt for the next data ready event.
	 */
	VL53L1X_ERROR VL53L1X_ClearInterrupt();

	/**
	 * @brief This function programs the interrupt polarity\n
	 * 1=active high (default), 0=active low
	 */
	VL53L1X_ERROR VL53L1X_SetInterruptPolarity(uint8_t IntPol);

	/**
	 * @brief This function returns the current interrupt polarity\n
	 * 1=active high (default), 0=active low
	 */
	VL53L1X_ERROR VL53L1X_GetInterruptPolarity(uint8_t *pIntPol);

	/**
	 * @brief This function starts the ranging distance operation\n
	 * The ranging operation is continuous. The clear interrupt has to be done after each get data to allow the interrupt to raise when the next data is ready\n
	 * 1=active high (default), 0=active low, use SetInterruptPolarity() to change the interrupt polarity if required.
	 */
	VL53L1X_ERROR VL53L1X_StartRanging();

	/**
	 * @brief This function stops the ranging.
	 */
	VL53L1X_ERROR VL53L1X_StopRanging();

	/**
	 * @brief This function checks if the new ranging data is available by polling the dedicated register.
	 * @param : isDataReady==0 -> not ready; isDataReady==1 -> ready
	 */
	VL53L1X_ERROR VL53L1X_CheckForDataReady(uint8_t *isDataReady);

	/**
	 * @brief This function programs the timing budget in ms.
	 * Predefined values = 15, 20, 33, 50, 100(default), 200, 500.
	 */
	VL53L1X_ERROR VL53L1X_SetTimingBudgetInMs(uint16_t TimingBudgetInMs);

	/**
	 * @brief This function returns the current timing budget in ms.
	 */
	VL53L1X_ERROR VL53L1X_GetTimingBudgetInMs(uint16_t *pTimingBudgetInMs);

	/**
	 * @brief This function programs the distance mode (1=short, 2=long(default)).
	 * Short mode max distance is limited to 1.3 m but better ambient immunity.\n
	 * Long mode can range up to 4 m in the dark with 200 ms timing budget.
	 */
	VL53L1X_ERROR VL53L1X_SetDistanceMode(uint16_t DistanceMode);

	/**
	 * @brief This function returns the current distance mode (1=short, 2=long).
	 */
	VL53L1X_ERROR VL53L1X_GetDistanceMode(uint16_t *pDistanceMode);

	/**
	 * @brief This function programs the Intermeasurement period in ms\n
	 * Intermeasurement period must be >/= timing budget. This condition is not checked by the API,
	 * the customer has the duty to check the condition. Default = 100 ms
	 */
	VL53L1X_ERROR VL53L1X_SetInterMeasurementInMs(uint16_t InterMeasurementInMs);

	/**
	 * @brief This function returns the Intermeasurement period in ms.
	 */
	VL53L1X_ERROR VL53L1X_GetInterMeasurementInMs(uint16_t * pIM);

	/**
	 * @brief This function returns the boot state of the device (1:booted, 0:not booted)
	 */
	VL53L1X_ERROR VL53L1X_BootState(uint8_t *state);

	/**
	 * @brief This function returns the sensor id, sensor Id must be 0xEEAC
	 */
	VL53L1X_ERROR VL53L1X_GetSensorId(uint16_t *id);

	/**
	 * @brief This function returns the distance measured by the sensor in mm
	 */
	VL53L1X_ERROR VL53L1X_GetDistance(uint16_t *distance);

	/**
	 * @brief This function returns the returned signal per SPAD in kcps/SPAD.
	 * With kcps stands for Kilo Count Per Second
	 */
	VL53L1X_ERROR VL53L1X_GetSignalPerSpad(uint16_t *signalPerSp);

	/**
	 * @brief This function returns the ambient per SPAD in kcps/SPAD
	 */
	VL53L1X_ERROR VL53L1X_GetAmbientPerSpad(uint16_t *amb);

	/**
	 * @brief This function returns the returned signal in kcps.
	 */
	VL53L1X_ERROR VL53L1X_GetSignalRate(uint16_t *signalRate);

	/**
	 * @brief This function returns the current number of enabled SPADs
	 */
	VL53L1X_ERROR VL53L1X_GetSpadNb(uint16_t *spNb);

	/**
	 * @brief This function returns the ambient rate in kcps
	 */
	VL53L1X_ERROR VL53L1X_GetAmbientRate(uint16_t *ambRate);

	/**
	 * @brief This function returns the ranging status error \n
	 * (0:no error, 1:sigma failed, 2:signal failed, ..., 7:wrap-around)
	 */
	VL53L1X_ERROR VL53L1X_GetRangeStatus(uint8_t *rangeStatus);

	/**
	 * @brief This function programs the offset correction in mm
	 * @param OffsetValue:the offset correction value to program in mm
	 */
	VL53L1X_ERROR VL53L1X_SetOffset(int16_t OffsetValue);

	/**
	 * @brief This function returns the programmed offset correction value in mm
	 */
	VL53L1X_ERROR VL53L1X_GetOffset(int16_t *Offset);

	/**
	 * @brief This function programs the xtalk correction value in cps (Count Per Second).\n
	 * This is the number of photons reflected back from the cover glass in cps.
	 */
	VL53L1X_ERROR VL53L1X_SetXtalk(uint16_t XtalkValue);

	/**
	 * @brief This function returns the current programmed xtalk correction value in cps
	 */
	VL53L1X_ERROR VL53L1X_GetXtalk(uint16_t *Xtalk);

	/**
	 * @brief This function programs the threshold detection mode\n
	 * Example:\n
	 * VL53L1X_SetDistanceThreshold(dev,100,300,0,1): Below 100 \n
	 * VL53L1X_SetDistanceThreshold(dev,100,300,1,1): Above 300 \n
	 * VL53L1X_SetDistanceThreshold(dev,100,300,2,1): Out of window \n
	 * VL53L1X_SetDistanceThreshold(dev,100,300,3,1): In window \n
	 * @param   dev : device address
	 * @param  	ThreshLow(in mm) : the threshold under which one the device raises an interrupt if Window = 0
	 * @param 	ThreshHigh(in mm) :  the threshold above which one the device raises an interrupt if Window = 1
	 * @param   Window detection mode : 0=below, 1=above, 2=out, 3=in
	 * @param   IntOnNoTarget = 1 (No longer used - just use 1)
	 */
	VL53L1X_ERROR VL53L1X_SetDistanceThreshold(uint16_t ThreshLow,
					  uint16_t ThreshHigh, uint8_t Window,
					  uint8_t IntOnNoTarget);

	/**
	 * @brief This function returns the window detection mode (0=below; 1=above; 2=out; 3=in)
	 */
	VL53L1X_ERROR VL53L1X_GetDistanceThresholdWindow(uint16_t *window);

	/**
	 * @brief This function returns the low threshold in mm
	 */
	VL53L1X_ERROR VL53L1X_GetDistanceThresholdLow(uint16_t *low);

	/**
	 * @brief This function returns the high threshold in mm
	 */
	VL53L1X_ERROR VL53L1X_GetDistanceThresholdHigh(uint16_t *high);

	/**
	 * @brief This function programs the ROI (Region of Interest)\n
	 * The ROI position is centered, only the ROI size can be reprogrammed.\n
	 * The smallest acceptable ROI size = 4\n
	 * @param X:ROI Width; Y=ROI Height
	 */
	VL53L1X_ERROR VL53L1X_SetROI(uint8_t X, uint8_t Y, uint8_t opticalCenter);

	/**
	 *@brief This function returns width X and height Y
	 */
	VL53L1X_ERROR VL53L1X_GetROI_XY(uint16_t *ROI_X, uint16_t *ROI_Y);

	/**
	 * @brief This function programs a new signal threshold in kcps (default=1024 kcps\n
	 */
	VL53L1X_ERROR VL53L1X_SetSignalThreshold(uint16_t signal);

	/**
	 * @brief This function returns the current signal threshold in kcps
	 */
	VL53L1X_ERROR VL53L1X_GetSignalThreshold(uint16_t *signal);

	/**
	 * @brief This function programs a new sigma threshold in mm (default=15 mm)
	 */
	VL53L1X_ERROR VL53L1X_SetSigmaThreshold(uint16_t sigma);

	/**
	 * @brief This function returns the current sigma threshold in mm
	 */
	VL53L1X_ERROR VL53L1X_GetSigmaThreshold(uint16_t *signal);

	/**
	 * @brief This function performs the temperature calibration.
	 * It is recommended to call this function any time the temperature might have changed by more than 8 deg C
	 * without sensor ranging activity for an extended period.
	 */
	VL53L1X_ERROR VL53L1X_StartTemperatureUpdate();


	/* VL53L1X_calibration.h functions */
	
		/**
	 * @brief This function performs the offset calibration.\n
	 * The function returns the offset value found and programs the offset compensation into the device.
	 * @param TargetDistInMm target distance in mm, ST recommended 100 mm
	 * Target reflectance = grey17%
	 * @return 0:success, !=0: failed
	 * @return offset pointer contains the offset found in mm
	 */
	int8_t VL53L1X_CalibrateOffset(uint16_t TargetDistInMm, int16_t *offset);

	/**
	 * @brief This function performs the xtalk calibration.\n
	 * The function returns the xtalk value found and programs the xtalk compensation to the device
	 * @param TargetDistInMm target distance in mm\n
	 * The target distance : the distance where the sensor start to "under range"\n
	 * due to the influence of the photons reflected back from the cover glass becoming strong\n
	 * It's also called inflection point\n
	 * Target reflectance = grey 17%
	 * @return 0: success, !=0: failed
	 * @return xtalk pointer contains the xtalk value found in cps (number of photons in count per second)
	 */
	int8_t VL53L1X_CalibrateXtalk(uint16_t TargetDistInMm, uint16_t *xtalk);





 protected:
    

    /* Write and read functions from I2C */

    VL53L1X_ERROR VL53L1_WrByte(VL53L1_DEV dev, uint16_t index, uint8_t data);
    VL53L1X_ERROR VL53L1_WrWord(VL53L1_DEV dev, uint16_t index, uint16_t data);
    VL53L1X_ERROR VL53L1_WrDWord(VL53L1_DEV dev, uint16_t index, uint32_t data);
    VL53L1X_ERROR VL53L1_RdByte(VL53L1_DEV dev, uint16_t index, uint8_t *data);
    VL53L1X_ERROR VL53L1_RdWord(VL53L1_DEV dev, uint16_t index, uint16_t *data);
    VL53L1X_ERROR VL53L1_RdDWord(VL53L1_DEV dev, uint16_t index, uint32_t *data);
    VL53L1X_ERROR VL53L1_UpdateByte(VL53L1_DEV dev, uint16_t index, uint8_t AndData, uint8_t OrData);

    VL53L1X_ERROR VL53L1_WriteMulti(VL53L1_DEV Dev, uint16_t index, uint8_t *pdata, uint32_t count);
    VL53L1X_ERROR VL53L1_ReadMulti(VL53L1_DEV Dev, uint16_t index, uint8_t *pdata, uint32_t count);

	VL53L1X_ERROR VL53L1_I2CWrite(uint8_t dev, uint16_t index, uint8_t *data, uint16_t number_of_bytes);
	VL53L1X_ERROR VL53L1_I2CRead(uint8_t dev, uint16_t index, uint8_t *data, uint16_t number_of_bytes);
	VL53L1X_ERROR VL53L1_GetTickCount(uint32_t *ptick_count_ms);
	VL53L1X_ERROR VL53L1_WaitUs(VL53L1_Dev_t *pdev, int32_t wait_us);
	VL53L1X_ERROR VL53L1_WaitMs(VL53L1_Dev_t *pdev, int32_t wait_ms);
	
	VL53L1X_ERROR VL53L1_WaitValueMaskEx(VL53L1_Dev_t *pdev, uint32_t timeout_ms, uint16_t index, uint8_t value, uint8_t mask, uint32_t poll_delay_ms);
	
	

 protected:

    /* IO Device */
    TwoWire *dev_i2c;
    /* Digital out pin */
	int gpio0;
	int gpio1Int;
    /* Device data */
	VL53L1_Dev_t MyDevice;
	VL53L1_DEV Device;
};


#endif /* _VL53L1X_CLASS_H_ */
