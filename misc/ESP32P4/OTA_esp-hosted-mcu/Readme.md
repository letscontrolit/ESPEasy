# ESP-Hosted SDIO WiFi

The ESP32-p4 does not have a WiFi radio built in.
Therefore each ESP32-p4 board is equiped with a separate ESP32-C6 module to add WiFi-support.

Both ESP32-p4 and ESP32-c6 are connected via a fast SDIO bus.

The ESP32-c6 does run a firmware on its own.
This is the esp-hosted-mcu firmware.
For more information see [esp-hosted-mcu GitHub docs](https://github.com/espressif/esp-hosted-mcu/blob/main/docs/sdio.md)


The factory flashed firmware of this ESP32-c6 is quite old and typically is running version 0.0.6

N.B. Whenever the ESP32-C6 module is not yet flashed, or an update failed, the C6 can be flashed again with build 0.0.6 using [this site](https://espressif.github.io/arduino-esp32/c6-hosted/)
For this, a separate USB to serial adapter must be wired to the RX/TX pins of the C6, which are accessible via some pin header. (at least on all P4 boards currently available)

ESP-IDF 5.5 is compiled to link against firmware 2.0.12

As stated in the [esp-hosted-mcu troubleshooting docs](https://github.com/espressif/esp-hosted-mcu/blob/main/docs/troubleshooting.md#3-make-sure-hosted-code-is-in-sync-for-master-and-slave), both the 'hosted code' (running on the ESP32-p4) and the 'slave code' (running on the ESP32-c6) should be kept in sync.



# OTA update ESP-Hosted-MCU firmware

In this folder, there is a freshly build firmware for the ESP32-C6, based on version 2.0.13

This file must be made available via some HTTP (not HTTPS) web server, which can be accessed by the ESP32-P4 via WiFi.

For example upload the file to another ESPEasy node with at least 1.2 MByte free space on its file system. (e.g. some ESP32-S3 with 16M flash)

Then this command must be given (adapt the URL for your own situation)

```
wifiotahostedmcu,"http://192.168.11.1/network_adapter.bin"
```

Make sure the complete URL is given, including `http://` and ending with `network_adapter.bin`


Keep an eye on the logs via the serial port of the P4 to see its progres.

After about 30 - 60 seconds (depending also on the connection speed of the ESP board serving the file) the ESP32-P4 will reboot and start using the newly flashed WiFi firmware.