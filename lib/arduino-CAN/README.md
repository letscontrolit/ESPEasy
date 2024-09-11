# Arduino CAN

[![Build Status](https://travis-ci.org/sandeepmistry/arduino-CAN.svg?branch=master)](https://travis-ci.org/sandeepmistry/arduino-CAN)

An Arduino library for sending and receiving data using CAN bus.

## Compatible Hardware

* [Microchip MCP2515](http://www.microchip.com/wwwproducts/en/en010406) based boards/shields
  * [Arduino MKR CAN shield](https://store.arduino.cc/arduino-mkr-can-shield)
* [Espressif ESP32](http://espressif.com/en/products/hardware/esp32/overview)'s built-in [SJA1000](https://www.nxp.com/products/analog/interfaces/in-vehicle-network/can-transceiver-and-controllers/stand-alone-can-controller:SJA1000T) compatible CAN controller with an external 3.3V CAN transceiver

### Microchip MCP2515 wiring

| Microchip MCP2515 | Arduino |
| :---------------: | :-----: |
| VCC | 5V |
| GND | GND |
| SCK | SCK |
| SO | MISO |
| SI | MOSI |
| CS | 10 |
| INT | 2 |


`CS` and `INT` pins can be changed by using `CAN.setPins(cs, irq)`. `INT` pin is optional, it is only needed for receive callback mode. If `INT` pin is used, it **must** be interrupt capable via [`attachInterrupt(...)`](https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/).

**NOTE**: Logic level converters must be used for boards which operate at 3.3V.

### Espressif ESP32 wiring

Requires an external 3.3V CAN transceiver, such as a [TI SN65HVD230](http://www.ti.com/product/SN65HVD230).

| CAN transceiver | ESP32 |
| :-------------: | :---: |
| 3V3 | 3V3 |
| GND | GND |
| CTX | GPIO_5 |
| CRX | GPIO_4 |

`CTX` and `CRX` pins can be changed by using `CAN.setPins(rx, tx)`.

## Installation

### Using the Arduino IDE Library Manager

1. Choose `Sketch` -> `Include Library` -> `Manage Libraries...`
2. Type `CAN` into the search box.
3. Click the row to select the library.
4. Click the `Install` button to install the library.

### Using Git

```sh
cd ~/Documents/Arduino/libraries/
git clone https://github.com/sandeepmistry/arduino-CAN CAN
```

## API

See [API.md](API.md).

## Examples

See [examples](examples) folder.

For OBD-II examples, checkout the [arduino-OBD2](https://github.com/sandeepmistry/arduino-OBD2) library's [examples](https://github.com/sandeepmistry/arduino-OBD2/examples).

## License

This library is [licensed](LICENSE) under the [MIT Licence](http://en.wikipedia.org/wiki/MIT_License).
