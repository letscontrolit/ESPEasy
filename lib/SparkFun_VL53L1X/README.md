SparkFun Qwiic 4m Distance Sensor with VL53L1X
========================================
[![Build Status](https://github.com/sparkfun/SparkFun_VL53L1X_Arduino_Library/workflows/LibraryBuild/badge.svg)](https://github.com/sparkfun/SparkFun_VL53L1X_Arduino_Library/actions)


![SparkFun Distance Sensor Breakout - 4 Meter, VL53L1X (Qwiic)](https://cdn.sparkfun.com//assets/parts/1/2/9/4/8/14722-SparkFun_Distance_Sensor_Breakout-_4_Meter__VL53L1X__Qwiic_-01.jpg)

[*SparkFun Distance Sensor Breakout - 4 Meter, VL53L1X (Qwiic)(SEN-14722)*](https://www.sparkfun.com/products/14722)

The VL53L1X is the latest Time Of Flight (ToF) sensor to be released. It uses a VCSEL (vertical cavity surface emitting laser) to emit a class 1 IR laser and time the reflection to the target. What does all this mean? You can measure the distance to an object up to 4 meters away with millimeter resolution! That’s pretty incredible.

We’re far from done: The VL53L1X is a highly complex sensor with a multitude of options and configurations. We’ve written example sketches that allow you to read the distance, signal rate, and range status. Because ST has chosen not to release a complete datasheet we are forced to reverse engineer the interface from their example code and I2C data stream captures. If you’re into puzzles we could use your help to make the library better!

We’ve found the precision of the sensor to be 1mm but the accuracy is around +/-5mm.

SparkFun labored with love to create this code. Feel like supporting open source hardware? 
Buy a [breakout board](https://www.sparkfun.com/products/14722) from SparkFun!

Repository Contents
-------------------

* **/Documents** - Datasheet and User Manual
* **/Hardware** - Eagle design files (.brd, .sch)
* **/Production** - .brd files

Documentation
--------------
* **[Hookup Guide](https://learn.sparkfun.com/tutorials/qwiic-distance-sensor-vl53l1x-hookup-guide)**

Product Versions
--------------
* **[SEN-14722](https://www.sparkfun.com/products/14722)** - SparkFun red version
* **[SPX-14667](https://www.sparkfun.com/products/14667)** - SparkX Version 

License Information
-------------------

This product is _**open source**_! 

Please review the LICENSE.md file for license information. 

If you have any questions or concerns on licensing, please contact techsupport@sparkfun.com.

Please use, reuse, and modify these files as you see fit. Please maintain attribution to SparkFun Electronics and release any derivative under the same license.

Distributed as-is; no warranty is given.

- Your friends at SparkFun.
