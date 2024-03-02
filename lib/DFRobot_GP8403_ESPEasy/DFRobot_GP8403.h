/*!
 * @file DFRobot_GP8403.h
 * @brief This is a method description file for the DAC module
 * @copyright	Copyright (c) 2021 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author [TangJie](jie.tang@dfrobot.com)
 * @version V1.0
 * @date 2022-03-07
 * @url https://github.com/DFRobot/DFRobot_Microphone
 * 
 * 2024-01-21 tonhuisman: Make Sine wave, Triangle wave and Square wave functions optional, for size and timing issues
 *                        Make store() feature optional, as that is absolutely NOT I2C friendly or compatible
 */
#ifndef _DFROBOT_GP8403_H_
#define _DFROBOT_GP8403_H

#include "Arduino.h"
#include "Wire.h"

// #define GP8403_SINE_WAVE_ENABLED        // Optionally enable Sine wave support
// #define GP8403_TRIANGLE_WAVE_ENABLED    // Optionally enable Triangle wave support
// #define GP8403_SQUARE_WAVE_ENABLED      // Optionally enable Square wave support
// #define GP8403_STORE_ENABLED            // Optionally enable storing the current value in the device (!!! *** _NOT_ I2C Compatible *** !!!)

// #define ENABLE_DBG //!< Open the macro and you can see the detailed procedure of the program
#ifdef ENABLE_DBG
#define DBG(...) {Serial.print("[");Serial.print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print(" ] "); Serial.println(__VA_ARGS__);}
#else
#define DBG(...)
#endif

#define GP8302_CONFIG_CURRENT_REG 0x02
#define OUTPUT_RANGE              0x01

class DFRobot_GP8403
{
public:
	/**
	 * @enum eOutPutRange_t
	 * @brief Analog voltage output range select
	 */
	 enum class eOutPutRange_t : uint8_t{
    eOutputRange5V  = 0x00,
    eOutputRange10V = 0x11,
  } ;
	/**
	 * @brief DFRobot_GP8403 constructor
	 * @param pWire I2C object
	 * @param addr I2C address
	 */
		DFRobot_GP8403(TwoWire *pWire = &Wire,uint8_t addr = 0x58);
	/**
   * @fn begin
   * @brief Initialize the module
   */
  uint8_t begin(void);

	/**
   * @fn setDACOutRange
   * @brief Set DAC output range
   * @param range DAC output range
   * @return NONE
   */
  void setDACOutRange(eOutPutRange_t range);
    
  /**
   * @fn setDACOutVoltage
   * @brief Set output DAC voltage of different channels
   * @param data The voltage value to be output
   * @param channel Output channel. 0: channel 0; 1: channel 1; 2: all the channels
   * @return NONE
   */
  void setDACOutVoltage(uint16_t data,uint8_t channel);
  
  #ifdef GP8403_STORE_ENABLED
  /**
   * @brief Save the set voltage in the chip
   */
	void store(void);
  #endif // ifdef GP8403_STORE_ENABLED

  #ifdef GP8403_SINE_WAVE_ENABLED
  /**
   * @brief Call the function to output sine wave
   * @param amp Set sine wave amplitude Vp
   * @param freq Set sine wave frequency f
   * @param offset Set sine wave DC offset Voffset
   * @param channel Output channel. 0: channel 0; 1: channel 1; 2: all the channels
   */
	void outputSin(uint16_t amp, uint16_t freq, uint16_t offset,uint8_t channel);
  #endif // ifdef GP8403_SINE_WAVE_ENABLED
  
  #ifdef GP8403_TRIANGLE_WAVE_ENABLED
  /**
   * @brief Call the function to output triangle wave
   * @param amp Set triangle wave amplitude Vp
   * @param freq Set triangle wave frequency f
   * @param offset Set triangle wave DC offset Voffset
   * @param dutyCycle Set triangle (sawtooth) wave duty cycle
   * @param channel Output channel. 0: channel 0; 1: channel 1; 2: all the channels
   */
	void outputTriangle(uint16_t amp, uint16_t freq, uint16_t offset, int8_t dutyCycle, uint8_t channel);
  #endif // ifdef GP8403_TRIANGLE_WAVE_ENABLED
  
  #ifdef GP8403_SQUARE_WAVE_ENABLED
  /**
   * @brief Call the function to output square wave
   * @param amp Set square wave amplitude Vp
   * @param freq Set square wave frequency f
   * @param offset Set square wave DC offset Voffset
   * @param dutyCycle Set square wave duty cycle
   * @param channel Output channel. 0: channel 0; 1: channel 1; 2: all the channels
   */
	void outputSquare(uint16_t amp, uint16_t freq, uint16_t offset, int8_t dutyCycle, uint8_t channel);
  #endif // ifdef GP8403_SQUARE_WAVE_ENABLED

#ifdef GP8403_STORE_ENABLED
protected:
  void startSignal(void);
  void stopSignal(void);
  uint8_t recvAck(uint8_t ack);
  uint8_t sendByte(uint8_t data, uint8_t ack = 0, uint8_t bits = 8, bool flag = true);
#endif // ifdef GP8403_STORE_ENABLED

private:
 /**
  * @fn writeReg
  * @brief   Write register value through IIC bus
  * @param reg  Register address 8bits
  * @param pBuf Storage cache to write data in
  * @param size The length of data to be written
  */
  void writeReg(uint8_t reg, void *pBuf, size_t size);
  TwoWire *_pWire;
	uint8_t _addr;
	uint16_t voltage = 0;
  #ifdef GP8403_STORE_ENABLED
  int _scl= SCL;
  int _sda = SDA;
  #endif // ifdef GP8403_STORE_ENABLED
  void sendData(uint16_t data, uint8_t channel);
};



#endif
