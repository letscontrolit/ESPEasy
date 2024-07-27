
[![Arduino CI](https://github.com/RobTillaart/AS5600/workflows/Arduino%20CI/badge.svg)](https://github.com/marketplace/actions/arduino_ci)
[![Arduino-lint](https://github.com/RobTillaart/AS5600/actions/workflows/arduino-lint.yml/badge.svg)](https://github.com/RobTillaart/AS5600/actions/workflows/arduino-lint.yml)
[![JSON check](https://github.com/RobTillaart/AS5600/actions/workflows/jsoncheck.yml/badge.svg)](https://github.com/RobTillaart/AS5600/actions/workflows/jsoncheck.yml)
[![GitHub issues](https://img.shields.io/github/issues/RobTillaart/AS5600.svg)](https://github.com/RobTillaart/AS5600/issues)

[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)](https://github.com/RobTillaart/AS5600/blob/master/LICENSE)
[![GitHub release](https://img.shields.io/github/release/RobTillaart/AS5600.svg?maxAge=3600)](https://github.com/RobTillaart/AS5600/releases)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/robtillaart/library/AS5600.svg)](https://registry.platformio.org/libraries/robtillaart/AS5600)


# AS5600

Arduino library for AS5600 and AS5600L magnetic rotation meter.


## Description

#### AS5600

**AS5600** is a library for an AS5600 / AS5600L based magnetic **rotation** meter.
More exact, it measures the angle (rotation w.r.t. reference) and not RPM.
Multiple angle measurements allows one to calculate or estimate the RPM.

The AS5600 and AS5600L sensors are pin compatible (always check your model's datasheet).

**Warning: experimental**

The sensor can measure a full rotation in 4096 steps.
The precision of the position is therefore limited to approx 0.1°.
Noise levels are unknown, but one might expect that the sensor is affected by electric
and or magnetic signals in the environment.
Also unknown is the influence of metals near the sensor or an unstable 
or fluctuating power supply.

Please share your experiences.


#### 0.5.0 Breaking change

Version 0.5.0 introduced a breaking change.
You cannot set the SDA and SCL pins in **begin()** any more.
This reduces the dependency of processor dependent Wire implementations.
The user has to call **Wire.begin()** and can optionally set the I2C pins 
before calling **AS5600.begin()**.


#### Related libraries

- https://github.com/RobTillaart/Angle
- https://github.com/RobTillaart/AngleConvertor
- https://github.com/RobTillaart/AverageAngle
- https://github.com/RobTillaart/runningAngle


## Hardware connection

The I2C address of the **AS5600** is always 0x36.

The sensor should connect the I2C lines SDA and SCL and the
VCC and GND to communicate with the processor.
Do not forget to add the pull up resistors to improve the I2C signals.

The AS5600 datasheet states it supports Fast-Mode == 400 KHz
and Fast-Mode-Plus == 1000 KHz.


#### Pull ups

I2C performance tests with an AS5600L with an UNO failed at 400 KHz. 
After investigation it became clear that pull ups are mandatory.
The UNO expects 5 Volt I2C signals from the AS5600.
However the device only provides 3V3 pulses on the bus.
So the signal was not stable fast enough (not "square enough").
After applying pull ups the AS5600L worked up to 1000 KHz.


#### DIR pin

From datasheet, page 30 - Direction (clockwise vs. counter-clockwise)

_**The AS5600 allows controlling the direction of the magnet 
rotation with the DIR pin. If DIR is connected to GND (DIR = 0)
a clockwise rotation viewed from the top will generate an 
increment of the calculated angle. If the DIR pin is connected
to VDD (DIR = 1) an increment of the calculated angle will 
happen with counter-clockwise rotation.**_

This AS5600 library offers a 3rd option for the DIR (direction) pin of the sensor:

1. Connect to **GND** ==> fixed clockwise(\*).  This is the default.
1. Connect to **VCC** ==> fixed counter-clockwise.
1. Connect to an IO pin of the processor == Hardware Direction Control by library.

In the 3rd configuration the library controls the direction of counting by initializing 
this pin in **begin(directionPin)**, followed by **setDirection(direction)**. 
For the parameter direction the library defines two constants named:

- **AS5600_CLOCK_WISE (0)**
- **AS5600_COUNTERCLOCK_WISE (1)**

(\*) if **begin()** is called without **directionPin** or with this 
parameter set to **255**, Software Direction Control is enabled.

See Software Direction Control below for more information.


#### OUT pin

The sensor has an output pin named **OUT**.
This pin can be used for an analogue or PWM output signal (AS5600),
and only for PWM by the AS5600L.

See **Analogue Pin** and **PWM Pin** below.

Examples are added to show how to use this pin with **setOutputMode()**.

See more in the sections Analog OUT and PWM OUT below.


##### Note: (From Zipdox2 - See issue #36)

Some AS5600 modules seem to have a resistor between **PGO** and **GND**.
This causes the AS5600 to disable the output (to use it for programming, see datasheet). 
This resistor needs to be removed to use the **OUT** pin.


#### PGO pin

Not tested. ==> Read the datasheet!

PGO stands for ProGramming Option, it is used to calibrate and program the sensor.
As the sensor can be programmed only a few times one should
use this functionality with extreme care.
See datasheet for a detailed list of steps to be done.

See **Make configuration persistent** below.


## I2C 

The I2C address of the **AS5600** is always 0x36.

#### Address

|  sensor  |  address  |  changeable  |
|:--------:|:---------:|:-------------|
|  AS5600  |    0x36   |  NO          |
|  AS5600L |    0x40   |  YES, check setAddress()  |

To use more than one **AS5600** on one I2C bus, see Multiplexing below.

The **AS5600L** supports the change of I2C address, optionally permanent.
Check the **setAddress()** function for non-permanent change. 


#### I2C multiplexing

Sometimes you need to control more devices than possible with the default
address range the device provides.
This is possible with an I2C multiplexer e.g. TCA9548 which creates up 
to eight channels (think of it as I2C subnets) which can use the complete 
address range of the device. 

Drawback of using a multiplexer is that it takes more administration in 
your code e.g. which device is on which channel. 
This will slow down the access, which must be taken into account when
deciding which devices are on which channel.
Also note that switching between channels will slow down other devices 
too if they are behind the multiplexer.

- https://github.com/RobTillaart/TCA9548

Alternative could be the use of a AND port for the I2C clock line to prevent 
the sensor from listening to signals on the I2C bus. 

Finally the sensor has an analogue output **OUT**.
This output could be used to connect multiple sensors to different analogue ports of the processor.

**Warning**: If and how well this analog option works is not verified or tested.


#### Performance

|     board     |  sensor   |  results         |  notes  |
|:-------------:|:---------:|:-----------------|:--------|
|  Arduino UNO  |  AS5600   |  up to 900 KHz.  |  https://github.com/RobTillaart/AS5600/issues/22
|  Arduino UNO  |  AS5600L  |  up to 300 KHz.  |
|  ESP32        |  AS5600   |  no data         |
|  ESP32        |  AS5600L  |  up to 800 KHz   |

No other boards tested yet.


#### ESP32 and I2C

When polling the AS5600 with an ESP32 to measure RPM an issue has been reported.
See https://github.com/RobTillaart/AS5600/issues/28

The problem is that the ESP32 can be blocking for up to one second if there is a
problem in the connection with the sensor. 
Using **setWireTimeout()** does not seem to solve the problem (2023-01-31). 
In the issue the goal was to measure the turns of a rotating device at around 3800 RPM.

3800 RPM == 64 rounds / second.

To measure speed one need at least 3 angle measurements per rotation.
This results in at least 192 measurements per second which is about 1 per 5 milliseconds.

Given that the ESP32 can block for a second, it can not be guaranteed to be up to date.
Not for speed, but also not for total number of rotations.


## Interface

```cpp
#include "AS5600.h"
```

#### Constants

Most important are:

```cpp
//  setDirection
const uint8_t AS5600_CLOCK_WISE         = 0;  //  LOW
const uint8_t AS5600_COUNTERCLOCK_WISE  = 1;  //  HIGH

//  0.087890625;
const float   AS5600_RAW_TO_DEGREES     = 360.0 / 4096;
//  0.00153398078788564122971808758949;
const float   AS5600_RAW_TO_RADIANS     = PI * 2.0 / 4096;
//  4.06901041666666e-6
const float   AS5600_RAW_TO_RPM         = 1.0 / 4096 / 60;

//  getAngularSpeed
const uint8_t AS5600_MODE_DEGREES       = 0;
const uint8_t AS5600_MODE_RADIANS       = 1;
const uint8_t AS5600_MODE_RPM           = 2;
```

See AS5600.h file (and datasheet) for all constants.
Also Configuration bits below for configuration related ones.


#### Constructor + I2C

- **AS5600(TwoWire \*wire = &Wire)** Constructor with optional Wire 
interface as parameter.
- **bool begin(uint8_t directionPin = AS5600_SW_DIRECTION_PIN)** 
  set the value for the directionPin. 
If the pin is set to AS5600_SW_DIRECTION_PIN, the default value, 
there will be software direction control instead of hardware control.
See below.
- **bool isConnected()** checks if the address 0x36 (AS5600) is on the I2C bus.
- **uint8_t getAddress()** returns the fixed device address 0x36 (AS5600).


#### Direction

To define in which way the sensor counts up.

- **void setDirection(uint8_t direction = AS5600_CLOCK_WISE)** idem.
- **uint8_t getDirection()** returns AS5600_CLOCK_WISE (0) or
AS5600_COUNTERCLOCK_WISE (1).

See Software Direction Control below for more information.


#### Configuration registers

Please read the datasheet for details.

- **bool setZPosition(uint16_t value)** set start position for limited range. 
Value = 0..4095. Returns false if parameter is out of range.
- **uint16_t getZPosition()** get current start position.
- **bool setMPosition(uint16_t value)** set stop position for limited range. 
Value = 0..4095. Returns false if parameter is out of range.
- **uint16_t getMPosition()** get current stop position.
- **bool setMaxAngle(uint16_t value)** set limited range.
Value = 0..4095. Returns false if parameter is out of range.
See datasheet **Angle Programming**
- **uint16_t getMaxAngle()** get limited range.


#### Configuration bits

Please read datasheet for details.

- **bool setConfigure(uint16_t value)** value == 0..0x3FFF
Access the register as bit mask.
Returns false if parameter is out of range.
- **uint16_t getConfigure()** returns the current configuration register a bit mask.


|  Bit    |  short  | Description     |  Values                                                | 
|:-------:|:--------|:----------------|:-------------------------------------------------------|
|  0-1    |  PM     |  Power mode     |  00 = NOM,  01 = LPM1,   10 = LPM2,   11 = LPM3        |
|  2-3    |  HYST   |  Hysteresis     |  00 = OFF,  01 = 1 LSB,  10 = 2 LSB,  11 = 3 LSB       |
|  4-5    |  OUTS   |  Output Stage   |  00 = analog (0-100%), 01 = analog (10-90%), 10 = PWM  |
|  6-7    |  PWMF   |  PWM frequency  |  00 = 115,  01 = 230,    10 = 460,    11 = 920 (Hz)    |
|  8-9    |  SF     |  Slow Filter    |  00 = 16x,  01 = 8x,     10 = 4x,     11 = 2x          |
|  10-12  |  FTH    |  Fast Filter    |  Threshold 000 - 111 check datasheet                   |
|  13     |  WD     |  Watch Dog      |  0 = OFF,    1 = ON                                    |
|  15-14  |         |  not used       |  


The library has functions to address these fields directly.

The setters() returns false if parameter is out of range.

- **bool setPowerMode(uint8_t powerMode)** 
- **uint8_t getPowerMode()**
- **bool setHysteresis(uint8_t hysteresis)**
Suppresses "noise" on the output when the magnet is not moving.
In a way one is trading precision for stability.
- **uint8_t getHysteresis()**
- **bool setOutputMode(uint8_t outputMode)**
- **uint8_t getOutputMode()**
- **bool setPWMFrequency(uint8_t pwmFreq)**
- **uint8_t getPWMFrequency()**
- **bool setSlowFilter(uint8_t mask)**
- **uint8_t getSlowFilter()**
- **bool setFastFilter(uint8_t mask)**
- **uint8_t getFastFilter()**
- **bool setWatchDog(uint8_t mask)**
- **uint8_t getWatchDog()**


#### Read Angle

- **uint16_t rawAngle()** returns 0 .. 4095. (12 bits) 
Conversion factor AS5600_RAW_TO_DEGREES = 360 / 4096 = 0.087890625 
or use AS5600_RAW_TO_RADIANS if needed. 
- **uint16_t readAngle()** returns 0 .. 4095. (12 bits) 
Conversion factor AS5600_RAW_TO_DEGREES = 360 / 4096 = 0.087890625 
or use AS5600_RAW_TO_RADIANS if needed.
The value of this register can be affected by the configuration bits above.
This is the one most used. 
- **bool setOffset(float degrees)** overwrites the **existing** offset.
It sets an offset in degrees, e.g. to calibrate the sensor after mounting.
Typical values are -359.99 - 359.99 probably smaller. 
Larger values will be mapped back to this interval.
Be aware that larger values will affect / decrease the precision of the 
measurements as floats have only 7 significant digits.
Verify this for your application.
Returns false if **degrees** > 360000.
- **float getOffset()** returns offset in degrees.
- **bool increaseOffset(float degrees)** adds degrees to the **existing** offset. 
If **setOffset(20)** is called first and **increaseOffset(-30)** thereafter the
new offset is -10 degrees.
Returns false if **degrees** > 360000.

In issue #14 there is a discussion about **setOffset()**.
A possible implementation is to ignore all values outside the
-359.99 - 359.99 range.
This would help to keep the precision high. User responsibility.

In #51 increaseOffset is discussed.

```cpp
//  offset == 0;
as.setOffset(20);
//  offset == 20;
as.setOffset(-30);
//  offset = -30;

//  versus

//  offset == 0;
as.setOffset(20);
//  offset == 20;
as.increaseOffset(-30);
//  offset = -10;
```


#### Angular Speed

- **float getAngularSpeed(uint8_t mode = AS5600_MODE_DEGREES)** is an experimental function that returns 
an approximation of the angular speed in rotations per second.
The function needs to be called at least **four** times per rotation
or once per second to get a reasonably precision. 

|  mode                 |  value  |  description   |  notes  |
|:----------------------|:-------:|:---------------|:--------|
|  AS5600_MODE_RADIANS  |    1    |  radians /sec  |         |
|  AS5600_MODE_DEGREES  |    0    |  degrees /sec  | default |
|  other                |    -    |  degrees /sec  |         |

Negative return values indicate reverse rotation. 
What that exactly means depends on the setup of your project.

Note: the first call will return an erroneous value as it has no
reference angle or time. 
Also if one stops calling this function 
for some time the first call after such delays will be incorrect.

Note: the frequency of calling this function of the sensor depends on the application. 
The faster the magnet rotates, the faster it may be called.
Also if one wants to detect minute movements, calling it more often is the way to go.

An alternative implementation is possible in which the angle is measured twice 
with a short interval. The only limitation then is that both measurements
should be within 180° = half a rotation. 


#### Cumulative position (experimental)

Since 0.3.3 an experimental cumulative position can be requested from the library.
The sensor does not provide interrupts to indicate a movement or revolution
Therefore one has to poll the sensor at a frequency at least **three** times 
per revolution with **getCumulativePosition()**

The cumulative position (32 bits) consists of 3 parts

|  bit    |  meaning      |  notes  |
|:-------:|:--------------|:--------|
|    31   |  sign         |  typical + == CW, - == CCW
|  30-12  |  revolutions  |
|  11-00  |  raw angle    |  call getCumulativePosition() 


Functions are:

- **int32_t getCumulativePosition()** reads sensor and updates cumulative position.
- **int32_t getRevolutions()** converts last position to whole revolutions.
Convenience function.
- **int32_t resetPosition(int32_t position = 0)** resets the "revolutions" to position (default 0).
It does not reset the delta (rotation) since last call to **getCumulativePosition()**.
Returns last position (before reset).
- **int32_t resetCumulativePosition(int32_t position = 0)** completely resets the cumulative counter. 
This includes the delta (rotation) since last call to **getCumulativePosition()**.
Returns last position (before reset).

As this code is experimental, names might change in the future (0.4.0)?
As the function are mostly about counting revolutions the current thoughts for new names are:

```cpp
int32_t updateRevolutions()  replaces getCumulativePosition()
int32_t getRevolutions()
int32_t resetRevolutions()   replaces resetPosition()
```


#### Status registers

- **uint8_t readStatus()** see Status bits below.
- **uint8_t readAGC()** returns the Automatic Gain Control.
0..255 in 5V mode, 0..128 in 3V3 mode.
- **uint16_t readMagnitude()** reads the current internal magnitude.
(page 9 datasheet)
Scale is unclear, can be used as relative scale.
- **bool detectMagnet()** returns true if device sees a magnet.
- **bool magnetTooStrong()** idem.
- **bool magnetTooWeak()** idem.


#### Status bits

Please read datasheet for details.

|  Bit  | short | Description   | Values                | 
|:-----:|:------|:-------------:|:----------------------|
|  0-2  |       | not used      |                       |
|  3    |  MH   | overflow      | 1 = magnet too strong |
|  4    |  ML   | underflow     | 1 = magnet too weak   |
|  5    |  MD   | magnet detect | 1 = magnet detected   |
|  6-7  |       | not used      |                       |


#### Error handling

Since 0.5.2 the library has added **experimental** error handling.
For now only lowest level I2C errors are checked for transmission errors.
Error handling might be improved upon in the future.

Note: The public functions do not act on error conditions.
This might change in the future.
So the user should check for error conditions.

```cpp
int e = lastError();
if (e != AS5600_OK)
{
  //  handle error
}
```


- **int lastError()** returns the last error code.
After reading the error status is cleared to **AS5600_OK**.


|  Error codes              |  value  |  notes    |
|:--------------------------|:-------:|:----------|
|  AS5600_OK                |     0   |  default  |
|  AS5600_ERROR_I2C_READ_0  |  -100   |
|  AS5600_ERROR_I2C_READ_1  |  -101   |
|  AS5600_ERROR_I2C_READ_2  |  -102   |
|  AS5600_ERROR_I2C_READ_3  |  -103   |
|  AS5600_ERROR_I2C_WRITE_0 |  -200   |
|  AS5600_ERROR_I2C_WRITE_1 |  -201   |


## Make configuration persistent. BURN

#### Read burn count

- **uint8_t getZMCO()** reads back how many times the ZPOS and MPOS 
registers are written to permanent memory. 
You can only burn a new Angle 3 times to the AS5600, and only 2 times for the AS5600L. This function is safe as it is readonly.


#### BURN function

The burn functions are used to make settings persistent. 
These burn functions are permanent, therefore they are commented in the library.
Please read datasheet twice, before uncomment them.

**USE AT OWN RISK**

Please read datasheet **twice** as these changes are not reversible.

The risk is that you make your AS5600 / AS5600L **USELESS**.

**USE AT OWN RISK**

These are the two "unsafe" functions:
- **void burnAngle()** writes the ZPOS and MPOS registers to permanent memory. 
You can only burn a new Angle maximum **THREE** times to the AS5600
and **TWO** times for the AS5600L.
- **void burnSetting()** writes the MANG register to permanent memory. 
You can write this only **ONE** time to the AS5600.

Some discussion about burning see issue #38  
(I have no hands on experience with this functions)

**USE AT OWN RISK**


## Software Direction Control

Experimental 0.2.0

Normally one controls the direction of the sensor by connecting the DIR pin 
to one of the available IO pins of the processor. 
This IO pin is set in the library as parameter of the **begin(directionPin)** function.

The directionPin is default set to 255, which defines a **Software Direction Control**.

To have this working one has to connect the **DIR pin of the sensor to GND**.
This puts the sensor in a hardware clockwise mode. Then it is up to the library 
to do the additional math so the **readAngle()** and **rawAngle()** behave as 
if the DIR pin was connected to a processor IO pin.

The user still calls **setDirection()** as before to change the direction
of the increments and decrements.

The advantage is one does not need that extra IO pin from the processor. 
This makes connecting the sensor a bit easier.

TODO: measure performance impact.

TODO: investigate impact on functionality of other registers.


## Analog OUT

(details datasheet - page 25 = AS5600)

The OUT pin can be configured with the function:

- **bool setOutputMode(uint8_t outputMode)**


#### AS5600

When the analog OUT mode is set the OUT pin provides a voltage
which is linear with the angle. 

| VDD |  mode  | percentage | output    |  1° in V   |
|:---:|:------:|:----------:|:---------:|:----------:|
| 5V0 |   0    |  0 - 100%  | 0.0 - 5.0 | 0.01388889 |
| 5V0 |   1    |  10 - 90%  | 0.5 - 4.5 | 0.01111111 |
| 3V3 |   0    |  0 - 100%  | 0.0 - 3.3 | 0.00916667 |
| 3V3 |   1    |  10 - 90%  | 0.3 - 3.0 | 0.00750000 |

To measure these angles a 10 bit ADC or higher is needed.

When analog OUT is selected **readAngle()** will still return valid values.


#### AS5600L

The AS5600L does **NOT** support analog OUT.
Both mode 0 and 1 will set the OUT pin to VDD (+5V0 or 3V3).


## PWM OUT

(details datasheet - page 27 = AS5600)

The OUT pin can be configured with the function:

- **bool setOutputMode(uint8_t outputMode)** outputMode = 2 = PWM

When the PWM OUT mode is set the OUT pin provides a duty cycle which is linear 
with the angle. However they PWM has a lead in (HIGH) and a lead out (LOW).

The pulse width is 4351 units, 128 high, 4095 angle, 128 low.

|  Angle  |   HIGH  |  LOW   |  HIGH %  |  LOW %   |  Notes  |
|:-------:|:-------:|:------:|:--------:|:--------:|:--------|
|     0   |    128  |  4223  |   2,94%  |  97,06%  |
|    10   |    242  |  4109  |   5,56%  |  94,44%  |
|    20   |    356  |  3996  |   8,17%  |  91,83%  |
|    45   |    640  |  3711  |  14,71%  |  85,29%  |
|    90   |   1152  |  3199  |  26,47%  |  73,53%  |
|   135   |   1664  |  2687  |  38,24%  |  61,76%  |
|   180   |   2176  |  2176  |  50,00%  |  50,00%  |
|   225   |   2687  |  1664  |  61,76%  |  38,24%  |
|   270   |   3199  |  1152  |  73,53%  |  26,47%  |
|   315   |   3711  |   640  |  85,29%  |  14,71%  |
|   360   |   4223  |   128  |  97,06%  |   2,94%  | in fact 359.9 something as 360 == 0


#### Formula:

based upon the table above ```angle = map(dutyCycle, 2.94, 97.06, 0.0, 359.9);```

or in code ..
```cpp
t0 = micros();  // rising;
t1 = micros();  // falling;
t2 = micros();  // rising;  new t0

//  note that t2 - t0 should be a constant depending on frequency set.
//  however as there might be up to 5% variation it cannot be hard coded.
float dutyCycle = (1.0 * (t1 - t0)) / (t2 - t0);  
float angle     = (dutyCycle - 0.0294) * (359.9 / (0.9706 - 0.0294));

```


#### PWM frequency

The AS5600 allows one to set the PWM base frequency (~5%)
- **bool setPWMFrequency(uint8_t pwmFreq)**

| mode | pwmFreq | step in us | 1° in time |
|:----:|:-------:|:----------:|:----------:|
|   0  |  115 Hz |    2.123   |    24.15   |
|   1  |  230 Hz |    1.062   |    12.77   |
|   2  |  460 Hz |    0.531   |     6.39   |
|   3  |  920 Hz |    0.216   |     3.20   |

Note that at the higher frequencies the step size becomes smaller
and smaller and it becomes harder to measure.
You need a sub-micro second hardware timer to measure the pulse width 
with enough precision to get the max resolution.

When PWM OUT is selected **readAngle()** will still return valid values.


----

## AS5600L class

- **AS5600L(uint8_t address = 0x40, TwoWire \*wire = &Wire)** constructor.
As the I2C address can be changed in the AS5600L, the address is a parameter of the constructor.
This is a difference with the AS5600 constructor.


#### Setting I2C address

- **bool setAddress(uint8_t address)** Returns false if the I2C address is not in valid range (8-119).


#### Setting I2C UPDT

UPDT = update  page 30 - AS5600L 

- **bool setI2CUPDT(uint8_t value)**
- **uint8_t getI2CUPDT()**

These functions seems to have only a function in relation to **setAddress()** so possibly obsolete in the future. 
If you got other insights on these functions please let me know.

----

## Operational

The base functions are:

```cpp
AS5600 as5600;

void setup()
{
  Serial.begin(115200);
  ...
  as5900.begin(4);     //  set the direction pin
  as5600.setDirection(AS5600_CLOCK_WISE);
  ...
}

void loop()
{
...
  Serial.println(as5600.readAngle());
  delay(1000);
...
}
```

See examples.


## Future

Some ideas are kept here so they won't get lost.
priority is relative.


#### Must

- re-organize readme
- rename revolution functions
  - to what?


#### Should

- implement extended error handling in public functions.
  - will increase footprint !! how much?
  - **call writeReg() only if readReg() is OK** ==> prevent incorrect writes
    - ```if (_error != 0) return false;```
    - idem readReg2()
  - set AS5600_ERROR_PARAMETER  e.g. setZPosition()
  - a derived class with extended error handling?
- investigate **readMagnitude()**
  - combination of AGC and MD, ML and MH flags?
- investigate **OUT** behaviour in practice
  - analogue
  - PWM
  - need AS5600 on breakout with support
- check / verify Power-up time
  - 1 minute (need HW)
- check Timing Characteristics (datasheet)
  - is there improvement possible.


#### Could

- investigate **PGO** programming pin.
- check for compatible devices
  - AS5200 ?
- investigate performance
  - basic performance per function
  - I2C improvements
  - software direction
- write examples:
  - as5600_calibration.ino (needs HW and lots of time)
  - different configuration options

#### Wont (unless)

- fix for AS5600L as it does not support analog OUT.
  - type field?
  - other class hierarchy? 
    - base class with commonalities?
  - ==> just ignore for now.
- add mode parameter to offset functions.
  - see getAngularSpeed()


## Support

If you appreciate my libraries, you can support the development and maintenance.
Improve the quality of the libraries by providing issues and Pull Requests, or
donate through PayPal or GitHub sponsors.

Thank you,


