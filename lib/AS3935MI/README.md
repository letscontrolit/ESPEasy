# Yet Another Arduino ams AS3935 'Franklin' lightning sensor library
home: https://bitbucket.org/christandlg/as3935mi
sensor: https://ams.com/as3935

## Features:
 - Supports I2C and SPI via the Wire and SPI libraries, respectively
 - Supports I2C and SPI interfaces via other libraries (e.g. Software I2C) by inheritance
 - Automatic antenna tuning

## Changelog:
- 1.3.5
	- fixed #50
	- implemented a more robust, interrupt based calibration procedure that is also faster. thanks to @td-er for reporting and impementing this. 
	- updated increase / decrease function signatures (should be backwards compatible)
	- updated examples
	
- 1.3.4
	- partially fixed https://bitbucket.org/christandlg/as3935mi/issues/50/resonance-frequency-calibration-inaccurate : fixed a bug where occasionally I2C comms will silently fail during oscillator calibration - thanks to @td-er for reporting and fixing this issue

- 1.3.3
	- fixed https://bitbucket.org/christandlg/as3935mi/issues/49/class-spiclass-has-no-member-named
	
- 1.3.2
	- fixed https://bitbucket.org/christandlg/as3935mi/issues/47/need-help-using-as3935spiclass
	
- 1.3.1
	- fixed https://bitbucket.org/christandlg/as3935mi/issues/48/clear-statistics-function-to-be-added
	
- 1.3.0
	- fixed https://bitbucket.org/christandlg/as3935mi/issues/12/autocalibrate-no-longer-working
	
- 1.2.1
	- Merged PR by Hernán Freschi https://bitbucket.org/christandlg/as3935mi/pull-requests/2
	
- 1.2.0
	- extended examples to include increasing sensitivity if no disturbances are detected. 

- 1.1.1
	- fixed an issue where ESP8266 would crash with message "ISR not in IRAM"

- 1.1.0
	- extended function calibrateResonanceFrequency() to return the resonance frequency of the antenna
	
- 1.0.0
	- added more values for watchdog threshold and spike rejection ratio settings
	- fixed an incorrect function name
	- added missing function names to keywords.txt

- 0.5.0
	- added new classes AS3935TwoWire and AS3935SPIClass for TwoWire and SPIClass interfaces
	- moved AS3935I2C and AS3935SPI classes into their own respecitve source files, further separating data processing from communications
	- when updating from an earlier version and using the AS3935I2C or AS3935SPI classes, change ```#include <AS3935MI.h>``` to ```#include <AS3935I2C.h>``` or ```#include <AS3935SPI.h>```, respectively
	- added examples for AS3935TwoWire and AS3935SPIClass 

- 0.4.1
	- fixed an issue where checkIRQ() causes a deadlock on Arduino Nano

- 0.4.0
	- added functions to increase / decrease noise floor threshold
	- added functions to increase / decrease watchdog threshold
	- added functions to increase / decrease spike rejection ratio
	- added function to check communication to sensor
	- added function to check IRQ pin assignment

- 0.3.0
	- derived classes must now implement function beginInterface() instead of begin()

- 0.2.0
	- split code into 3 classes - AS3935MI, AS3935I2C, AS3935SPI
	- users can now implement classes derived from AS3935MI easily
	- writeRegister is nur used to send direct commands
	- updated examples
	- added arduino due I2C issue workaround
	- minor fixes
	- derived 

- 0.1.2
	- renamed library

- 0.1.1
	- added license information

- 0.1.0
	- initial release