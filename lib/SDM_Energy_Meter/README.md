## Library for reading SDM72 SDM120 SDM220 SDM230 SDM630 DDM18SD Modbus Energy meters. ##

### SECTIONS: ###
#### 1. [INTRODUCTION](#introduction) ####
#### 2. [SCREENSHOTS](#screenshots) ####
#### 3. [CONFIGURING](#configuring) ####
#### 4. [INITIALIZING](#initializing) ####
#### 5. [READING](#reading) ####
#### 6. [PROBLEMS](#problems) ####
#### 7. [CREDITS](#credits) ####

---

### Introduction: ###
This library allows you reading SDM module(s) using:
- [x] Hardware Serial (<i><b>recommended option</b>, smallest number of reads errors, especially for esp8266</i>) <b><i>or</i></b>
- [x] Software Serial, attached as core libraries for ESP8266 and AVR or as external lib for ESP32</br>
     (<i>the new version of esp Software Serial library</br>
      has a different initialization compared to avr!</br>
      <b>This version of SDM library (>=2.2.2) works only with esp Software Serial 8.0.1 or higher!!!</b></br>
      If you have an older esp Software Serial version</br>
      then use other SDM library, details below the Credits section</i>)</br>

you also need rs232<->rs485 converter:
- [x] with automatic flow direction control (<i>look at images below</i>) <b><i>or</i></b>
- [x] with additional pins for flow control, like MAX485</br>
     (<i>in this case MAX485 DE and RE pins must be connected together to one of uC pin</br>
     and this pin must be passed when initializing the library</i>)

_Tested on Wemos D1 Mini with Arduino IDE 1.8.3-1.8.10 & ESP8266 core 2.3.0-2.5.2_

---

### Screenshots: ###
<p align="center">
  <img src="https://github.com/reaper7/SDM_Energy_Meter/blob/master/img/hardware_sdm220_1.jpg" height="330"></br>
  <img src="https://github.com/reaper7/SDM_Energy_Meter/blob/master/img/hardware_sdm220_2.jpg" height="330"></br>
  <img src="https://github.com/reaper7/SDM_Energy_Meter/blob/master/img/livepage.gif"></br>
  <i>live page example (extended) screenshot</i>
</p>

---

