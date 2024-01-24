# DFRobot_GP8403

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
1. Set the voltage of 0-5V or 0-10V directly；
2. Output the corresponding voltage by setting the DAC range of 0-0xFFF；
3. Save the voltage config(Will not be lost when powered down).

## Installation
1. To use this library, first download the library file<br>
```python
sudo git clone https://github.com/DFRobot/DFRobot_GP8302
```
2. Open and run the routine. To execute a routine demo_x.py, enter python demo_x.py in the command line. For example, to execute the demo_set_current.py routine, you need to enter :<br>

```python
python demo_set_current.py 
or
python2 demo_set_current.py 
or 
python3 demo_set_current.py
```

## Methods

```python
  '''!
    @param Initialize the sensor
  '''
  def begin(self):
    
  '''!
    @brief Set DAC output range
    @param mode Select DAC output range
  '''
  def set_DAC_outrange(self,mode):
    
  '''!
    @brief Select DAC output channel & range
    @param data Set the output data
    @param channel Output channel. 0: channel 0; 1: channel 1; 2: all the channels
  '''
  def set_DAC_out_voltage(self,data,channel)
    
  '''!
    @brief   Save the present current config, after the config is saved successfully, it will be enabled when the module is powered down and restarts.
  '''
  def store(self)
    
  '''!
    @brief Set the sensor to output sine wave
    @param amp Set sine wave amplitude Vp
    @param freq Set sine wave frequency f
    @param offset Set sine wave DC offset Voffset
    @param channel Output channel. 0: channel 0; 1: channel 1; 2: all the channels
  '''
  def output_sin(self,amp,freq,offset,channel)
    

  '''!
    @brief Call the function to output triangle wave
    @param amp Set triangle wave amplitude Vp
    @param freq Set triangle wave frequency f
    @param offset Set triangle wave DC offset Voffset
    @param dutyCycle Set triangle (sawtooth) wave duty cycle
    @param channel Output channel. 0: channel 0; 1: channel 1; 2: all the channels
  '''
  def output_triangle(self,amp,freq,offset,dutyCycle,channel):
    
  '''!
    @brief Call the function to output square wave
    @param amp Set square wave amplitude Vp
    @param freq Set square wave frequency f
    @param offset Set square wave DC offset Voffset
    @param dutyCycle Set square wave duty cycle
    @param channel Output channel. 0: channel 0; 1: channel 1; 2: all the channels
  '''
  def output_square(self,amp,freq,offset,dutyCycle,channel)
    
```

## Compatibility

| MCU         | Work Well | Work Wrong | Untested | Remarks |
| ------------ | :--: | :----: | :----: | :--: |
| RaspberryPi2 |      |        |   √    |      |
| RaspberryPi3 |      |        |   √    |      |
| RaspberryPi4 |  √   |        |        |      |

* Python Version

| Python  | Work Well | Work Wrong | Untested | Remarks |
| ------- | :--: | :----: | :----: | ---- |
| Python2 |  √   |        |        |      |
| Python3 |  √   |        |        |      |


## History

- 2022/03/10 - Version 1.0.0 released.

## Credits

Written by tangjie(jie.tang@dfrobot.com), 2022. (Welcome to our [website](https://www.dfrobot.com/))





