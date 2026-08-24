Supported ESP Chips
*******************

.. _esp8266-feature-complete:

ESP8266 is Feature Complete!
============================

(Since 2025-04-30)

The core team has decided to declare the ESP8266 builds (that also support ESP8285) Feature Complete, for multiple reasons:

* **Binary size**: Making (new or enhanced) plugins and growing feature set fit in the limited available binary space of an ESP8266 (1020kB) is getting harder and harder. Many hundreds of hours have been spent already to reduce size as much as possble, and there isn't much room for improvement anymore.
* **Limited RAM available**: The ESP8266 has only 80 kB of RAM available, and with the core of ESPEasy loaded, that leaves only between 15 and 25 kB of free memory for plugins to work with. This is a major cause for crashes when some complicated tasks are handling data of some size.
* **Chip development progress**: New features for ESP chips are being developed, but the ESP8266 platform, while still being manufactured, does not get technical improvements anymore.
* **Software support by Espressif**: New software development in the framework for supporting the ESP platform is only directed at the ESP32 line of MCUs. No new development for ESP8266 is being done.

The consequences of this decision are that:

* New plugins and features are **not** included in the regularly made available ESP8266 builds.
* To enable some of the new features (many are still useable) for ESP8266, a Custom build can be configured and built by the user, as documented in :ref:`PlatformIO_page`.
* A notable exception is support for TLS: That's *not* possible to enable on ESPEasy in ESP8266 builds as it doesn't fit in the limited available RAM memory.

For new projects, it is strongly advised to select one of the many available ESP32 boards, or chips when designing a custom board. Some of the ESP32 variants are available with 8MB, 16 MB, and 32MB, Flash (ESP32 Classic, ESP32-C3, ESP32-C5, ESP32-C6/C61, ESP32-S3 and ESP32-P4), and have 1 or more MAX builds available in ESPEasy, that include all plugins and features available, and also a larger selection of fonts for displays (TFT/LCD, OLed and 7-segment).

Overview of supported chips
===========================

ESPEasy does support a number of variants of the processors manufactured by Espressif.

* **ESP8266** The original ESP processor, with external flash.
* **ESP8285** Mainly found in "Chinese" products. Has flash built in the processor. (almost always 1MByte flash)
* **ESP32** The first successor of the ESP82xx, supporting 2 CPU cores, Bluetooth and more RAM and more GPIO pins. (and other new features)
* **ESP32-solo1** Same as ESP32, but with 1 core, running at 160 MHz. Used in some commercially sold devices like the early editions of the orange Shelly modules.
* **ESP32-S2** Has more GPIO pins than the ESP32, but only 1 CPU core. Initial support in ESPEasy added since 2021-09-19.
* **ESP32-S3** Support added: 2023-05-03
* **ESP32-S31** ❌ (No chip available yet)
* **ESP32-C2 / ESP8684** Support added: 2023-11-10
* **ESP32-C3 / ESP8685** Support added: 2023-05-03
* **ESP32-C5** Preliminary support added: 2026-01-09
* **ESP32-C6** Support added: 2023-11-10
* **ESP32-C61** Preliminary support added: 2026-01-09
* **ESP32-E22** ❌ (No chip available yet)
* **ESP32-H2** ❌
* **ESP32-H21** ❌
* **ESP32-H4** ❌
* **ESP32-P4** Support added: 2026-01-08
* **ESP32-P4rev3** Support added: 2026-04-20

.. note::
   ESP32-P4 rev. 1.x was renamed by Espressif to "ESP32-P4 ES" as in "Engineering Sample". The newer 'rev.3.x' silicon is now named "ESP32-P4", though some pages on the Espressif site refer to this as "ESP32-P4X".
   It is unclear yet what will be the common naming schema for these.  Both silicon revisions do require different build files.


.. note::
   ESP32-Hxx will likely never be supported in ESPEasy as these do not have WiFi or other networking options. Only Zigbee.

