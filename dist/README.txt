  _____ ____  ____
 | ____/ ___||  _ \ ___  __ _ ___ _   _
 |  _| \___ \| |_) / _ \/ _` / __| | | |
 | |___ ___) |  __/  __/ (_| \__ \ |_| |
 |_____|____/|_|   \___|\__,_|___/\__, |
                                  |___/


This is the distribution folder for ESPeasy.
In here you will find a flash tool to program the ESP module.

Also a number of binary images is included.

The filename is quite descriptive:
  ESP_Easy_mega-<date>_<buildType>_<chip>_<memorySize>.bin

Build type can be:  (differ in included plugins)
- normal  => Only Stable plugins and controllers
- test    => Stable + Testing
- dev     => Stable + Testing + Development

There is also a number of special builds:
- normal_IR => "Normal" + IR receiver/transmitter plugins and library
- hard_xxxxx => Special builds for some off-the-shelf hardware.
- minimal_ESP82xx_1M_OTA => Minimum number of plugins and a limited set of controllers included to be able to perform a 2-step OTA on 1 MB flash nodes.
- normal_core_241 => "Normal" using core 2.4.1, since 2.4.2 has issues with PWM
- xxx_core_260_sdk2_alpha -> core 2.6.0 alpha version (under development) using SDK 2.2.1
- xxx_core_260_sdk222_alpha -> core 2.6.0 alpha version (under development) using SDK 2.2.2
- xxx_core_260_sdk3_alpha -> core 2.6.0 alpha version (under development) using SDK 3.0.0-dev (under development too)


Chip can be:
- ESP8266      => Most likely option
- ESP8285      => Used in some Sonoff modules
- ESP32        => Experimental support at this moment

MemorySize can be:
- 1M  => 1 MB flash modules (e.g. almost all Sonoff modules)
- 2M  => 2 MB flash modules (e.g. Shelly1/WROOM02)
- 4M  => 4 MB flash modules (e.g. NodeMCU/ESP32)
- 16M => 16 MB flash modules (e.g. Wemos D1 mini pro)

Please note that the performance of 14MB SPIFFS (16M flash modules) is really slow.
All file access takes a lot longer and since the settings are also read from flash, the entire node will perform slower.
See [Arduino issue - SPIFFS file access slow on 16/14M flash config](https://github.com/esp8266/Arduino/issues/5932)

If these speed issues will be fixed, it is very likely the SPIFFS must then be re-partitioned, thus loosing all data in the SPIFFS.


Special memory partitioning:
- 2M256  => 2 MB flash modules (e.g. Shelly1/WROOM02) with 256k SPIFFS (only core 2.5.0 or newer)
- 1M8_partition => For ESP32 with 4MB flash, sketch size is set to 1.8 MByte (default: 1.4 MByte)


ESP32 now has 3 builds:
- esp32dev   Using the default partition layout (1.4 MB for the sketch)
- esp32test_1M8_partition   Larger sketch partition (1.8MB) smaller SPIFFS (316 kB)
- esp-wrover-kit_test_1M8_partition  A build for ESP32 including build flags for the official WRover test kit.

Please note that changing between those versions will destroy the settings!
The SPIFFS partition will be lost and that contains all settings.


To help recover from a bad flash, there are also blank images included.
- blank_1MB.bin
- blank_2MB.bin
- blank_4MB.bin
- blank_16MB.bin

When the wrong image is flashed, or the module behaves unstable, or is in a reboot loop,
flash these images first and then the right image for the module.

ESP.Easy.Flasher.exe...
... is the new flashing tool for ESP Easy. You need to run it in elevated mode (as admin)
for it to fetch the COM ports correctly. If you want you may save YOUR settings using the
"Save as default settings" button in the lower left corner. If the window is too big or
too small you can experiment with the "Pixels Per Inch" variable in the
..\Settings\Default.ini file. Setting it to =96 is working for most users but the
application will try to find the most optimal value by default. Sometimes it fails to
do that. More information about the tool is found here:
https://github.com/Grovkillen/ESP_Easy_Flasher
You can also have custom serial commands entered in a txt file. One command per line.

Further reading:
For more information, see: https://github.com/letscontrolit/ESPEasy
Or our forum: https://www.letscontrolit.com/forum/
