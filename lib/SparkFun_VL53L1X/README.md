SparkFun Qwiic 4m Distance Sensor with VL53L1X
========================================
[![Build Status](https://github.com/sparkfun/SparkFun_VL53L1X_Arduino_Library/workflows/LibraryBuild/badge.svg)](https://github.com/sparkfun/SparkFun_VL53L1X_Arduino_Library/actions)


<table class="table table-hover table-striped table-bordered">
  <tr align="center">
   <td><a href="https://www.sparkfun.com/products/14722"><img src="https://cdn.sparkfun.com//assets/parts/1/2/9/4/8/14722-SparkFun_Distance_Sensor_Breakout-_4_Meter__VL53L1X__Qwiic_-01.jpg" alt="SparkFun Distance Sensor Breakout - 4 Meter, VL53L1X (Qwiic)"></a></td>
   <td><a href="https://www.sparkfun.com/products/18993"><img src="https://cdn.sparkfun.com//assets/parts/1/8/5/7/2/18993-SparkFun_Distance_Sensor_-_1.3_Meter__VL53L4CD__Qwiic_-01.jpg" alt="SparkFun Distance Sensor - 1.3 Meter, VL53L4CD (Qwiic)"></a></td>
  </tr>
  <tr align="center">
   <td>SparkFun Distance Sensor Breakout - 4 Meter, VL53L1X (Qwiic) <i>[<a href="https://www.sparkfun.com/products/14722">SEN-14722</a>]</i></td>
   <td>SparkFun Distance Sensor - 1.3 Meter, VL53L4CD (Qwiic)<i>[<a href="https://www.sparkfun.com/products/18993">SEN-18993</a>]</i></td>
  </tr>
</table>

The VL53L1X is a Time Of Flight (ToF) sensor that use a VCSEL (vertical cavity surface emitting laser) to emit a class 1 IR laser and time the reflection to the target. What does all this mean? Using the VL53L1X, you can measure the distance to an object up to 4 meters away with millimeter resolution! That’s pretty incredible.

We’re far from done: The VL53L1X is a highly complex sensor with a multitude of options and configurations.  We’ve written example sketches that allow you to read the distance, signal rate, and range status. Because STMicroelectronics has chosen not to release a complete datasheet we are forced to reverse engineer the interface from their example code and I2C data stream captures. If you’re into puzzles we could use your help to make the library better!

We’ve found the precision of VL53L1X sensor to be 1mm but the accuracy is around +/-5mm. 

SparkFun labored with love to create this code. Feel like supporting open source hardware? 
Buy a [breakout board](https://www.sparkfun.com/products/14722) from SparkFun!

*Note: The VL53L4CD is the cousin of VL53L1X. Overall, the sensor functions the same except for a few differences in the specifications. We've also found the precision of VL53L4CD sensor to be 1mm but the accuracy is around +/-7mm  (white target: 88%, indoor, no infrared).*

Repository Contents
-------------------

* **/Documents** - Datasheet and User Manual
* **/Hardware** - Eagle design files (.brd, .sch)
* **/Production** - .brd files

Documentation
--------------
* **[Installing an Arduino Library Guide](https://learn.sparkfun.com/tutorials/installing-an-arduino-library)** - Basic information on how to install an Arduino library.
* **[Hookup Guide](https://learn.sparkfun.com/tutorials/qwiic-distance-sensor-vl53l1x-vl53l4cd-hookup-guide)**- Basic tutorial for the VL53L1X and VL53L4CD

Products that use this Library
--------------

* **[SEN-18993](https://www.sparkfun.com/products/18993)** - SparkFun red version for VL53L4CD
* **[SEN-14722](https://www.sparkfun.com/products/14722)** - SparkFun red version for VL53L1X
* **[SPX-14667](https://www.sparkfun.com/products/14667)** - SparkX Version for VL53L1X

License Information
-------------------

SparkFun's source files are _**open source**_! 

Please review the LICENSE.md file for license information. 

If you have any questions or concerns on licensing, please contact techsupport@sparkfun.com.

Please use, reuse, and modify these files as you see fit. Please maintain attribution to SparkFun Electronics and release any derivative under the same license.

Distributed as-is; no warranty is given.

The source files included in the subfolder **st_src** are licensed differently. They are licensed under the BSD-3 license, check the License.md in
that subfolder for specifics.
- Your friends at SparkFun.