.. list-table:: Espressif platforms
   :header-rows: 1
   :widths: 7 7 7 7 7 7 7 7 7 7 7 7 10
   :stub-columns: 1   
   
   *  - 
      - ESP8266 (ESP8285)
      - ESP32 (ESP32solo1)
      - ESP32-S2
      - ESP32-S3
      - ESP32-S31
      - ESP32-C2 (ESP8684)
      - ESP32-C3 (ESP8685)
      - ESP32-C5
      - ESP32-C6
      - ESP32-C61
      - ESP32-H2
      - ESP32-P4 (1.x) / ESP32-P4X (3.x)
   *  - CPU
      - Xtensa® single-core 32-bit L106
      - Xtensa® dual-core 32-bit LX6 (solo1:single core)
      - Xtensa® single-core 32-bit LX7
      - Xtensa® dual-core 32-bit LX7
      - 32-bit RISC-V dual-core processor
      - 32-bit RISC-V single-core processor
      - 32-bit RISC-V single-core processor
      - 32-bit RISC-V single-core processor
      - 32-bit RISC-V single-core processor
      - 32-bit RISC-V single-core processor
      - 32-bit RISC-V single-core processor
      - 32-bit RISC-V dual-core processor, AI instr & FPU
   *  - Core
      - 1
      - 2 (solo1:1)
      - 1
      - 2
      - 2
      - 1
      - 1
      - 1
      - 1
      - 1
      - 1
      - 2 
   *  - Freq. (MHz)
      - 80
      - 240 (solo1:160)
      - 240
      - 240
      - 320
      - 120
      - 160
      - 240
      - 160
      - 120
      - 96
      - v1.x:360 / v3.x:400
   *  - Voltage (V)
      - 2.5 ~ 3.6
      - 3.0 ~ 3.6
      - 3.0 ~ 3.6
      - 3.0 ~ 3.6
      - 3.0 ~ 3.6
      - 3.0 ~ 3.6
      - 3.0 ~ 3.6
      - 3.0 ~ 3.6
      - 3.0 ~ 3.6
      - 3.0 ~ 3.6
      - 3.0 ~ 3.6
      - 3.0 ~ 3.6 
   *  - ESPEasy supported since
      - 2015/05
      - 2017/12 (solo1:2023/05)
      - 2021/09
      - 2023/05
      - ❌
      - 2023/11
      - 2023/05
      - 2026/01
      - 2023/11
      - 2026/01
      - ❌
      - 2026/01 (v3.x:2026/04)
   *  - Introduction
      - 2014
      - 2016
      - 2019
      - 2021
      - 2026
      - 2022
      - 2020
      - 2022
      - 2021
      - 2024
      - 2021
      - 2023 (v3.x:2026)
   *  - Status (2026/04)
      - End-Of-Life
      - Mass Production (solo1: NRND)
      - NRND
      - Mass Production
      - Announced
      - Mass Production
      - Mass Production
      - Mass Production
      - Mass Production
      - Mass Production
      - Mass Production
      - V1.x: NRND, V3.x: Sample
   *  - Wi-Fi
      - IEEE 802.11 b/g/n; 2.4 GHz; HT20; up to 72 Mbps
      - IEEE 802.11 b/g/n; 2.4 GHz; HT20/40; up to 150 Mbps
      - IEEE 802.11 b/g/n; 2.4 GHz; HT20/40; up to 150 Mbps
      - IEEE 802.11 b/g/n; 2.4 GHz; HT20/40; up to 150 Mbps
      - IEEE 802.11 b/g/n; 2.4 GHz; HT20/40; up to 150 Mbps
      - IEEE 802.11 b/g/n; 2.4 GHz; HT20; up to 150 Mbps
      - IEEE 802.11 b/g/n; 2.4 GHz; HT20/40; up to 150 Mbps
      - IEEE 802.11 b/g/n; 2.4/5 GHz; HT20/40; up to 150 Mbps
      - IEEE 802.11 b/g/n; 2.4 GHz; HT20/40; up to 150 Mbps
      - IEEE 802.11 b/g/n; 2.4 GHz; HT20/40; up to 150 Mbps
      - ❌
      - ❌ (via ESP-Hosted-MCU on secondary ESP32-C6)
   *  - Wi-Fi 6
      - ❌
      - ❌
      - ❌
      - ❌
      - IEEE 802.11 ax; 2.4 GHz; HT20; up to 150 Mbps
      - ❌
      - ❌
      - IEEE 802.11 ax; 2.4/5 GHz; HT20; up to 150 Mbps
      - IEEE 802.11 ax; 2.4 GHz; HT20; up to 150 Mbps
      - IEEE 802.11 ax; 2.4 GHz; HT20; up to 150 Mbps
      - ❌
      - ❌ (via ESP-Hosted-MCU on secondary ESP32-C6)
   *  - 5 GHz Wi-Fi
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ✔
      - ❌
      - ❌
      - ❌
      - ❌
   *  - Zigbee / Thread (802.15.4)
      - ❌
      - ❌
      - ❌
      - ❌
      - ✔
      - ❌
      - ❌
      - ✔
      - ✔
      - ❌
      - ✔
      - ❌ (via ESP-Hosted-MCU on secondary ESP32-C6)
   *  - Bluetooth
      - ❌
      - BR/EDR + Bluetooth LE v4.2
      - ❌
      - Bluetooth LE v5.0
      - Bluetooth LE+BR/EDR v5.4
      - Bluetooth LE v5.0
      - Bluetooth LE v5.0
      - Bluetooth LE v5.0
      - Bluetooth LE v5.3
      - Bluetooth LE v5.0
      - Bluetooth LE v5.0
      - ❌ (via ESP-Hosted-MCU on secondary ESP32-C6)
   *  - SRAM (KB)
      - 160
      - 520
      - 320
      - 512
      - 512
      - 272
      - 400
      - 384
      - 512
      - 320
      - 320
      - 768 
   *  - ROM (KB)
      - 
      - 448
      - 128
      - 384
      - 320
      - 576
      - 384
      - 320
      - 320
      - 256
      - 128
      - 128 
   *  - RTC SRAM (KB)
      - 1
      - 16
      - 16
      - 16
      - 32
      - 32
      - 8
      - 16
      - 16
      - 16
      - 4
      - 32 
   *  - ADC
      - 1*10-bit ADC, 1 channel
      - 2*12-bit ADC, 18 channels
      - 2*13-bit ADC, 20 channels
      - 2*12-bit ADC, 20 channels
      - 2*12-bit ADC, 16 channels
      - 1*12-bit ADC, 5 channels
      - 2*12-bit ADC, 6 channels
      - 1*12-bit ADC, 6 channels
      - 1*12-bit ADC, 7 channels
      - 1*12-bit ADC, 4 channels
      - 1*12-bit ADC, 5 channels
      - 2*12-bit ADC, 14 channels 
   *  - DAC
      - ❌
      - 2*8-bit DAC
      - 2*8-bit DAC
      - ❌
      - 2*10-bit DAC
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌ 
   *  - Touch
      - ❌
      - 10
      - 14
      - 14
      - 1
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - 14 
   *  - Temp Sensor
      - ❌
      - ❌
      - 1
      - 1
      - 1
      - 1
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
      - 60
      - 14
      - 15
      - 22
      - 30
      - 18
      - 19
      - 55 
   *  - Strapping GPIO
      - 0, 2, 15
      - 0, 2, 5, 12, 15
      - 0, 45, 46
      - 0, 3, 45, 46
      - 48,50,75,76
      - 8, 9
      - 2, 8, 9
      - 8, 9
      - 4, 5, 8, 9, 15
      - 4, 5, 8, 9, 15
      - 8, 9
      - 35, 36, 37, 38
   *  - GPIO for flash/PSRAM
      - 6, 7, 8, 9, 10, 11
      - 6, 7, 8, 9, 10, 11 (PSRAM or embedded flash: 16, 17)
      - 27, 28, 29, 30, 31, 32 (OPI: 33, 34, 35, 36, 37)
      - 27, 28, 29, 30, 31, 32 (OPI: 33, 34, 35, 36, 37)
      - ❌
      - 11, 12, 13, 14, 15, 16, 17
      - 11, 12, 13, 14, 15, 16, 17
      - 11, 12, 13, 14, 15, 16, 17
      - 20, 21, 22, 24, 25, 26
      - 20, 21, 22, 24, 25, 26
      - 
      - ❌
   *  - UART
      - 1.5 (Serial1 out only)
      - 3
      - 2
      - 3
      - 5
      - 2
      - 2
      - 3
      - 2
      - 2
      - 2
      - 5 
   *  - SPI
      - 2
      - 4
      - 4
      - 4
      - 7
      - 3
      - 3
      - 1
      - 1
      - 1
      - 3
      - 5 
   *  - SDIO HOST
      - ❌
      - 1
      - ❌
      - 2
      - 1
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - 1 
   *  - SDIO SLAVE
      - ❌
      - 1
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - 1
      - 1
      - ❌
      - 1 
   *  - I2C
      - 1(soft)
      - 2
      - 2
      - 2
      - 3
      - 1
      - 1
      - 2
      - 2
      - 1
      - 2
      - 3 
   *  - I3C
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - 1 
   *  - I2S
      - 1
      - 2
      - 1
      - 2
      - 2
      - ❌
      - 1
      - 1
      - 1
      - 1
      - 1
      - 3 
   *  - RMT
      - ❌
      - 1*8 channels
      - 1*4 channels
      - 1*4 channels
      - 
      - ❌
      - 1*4 channels
      - 1 channel
      - 1*4 channels
      - 
      - 1*2 channels
      - 1*4 channels 
   *  - LEDC PWM
      - ❌
      - 2*8 channels
      - 1*8 channels
      - 1*8 channels
      - 2*8 channels
      - 1*6 channels
      - 1*6 channels
      - 1*6 channels
      - 1*6 channels
      - 1*6 channels
      - 1*6 channels
      - 1*8 channels 
   *  - MCPWM
      - ❌
      - 2
      - ❌
      - 2
      - 4
      - ❌
      - ❌
      - ❌
      - 1
      - ❌
      - 1
      - 2 
   *  - USB OTG
      - ❌
      - ❌
      - 1
      - 1
      - 1
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - 2 
   *  - USB Serial / JTAG
      - ❌
      - ❌
      - ✔
      - ✔
      - ✔
      - ❌
      - ✔
      - ❌
      - ✔
      - ✔
      - ✔
      - ✔ 
   *  - Hall
      - ❌
      - 1 (removed in ESP-IDF5)
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌ 
   *  - Ethernet
      - ❌
      - 1 (RMII and SPI)
      - 1 (SPI)
      - 1 (SPI)
      - 1 (RMII Gbit and SPI)
      - 1 (SPI)
      - 1 (SPI)
      - 1 (SPI)
      - 1 (SPI)
      - 1 (SPI)
      - ❌
      - 1 (RMII and SPI) 
   *  - TWAI (CAN)
      - ❌
      - 1
      - 1
      - 1
      - 1
      - ❌
      - 1
      - 2
      - 2
      - 2
      - 1
      - 3 
   *  - JTAG
      - ❌
      - ✔
      - ✔
      - ✔
      - ✔
      - ✔
      - ✔
      - ✔
      - ✔
      - ✔
      - ✔
      - ✔ 
   *  - Camera
      - ❌
      - 1*DVP 8/16-bit
      - 1*DVP 8/16-bit
      - 1*DVP 8/16-bit
      - 1*DVP 8/16-bit
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - 1*DVP 8/16-bit 
   *  - TOF
      - ❌
      - ❌
      - ✔
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌
      - ❌ 
   *  - BT Certification
      - 
      - BT SIG
      - 
      - BT SIG
      - 
      - 
      - BT SIG
      - 
      - BQB
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
      - 
      - 
      - 
      -  
   *  - Green Certification
      - 
      - RoHS/REACH
      - RoHS/REACH
      - RoHS/REACH
      - 
      - RoHS/REACH
      - RoHS/REACH
      - 
      - RoHS/REACH
      - RoHS/REACH
      - 
      -  
   *  - RF Certification
      - 
      - FCC / CE-RED / IC / TELEC / KCC / SRRC / NCC
      - FCC / CE-RED / SRRC / IC
      - SRRC / CE / FCC / IC / MIC / NCC / KCC
      - 
      - SRRC
      - FCC / CE-RED / SRRC / IC
      - 
      - 
      - 
      - 
      -  
   *  - Sleep Power Consumption
      - 900 µA light sleep, 20 µA deep sleep
      - 800 µA light sleep, 10 µA deep sleep
      - 750 µA light sleep, 25 µA deep sleep
      - 240 µA light sleep, 8 µA deep sleep
      - 
      - 140 µA light sleep, 5 µA deep sleep
      - 130 µA light sleep, 5 µA deep sleep
      - 250 µA / 60 µA light sleep, 12 µA deep sleep
      - 180 µA / 35 µA light sleep, 7 µA deep sleep
      - 200 µA / 50 µA light sleep, 10 µA deep sleep
      - 85 µA / 25 µA light sleep, 7 µA deep sleep
      - 3.5 mA / 250 µA light sleep, 25 µA deep sleep

