# Adafruit HMC5883L Driver (3-Axis Magnetometer) [![Build Status](https://github.com/adafruit/Adafruit_HMC5883_Unified/workflows/Arduino%20Library%20CI/badge.svg)](https://github.com/adafruit/Adafruit_HMC5883_Unified/actions)[![Documentation](https://github.com/adafruit/ci-arduino/blob/master/assets/doxygen_badge.svg)](http://adafruit.github.io/Adafruit_HMC5883_Unified/html/index.html)

This driver is for the Adafruit HMC5883L Breakout (http://www.adafruit.com/products/1746), and is based on Adafruit's Unified Sensor Library (Adafruit_Sensor).

## About the HMC5883 ##

The HMC5883L is a digital (I2C) compass (magnetometer).  The magnetometer measure magnetic force, which is useful to detect magnetic north.

More information on the HMC5883L can be found in the datasheet: http://www.adafruit.com/datasheets/HMC5883L_3-Axis_Digital_Compass_IC.pdf

## What is the Adafruit Unified Sensor Library? ##

The Adafruit Unified Sensor Library (**Adafruit_Sensor**) provides a common interface and data type for any supported sensor.  It defines some basic information about the sensor (sensor limits, etc.), and returns standard SI units of a specific type and scale for each supported sensor type.

It provides a simple abstraction layer between your application and the actual sensor HW, allowing you to drop in any comparable sensor with only one or two lines of code to change in your project (essentially the constructor since the functions to read sensor data and get information about the sensor are defined in the base Adafruit_Sensor class).

This is imporant useful for two reasons:

1.) You can use the data right away because it's already converted to SI units that you understand and can compare, rather than meaningless values like 0..1023.

2.) Because SI units are standardised in the sensor library, you can also do quick sanity checks working with new sensors, or drop in any comparable sensor if you need better sensitivity or if a lower cost unit becomes available, etc. 

Light sensors will always report units in lux, gyroscopes will always report units in rad/s, etc. ... freeing you up to focus on the data, rather than digging through the datasheet to understand what the sensor's raw numbers really mean.

## About this Driver ##

Adafruit invests time and resources providing this open source code.  Please support Adafruit and open-source hardware by purchasing products from Adafruit!

Written by Kevin (KTOWN) Townsend for Adafruit Industries.
