Supported ESP Chips
*******************

ESPEasy does support a number of variants of the processors manufactured by Espressif.

* ESP8266 The original ESP processor, with external flash.
* ESP8285 Mainly found in "Chinese" products. Has flash built in the processor. (almost always 1MByte flash)
* ESP32 The first successor of the ESP82xx, supporting 2 CPU cores, Bluetooth and more RAM and more GPIO pins. (and other new features)
* ESP32-S2 Has more GPIO pins than the ESP32, but only 1 CPU core. Initial support in ESPEasy added since 2021-09-19.
* ESP32-S3 Support added: 2023-05-03
* ESP32-C3 Support added: 2023-05-03
* ESP32-C6 Not yet supported
* ESP32-H2 Not yet supported


.. list-table:: Espressif platforms
   :header-rows: 1
   :widths: 7 7 7 7 7 7
   :stub-columns: 1

   *  - 
      - ESP8266
      - ESP32
      - ESP32-S2
      - ESP32-S3
      - ESP32-C3
   *  - release Year
      - 2014
      - 2016
      - 2019
      - 2021
      - 2020
   *  - Microcontroller
      - Xtensa single-core 32-bit L106
      - Xtensa single/dual-core 32-bit LX6
      - Xtensa single-core 32-bit LX7
      - Xtensa dual-core 32-bit LX7
      - RISC-V single-core 32-bit
   *  - Clock Frequency
      - 80 MHz
      - 160/240 MHz
      - 240 MHz
      - 240 MHz
      - 160 MHz
   *  - Co-processor
      - No
      - ULP
      - ULP (RISC-V)
      - ULP (RISC-V)
      - No
   *  - SRAM
      - 160 kB
      - 520 kB
      - 320 kB
      - 512 kB
      - 400 kB
   *  - ROM
      - No
      - 448 kB
      - 128 kB
      - 384 kB
      - 384 kB
   *  - RTC Memory
      - No
      - 16 kB
      - 16 kB
      - 16 kB
      - 8 kB
   *  - External SPIRAM
      - No
      - Up to 16MB
      - Up to 128MB
      -
      -
   *  - External Flash
      - Up to 16MB
      - Up to 16MB
      - Up to 1GB
      -
      -
   *  - Wi-Fi (802.11 b/g/n)
      - HT20
      - HT20
      - HT20
      -
      -
   *  - ESP-MESH
      - Yes
      - Yes
      - Yes
      - Yes
      -
   *  - Bluetooth
      - No
      - BT 4.2 BR/EDR & BLE
      - No
      - BT 5
      - BT 5
   *  - Ethernet
      - No
      - 10/100 Mbps
      - No
      - No
      - No
   *  - CAN
      - No
      - 2.0
      - No
      - 2.0
      - 2.0
   *  - Time of Flight
      - No
      - No
      - Yes
      - Yes
      - Yes
   *  - GPIO (total)
      - 16
      - 34
      - 43
      - 45
      - 22 or 16
   *  - Touch Sensors
      - No
      - 10
      - 14
      - 14
      - No
   *  - SPI
      - 2
      - 4
      - 4 (OSPI)
      - 4 (OSPI)
      - 3
   *  - I2C
      - 1 (soft)
      - 2
      - 2
      - 2
      - 1
   *  - I2S
      - 2
      - 2
      - 1
      - 2
      - 1
   *  - UART
      - 2 (1 ½ actually)
      - 3
      - 2
      - 3
      - 2
   *  - ADC
      - 1 (10-bit)
      - 18 (12-bit)
      - 20 (12-bit)
      - 20 (12-bit)
      - 6 (12-bit)
   *  - DAC
      - No
      - 2 (8-bit)
      - 2 (8-bit)
      - No (??)
      - No
   *  - PWM (soft)
      - 8
      - 16
      - 8
      - 8
      - 6
   *  - SDMMC
      - No
      - Yes
      - No
      - Yes
      - No
   *  - RMT (remote control)
      - No
      - Yes
      - Yes
      - Yes
      - Yes (2 ch)
   *  - USB
      - No
      - No
      - USB OTG/Serial/JTAG
      - USB OTG/Serial/JTAG
      - USB Serial/JTAG
   *  - LCD Interface
      - No
      - No
      - Yes
      - Yes
      - No
   *  - Camera Interface
      - No
      - No
      - Yes
      - Yes
      - No
   *  - Temperature sensor
      - No
      - Yes
      - Yes
      - Yes
      - Yes
   *  - Hall sensor
      - No
      - Yes
      - No
      - No
      - No
   *  - Security
      - No
      - Secure boot Flash encryption 1024-bit OTP
      - Secure boot Flash encryption 4096-bit OTP
      -
      -
   *  - Crypto
      - No
      - AES, SHA-2, RSA, ECC, RNG
      - AES-128/192/256, SHA-2, RSA, RNG, HMAC, Digital Signature
      - AES-128/256, Hash, RSA, RNG, HMAC, Digital signature
      - AES-128/256, SHA, RSA, RNG, HMAC, Digital signature
   *  - Low Power Consumption
      - 20uA
      - 800 uA light sleep, 10 uA deep sleep
      - 750 uA light sleep, 25 uA deep sleep
      - 240 uA light sleep, 8 uA deep sleep
      - 130 uA light sleep, 5 uA deep sleep