.. note:: Whether a specific MCU supports a feature, does not mean it can easily be used in ESPEasy or added as a new feature.  For this we also need support in either Arduino-ESP32 or ESP-IDF.  See `Arduino ESP32 "Supported Features and Peripherals" <https://docs.espressif.com/projects/arduino-esp32/en/latest/libraries.html#supported-features-and-peripherals>`_

Sources:

* `Table content source <https://maker.pro/esp8266/tutorial/a-comparison-of-the-new-esp32-s2-to-the-esp32>`_
* `Espressif docs <https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32c2/hw-reference/chip-series-comparison.html>`_
* `Espressif Product Selector <https://products.espressif.com/#/product-comparison>`_
* `Espressif ESP32-xx modules overview <https://www.espressif.com/en/products/modules>`_
* `Espressif Product overview SoCs <https://www.espressif.com/en/products/socs>`_
* `Espressif Arduino Libraries: Supported Features and Peripherals <https://docs.espressif.com/projects/arduino-esp32/en/latest/libraries.html>`_


Datasheets
==========

* `DS:ESP8266 <https://documentation.espressif.com/0a-esp8266ex_datasheet_en.pdf>`_
* `DS:ESP8285 <https://documentation.espressif.com/0a-esp8285_datasheet_en.pdf>`_
* `DS:ESP32 <https://documentation.espressif.com/esp32_datasheet_en.pdf>`_ 
* `DS:ESP32-S2 <https://documentation.espressif.com/esp32-s2_datasheet_en.pdf>`_
* `DS:ESP32-S3 <https://documentation.espressif.com/esp32-s3_datasheet_en.pdf>`_
* `DS:ESP32-S31 <https://documentation.espressif.com/esp32-s31_datasheet_en.pdf>`_
* `DS:ESP32-C2 <https://documentation.espressif.com/esp8684_datasheet_en.pdf>`_ 
* `DS:ESP32-C3 <https://documentation.espressif.com/esp32-c3_datasheet_en.pdf>`_ 
* `DS:ESP32-C5 <https://documentation.espressif.com/esp32-c5_datasheet_en.pdf>`_
* `DS:ESP32-C6 <https://documentation.espressif.com/esp32-c6_datasheet_en.pdf>`_
* `DS:ESP32-C61 <https://documentation.espressif.com/esp32-c61_datasheet_en.pdf>`_
* `DS:ESP32-H2 <https://documentation.espressif.com/esp32-h2_datasheet_en.pdf>`_
* `DS:ESP32-P4 <https://documentation.espressif.com/esp32-p4_datasheet_en.pdf>`_
* `DS:ESP32-P4 v1.3 <https://documentation.espressif.com/esp32-p4-chip-revision-v1.3_datasheet_en.pdf>`_


