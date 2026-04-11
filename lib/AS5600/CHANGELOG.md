# Change Log AS5600
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).


## [0.6.1] - 2024-03-31
- improve **getCumulativePosition()**, catch I2C error, see #62
- update readme.md (incl reorder future work).
- update GitHub actions
- minor edits


## [0.6.0] - 2024-01-25
- add experimental error handling
- add **int lastError()** so user can check the status of last I2C actions.
- update readme.md
- update examples
- minor edits

----

## [0.5.1] - 2023-12-31
- fix #51, add **increaseOffset(float degrees)**
- update keywords.txt
- update readme.md (several cleanups)

## [0.5.0] - 2023-12-07
- refactor API, begin()
- update readme.md
- update examples
- add examples
- patch library.properties  =>  category=Sensors

----

## [0.4.1] - 2023-09-16
- fix #45 support STM32 set I2C pins  ARDUINO_ARCH_STM32
- update readme badges
- minor edits

## [0.4.0] - 2023-06-27
- fix #39 support for Wire2 on ESP32
- update readme.md

----

## [0.3.8] - 2023-06-18
- add **void burnSetting()** improvements from #38
  - use with care
  - add sketches to burn settings (use with care!)
- minor edits.

## [0.3.7] - 2023-05-09
- change **getCumulativePosition()** to use **AS5600_ANGLE**
  so filters can be applied.
- add **AS5600_DEGREES_TO_RAW** to constants.
- add **AS5600_SW_DIRECTION_PIN** to constants.
- minor edits.

## [0.3.6] - 2023-02-20
- add **resetCumulativePosition(int32_t position)** to completely reset the cumulative counter.
This includes the delta since last call to **getCumulativePosition()**.
- add parameter position to **resetPosition(int32_t position)** so a new position can be set.
This does not reset the delta since last call to **getCumulativePosition()**.
- update readme.md

## [0.3.5] - 2023-02-01
- update GitHub actions
- update license 2023
- update readme.md

## [0.3.4] - 2022-12-22
- fix #26 edges problem of the experimental cumulative position (CP).
- decoupled CP from **rawAngle()**
  - now one needs to call **getCumulativePosition()** to update the CP.
- updated the readme.md section about CP.

## [0.3.3] - 2022-12-19
- add experimental continuous position.
  - add **getCumulativePosition()**
  - add **resetPosition()**
  - add **getRevolutions()**
- move code from .h to .cpp
- add AS5600_MODE_RPM to **getAngularSpeed()**
- add AS5600_RAW_TO_RPM
- add example for AS5600_MODE_RPM.
- add AS5600_DEFAULT_ADDRESS
- add AS5600L_DEFAULT_ADDRESS
- update readme.md

## [0.3.2] - 2022-10-16
- add CHANGELOG.md
- update readme.md
- update build-CI to support RP2040

## [0.3.1] - 2022-08-11
- add support for AS5600L (I2C address)
- add magnetTooStrong() + magnetTooWeak();
- add / update examples
- update documentation

## [0.3.0] - 2022-07-07
- fix #18 invalid mask setConfigure().

----

## [0.2.1] - not released
- add bool return to set() functions.
- update Readme (analog / PWM out)

## [0.2.0] - 2022-06-28
- add software based direction control.
- add examples
- define constants for configuration functions.
- fix conversion constants (4096 based)
- add get- setOffset(degrees)   functions. (no radians yet)

----

## [0.1.4] - 2022-06-27
- fix #7 use readReg2() to improve I2C performance.
- define constants for configuration functions.
- add examples - especially OUT pin related.
- fix default parameter of the begin function.

## [0.1.3] - 2022-06-26
- add AS5600_RAW_TO_RADIANS.
- add getAngularSpeed() mode parameter.
- fix #8 bug in configure.

## [0.1.2] - 2022-06-02
- add getAngularSpeed().

## [0.1.1] - 2022-05-31
- add readReg2() to speed up reading 2 byte values.
- fix clock wise and counter clock wise.
- fix shift-direction @ getZPosition, getMPosition,
  getMaxAngle and getConfigure.

## [0.1.0] - 2022-05-28
- initial version

