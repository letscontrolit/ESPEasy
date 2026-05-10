Project Structure
*****************

The files and directories in the ESPEasy file repository are organized in folders.
Some of the choices for folder structure may not look very logical at first sight and may need some explanation to understand.


ESPEasy Project Directories
===========================

Below a list of the most important directories and files used in this project.

* ``.github/`` Buildscripts used by the Github Actions build and release CI jobs.
* ``.pio/`` Working dir for pioarduino (Ignored by Git)
* ``.platformio/`` Working dir for pioarduino (Ignored by Git)
* ``.platformio/penv/`` Directory to store the used Python Virtual Environment.
* ``boards/`` Used board definitions.
* ``boards/partitions/`` Used partition layout in ESP32 builds.
* ``build_output/`` The output directory where all built files are collected. (Ignored by Git)
* ``dist/`` Files also includd in the nightly builds (e.g. blank flash files and Espressif flash download tool)
* ``docs/`` Documentation tree using Sphinx to build the documents.
* ``hooks/`` Used for the continous integration process, used by Travis CI.
* ``include/`` PlatformIO's suggested folder for .h files (not yet used)
* ``lib/`` Libraries used in this project. Some have patches which make them different from the maintainers version.
* ``misc/`` Additional files needed for some use cases, like specialized a firmware for some boards or decoder files for the TTN project.
* ``patches/`` Patches we apply during build for core libraries.
* ``src/`` The source code of ESPeasy
* ``src/ESPEasy/`` Parts of ESPeasy use C++ namespaces, and are placed in this directory, matching the namespace structure.
* ``static/`` Static files we use in ESPEasy, like CSS/JS/ICO files.
* ``test/`` Test scripts to be used in the continous integration process.
* ``tools/`` Tools to help build ESPeasy.
* ``tools/pio/pre_custom_esp*.py`` Python helper scripts for building custom ESP binaries.
* ``platformio.ini``  Configuration file for pioarduino to define various build setups.
* ``requirements.txt``  List of used Python libraries and their version (result of ``pip freeze`` with Virtual env active)
* ``uncrustify.cfg``  Configuration file for Uncrustify, to format source code using some uniform formatting rules.


ESPEasy src dir
===============

In order to compile the project using PlatformIO, you need to make sure the ``platformio.ini`` file is in the 
root of the project dir opened in the editor.

The ``src/`` directory has the source files of the project.

In this directory, there is another ``src/`` dir, which is needed by ArduinoIDE in order to find files in sub-directories.
Arduino IDE also demands include directives to be relative compared to the file they are used in. 
For example: ``#include "../DataStructs/Settings.h"``

