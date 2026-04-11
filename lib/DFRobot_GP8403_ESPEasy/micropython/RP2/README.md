# DFRobot_GP8403

Port of the Library for Raspberry Pi Pico. 

This I2C to 0-5V/0-10V DAC module can be used to output voltage of 0-5V or 0-10V. It has the following features:
1. Output voltage of 0-5V or 0-10V.
2. It can control the output voltage with an I2C interface, the I2C address is default to be 0x58. 
3. The output voltage config will be lost after the module is powered down. Save the config if you want to use it for the next power-up.


## Product Link（[www.dfrobot.com](www.dfrobot.com)）
    SKU: DFR0971 

## Table of Contents
  - [Summary](#summary)
  - [Methods](#methods)
  - [Examples](#examples)
  - [Compatibility](#compatibility)
  - [History](#history)
  - [Credits](#credits)

## Summary
The Arduino library is provided for the I2C 0-5V/0-10V DAC module to set and save the output voltage config of the module. And the library has the following functions:
1. Set the voltage of 0-5V or 0-10V directly；
2. Output the corresponding voltage by setting the DAC range of 0-0xFFF；
3. Save the voltage config(Will not be lost when powered down).

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
    
```

## Examples

```python
  from DfrobotGP8403 import *
  import utime

  def store():
      # Store data in the chip
      DAC.store()

  def wave():
      # Triangle with 180 phase output 1 and 2
      vmax = DAC.get_dac_out_range()
      for i in range(10):
          for x in range(vmax+1):
              DAC.set_dac_out_voltage(x*1000,0)
              DAC.set_dac_out_voltage((vmax-x)*1000,1)
              utime.sleep(0.25)

          for x in reversed(range(vmax+1)):
              DAC.set_dac_out_voltage(x*1000,0)
              DAC.set_dac_out_voltage((vmax-x)*1000,1)
              utime.sleep(0.25)

  if __name__ == "__main__":   
      # Init DAC with desired address, pins, and hard/soft mode
      DAC = DfrobotGP8403 (0x5F, 5, 4, 400000, True)

      while DAC.begin() != 0:
          print("Init error")
          utime.sleep(1)
      print("Init succeed")

      # Set output range
      DAC.set_dac_out_range(OUTPUT_RANGE_10V)

      # Output value from DAC channel 0
      # Value in mV = 0-5000 or 0-10000 depending on range
      DAC.set_dac_out_voltage(1000,0)
      DAC.set_dac_out_voltage(2000,1)
```

## Compatibility

| MCU         | Work Well | Work Wrong | Untested | Remarks |
| ------------ | :--: | :----: | :----: | :--: |
| RaspberryPi Pico |   √   |        |       |      |


* Python Version

| Python  | Work Well | Work Wrong | Untested | Remarks |
| ------- | :--: | :----: | :----: | ---- |
| Python3 |     |        |    √    |      |
| MicroPython 1.19 |  √   |        |        |      |


## History

- 2023-04-03 - Version 1.0.0.
- 2023-04-04 - Version 1.0.1.

## Credits

Written by Rémi, 2023.





