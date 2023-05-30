Supported ESP Chips
*******************

ESPEasy does support a number of variants of the processors manufactured by Espressif.

* ESP8266 The original ESP processor, with external flash.
* ESP8285 Mainly found in "Chinese" products. Has flash built in the processor. (almost always 1MByte flash)
* ESP32 The first successor of the ESP82xx, supporting 2 CPU cores, Bluetooth and more RAM and more GPIO pins. (and other new features)
* ESP32-S2 Has more GPIO pins than the ESP32, but only 1 CPU core. Initial support in ESPEasy added since 2021-09-19.
* ESP32-S3 Support added: 2023-05-03
* ESP32-C3 Support added: 2023-05-03
* ESP32-C2 (ESP8684) Not yet supported
* ESP32-C6 Not yet supported
* ESP32-H2 Not yet supported


.. list-table:: Espressif platforms
   :header-rows: 1
   :widths: 7 7 7 7 7 7 7 7 7
   :stub-columns: 1

   *  - 
      - ESP8266 (ESP8285)
      - ESP32
      - ESP32-S2
      - ESP32-S3
      - ESP32-C3 (ESP8685)
      - ESP32-C2 (ESP8684)
      - ESP32-C6
      - ESP32-H2
   *  - CPU
      - Xtensa® single-core 32-bit L106
      - Xtensa® dual-core 32-bit LX6
      - Xtensa® single-core 32-bit LX7
      - Xtensa® dual-core 32-bit LX7
      - 32-bit RISC-V single-core processor
      - 32-bit RISC-V single-core processor
      - 32-bit RISC-V single-core processor
      - 32-bit RISC-V single-core processor
   *  - Core
      - 1
      - 2
      - 1
      - 2
      - 1
      - 1
      - 1
      - 1
   *  - Freq. (MHz)
      - 80
      - 240
      - 240
      - 240
      - 160
      - 120
      - 160
      - 96
   *  - Voltage (V)
      - 2.5 ~ 3.6
      - 3.0 ~ 3.6
      - 3.0 ~ 3.6
      - 3.0 ~ 3.6
      - 3.0 ~ 3.6
      - 3.0 ~ 3.6
      - 3.0 ~ 3.6
      - 3.3 ~ 3.6
   *  - ESPEasy supported since
      - 2015/05
      - 2017/12
      - 2021/09
      - 2023/05
      - 2023/05
      - 
      - 
      - 
   *  - Introduction
      - 2014
      - 2016
      - 2019
      - 2021
      - 2020
      - 2022
      - 2021
      - 2021
   *  - Status (2023/05)
      - NRND
      - Mass Production
      - NRND
      - Mass Production
      - Mass Production
      - Mass Production
      - Mass Production
      - Sample
   *  - Wi-Fi
      - IEEE 802.11 b/g/n; 2.4 GHz; HT20; up to 72 Mbps
      - IEEE 802.11 b/g/n; 2.4 GHz; HT20/40; up to 150 Mbps
      - IEEE 802.11 b/g/n; 2.4 GHz; HT20/40; up to 150 Mbps
      - IEEE 802.11 b/g/n; 2.4 GHz; HT20/40; up to 150 Mbps
      - IEEE 802.11 b/g/n; 2.4 GHz; HT20/40; up to 150 Mbps
      - IEEE 802.11 b/g/n; 2.4 GHz; HT20; up to 150 Mbps
      - IEEE 802.11 b/g/n; 2.4 GHz; HT20/40; up to 150 Mbps
      - No Wi-Fi
   *  - Wi-Fi 6
      - 
      - 
      - 
      - 
      - 
      - 
      - IEEE 802.11 ax; 2.4 GHz; HT20; up to 150 Mbps
      - 
   *  - Zigbee / Thread (802.15.4)
      - N/A
      - N/A
      - N/A
      - N/A
      - N/A
      - N/A
      - Available
      - Available
   *  - Bluetooth
      - N/A
      - BR/EDR + Bluetooth LE v4.2
      - N/A
      - Bluetooth LE v5.0
      - Bluetooth LE v5.0
      - Bluetooth LE v5.0
      - Bluetooth LE v5.3
      - Bluetooth LE v5.0
   *  - SRAM (KB)
      - 160
      - 520
      - 320
      - 512
      - 400
      - 272
      - 512
      - 320
   *  - ROM (KB)
      - 
      - 448
      - 128
      - 384
      - 384
      - 576
      - 320
      - 128
   *  - RTC SRAM (KB)
      - 1
      - 16
      - 16
      - 16
      - 8
      - 0
      - 16
      - 4
   *  - ADC
      - 1*10-bit ADC, 1 channel
      - 2*12-bit ADC, 18 channels
      - 2*13-bit ADC, 20 channels
      - 2*12-bit ADC, 20 channels
      - 2*12-bit ADC, 6 channels
      - 1*12-bit ADC, 5 channels
      - 1*12-bit ADC, 7 channels
      - 1*12-bit ADC, 5 channels
   *  - DAC
      - 0
      - 2*8-bit DAC
      - 2*8-bit DAC
      - 0
      - 0
      - 0
      - 0
      - 0
   *  - Touch
      - 0
      - 10
      - 14
      - 14
      - 0
      - 0
      - 0
      - 0
   *  - Temp Sensor
      - 
      - 0
      - 1
      - 1
      - 1
      - 1
      - 1
      - 1
   *  - GPIO
      - 16
      - 26
      - 37
      - 36
      - 15
      - 14
      - 23
      - 19
   *  - Strapping GPIO
      - 0, 2, 15
      - 0, 2, 5, 12, 15
      - 0, 45, 46
      - 0, 3, 45, 46
      - 2, 8, 9
      - 8, 9
      - 4, 5, 8, 9, 15
      - 8, 9
   *  - GPIO for flash/PSRAM
      - 6, 7, 8, 9, 10, 11
      - 6, 7, 8, 9, 10, 11 (PSRAM or embedded flash: 16, 17)
      - 27, 28, 29, 30, 31, 32 (OPI: 33, 34, 35, 36, 37)
      - 27, 28, 29, 30, 31, 32 (OPI: 33, 34, 35, 36, 37)
      - 11, 12, 13, 14, 15, 16, 17
      - 11, 12, 13, 14, 15, 16, 17
      - 20, 21, 22, 24, 25, 26
      - 
   *  - UART
      - 1.5 (Serial1 out only)
      - 3
      - 2
      - 3
      - 2
      - 2
      - 3
      - 2
   *  - SPI
      - 2
      - 4
      - 4
      - 4
      - 3
      - 3
      - 1
      - 3
   *  - SDIO HOST
      - 
      - 1
      - 0
      - 2
      - 0
      - 0
      - 0
      - 0
   *  - SDIO SLAVE
      - 
      - 1
      - 0
      - 0
      - 0
      - 0
      - 1
      - 0
   *  - I2C
      - 1(soft)
      - 2
      - 2
      - 2
      - 1
      - 1
      - 2
      - 2
   *  - I2S
      - 1
      - 2
      - 1
      - 2
      - 1
      - 0
      - 1
      - 1
   *  - RMT
      - 0
      - 1*8 channels
      - 1*4 channels
      - 1*4 channels
      - 1*4 channels
      - 0
      - 1*4 channels
      - 1*2 channels
   *  - LED PWM
      - 0
      - 2*8 channels
      - 1*8 channels
      - 1*8 channels
      - 1*6 channels
      - 1*6 channels
      - 1*6 channels
      - 1*6 channels
   *  - MCPWM
      - 0
      - 2
      - 0
      - 2
      - 0
      - 0
      - 1
      - 1
   *  - USB OTG
      - 0
      - 0
      - 1
      - 1
      - 0
      - 0
      - 0
      - 0
   *  - USB Serial / JTAG
      - N/A
      - N/A
      - YES
      - YES
      - YES
      - N/A
      - YES
      - YES
   *  - Hall
      - 0
      - 1
      - 0
      - 0
      - 0
      - 0
      - 0
      - 0
   *  - Ethernet
      - 0
      - 1
      - 0
      - 0
      - 0
      - 0
      - 0
      - 0
   *  - TWAI (CAN)
      - 0
      - 1
      - 1
      - 1
      - 1
      - 0
      - 2
      - 1
   *  - JTAG
      - 
      - YES
      - YES
      - YES
      - YES
      - YES
      - YES
      - YES
   *  - Camera
      - N/A
      - 1*DVP 8/16-bit
      - 1*DVP 8/16-bit
      - 1*DVP 8/16-bit
      - N/A
      - N/A
      - N/A
      - N/A
   *  - TOF
      - N/A
      - N/A
      - YES
      - N/A
      - N/A
      - N/A
      - N/A
      - N/A
   *  - BT Certification
      - 
      - BT SIG
      - 
      - BT SIG
      - BT SIG
      - 
      - 
      - 
   *  - Wi-Fi Certification
      - WPA 2
      - Wi-Fi Alliance/WPA 3
      - Wi-Fi Alliance/WPA 3
      - 
      - 
      - WFA
      - 
      - 
   *  - Green Certification
      - 
      - RoHS/REACH
      - RoHS/REACH
      - RoHS/REACH
      - RoHS/REACH
      - RoHS/REACH
      - 
      - 
   *  - RF Certification
      - 
      - FCC / CE-RED / IC / TELEC / KCC / SRRC / NCC
      - FCC / CE-RED / SRRC / IC
      - SRRC / CE / FCC / IC / MIC / NCC / KCC
      - FCC / CE-RED / SRRC / IC
      - SRRC
      - 
      - 
   *  - Sleep Power Consumption 
      - 900 µA light sleep, 20 µA deep sleep
      - 800 µA light sleep, 10 µA deep sleep
      - 750 µA light sleep, 25 µA deep sleep
      - 240 µA light sleep, 8 µA deep sleep
      - 130 µA light sleep, 5 µA deep sleep
      - 140 µA light sleep, 5 µA deep sleep
      - 180 µA / 35 µA light sleep, 7 µA deep sleep
      - 


