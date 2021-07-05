|Latest Nightly  | Build Status | Downloads | Docs | Patreon | Ko-Fi | PayPal |
|-------|-------|-------|-------|-------|-------|-------|
| [![GitHub version](https://img.shields.io/github/release/letscontrolit/ESPEasy/all.svg)](https://github.com/letscontrolit/ESPEasy/releases/latest) | ![Build status](https://github.com/letscontrolit/ESPEasy/actions/workflows/build.yml/badge.svg) | [![Downloads](https://img.shields.io/github/downloads/letscontrolit/ESPEasy/total.svg)](https://github.com/letscontrolit/ESPEasy/releases) | [![Documentation Status](https://readthedocs.org/projects/espeasy/badge/?version=latest)](https://espeasy.readthedocs.io/en/latest/?badge=latest) | [![donate](https://img.shields.io/badge/donate-Patreon-blue.svg)](https://www.patreon.com/GrovkillenTDer) | [![donate](https://img.shields.io/badge/donate-KoFi-blue.svg)](https://ko-fi.com/grovkillentder) | [![donate](https://img.shields.io/badge/donate-PayPal-blue.svg)](https://www.paypal.me/espeasy) |

For ways to *support* us, see [this announcement on the forum](https://www.letscontrolit.com/forum/viewtopic.php?f=14&t=5787), or have a look at the [Patreon](https://www.patreon.com/GrovkillenTDer), [Ko-Fi](https://ko-fi.com/grovkillentder) or [PayPal](https://www.paypal.me/espeasy) links above.

# ESPEasy (development branch)


Introduction and wiki: https://www.letscontrolit.com/wiki/index.php/ESPEasy#Introduction

**MEGA**
:warning:This is the development branch of ESPEasy. All new untested features go into this branch. If you want to do a bugfix, do it on the stable branch, we will merge the fix to the development branch as well.:warning:


Check here to learn how to use this branch and help us improving ESPEasy: http://www.letscontrolit.com/wiki/index.php/ESPEasy#Source_code_development

## Web based flasher (experimental)

To make it easier to get started, one may flash a build directly to the ESP from your browser.
Currently only Chrome and Edge are supported.

See [this flash page](https://td-er.nl/ESPEasy/) to try the new web flash feature.

The web flasher is using [ESP Web Tools](https://esphome.github.io/esp-web-tools/) made by the people behind ESPHome and Home Assistant.


## Binary releases

On demand, controlled by the repo owner, our build-bot will build a new binary release: https://github.com/letscontrolit/ESPEasy/releases

The releases are named something like 'mega-20210501' (last number is the build date)

Depending on your needs, we release different types of files:

The name is built up from a few key parts:

ESPEasy_mega\__[releasedate]_\__[build-type]_\__[opt-arduino-library]_\__[hardware-type]_\__[flash-size]__[filesystem-size]_\__[opt-build-features]_.bin

_[build-type]_ can be any of:
Build type  | Description                               | included plugins               |
------------|-------------------------------------------|--------------------------------|
custom      | Custom predefined set/Defined in Custom.h | Specific                       |
normal      | Standard plugins                          | Stable                         |
test_A      | Normal + test set A                       | Stable + Testing base + set A  |
test_B      | Normal + test set B                       | Stable + Testing base + set B  |
test_C      | Normal + test set C                       | Stable + Testing base + set C  |
test_D      | Normal + test set D                       | Stable + Testing base + set D  |
max         | All available plugins                     | All available                  |
energy      | All plugins related to energy measurement | Stable + Energy measurement    |
display     | All plugins related to displays           | Stable + Displays              |
hard        | hardware specific builds                  | Minimal                        |
minimal     | minimal plugins for specific use-cases    | Switch and Controller          |
spec_*      | specialized technical builds              | Not intended for regular use   |
IRext       | Infra-red hardware specific               | Sending and receiving IR cmd   |


_[opt-arduino-library]_ (optional) can be any of:
Arduino library | Description                        |
----------------|------------------------------------|
alt_wifi        | Alternative WiFi configuration     |
beta            | Arduino Beta release               |
sdk3            | Arduino SDK v.3                    |
core_274        | Arduino Core 2.7.4 release         |
core_274_sdk3   | Arduino Core 2.7.4 SDK v.3 release |


_[hardware-type]_ cn be any of:
Hardware type    | Description                                 |
-----------------|---------------------------------------------|
ESP8266          | Espressif ESP8266 generic boards            |
ESP8285          | Espressif ESP8285 generic boards 1MB Flash  |
WROOM02          | Espressif ESP8266 WRoom02 boards            |
ESP32            | Espressif ESP32 generic boards              |
ESP32-wrover-kit | Espressif ESP32 wrover-kit boards           |
SONOFF           | Sonoff hardware specific                    |
other_POW        | Switch with power measurement               |
Shelly_1         | Shelly 1 switch                             |
Shelly_PLUG_S    | Shelly plug S switch with power measurement |
Ventus           | Ventus W266 weather station                 |
LCtech           | LC-tech serial switch                       |


_[flash-size]_ can be any of:
Flash size | Description                 |
-----------|-----------------------------|
1M         | 1 MB with 128 kB filesystem |
2M         | 2 MB with 128 kB filesystem |
2M256      | 2 MB with 256 kB filesystem |
4M1M       | 4 MB with 1 MB filesystem   |
4M2M       | 4 MB with 2 MB filesystem   |
16M        | 16 MB with 14 MB filesystem |
4M316k     | 4 MB with 316 kB filesystem |
16M1M      | 16 MB with 1 MB filesystem  |
16M2M      | 16 MB with 2 MB filesystem  |
16M8M      | 16 MB with 8 MB filesystem  |


_[opt-build-features]_ can be any of:
Build features  | Description                                                              |
----------------|--------------------------------------------------------------------------|
LittleFS        | Use LittleFS instead of SPIFFS filesystem (SPIFFS is unstable \> 2 MB)    |
VCC             | Analog input configured to measure VCC voltage                           |
OTA             | Arduino OTA (Over The Air) update feature enabled                        |
Domoticz        | Only Domoticz controllers (HTTP+MQTT) and plugins included               |
FHEM_HA         | Only FHEM/OpenHAB/Home Assistant (MQTT) controllers and plugins included |
lolin_d32_pro   | Specific Lolin hardware options enabled                                  |
ETH             | Ethernet interface enabled (ESP32 only)                                  |

Some example firmware names:
Firmware name                                      | Hardware                | Included plugins              |
---------------------------------------------------|-------------------------|-------------------------------|
ESPEasy_mega-20210501_normal_ESP8266_1M.bin        | ESP8266 with 1MB flash  | Stable                        |
ESPEasy_mega-20210501_normal_ESP8285_1M.bin        | ESP8285 with 1MB flash  | Stable                        |
ESPEasy_mega-20210501_normal_ESP8266_4M1M.bin      | ESP8266 with 4MB flash  | Stable                        |
ESPEasy_mega-20210501_test_A_ESP8266_4M1M.bin      | ESP8266 with 4MB flash  | Stable + Testing base + set A |
ESPEasy_mega-20210501_normal_ESP32_4M316k.bin      | ESP32 with 4MB flash    | Stable                        |
ESPEasy_mega-20210501_test_A_ESP32_4M316k.bin      | ESP32 with 4MB flash    | Stable + Testing base + set A |
ESPEasy_mega-20210501_test_B_ESP32_4M316k.bin      | ESP32 with 4MB flash    | Stable + Testing base + set B |
ESPEasy_mega-20210501_max_ESP32_16M1M.bin          | ESP32 with 16MB flash   | All available plugins         |
ESPEasy_mega-20210501_max_ESP32_16M8M_LittleFS.bin | ESP32 with 16MB flash   | All available plugins         |

To see what plugins are included in which testing set, you can find that on the [ESPEasy Plugin overview page](https://espeasy.readthedocs.io/en/latest/Plugin/_Plugin.html)

## Documentation & more info

Our new, in-depth documentation can be found at [ESPEasy.readthedocs.io](https://espeasy.readthedocs.io/en/latest/). Automatically built, so always up-to-date according to the contributed contents. The old Wiki documention can be found at [letscontrolit.com/wiki](https://www.letscontrolit.com/wiki/index.php?title=ESPEasy).

Additional details and discussion are on the "Experimental" section of the forum: https://www.letscontrolit.com/forum/viewforum.php?f=18

[![Gitpod Ready-to-Code](https://img.shields.io/badge/Gitpod-Ready--to--Code-blue?logo=gitpod)](https://gitpod.io/#https://github.com/letscontrolit/ESPEasy) 


## Icons used

Icons on courtesy of [ICONS8](https://icons8.com/).