Technical Reference Manuals
===========================

* `TR:ESP8266 <https://documentation.espressif.com/esp8266-technical_reference_en.pdf>`_
* `TR:ESP32 <https://documentation.espressif.com/esp32_technical_reference_manual_en.pdf>`_
* `TR:ESP32-S2 <https://documentation.espressif.com/esp32-s2_technical_reference_manual_en.pdf>`_
* `TR:ESP32-S3 <https://documentation.espressif.com/esp32-s3_technical_reference_manual_en.pdf>`_
* `TR:ESP32-C2 <https://documentation.espressif.com/esp8684_technical_reference_manual_en.pdf>`_
* `TR:ESP32-C3 <https://documentation.espressif.com/esp32-c3_technical_reference_manual_en.pdf>`_
* `TR:ESP32-C5 <https://documentation.espressif.com/esp32-c5_technical_reference_manual_en.pdf>`_
* `TR:ESP32-C6 <https://documentation.espressif.com/esp32-c6_technical_reference_manual_en.pdf>`_
* `TR:ESP32-C61 <https://documentation.espressif.com/esp32-c61_technical_reference_manual_en.pdf>`_
* `TR:ESP32-H2 <https://documentation.espressif.com/esp32-h2_technical_reference_manual_en.pdf>`_
* `TR:ESP32-P4 <https://documentation.espressif.com/esp32-p4_technical_reference_manual_en.pdf>`_
* `TR:ESP32-P4 v1.3 <https://documentation.espressif.com/esp32-p4-chip-revision-v1.3_technical_reference_manual_en.pdf>`_


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




