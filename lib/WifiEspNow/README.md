# ESP-NOW Arduino library for ESP8266 and ESP32

[![GitHub build status](https://img.shields.io/github/workflow/status/yoursunny/WifiEspNow/build)](https://github.com/yoursunny/WifiEspNow/actions) [![GitHub code size](https://img.shields.io/github/languages/code-size/yoursunny/WifiEspNow?style=flat)](https://github.com/yoursunny/WifiEspNow)

**WifiEspNow** is an Arduino library for ESP-NOW, a connectionless WiFi communication protocol defined by Espressif.
Refer to [ESP-NOW reference](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/network/esp_now.html) for more information about how ESP-NOW works and its limitations.

* [Doxygen documentation](https://wifiespnow.yoursunny.dev/)

## Features

[`WifiEspNow`](src/WifiEspNow.h) is a simple wrapper of ESP-NOW functions in ESP-IDF.
On ESP8266, it supports unicast only.
On ESP32, it supports both unicast and multicast.

[`WifiEspNowBroadcast`](src/WifiEspNowBroadcast.h) implements *pseudo* broadcast over ESP-NOW.
Each device advertises a specific WiFi SSID, and discovers each other through BSSID scanning.
Then, messages are transmitted separately toward every peer via ESP-NOW unicast.
This is my custom protocol, which differs from `WifiEspNow` multicast.

## Installation

1. Clone this repository under `$HOME/Arduino/libraries` directory.
2. Add `#include <WifiEspNow.h>` or `#include <WifiEspNowBroadcast.h>` to your sketch.
3. Check out the [examples](examples/) for how to use.
