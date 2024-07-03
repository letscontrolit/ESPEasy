BitBank Capacitive Touch Sensor Library<br>
---------------------------------------
Copyright (c) 2023 BitBank Software, Inc.<br>
Written by Larry Bank<br>
email: bitbank@pobox.com<br>
<br>
There are a growing list of development boards which include LCDs with capacitive touch plates on them. These are overwhelmingly controlled by different versions of the ESP32 MCU. The boards normally only utilize GOODiX and FocalTech capacitive touch controllers and this library supports the CST820, GT911 and FT6x36 in a generic way. Each has different capabilities and usually come pre-programmed for the specific pixel width and height of the target application. A feature supported by this library that may not be present in the device you're using is the touch area and pressure values. Some of their controllers also have built-in gesture detection. The common features of the controllers is that they will generate an interrupt signal when a touch event is occurring. This library allows you to request the latest touch information and it returns the number of active touch points (0-5) along with the coordinates (and pressure/area of each if available). The sensor type and address is auto-detected when calling the init() method. The only info that must be correctly supplied to the library are the GPIO pins used for the SDA/SCL/INT/RESET signals. Once initialized, repeatedly call getSamples() to test for and read any touch samples available.<br>

There are only 3 methods exposed by the class:<br>
init() - detects if a supported CT controller is available and initializes it<br>
getSamples() - returns touch points if available<br>
sensorType() - returns an enumerated value of the sensor detected<>
Here is the TOUCHINFO structure filled by the getSamples() method:<br>
```
typedef struct _fttouchinfo
{
  int count;
  uint16_t x[5], y[5];
  uint8_t pressure[5], area[5];
} TOUCHINFO;
```

If you find this code useful, please consider becoming a sponsor or sending a donation.

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=SR4F44J2UR8S4)


