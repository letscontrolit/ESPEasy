# AM2320
Arduino library for Aosong AM2320 temperature and humidity sensor.

# Downloads

[Latest Version](https://github.com/hibikiledo/AM2320/releases)

# API Reference

## Constructor
A constructor should be called first. This is where you can obtain a variable of the sensor library.

### Signature
```cpp
AM2320();
```

### Parameters
None

### Usage Example
```cpp
#include "AM2320.h"

// create a variable of sensor library
AM2320 sensor;
```

---



## Initialize the sensor
This should be called once in `void setup()` of an Arduino sketch.

### Signature
```cpp
void begin(void);
```
### Parameters
None

### Return Value
None

### Usage Example
```cpp
#include "AM2320.h"
AM2320 sensor;

void setup() {
  // initialize sensor library
  sensor.begin();
}

void loop() {

}
```
---

## Initialize the sensor with specific SDA and SCL pin numbers
Same as [Initialize the sensor](#initialize-the-sensor) with support for ESP8266 Arduino core.
This is useful on ESP-01 where SDA and SCL pins aren't default ones.

### Signature
```cpp
void begin(int sda, int scl);
```

### Parameters
- `int sda` - pin number of sda line of I<sup>2</sup>C bus
- `int scl` - pin number of scl line of I<sup>2</sup>C bus

### Return Value
None

#### Example
```cpp
#include "AM2320.h"
AM2320 sensor;

void setup() {
  // initialize sensor library with SDA and SCL pins
  // https://github.com/esp8266/Arduino/blob/master/doc/libraries.md#i2c-wire-library
  sensor.begin(0, 2);
}

void loop() {

}
```

---

## Measure temperature and humidity (%RH)
Tell sensor to perform both temperature and humidity acquisitions.

### Signature
```cpp
bool measure();
```

### Parameters
None

### Return Value
- `true` - the operation is successful
- `false` - an error occurs. use `getErrorCode()` to get an error code. [More info](#error-codes)

### Usage Example
You are encouraged to check a returned value of `measure()`.
Chances are that the call to `measure()` hits the sensor when it is sleeping.
This is normal behavior of AM2320 to prevent measurement errors from self-heating.

By checking the returned value, you can decide what to do.

See [**Example**](#example)

---

## Get the temperature
Retrieve the latest temperature measurement from the call to `measure()`

### Signature
```cpp
float getTemperature();
```

### Parameters
None

### Return Value
Floating point number representing the temperature in degree celcius.

### Usage Example
See [**Example**](#example)

---

## Get the humidity
Retrieve the latest humudity measurement from the call to `measure()`

### Signature
```cpp
float getHumidity();
```

### Parameters
None

### Return Value
Floating point number representing the humudity in % RH (Relative Humidity).

### Usage Example
See [**Example**](#example)

---

## Get error code
Retrieve the error code of latest call to `measure()`. Error code provides more detail on the errors.

### Signature
```cpp
int getErrorCode();
```
#### Summary
Return error code from latest operation.

#### Parameters
None

#### Return Value
Integer representing an error code. [More info](#error-codes)

### Usage Example
See [**Example**](#example)



# Error Codes
### Code 0
**This indicates no error.** Everything works fine.
### Code 1
**Sensor is offline.** This happens when Arduino cannot connect to the sensor. 

If this happens all the time from call to `measure()`. It is likely the the sensor is connected incorrectly or
the sensor does not functional properly.

If this happens occasionally, It is possible that the call to `measure()` hits the sensor when it is sleeping.  
Try to slow down the call to `measure()`.
### Code 2
**CRC validation failed.** This happends when data transmitted from the sensor is received with errors. 

Possible causes may be bad connections, lengthy cable, etc.

# Example
```cpp
// Include library into the sketch
#include <AM2320.h>

// Create an instance of sensor
AM2320 sensor;

void setup() {
  // enable serial communication
// Include library into the sketch
#include <AM2320.h>

// Create an instance of sensor
AM2320 sensor;

void setup() {
  // enable serial communication
  Serial.begin(115200);
  // call sensor.begin() to initialize the library
  sensor.begin();
}

void loop() {

  // sensor.measure() returns boolean value
  // - true indicates measurement is completed and success
  // - false indicates that either sensor is not ready or crc validation failed
  //   use getErrorCode() to check for cause of error.
  if (sensor.measure()) {
    Serial.print("Temperature: ");
    Serial.println(sensor.getTemperature());
    Serial.print("Humidity: ");
    Serial.println(sensor.getHumidity());
  }
  else {  // error has occured
    int errorCode = sensor.getErrorCode();
    switch (errorCode) {
      case 1: Serial.println("ERR: Sensor is offline"); break;
      case 2: Serial.println("ERR: CRC validation failed."); break;
    }    
  }

  delay(500);
}
```
# Contributions
Open an issue if something doesn't work right. Submit pull request if you want to improve someting.

# The Story
I wrote blog posts about the process of writing this library. You can find it here.

[Part 1](https://hibikiledo.xyz/2016/11/04/writing-am2320-arduino-library-1/)
Part 2 .. working on it
