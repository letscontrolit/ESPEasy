# ServoESP32 [![Build Status](https://travis-ci.com/RoboticsBrno/ServoESP32.svg?branch=master)](https://travis-ci.com/RoboticsBrno/ServoESP32)

Generate RC servo signal on a selected pins with ESP32 device and Arduino framework.

Base on [servo library for stm32f4 (d2a4a47)](https://github.com/arduino-libraries/Servo/blob/master/src/stm32f4/ServoTimers.h).

## Interface

The interface is similar to Arduino/Servo: https://www.arduino.cc/en/Reference/Servo

But the function `attach()` is different:

```c
bool attach(
    int pin,
    int channel = CHANNEL_NOT_ATTACHED,
    int minAngle = DEFAULT_MIN_ANGLE,
    int maxAngle = DEFAULT_MAX_ANGLE,
    int minPulseWidthUs = DEFAULT_MIN_PULSE_WIDTH_US,
    int maxPulseWidthUs = DEFAULT_MAX_PULSE_WIDTH_US,
    int frequency = DEFAULT_FREQUENCY
);
```

More information in [source code documentation](src/Servo.h).

Example: [04-SimpleServoAngles](examples/04-SimpleServoAngles/04-SimpleServoAngles.ino)

There are also a ServoFloat and ServoDouble variant available. Use one of these when working in radians. 

Example: : [05-SimpleServoRadians](examples/05-SimpleServoRadians/05-SimpleServoRadians.ino)

### IMPORTANT INFO
According testings, the frequency for ESP32 S2/S3/C3 has to be set at least to 200 Hz. Here is an example, how to set just frequency:

```cpp
Servo servo1;
const int servoPin = 4;
const int frequency = 200; // Hz

servo1.attach(
    servoPin, 
    Servo::CHANNEL_NOT_ATTACHED, 
    Servo::DEFAULT_MIN_ANGLE, 
    Servo::DEFAULT_MAX_ANGLE, 
    Servo::DEFAULT_MIN_PULSE_WIDTH_US, 
    Servo::DEFAULT_MAX_PULSE_WIDTH_US, 
    frequency
);
```

For more information look at the [PR25](https://github.com/RoboticsBrno/ServoESP32/pull/25) 

## PlatformIO

This library is also available at the [PlatformIO](https://platformio.org) as [ServoESP32](https://platformio.org/lib/show/1739/ServoESP32).

## Arduino IDE

This library is available in Arduino IDE Library Manager as `ServoESP32`.

## Known issues

### Problem with build in Arduino IDE 1.8.10

There was an [issue](https://github.com/arduino/arduino-cli/pull/565) with building this library in Arduino IDE 1.8.10. But this issue should be fixed in Arduino IDE 1.8.11.