We are currently working on converting the core ESPeasy code from .ino files into .h/.cpp files. (see PR #2617 and issue #2621)

The header and source files are organized in a few directories in ``src/src/``.

* ``Commands`` Functions to process commands.
* ``ControllerQueue`` Mainly macro definitions for the queue system used by controllers.
* ``CustomBuild`` Header files for defining the ESPEasy project defaults and enabled plugins.
* ``DataStructs`` Data structures used in ESPEasy. Note that some of them are stored in settings files, so take care when changing them.
* ``DataTypes`` Definitions for data types used.
* ``ESPEasyCore`` Core functionality of ESPEasy, managing controllers, plugins, networking, rules, etc.
* ``Globals`` Declaration of global variables. Declare them using ``extern`` in .h files and construct them in .cpp files or else you may end up with several instances of the same object with the same name.
* ``Helpers`` Shared helper classes and functions.
* ``PluginStructs`` Data and code structs per plugin, to lower memory footprint split into separated source files.
* ``Static`` C++ encoded version of objects which have to be included in the binary. N.B. JS and CSS files are minified.
* ``WebServer`` Sources for web handling remote command processing and UI.

The Arduino based .ino files are still in the ``src/`` directory. (and some .h files, which are not yet moved)

In the end, there should be only .ino files left for the plugins and controllers and ``ESPEasy.ino`` 
All other .ino files used in the ESPEasy core should be converted to .h/.cpp.

This conversion is needed to have full control over ``#define`` statements used to determine what is included in a binary and what not.
This also allows for more flexible control over user defined builds.


ESPEasy 'nightly builds'
========================

On a regular basis we make so called "nightly builds".
These can be found here: `ESPEasy releases download <https://github.com/letscontrolit/ESPEasy/releases>`_ .

See the included ``README.txt`` in the downloaded ZIP file for more detailed information of the content of that build.

We have 2 main build platforms:

* ESP8266/ESP8285
* ESP32

For these platforms we also have a lot of different build setups.

The filename is quite descriptive:
  ESP_Easy_mega-<date>_<buildType>_<chip>_<memorySize>_<build-options>.bin

Build Type
----------

Build type can be:  (differences in included plugins)

* normal     => Only Stable plugins and controllers
* collection => Stable + Collection (split into multiple sets, A/B/C/D/E/F/G)
* max        => All available plugins and features

There is also a number of special builds:

* normal_IR => "Normal" + IR receiver/transmitter plugins and library
* hard_xxxxx => Special builds for some off-the-shelf hardware.
* minimal_ESP82xx_1M_OTA => Minimum number of plugins and a limited set of controllers included to be able to perform a 2-step OTA on 1 MB flash nodes.
* normal_core_xxx => "Normal" using core xxx (e.g. 2.7.4)
* normal_beta => "Normal" using the staged (beta) branch of the esp8266/Arduino repository.

ESP Chip Type
-------------

* ``ESP8266`` Deprecated for new projects, as support from Espressif has been stopped, even though new chips are still being produced.
* ``ESP8285`` Supported in ``ESP8266`` builds. Used in some Sonoff modules. This chip has embedded flash, so no extra flash chip.
* ``ESP32``   Allows for more memory and more GPIO pins.
* ``ESP32-S2`` Newer version of ESP32. Has even more GPIO pins, but some specific features of ESP32 were removed.
* ``ESP32-S3`` Newer version of ESP32 and ESP32-S2. Has even more GPIO pins, some specific features of ESP32 were removed, and some design choices of ESP32-S2 are reverted and implemented differently.
* ``ESP32-C2`` Cheaper variant of ESP32-C3, and also an ESP8266 replacement. Available as pin-compatible module for ESP8266. Single core, and max 120 MHz clock speed.
* ``ESP32-C3`` Intended as a replacement for ESP8266, using ESP32 technology, though single-core and with limited clock speed (160 MHz, some models 120 MHz).
* ``ESP32-C6`` Will allow connectivity with IEEE 802.15.4 (Thread/Zigbee) wireless protocol.
* ``ESP32-C61`` Preliminary supported. Will allow connectivity with IEEE 802.15.4 (Thread/Zigbee) wireless protocol.
* ``ESP32-C5`` Will allow connectivity with WiFi 2.4GHz and 5GHz support.
* ``ESP32-P4`` High-end dual-core MCU, with many GPIO pins and support for camera, display, RMII Ethernet and many other protocols. No on-chip WiFi support. Often has an additional ESP32-C6 with ESP-HostedMCU to provide wireless support via an SDIO connection.

Memory Size and Partitioning
----------------------------

* ``1M`` 1 MB flash modules (e.g. almost all Sonoff modules)
* ``2M`` 2 MB flash modules (e.g. Shelly1/WROOM02)
* ``4M`` 4 MB flash modules (e.g. NodeMCU/ESP32)
* ``16M`` 16 MB flash modules (e.g. Wemos D1 mini pro) (has 14 MB LittleFS filesystem, as SPIFFS is unstable > 2 MB)
* ``4M1M`` 4 MB flash modules with 1 MB filesystem (SPIFFS)
* ``4M2M`` 4 MB flash modules with 2 MB filesystem (SPIFFS)
* ``4M316k`` 4 MB flash modules using 1.8 MB sketch size, with 316 kB filesystem (LittleFS) (for ESP32)
* ``8M1M`` 8 MB flash modules using 3.5MB sketch size, with 1 MB filesystem (LittleFS) (ESP32 only)
* ``16M8M`` 16 MB flash modules using 4MB sketch size, with 8 MB filesystem (LittleFS) (ESP32 only)
* ``32M20M`` 32 MB flash modules using 6MB sketch size, with 20 MB filesystem (LittleFS) (ESP32 only)

Optional build options
----------------------

* ``LittleFS`` Use LittleFS instead of SPIFFS filesystem (SPIFFS is unstable \> 2 MB and no longer available from IDF 5.x)
* ``VCC`` Analog input configured to measure VCC voltage
* ``OTA`` Arduino OTA (Over The Air) update feature enabled
* ``Domoticz`` Only Domoticz controllers (HTTP+MQTT) and plugins included
* ``FHEM_HA`` Only FHEM/OpenHAB/Home Assistant (MQTT) controllers and plugins included
* ``lolin_d32_pro`` Specific Lolin hardware options enabled
* ``PSRAM`` Additional PSRAM support (ESP32 only)
* ``OPI`` Flash via OPI protocol support (ESP32 only)
* ``QIO`` Flash via QIO protocol support (ESP32 only)
* ``CDC`` CDC Serial (built-in USB) support (ESP32 only)
* ``ETH`` Ethernet interface enabled (ESP32 only, no longer used, as all ESP32, except for ESP32-C2, have support for Ethernet included)


Please note that the performance of 14MB SPIFFS (16M flash ESP8266 modules) is really slow.
All file access takes a lot longer and since the settings are also read from flash, the entire node will perform slower.
See `Arduino issue - SPIFFS file access slow on 16/14M flash config <https://github.com/esp8266/Arduino/issues/5932>`_

If these speed issues will be fixed, it is very likely the SPIFFS must then be re-partitioned, thus loosing all data in the SPIFFS.

Special memory partitioning:

* ``2M256``  2 MB flash modules (e.g. Shelly1/WROOM02) with 256k SPIFFS (only core 2.5.0 or newer)
* ``4M316k`` For ESP32 with 4MB flash, sketch size is set to 1.8 MByte (default: 1.4 MByte)
* ``4M1M``   4MB flash, 1 MB SPIFFS. Default layout for ESP8266 4MB flash.
* ``4M2M``   4MB flash, 2 MB SPIFFS. Introduced in October 2019. Only possible with core 2.5.2 or newer.

.. warning::
    Changing between builds with different flash layout will destroy the settings!

    The SPIFFS partition will be lost, which contains all settings.



Difference between .bin and .bin.gz
-----------------------------------

Starting on esp8266/Arduino core 2.7.0, it is possible to flash images that have been compressed using GZip.

Please note that this only can be used on installs already running a recent build.

This also means we still need to update the 2-step updater to support .bin.gz files.


.. warning::
    Currently there is NO 2-step OTA image available supporting ``.bin.gz`` images.


ESP32 builds
------------

There are several builds for ESP32, this is just a selection:

* ``normal_ESP32_4M316k``  Build using the "stable" set of plugins for ESP32, with support for an on-board Ethernet controller
* ``custom_ESP32_4M316k``  Build template using either the plugin set defined in ``Custom.h`` or ``tools/pio/pre_custom_esp32.py``
* ``collection_A_ESP32_4M316k``  Build using the "Collection" set "A" of plugins for ESP32
* ``collection_B_ESP32_4M316k``  Build using the "Collection" set "B" of plugins for ESP32
* ``collection_C_ESP32_4M316k``  Build using the "Collection" set "C" of plugins for ESP32
* ``collection_D_ESP32_4M316k``  Build using the "Collection" set "D" of plugins for ESP32
* ``max_ESP32_16M8M``  Build using all available plugins and controllers for ESP32 with 16 MB flash (like lolin_d32_pro boards)

Since ESP32 does have its flash partitioned in several blocks, we have 2 bin files of each ESP32 build, f.e.:

* ``collection_D_ESP32_4M316k.bin`` Use for OTA upgrades.
* ``collection_D_ESP32_4M316k.factory.bin`` Use on clean nodes as initial inistall.

The binary with ``.factory`` in the name must be flashed on a new node, via the serial interface of the board.
This flash must be started at address 0.

The binary without ``.factory`` can be used for OTA updates. (OTA for ESP32 is added in May 2020)


Blank Images
------------

To help recover from a bad flash, there are also blank images included.

* ``blank_1MB.bin``
* ``blank_2MB.bin``
* ``blank_4MB.bin``
* ``blank_8MB.bin``
* ``blank_16MB.bin``

When the wrong image is flashed, or the module behaves unstable, or is in a reboot loop,
flash these images first to clear out any remaining or hidden settings (Arduino framework...) and then the right image for the module.
