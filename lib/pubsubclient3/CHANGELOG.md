# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to
[Semantic Versioning](https://semver.org/).

## Unreleased

### Changed

* Improved test coverage for all QoS levels (0, 1, 2)
* Increased robustness and correctness in client-side handling of QoS 1 and QoS 2 messages (proper handling of PUBACK, PUBREC, PUBREL, PUBCOMP sequences)
* Merged and cleaned up tests to ensure MQTT protocol compliance for all QoS scenarios

## [3.3.0] - 2025-12-14

### Added

* Speedup publish large messages by using a buffer by @TD-er in [#59](https://github.com/hmueller01/pubsubclient3/pull/59)
* Support `PROGMEM` and `__FlashStringHelper` topics by @hmueller01 in [#72](https://github.com/hmueller01/pubsubclient3/pull/72)
* Added issue template by @hmueller01 in [#76](https://github.com/hmueller01/pubsubclient3/pull/76)
* Report delta sizes in pull requests by @hmueller01 in [#77](https://github.com/hmueller01/pubsubclient3/pull/77)

### Changed

* Refactored private members by @hmueller01 in [#68](https://github.com/hmueller01/pubsubclient3/pull/68)
* Refactored private functions by @hmueller01 in [#70](https://github.com/hmueller01/pubsubclient3/pull/70)
* Do not crash if `_client` not set by @hmueller01 in [#69](https://github.com/hmueller01/pubsubclient3/pull/69)
* Minor updates that come across by @hmueller01 in [#73](https://github.com/hmueller01/pubsubclient3/pull/73)
* Inline optimisation by @hmueller01 in [#75](https://github.com/hmueller01/pubsubclient3/pull/75)
* Bump actions/upload-pages-artifact from 3 to 4 by @dependabot[bot] in [#78](https://github.com/hmueller01/pubsubclient3/pull/78)
* Bump actions/checkout from 2 to 5 by @dependabot[bot] in [#79](https://github.com/hmueller01/pubsubclient3/pull/79)
* Tag unused parameter to avoid warnings by @hmueller01 in [#82](https://github.com/hmueller01/pubsubclient3/pull/82)
* Fix compile warning at strnlen bound by @hmueller01 in [#83](https://github.com/hmueller01/pubsubclient3/pull/83)
* Fix deprecated `BUILTIN_LED` by @hmueller01 in [#84](https://github.com/hmueller01/pubsubclient3/pull/84)
* Bump actions/checkout from 5 to 6 by @dependabot[bot] in [#85](https://github.com/hmueller01/pubsubclient3/pull/85)
* Update workflows by @hmueller01 in [#86](https://github.com/hmueller01/pubsubclient3/pull/86)

## [3.2.1] - 2025-09-11

### Fixed

- Fixed bug introduced with QoS &gt; 0 support (#65)

## [3.2.0] - 2025-07-20

### Added

- Support publish at QoS 1 and 2 (by @hmueller01) (#49)
- Doxygen API documentation on gh-pages (by @hmueller01) (#51)

### Changed

- Add check for nullptr in `CHECK_STRING_LENGTH` (by @TD-er) (#48)
- Move `CHECK_STRING_LENGTH` to `PubSubClient.cpp` (by @hmueller01) (#50)
- Get rid of lazy calling parameter (by @hmueller01) (#53)
- Implemented `writeNextMsgId()` (by @hmueller01) (#54)
- Cleanup `writeString()` (by @hmueller01) (#55)

## [3.1.0] - 2025-03-20

### Added

- Added `ERROR_PSC_PRINTF` and `ERROR_PSC_PRINTF_P` outputs (#25)

### Changed

- Fix `setServer(domain, ...)` by copying domain string to an internal buffer (prevent potential dangling pointer) (#10)
- Connect to broker only if port != 0 (protect against incorrect initialization) (#10)
- Refactored buffer length types to `size_t` (#11)
- Added warnings and `-Werror` to tests Makefile (#11)
- Reformatted tests code using Google style (see `.clang-format`) (#11)
- Fix keepalive handling (thanks to @uschnindler) (#14)
- Use initializer lists for constructors (#15)
- Fix `DEBUG_PSC_PRINTF` outputs (#18)
- Introduce `MQTTRETAINED` flag (#21)
- Added documentation on `readByte()` and `readPacket()` (#23)
- Refactored `buildHeader()`, `readPacket()`, `readByte()`, `beginPublish()`/`endPublish()`, `publish()`/`publish_P()`, `connect()`, `connected()`, `subscribe()`/`unsubscribe()` and internal timing handling (#22, #23, #28, #30, #31, #33, #34, #37, #38, #39, #40, #44)
- Fixed potential memory corruption in `MQTTPUBLISH` callback preparation (#25)
- Fixed a missing failure test on `_client->read()` in `readByte()` (#32)
- Switch from constructor init-lists to class member initialization (C++ core guideline C.45) (#42)

## [3.0.2] - 2025-02-27

### Added

- GitHub workflow to execute tests
- GitHub workflow to compile examples using Arduino CLI
- Examples: Use a better source of randomness (#9)
- Optionally deactivate `std::function` usage (#7)

## [3.0.1] - 2025-02-20

### Added

- Added `.editorconfig`
- Added GitHub workflow to check code style on push/PR

### Changed

- Reformatted code using Google style
- Fixes for tests: missing def for `strlen_P` & Python libs

## [3.0] - 2025-02-17

Maintenance taken over by Holger Mueller

### Added

- In-source documentation added to `PubSubClient` header
- Add flag for enabling debugging of library

### Changed

- Always use `PubSubClient()` constructor
- Use `bool` instead of `boolean`
- Add `yield()` calls in `connect()` and `write()` to avoid watchdog resets
- Fix bug in `publish_P` handling of PROGMEM payload length
- Fix increase `bytesToWrite` to `uint16_t` to prevent overflow
- Remove compiler warning about unsigned `expectedLength`
- Extend usage of `std::function` where available
- Fix for keep alive zero

## [2.8] - 2020-05-20

### Added

- `setBufferSize()` to override `MQTT_MAX_PACKET_SIZE`
- `setKeepAlive()` to override `MQTT_KEEPALIVE`
- `setSocketTimeout()` to override `MQTT_SOCKET_TIMEOUT`
- Check to prevent subscribe/unsubscribe to empty topics
- Declare WiFi mode prior to connect in ESP example
- Use `strnlen` to avoid overruns
- Support pre-connected `Client` objects

## [2.7] - 2018-11-02

### Added

- Added large-payload API: `beginPublish`/`write`/`publish`/`endPublish`
- Added `yield` calls to improve reliability on ESP
- Added Clean Session flag to connect options
- Added ESP32 support for functional callback signature

### Fixed

- Fixed remaining-length handling to prevent buffer overrun

## [2.4] - 2015-11-21

### Added

- `MQTT_SOCKET_TIMEOUT` to avoid blocking indefinitely while waiting for inbound data

### Fixed

- Fixed return code when publishing &gt;256 bytes

## [2.3] - 2015-09-11

### Added

- `publish(topic,payload,retained)` convenience overload

## [2.2] - 2015-09-07

### Changed

- Changed code layout to match Arduino Library requirements

## [2.1] - 2015-09-07

### Added

- `MAX_TRANSFER_SIZE` to chunk messages if needed
- Reject topic/payloads that exceed `MQTT_MAX_PACKET_SIZE`

## [2.0] - 2015-08-28

### Added

- MQTT 3.1.1 support
- Fix PROGMEM handling for Intel Galileo/ESP8266
- Overloaded constructors for convenience
- Chainable setters for server/callback/client/stream
- `state()` function to return CONNACK code

## [1.0] - 2009-02-02

- Initial release notes: QoS 0 only, 128 byte max message size, keepalive 30s, no Will messages