ESP32/ESP32-solo1
=================

The most important new features of the ESP32, compared to ESP82xx are:

- Dual core CPU @ 240 MHz (solo1: Single core CPU @ 160 MHz)
- Upto 320 kByte of RAM
- 3 Hardware serial ports
- Bluetooth (not supported yet in ESPEasy)
- Extra GPIO pins
- Upto 18 GPIO pins can operate as ADC.
- Upto 10 touch pins
- Support for Ethernet

.. note:: Use ESP32-solo1 build for unknown ESP32-based devices, or when flashing an "ESP32-classic" build results in a boot-loop.

The ESP32-solo1 is known to be used in:

* Shelly Plus 1
* Shelly Plus 1PM
* Shelly Plus 2PM
* Shelly Plus i4 (only early shipped units)
* Most Xiaomi devices seem to have an OEM ESP32-solo1

Support for the ESP32-solo1 is only added because some vendors have used it in their products. 
Since there is only a very limited set of devices using a single core ESP32, we only provide the basic versions of ESPEasy builds for this platform.

The "solo1" variant does not add any extras compared to the ESP32 dual core and thus should not be used in new products.


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
- Native USB (supported only for ESPEasy Serial console)
- LCD interface (not yet supported in ESPEasy)
- Camera interface (not yet supported in ESPEasy)
- Extra hardware accelerated encryption functions (not yet supported in ESPEasy)
- Supposedly lower power consumption (not yet verified)
- Time of Flight (TOF) support that would (theoretically) allow indoor positioning (not yet supported in ESPEasy)