Sources:

* `Table content source <https://maker.pro/esp8266/tutorial/a-comparison-of-the-new-esp32-s2-to-the-esp32>`_
* `Espressif docs <https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32c2/hw-reference/chip-series-comparison.html>`_
* `Espressif Product Selector <https://products.espressif.com/#/product-comparison>`_

Datasheets
==========

* `DS:ESP8266 (ESP8285) <https://www.espressif.com/sites/default/files/documentation/0a-esp8266ex_datasheet_en.pdf>`_
* `DS:ESP32 <https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf>`_
* `DS:ESP32-S2 <https://www.espressif.com/sites/default/files/documentation/esp32-s2_datasheet_en.pdf>`_
* `DS:ESP32-S3 <https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf>`_
* `DS:ESP32-C3 (ESP8685) <https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf>`_
* `DS:ESP32-C2 (ESP8684) <https://www.espressif.com/sites/default/files/documentation/esp8684_datasheet_en.pdf>`_
* `DS:ESP32-C6 <https://www.espressif.com/sites/default/files/documentation/esp32-c6_datasheet_en.pdf>`_
* `DS:ESP32-H2 <https://cdn-shop.adafruit.com/product-files/5715/esp32-h2_datasheet_en.pdf>`_


Technical Reference Manuals
===========================

