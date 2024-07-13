# DFRobot_GP8403

Update: port for Raspberry Pi Pico in Python in RP2 folder.

This I2C to 0-5V/0-10V DAC module can be used to output voltage of 0-5V or 0-10V. It has the following features:
1. Output voltage of 0-5V or 0-10V.
2. It can control the output voltage with an I2C interface, the I2C address is default to be 0x58. 
3. The output voltage config will be lost after the module is powered down. Save the config if you want to use it for the next power-up.


## Product Link（[www.dfrobot.com](www.dfrobot.com)）
    SKU: DFR0971 

## Table of Contents
  - [Summary](#summary)
  - [Installation](#installation)
  - [Methods](#methods)
  - [Compatibility](#compatibility)
  - [History](#history)
  - [Credits](#credits)

## Summary
The Arduino library is provided for the I2C 0-5V/0-10V DAC module to set and save the output voltage config of the module. And the library has the following functions:
1. Set the voltage of 0-5V or 0-10V directly;
2. Output the corresponding voltage by setting the DAC range of 0-0xFFF;
3. Save the voltage config(Will not be lost when powered down).

## Installation

There two methods: 
1. To use this library, first download the library file, paste it into the \Arduino\libraries directory, then open the examples folder and run the demo in the folder.
2. Search the DFRobot_GP8302 library from the Arduino Software Library Manager and download it.

## Methods

```C++
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
  void setDACOutRange(eOutPutRange range);
    
  /**
   * @fn setDACOutVoltage
   * @brief Set output DAC voltage of different channels
   * @param data The voltage value to be output
   * @param channel Output channel. 0: channel 0; 1: channel 1; 2: all the channels
   * @return NONE
   */
  void setDACOutVoltage(uint16_t data,uint8_t channel);
  /**
   * @brief Save the set voltage inside the chip
   */
	void store(void);
  /**
   * @brief Call the function to output sine wave
   * @param amp Set sine wave amplitude Vp
   * @param freq Set sine wave frequency f
   * @param offset Set sine wave DC offset Voffset
   * @param channel Output channel. 0: channel 0; 1: channel 1; 2: all the channels
   */
	void outputSin(uint16_t amp, uint16_t freq, uint16_t offset,uint8_t channel);
  /**
   * @brief Call the function to output triangle wave
   * @param amp Set triangle wave amplitude Vp
   * @param freq Set triangle wave frequency f
   * @param offset Set triangle wave DC offset Voffset
   * @param dutyCycle Set triangle (sawtooth) wave duty cycle
   * @param channel Output channel. 0: channel 0; 1: channel 1; 2: all the channels
   */
	void outputTriangle(uint16_t amp, uint16_t freq, uint16_t offset, int8_t dutyCycle, uint8_t channel);
  /**
   * @brief Call the function to output square wave
   * @param amp Set square wave amplitude Vp
   * @param freq Set square wave frequency f
   * @param offset Set square wave DC offset Voffset
   * @param dutyCycle Set square wave duty cycle
   * @param channel Output channel. 0: channel 0; 1: channel 1; 2: all the channels
   */
	void outputSquare(uint16_t amp, uint16_t freq, uint16_t offset, int8_t dutyCycle, uint8_t channel);
```
## Compatibility

MCU                |  Work Well    | Work Wrong   | Untested    | Remarks
------------------ | :----------: | :----------: | :---------: | -----
Arduino Uno        |       √       |              |             | 
Mega2560           |      √       |              |             | 
Leonardo           |      √       |              |             | 
ESP32              |      √       |              |             | 
ESP8266            |      √       |              |             | 
micro:bit          |      √       |              |             | 
FireBeetle M0      |      √       |              |             | 

## History

- 2022/03/10 - Version 1.0.0 released.

## Credits

Written by tangjie(jie.tang@dfrobot.com), 2022. (Welcome to our [website](https://www.dfrobot.com/))