ESP32-S3
========

Added: 2023/05/09

The most powerful and versatile ESP32 variant currently available.

It outperforms the classic ESP32 in almost any way.

The only drawback is that it doesn't support a RMII ethernet interface.

.. note:: Support for the ESP32-S3 is very preliminary, as in it is hardly tested (as of May 2023)


Quad/Octal SPI mode
^^^^^^^^^^^^^^^^^^^

SPI wiring to flash/PSRAM on ESP32-S3 is a bit of a mess.

Some ESP32-S3 chips have embedded PSRAM.
When they do, you may need to have the SPI bus for memory/flash set to QIO/OPI mode.

Flash and PSRAM can be wired using 4 (quad/QIO/QSPI mode) or 8 (octal/OPI mode) lines to the SPI bus.
However a device intended for octal mode cannot work in quad mode and vice verse.

* 2 MB PSRAM typically operates in quad mode.
* 8 MB PSRAM typically needs octal (OPI) mode.

8 MB PSRAM addressed in quad (QIO/QSPI) mode, will simply not be detected.

Using the wrong SPI mode to address flash is even worse as it isn't really clear which flash sizes may use quad and which use octal wired flash.
Also it is impossible to simply detect how it is wired at runtime and change these access modes when booting the device.

To support all modes, we simply need to make several versions

