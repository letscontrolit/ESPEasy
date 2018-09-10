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
- hard_xxxxx => Special builds for some off-the-shelff hardware.

Chip can be:
- ESP8266      => Most likely option
- ESP8285      => Used in some Sonoff modules
- ESP32        => Experimental support at this moment
- ESP8266PUYA  => For ESP modules with a flash chip labeled "PUYA" (often 1 MB)

The "PUYA" version deserves a bit more attention.
If you cannot save settings, better try to see if the flash chip has "PUYA" written on it.
If so, try the "PUYA" build.

MemorySize can be:
- 1024  => 1 MB flash modules (e.g. almost all Sonoff modules)
- 2048  => 2 MB flash modules (e.g. Shelly1)
- 4096  => 4 MB flash modules (e.g. NodeMCU)


To help recover from a bad flash, there are also blank images included.
- blank_1MB.bin
- blank_2MB.bin
- blank_4MB.bin

When the wrong image is flashed, or the module behaves unstable, or is in a reboot loop,
flash these images first and then the right image for the module.

Another great flash tool can be found here: (Windows only)
  https://github.com/Grovkillen/ESP_Easy_Flasher

For more information, see: https://github.com/letscontrolit/ESPEasy
Or our forum: https://www.letscontrolit.com/forum/
