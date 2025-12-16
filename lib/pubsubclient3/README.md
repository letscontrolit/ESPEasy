# Arduino Client for MQTT

This library provides a client for doing simple publish/subscribe messaging with
a server that supports MQTT.

## Notes of this fork

This is a fork of the repository [knolleary/pubsubclient v2.8](https://github.com/knolleary/pubsubclient/releases/tag/v2.8), which was last updated in May 20, 2020. There was an update approach in [#1045](https://github.com/knolleary/pubsubclient/issues/1045), but it's also stale.

I tried lot's of different other MQTT libs, but they need more resources than PubSubClient or lacking maintenance as well:

 - https://github.com/256dpi/arduino-mqtt
 - https://github.com/hideakitai/MQTTPubSubClient
 - https://github.com/bertmelis/espMqttClient
 - https://github.com/arduino-libraries/ArduinoMqttClient
 - https://github.com/thingsboard/pubsubclient

Since there was no progress I decided to merge the most important PRs manually and publish a new major version. I also renamed to PubSubClient3 to have a similar but different name of the library.

I appreciate every contribution to this library.

## Examples

The library comes with a number of example sketches. See File > Examples > PubSubClient
within the Arduino application.

Full API documentation is available here: https://pubsubclient.knolleary.net

## Limitations

 - The client is based on the [MQTT Version 3.1.1 specification](https://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html) with some limitations.
 - It can only publish QoS 0 messages. It can subscribe at QoS 0 or QoS 1.
 - The maximum message size, including header, is **256 bytes** by default. This
   is configurable via `MQTT_MAX_PACKET_SIZE` in `PubSubClient.h` or can be changed
   by calling `PubSubClient::setBufferSize(size)`.
 - The keepalive interval is set to 15 seconds by default. This is configurable
   via `MQTT_KEEPALIVE` in `PubSubClient.h` or can be changed by calling
   `PubSubClient::setKeepAlive(keepAlive)`.
 - The client uses MQTT 3.1.1 by default. It can be changed to use MQTT 3.1 by
   changing value of `MQTT_VERSION` in `PubSubClient.h`.


## Compatible Hardware

The library uses the Arduino Ethernet Client api for interacting with the
underlying network hardware. This means it Just Works with a growing number of
boards and shields, including:

 - Arduino Ethernet
 - Arduino Ethernet Shield
 - Arduino YUN – use the included `YunClient` in place of `EthernetClient`, and
   be sure to do a `Bridge.begin()` first
 - Arduino WiFi Shield - if you want to send packets > 90 bytes with this shield,
   enable the `MQTT_MAX_TRANSFER_SIZE` define in `PubSubClient.h`.
 - Sparkfun WiFly Shield – [library](https://github.com/dpslwk/WiFly)
 - TI CC3000 WiFi - [library](https://github.com/sparkfun/SFE_CC3000_Library)
 - Intel Galileo/Edison
 - ESP8266
 - ESP32

The library cannot currently be used with hardware based on the ENC28J60 chip –
such as the Nanode or the Nuelectronics Ethernet Shield. For those, there is an
[alternative library](https://github.com/njh/NanodeMQTT) available.

## License

This code is released under the MIT License.