.. list-table:: ESP32-S3 variants
   :header-rows: 1
   :widths: 7 7 7 7 7
   :stub-columns: 1

   *  - Module
      - Chip
      - Flash (Mode)
      - SPI RAM (Mode)
      - Build memory_type
   *  - ESP32-S3-WROOM-1x-N4
      - ESP32-S3
      - 4 MB (Quad SPI)
      - -
      - ``qio_qspi``
   *  - ESP32-S3-WROOM-1x-N8
      - ESP32-S3
      - 8 MB (Quad SPI)
      - -
      - ``qio_qspi``
   *  - ESP32-S3-WROOM-1x-N16
      - ESP32-S3
      - 16 MB (Quad SPI)
      - -
      - ``qio_qspi``
   *  - ESP32-S3-WROOM-1x-H4
      - ESP32-S3
      - 4 MB (Quad SPI)
      - -
      - ``qio_qspi``
   *  - ESP32-S3-WROOM-1x-N4R2
      - ESP32-S3R2
      - 4 MB (Quad SPI)
      - 2 MB (Quad SPI)
      - ``qio_qspi``
   *  - ESP32-S3-WROOM-1x-N8R2
      - ESP32-S3R2
      - 8 MB (Quad SPI)
      - 2 MB (Quad SPI)
      - ``qio_qspi``
   *  - ESP32-S3-WROOM-1x-N16R2
      - ESP32-S3R2
      - 16 MB (Quad SPI)
      - 2 MB (Quad SPI)
      - ``qio_qspi``
   *  - ESP32-S3-WROOM-1x-N4R8
      - ESP32-S3R8
      - 4 MB (Quad SPI)
      - 8 MB (Octal SPI)
      - ``qio_opi``
   *  - ESP32-S3-WROOM-1x-N8R8
      - ESP32-S3R8
      - 8 MB (Quad SPI)
      - 8 MB (Octal SPI)
      - ``qio_opi``
   *  - ESP32-S3-WROOM-1x-N16R8
      - ESP32-S3R8
      - 16 MB (Quad SPI)
      - 8 MB (Octal SPI)
      - ``qio_opi``
   *  - ESP32-S3-WROOM-2-N16R8V
      - ESP32-S3R8V
      - 16 MB (Octal SPI)
      - 8 MB (Octal SPI)
      - ``opi_opi``
   *  - ESP32-S3-WROOM-2-N32R8V
      - ESP32-S3R8V
      - 32 MB (Octal SPI)
      - 8 MB (Octal SPI)
      - ``opi_opi``
   *  - ESP32-S3-MINI-1x-N8
      - ESP32-S3FN8
      - 8 MB (Quad SPI)
      - -
      - ``qio_qspi``
   *  - ESP32-S3-MINI-1x-N4R2
      - ESP32-S3FH4R2
      - 4 MB (Quad SPI)
      - 2 MB (Quad SPI)
      - ``qio_qspi``
   *  - ESP32-S3-MINI-1x-H4R2
      - ESP32-S3FH4R2
      - 4 MB (Quad SPI)
      - 2 MB (Quad SPI)
      - ``qio_qspi``

