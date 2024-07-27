
## notes on PWM 

#### Description

If you do not know the PWM frequency you can determine the angle
with PWM in the following way.

In one period there are 4351 bits (128 + 4095 + 128)
These come typically in one pulse like

```
00001111111111111111111111111111111111100000000
    HEADER          DATA               POSTLOW
```

#### Step 1

Determine the duration of one cycle by measuring the time between two RISING edges.
Lets call this time FULLSCALE == 4351 bits.

Then the time for one bit BITTIME = round(FULLSCALE / 4351.0);

#### Step 2

Measure the duration of the HIGHPERIOD == HEADER + DATA

We know the HEADER is 128 bits and FULLSCALE = 4351 bits.

DATAPERIOD = HIGHPERIOD - 128 \* BITTIME

ANGLE = 360 \* DATAPERIOD / (4095 \* BITTIME)

Code: AS5600_pwm_test.ino

#### Related

https://github.com/RobTillaart/AS5600
https://forum.arduino.cc/t/questions-using-the-as5600-in-pwm-mode/1266957/3