* `TR:ESP8266 <https://www.espressif.com/sites/default/files/documentation/esp8266-technical_reference_en.pdf>`_
* `TR:ESP32 <https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf>`_
* `TR:ESP32-S2 <https://www.espressif.com/sites/default/files/documentation/esp32-s2_technical_reference_manual_en.pdf>`_
* `TR:ESP32-S3 <https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf>`_
* `TR:ESP32-C3 <https://www.espressif.com/sites/default/files/documentation/esp32-c3_technical_reference_manual_en.pdf>`_
* `TR:ESP32-C2 <https://www.espressif.com/sites/default/files/documentation/esp8684_technical_reference_manual_en.pdf>`_
* `TR:ESP32-C6 <https://www.espressif.com/sites/default/files/documentation/esp32-c6_technical_reference_manual_en.pdf>`_
* `TR:ESP32-H2 <https://www.espressif.com/sites/default/files/documentation/esp32-h2_technical_reference_manual_en.pdf>`_


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

Added: 2023/05/09

The most powerful and versatile ESP32 variant currently available.

It outperforms the classic ESP32 in almost any way.

The only drawback is that it doesn't support a RMII ethernet interface.

.. note:: Support for the ESP32-S3 is very preliminary, as in it is hardly tested (as of May 2023)

PSRAM support on ESP32-S3 is a bit of a mess.

Some ESP32-S3 chips have embedded PSRAM.
When they do, you need to have the SPI bus for memory/flash set to QIO/OPI mode.
With QIO/QSPI mode, the PSRAM will not be detected.

However, if the chip does not have PSRAM and the SPI bus is set to QIO/OPI mode, the flash will lockup and no longer respond when accessing it.
So for now only the ``max_ESP32s3_16M8M_LittleFS_PSRAM`` build will be set to use QIO/OPI mode.


ESP32-C3
========

Added: 2023/05/09

The ESP32-C3 is available in various versions.

For example there is an ESP32-C3-12F module made by Espressif clearly aimed to be a 1-to-1 replacement of the ESP12-F, which uses the ESP8266.

Due to the used RISC-V core used in the ESP32-C3, this is a very 'snappy' device and the SDK support appears to be far more mature then what one might expect given its relative recent introduction.

.. note:: Support for the ESP32-C3 is very preliminary, as in it is hardly tested (as of May 2023)


ESP32-C2/ESP8684
================

The ESP32-C2 is only available with embedded flash and can only be found labelled as "ESP8684".

It looks like it is aimed to be used in single purpose devices, due to its low GPIO count and only requiring a bare minimum of external parts.


.. note:: Not yet supported (as of May 2023)

ESP32-C6
========

The ESP32-C6 seems to be aimed at being used as a gateway for the new Thread protocol and Wi-Fi.

It is the more powerful version of the ESP32-H2 and also includes not only the traditional 2.4 GHz Wi-Fi, but also the new Wi-Fi6 standard on 2.4 GHz.

.. note:: Not yet supported (as of May 2023)

ESP32-H2
========

This is a rather strange product as it does not support any Wi-Fi.
However it is the first device aimed at the new Thread standard.

Since it does not support any Wi-Fi, it is unsure if there will be ESPEasy support for it in the near future.

.. note:: Not yet supported (as of May 2023)