### Configuring: ###
Default configuration is specified in the [SDM.h](https://github.com/reaper7/SDM_Energy_Meter/blob/master/SDM.h#L18) file, and parameters are set to:</br>
<i>Software Serial mode, baud 4800, uart config SERIAL_8N1, without DE/RE pin,</br>
uart pins for esp32 hwserial and esp32/esp8266/avr swserial as NOT_A_PIN (-1).</br></br>
For esp32 hwserial this means using the default pins for the selected uart port,</br>
specified in the core library (HardwareSerial.cpp).</br>
For swserial option (esp32/esp8266/avr) is necessary</br>
to specify the pin numbers, as described below.</i>

User can set the parameters in two ways:
- by editing the [SDM_Config_User.h](https://github.com/reaper7/SDM_Energy_Meter/blob/master/SDM_Config_User.h) file
- by passing values during initialization (section below)

[SDM_Config_User.h](https://github.com/reaper7/SDM_Energy_Meter/blob/master/SDM_Config_User.h) file includes also two parameters that can be adjusted depending on your needs:
- WAITING_TURNAROUND_DELAY (default set to 200ms) defines the time (after sending the query) for the response from the slave device.
  If the slave device does not send the required number of bytes (FRAMESIZE) within this time, an SDM_ERR_TIMEOUT error will be returned.
- RESPONSE_TIMEOUT (default set to 500ms) defines the time (after sending the request and receiving the reply) to a possible response 
  from other slave devices on the bus, during this time it will not be possible to execute another query.
  It is a protection time for devices that are not able to quickly respond to inquiries.

NOTE for Hardware Serial mode: <i>to force the Hardware Serial mode,</br>
user must edit the corresponding entry in [SDM_Config_User.h](https://github.com/reaper7/SDM_Energy_Meter/blob/master/SDM_Config_User.h#L17) file.</br>
adding #define USE_HARDWARESERIAL to the main ino file is not enough.</i>

---

### Initializing: ###
If the user configuration is specified in the [SDM_Config_User.h](https://github.com/reaper7/SDM_Energy_Meter/blob/master/SDM_Config_User.h) file</br>
or if the default configuration from the [SDM.h](https://github.com/reaper7/SDM_Energy_Meter/blob/master/SDM.h#L18) file is suitable</br>
initialization is limited to passing serial port reference (software or hardware)</br>
and looks as follows:
```cpp
//lib init when Software Serial is used:
#include <SDM.h>
#include <SoftwareSerial.h>

// for ESP8266 and ESP32
SoftwareSerial swSerSDM;
//              _______________________________software serial reference
//             |
SDM sdm(swSerSDM);


// for AVR
SoftwareSerial swSerSDM(SDM_RX_PIN, SDM_TX_PIN);
//                               |           |_tx pin definition(from SDM_Config_User.h)
//                               |_____________rx pin definition(from SDM_Config_User.h)
//
//              _______________________________software serial reference
//             |
SDM sdm(swSerSDM);
```

```cpp
//lib init when Hardware Serial is used:
#include <SDM.h>

//            _________________________________hardware serial reference
//           |
SDM sdm(Serial);
```
If the user wants to temporarily change the configuration during the initialization process</br>
then can pass additional parameters as below:
```cpp
//lib init when Software Serial is used:
#include <SDM.h>
#include <SoftwareSerial.h>

// for ESP8266 and ESP32
SoftwareSerial swSerSDM;
//              ________________________________________software serial reference
//             |      __________________________________baudrate(optional, default from SDM_Config_User.h)
//             |     |           _______________________dere pin for max485(optional, default from SDM_Config_User.h)
//             |     |          |              _________software uart config(optional, default from SDM_Config_User.h)
//             |     |          |             |    _____rx pin number(optional, default from SDM_Config_User.h)
//             |     |          |             |   |    _tx pin number(optional, default from SDM_Config_User.h)
//             |     |          |             |   |   | 
SDM sdm(swSerSDM, 9600, NOT_A_PIN, SWSERIAL_8N1, 13, 15);


// for AVR
SoftwareSerial swSerSDM(10, 11);
//              ________________________________________software serial reference
//             |      __________________________________baudrate(optional, default from SDM_Config_User.h)   
//             |     |           _______________________dere pin for max485(optional, default from SDM_Config_User.h)
//             |     |          |
SDM sdm(swSerSDM, 9600, NOT_A_PIN);
```

```cpp
//lib init when Hardware Serial is used:
#include <SDM.h>

// for ESP8266
//            ______________________________________hardware serial reference
//           |      ________________________________baudrate(optional, default from SDM_Config_User.h)
//           |     |           _____________________dere pin for max485(optional, default from SDM_Config_User.h)
//           |     |          |            _________hardware uart config(optional, default from SDM_Config_User.h)
//           |     |          |           |       __swap hw serial pins from 3/1 to 13/15(optional, default from SDM_Config_User.h)
//           |     |          |           |      |
SDM sdm(Serial, 9600, NOT_A_PIN, SERIAL_8N1, false);


// for ESP32
//            ______________________________________hardware serial reference
//           |      ________________________________baudrate(optional, default from SDM_Config_User.h)
//           |     |           _____________________dere pin for max485(optional, default from SDM_Config_User.h)
//           |     |          |            _________hardware uart config(optional, default from SDM_Config_User.h)
//           |     |          |           |    _____rx pin number(optional, default from SDM_Config_User.h)
//           |     |          |           |   |    _tx pin number(optional, default from SDM_Config_User.h)
//           |     |          |           |   |   | 
SDM sdm(Serial, 9600, NOT_A_PIN, SERIAL_8N1, 13, 15);


// for AVR
//            ______________________________________hardware serial reference
//           |      ________________________________baudrate(optional, default from SDM_Config_User.h)
//           |     |           _____________________dere pin for max485(optional, default from SDM_Config_User.h)
//           |     |          |            _________hardware uart config(optional, default from SDM_Config_User.h)
//           |     |          |           |
//           |     |          |           |
SDM sdm(Serial, 9600, NOT_A_PIN, SERIAL_8N1);
```
NOTE for ESP8266: <i>when GPIO15 is used (especially for swapped hardware serial):</br>
some converters (like mine) have built-in pullup resistors on TX/RX lines from rs232 side,</br>
connection this type of converters to ESP8266 pin GPIO15 block booting process.</br>
In this case you can replace the pull-up resistor on converter with higher value (100k),</br>
to ensure low level on GPIO15 by built-in in most ESP8266 modules pulldown resistor.</br></i>

---

### Reading: ###
List of available registers for SDM72/120/220/230/630:</br>
https://github.com/reaper7/SDM_Energy_Meter/blob/master/SDM.h#L103
```cpp
//reading voltage from SDM with slave address 0x01 (default)
//                                         ____register name
//                                        |
float voltage = sdm.readVal(SDM_PHASE_1_VOLTAGE);

//reading power from 1st SDM with slave address ID = 0x01
//reading power from 2nd SDM with slave address ID = 0x02
//useful with several meters on RS485 line
//                                      _______register name
//                                     |       SDM device ID  
//                                     |      |
float power1 = sdm.readVal(SDM_PHASE_1_POWER, 0x01);
float power2 = sdm.readVal(SDM_PHASE_1_POWER, 0x02);
```
NOTE: <i>if you reading multiple SDM devices on the same RS485 line,</br>
remember to set the same transmission parameters on each device,</br>
only ID must be different for each SDM device.</i>

---

### Problems: ###
Sometimes <b>readVal</b> return <b>NaN</b> value (not a number),</br>
this means that the requested value could not be read from the sdm module for various reasons.</br>

__Please check out open and close issues, maybe the cause of your error is explained or solved there.__

The most common problems are:
- weak or poorly filtered power supply / LDO, causing NaN readings and ESP crashes</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/13#issuecomment-353532711</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/13#issuecomment-353572909</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/8#issuecomment-381402008</br>
- faulty or incorrectly prepared converter</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/16#issue-311042308</br>
- faulty esp module</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/8#issuecomment-381398551</br>
- many users report that between each readings should be placed <i>delay(50);</i></br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/7#issuecomment-272080139</br>
  (I did not observe such problems using the HardwareSerial connection)</br>
- using GPIO15 without checking signal level (note above)</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/17#issue-313606825</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/13#issuecomment-353413146</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/13#issuecomment-353417658</br>
- compilation error for hardware serial mode</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/23</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/24</br>
- SDM630 Modbus V2 serial stopbit problem</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/49</br>
- Subsequent inquiries for slow slaves</br>
  https://github.com/reaper7/SDM_Energy_Meter/issues/50</br>

You can get last error code using function:
```cpp
//get last error code
//                                      ______optional parameter,
//                                     |      true -> read and reset error code
//                                     |      false or no parameter -> read error code
//                                     |      but not reset stored code (for future checking)
//                                     |      will be overwriten when next error occurs
uint16_t lasterror = sdm.getErrCode(true);

//clear error code also available with:
sdm.clearErrCode();
```
Errors list returned by <b>getErrCode</b>:</br>
https://github.com/reaper7/SDM_Energy_Meter/blob/master/SDM.h#L86</br>

You can also check total number of errors using function:
```cpp
//get total errors counter
//                                       _____optional parameter,
//                                      |     true -> read and reset errors counter
//                                      |     false or no parameter -> read errors counter
//                                      |     but not reset stored counter (for future checking)
uint16_t cnterrors = sdm.getErrCount(true);

//clear errors counter also available with:
sdm.clearErrCount();
```

And finally you can read the counter of correctly made readings:
```cpp
//get total success counter
//                                         ___optional parameter,
//                                        |   true -> read and reset success counter
//                                        |   false or no parameter -> read success counter
//                                        |   but not reset stored counter (for future checking)
uint16_t cntsuccess = sdm.getSuccCount(true);

//clear success counter also available with:
sdm.clearSuccCount();
```

---

### Credits: ###

contribution to this project:</br>
:+1: ESP SoftwareSerial library by Peter Lerup (https://github.com/plerup/espsoftwareserial)</br>
:+1: crc calculation by Jaime García (https://github.com/peninquen/Modbus-Energy-Monitor-Arduino)</br>
:+1: new registers for SDM120 and SDM630 by bart.e (https://github.com/reaper7/SDM_Energy_Meter/pull/3)</br>
:+1: new registers for SDM72 by jegaha (https://github.com/reaper7/SDM_Energy_Meter/pull/34)</br>
:+1: new registers for SDM120CT by JeroenSt (https://github.com/reaper7/SDM_Energy_Meter/pull/41)</br>
:+1: new registers for DDM18SD  by JeroenSt (https://github.com/reaper7/SDM_Energy_Meter/pull/44)</br>
:+1: additional SDM630 registers and influxdb example by AndersV209 (https://github.com/reaper7/SDM_Energy_Meter/pull/45)</br>
:+1: new registers for SDM72DM V2 by datjan (https://github.com/reaper7/SDM_Energy_Meter/pull/62)</br>
:+1: compatibility with EspSoftwareSerial >= 8.0.1 by maxpautsch (https://github.com/reaper7/SDM_Energy_Meter/pull/75)</br>

other projects based on or using this library</br>
:point_right: BZ40i Energy Meter by adlerweb (https://github.com/adlerweb/BZ40i_Energy_Meter)</br>
:point_right: DDS238 Energy Meter by E-NINA (https://github.com/E-NINA/dds238_Energy_Meter)</br>
:point_right: ESPEasy by TD-er (https://github.com/TD-er/ESPEasy)</br>
:point_right: Sonoff-Tasmota by arendst (https://github.com/arendst/Sonoff-Tasmota)</br>

---

<i>library version for old esp software serial (6.0.0 - 7.0.1) is available at [old_esp_swserial_600_701 branch](https://github.com/reaper7/SDM_Energy_Meter/tree/old_esp_swserial_600_701)</i><br>
<i>library version for old esp software serial (5.2.0 - 5.4.0) is available at [old_esp_swserial_520_540 branch](https://github.com/reaper7/SDM_Energy_Meter/tree/old_esp_swserial_520_540)</i><br>
<i>library version for old esp software serial (< 5.2.0) is available at [old_esp_swserial_lib branch](https://github.com/reaper7/SDM_Energy_Meter/tree/old_esp_swserial_lib)</i><br>
<i>old template library version is available at [old_template branch](https://github.com/reaper7/SDM_Energy_Meter/tree/old_template)</i><br>

---

**2016-2023 Reaper7**

[paypal.me/reaper7md](https://www.paypal.me/reaper7md)