`Table Source <https://api.riot-os.org/group__cpu__esp32__esp32s3.html>`_


Build versions:

* All ESP32-S3 builds have PSRAM enabled.
* The default SPI mode will be quad mode for both flash and PSRAM
* ``max_ESP32s3_16M8M_LittleFS_OPI_PSRAM_CDC`` will have quad mode for flash and octal (OPI) mode for PSRAM. (typical 8MB PSRAM)

ESP32-S31
=========

There is not yet much known about the ESP32-S31.
Announcement `here <https://www.espressif.com/en/news/ESP32_S31_Release>`_



ESP32-C2/ESP8684
================

Added: 2023/11/10

The ESP32-C2 is only available with embedded flash and can also be found labeled as "ESP8684".

It looks like it is aimed to be used in single purpose devices, due to its low GPIO count and only requiring a bare minimum of external parts.

Espressif suggests this SoC as replacement for the ESP8266/ESP8285

It is yet unclear whether the ESP8684 and ESP32-C2 are exchangable, like with the ESP32-C3 and the ESP8685.

.. note:: No official support from Arduino (as of Nov 2023), preliminary support  in ESPEasy.


ESP32-C3/ESP8685
================

Added: 2023/05/09

The ESP32-C3 is available in various versions.

For example there is an ESP32-C3-12F module made by Espressif clearly aimed to be a 1-to-1 replacement of the ESP12-F, which uses the ESP8266.

The ESP8685 seems to be low budget alternative for the ESP32-C3 with only difference being the embedded flash inside the ESP chip and slightly smaller dimensions as the GPIO pins for flash are not made available outside the chip.

Due to the RISC-V core used in the ESP32-C3, this is a very 'snappy' device and the SDK support appears to be far more mature then what one might expect given its relative recent introduction.


ESP32-C5
========

Added: 2026/01/26

This will be the first Espressif SoC supporting 5 GHz WiFi.


ESP32-C6
========

Added: 2023/11/10

The ESP32-C6 seems to be aimed at being used as a gateway for the new Thread protocol and Wi-Fi.

It is the more powerful version of the ESP32-H2 and also includes not only the traditional 2.4 GHz Wi-Fi, but also the new Wi-Fi6 standard on 2.4 GHz and IEEE 802.15.4 (Zigbee/Thread). Zigbee/Thread not yet supported by ESPEasy (March 2024).


ESP32-C61
=========

Added: 2026/01/26

This is a stripped-down version of the ESP32-C6, but with PSRAM versions available.



ESP32-H2
========

This is a rather strange product as it does not support any Wi-Fi.
However it is the first device aimed at the new Thread standard.

Since it does not support any Wi-Fi, it is unsure if there will be ESPEasy support for it in the near future.

.. note:: Not yet supported (as of May 2023)


ESP32-P4
========

The first Espressif SoC without any RF support. (thus NO support for Wi-Fi / Bluetooth / etc.)

This processor seems to be aimed at digital signage and/or AI use cases as it has extensive support for displays (1.5 Gbps link speed) and camera with quite a lot of video processing capabilities.

The CPU is rather powerful and there are versions with quite a large amount of PSRAM present and large flash size.

It does have a RMII interface for Ethernet, like the ESP32-classic does.

.. note:: Not yet supported. Beta silicon is available (as of June 2025), which may change in Q3 or Q4 of 2025. So not yet adviced to be used in real products

ESP32-P4 rev3.x
===============

New revision of the ESP32-P4, which is incompatible with the initial P4.

.. note:: Not yet supported (as of March 2026)