`Table content source <https://maker.pro/esp8266/tutorial/a-comparison-of-the-new-esp32-s2-to-the-esp32>`_


ESP8266/ESP8285
===============

The ESP8266 was the "original" ESP platform and was found in the original Sonoff Basic modules.
The ESP8266 does need an external SPI flash module to store its progam data and configuration.

Later a (probably) less expensive ESP8285 appeared, which does have the SPI flash integrated in the chip.
This means you cannot upgrade it by soldering a larger flash chip on it.
It does however free up 2 GPIO pins, which were previously unavailable as they were used to access the SPI flash.
For some products like the Sonoff 4ch modules this chip was a must-have.

The small 1MB flash does impose some issues when trying to perform an "OTA" update of the firmware as there is not enough space to store the active firmware and the new version at the same time.
For 1MB modules (some ESP8266 modules were also sold with 1MB flash, like the ESP-01 modules) you need to take some extra steps to perform an OTA update.
Otherwise the only way to upgrade to a newer build is by flashing using the serial port.




ESP32
=====

The most important new features of the ESP32, compared to ESP82xx are:

- Dual core CPU @ 240 MHz
- Upto 320 kByte of RAM
- 3 Hardware serial ports
- Bluetooth (not supported yet in ESPEasy)
- Extra GPIO pins
- Upto 18 GPIO pins can operate as ADC.
- Upto 10 touch pins
- Support for Ethernet



ESP32-S2
========

Added: 2021/09

The ESP32-S2 is a bit strange when looking at its features and taking into account it was introduced about 3 years after the ESP32.

The ESP32-S2 is missing quite a lot of useful features its predecessor had:

- No Bluetooth
- Single core
- No support for Ethernet
- No support for CAN
- 2 Harware Serial ports.
- Less RAM

The only advantages of the ESP32-S2 compared to its predecessor are:

- More GPIO pins
- 2 extra ADC capable pins
- 4 extra touch capable pins
- USB OTG (not yet supported in ESPEasy)
- LCD interface (not yet supported in ESPEasy)
- Camera interface (not yet supported in ESPEasy)
- Extra hardware accelerated encryption functions (not yet supported in ESPEasy)
- Supposedly lower power consumption (not yet verified)
- Time of Flight (TOF) support that would (theoretically) allow indoor positioning


ESP32-S3
========

Added: 2023/05/03

.. note:: Support for the ESP32-S3 is very preliminary, as in it is hardly tested (as of May 2023)


ESP32-C3
========

Added: 2023/05/03

.. note:: Support for the ESP32-C3 is very preliminary, as in it is hardly tested (as of May 2023)
