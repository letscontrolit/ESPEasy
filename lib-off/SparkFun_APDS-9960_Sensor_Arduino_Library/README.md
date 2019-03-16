SparkFun APDS9960 RGB and Gesture Sensor Arduino Library
=========================================================

![Avago APDS-9960 Breakout Board - SEN-12787 ](https://cdn.sparkfun.com/r/92-92/assets/parts/9/6/0/3/12787-01.jpg)

[*Avago APDS-9960 Breakout Board (SEN-12787)*](https://www.sparkfun.com/products/12787)

Getting Started
---------------

* Download the Git repository as a ZIP ("Download ZIP" button)
* Unzip
* Copy the entire library directory (APDS-9960_RGB_and_Gesture_Sensor_Arduino_Library
) to \<Arduino installation directory\>/libraries
* Open the Arduino program
* Select File -> Examples -> SparkFun_APDS9960 -> GestureTest
* Plug in your Arduino and APDS-9960 with the following connections

*-OR-*

* Use the library manager

| Arduino Pin | APDS-9960 Board | Function |
|---|---|---| 
| 3.3V | VCC | Power |
| GND | GND | Ground |
| A4 | SDA | I2C Data |
| A5 | SCL | I2C Clock |
| 2 | INT | Interrupt |

* Go to Tools -> Board and select your Arduino board
* Go to Tools -> Serial Port and select the COM port of your Arduino board
* Click "Upload"
* Go to Tools -> Serial Monitor
* Ensure the baud rate is set at 9600 baud
* Swipe your hand over the sensor in various directions!

Repository Contents
-------------------

* **/examples** - Example sketches for the library (.ino). Run these from the Arduino IDE. 
* **/src** - Source files for the library (.cpp, .h).
* **library.properties** - General library properties for the Arduino package manager. 

Documentation
--------------

* **[Installing an Arduino Library Guide](https://learn.sparkfun.com/tutorials/installing-an-arduino-library)** - Basic information on how to install an Arduino library.
* **[Product Repository](https://github.com/sparkfun/APDS-9960_RGB_and_Gesture_Sensor)** - Main repository (including hardware files) for the SparkFun_APDS9960 RGB and Gesture Sensor.
* **[Hookup Guide](https://learn.sparkfun.com/tutorials/apds-9960-rgb-and-gesture-sensor-hookup-guide)** - Basic hookup guide for the sensor.

Products that use this Library 
---------------------------------

* [SEN-12787](https://www.sparkfun.com/products/12787)- Avago APDS-9960 

Version History
---------------
* [V_1.4.1](https://github.com/sparkfun/SparkFun_APDS-9960_Sensor_Arduino_Library/tree/V_1.4.1) - Removing blank files, updating library.properties file. 
* [V_1.4.0](https://github.com/sparkfun/APDS-9960_RGB_and_Gesture_Sensor_Arduino_Library/tree/V_1.4.0) - Updated to new library structure
* V_1.3.0 - Implemented disableProximitySensor(). Thanks to jmg5150 for catching that!
* V_1.2.0 - Added pinMode line to GestureTest demo to fix interrupt bug with some Arduinos
* V_1.1.0 - Updated GestureTest demo to not freeze with fast swipes
* V_1.0.0: Initial release
* Ambient and RGB light sensing implemented
* Ambient light interrupts working
* Proximity sensing implemented
* Proximity interrupts working
* Gesture (UP, DOWN, LEFT, RIGHT, NEAR, FAR) sensing implemented

License Information
-------------------

This product is _**open source**_! 

The **code** is beerware; if you see me (or any other SparkFun employee) at the local, and you've found our code helpful, please buy us a round!

Please use, reuse, and modify these files as you see fit. Please maintain attribution to SparkFun Electronics and release anything derivative under the same license.

Distributed as-is; no warranty is given.

- Your friends at SparkFun.
