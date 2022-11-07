![IRremoteESP8266 Library](./assets/images/banner.svg)

[![Build Status](https://github.com/crankyoldgit/IRremoteESP8266/actions/workflows/Build.yml/badge.svg)](../../actions/workflows/Build.yml)
[![Code Lint](https://github.com/crankyoldgit/IRremoteESP8266/actions/workflows/Lint.yml/badge.svg)](../../actions/workflows/Lint.yml)
[![Tests](https://github.com/crankyoldgit/IRremoteESP8266/actions/workflows/UnitTests.yml/badge.svg)](../../actions/workflows/UnitTests.yml)
[![Documentation](https://github.com/crankyoldgit/IRremoteESP8266/actions/workflows/Documentation.yml/badge.svg)](../../actions/workflows/Documentation.yml/badge.svg)
[![arduino-library-badge](https://www.ardu-badge.com/badge/IRremoteESP8266.svg?)](https://www.ardu-badge.com/IRremoteESP8266)
[![GitLicense](https://gitlicense.com/badge/crankyoldgit/IRremoteESP8266)](https://gitlicense.com/license/crankyoldgit/IRremoteESP8266)

This library enables you to **send _and_ receive** infra-red signals on an [ESP8266](https://github.com/esp8266/Arduino) or an
[ESP32](https://github.com/espressif/arduino-esp32) using the [Arduino framework](https://www.arduino.cc/) using common 940nm IR LEDs and common IR receiver modules. e.g. TSOP{17,22,24,36,38,44,48}* demodulators etc.

## v2.8.2 Now Available
Version 2.8.2 of the library is now [available](https://github.com/crankyoldgit/IRremoteESP8266/releases/latest). You can view the [Release Notes](ReleaseNotes.md) for all the significant changes.

#### Upgrading from pre-v2.0
Usage of the library has been slightly changed in v2.0. You will need to change your usage to work with v2.0 and beyond. You can read more about the changes required on our [Upgrade to v2.0](https://github.com/crankyoldgit/IRremoteESP8266/wiki/Upgrading-to-v2.0) page.

#### Upgrading from pre-v2.5
The library has changed from using constants declared as `#define` to
[const](https://google.github.io/styleguide/cppguide.html#Constant_Names) with
the appropriate naming per the
[C++ style guide](https://google.github.io/styleguide/cppguide.html).
This may potentially cause old programs to not compile.
The most likely externally used `#define`s have been _aliased_ for limited
backward compatibility for projects using the old style. Going forward, only the
new `kConstantName` style will be supported for new protocol additions.

In the unlikely case, it does break your code, then you may have been referencing
something you likely should not have. You should be able to quickly determine
the new name from the old. e.g. `CONSTANT_NAME` to `kConstantName`.
Use common sense or examining the library's code if this does affect code.

## Supported Protocols
You can find the details of which protocols & devices are supported
[here](https://github.com/crankyoldgit/IRremoteESP8266/blob/master/SupportedProtocols.md).

## Troubleshooting
Before reporting an issue or asking for help, please try to follow our [Troubleshooting Guide](https://github.com/crankyoldgit/IRremoteESP8266/wiki/Troubleshooting-Guide) first.

## Frequently Asked Questions
Some common answers to common questions and problems are on our [F.A.Q. wiki page](https://github.com/crankyoldgit/IRremoteESP8266/wiki/Frequently-Asked-Questions).

## Library API Documentation
This library uses [Doxygen](https://www.doxygen.nl/index.html) to [automatically document](https://crankyoldgit.github.io/IRremoteESP8266/doxygen/html/) the [library's](https://crankyoldgit.github.io/IRremoteESP8266/doxygen/html/) [API](https://en.wikipedia.org/wiki/Application_programming_interface).
You can find it [here](https://crankyoldgit.github.io/IRremoteESP8266/doxygen/html/).

## Installation
##### Official releases via the Arduino IDE v1.8+ (Windows & Linux)
1. Click the _"Sketch"_ -> _"Include Library"_ -> _"Manage Libraries..."_ Menu items.
1. Enter `IRremoteESP8266` into the _"Filter your search..."_ top right search box.
1. Click on the IRremoteESP8266 result of the search.
1. Select the version you wish to install and click _"Install"_.

##### Manual Installation for Windows
1. Click on _"Clone or Download"_ button, then _"[Download ZIP](https://github.com/crankyoldgit/IRremoteESP8266/archive->master.zip)"_ on the page.
1. Extract the contents of the downloaded zip file.
1. Rename the extracted folder to _"IRremoteESP8266"_.
1. Move this folder to your libraries directory. (under windows: `C:\Users\YOURNAME\Documents\Arduino\libraries\`)
1. Restart your Arduino IDE.
1. Check out the examples.

##### Using Git to install the library ( Linux )
```
cd ~/Arduino/libraries
git clone https://github.com/crankyoldgit/IRremoteESP8266.git
```
###### To update to the latest version of the library
```
cd ~/Arduino/libraries/IRremoteESP8266 && git pull
```

## Contributing
If you want to [contribute](.github/CONTRIBUTING.md#how-can-i-contribute) to this project, consider:
- [Reporting](.github/CONTRIBUTING.md#reporting-bugs) bugs and errors
- Ask for enhancements
- Improve our documentation
- [Creating issues](.github/CONTRIBUTING.md#reporting-bugs) and [pull requests](.github/CONTRIBUTING.md#pull-requests)
- Tell other people about this library

## Contributors
Available [here](.github/Contributors.md)

## Library History
This library was originally based on Ken Shirriff's work (https://github.com/shirriff/Arduino-IRremote/)

[Mark Szabo](https://github.com/crankyoldgit/IRremoteESP8266) has updated the IRsend class to work on ESP8266 and [Sebastien Warin](https://github.com/sebastienwarin/IRremoteESP8266) the receiving & decoding part (IRrecv class).

As of v2.0, the library was almost entirely re-written with the ESP8266's resources in mind.
