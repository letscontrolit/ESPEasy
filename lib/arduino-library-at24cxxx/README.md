# AT24C Two-wire Serial EEPROM Library
This is an Arduino library for using the AT24C-series serial persistant memory chips. The library supports the following chips:
* AT24C01  - 128 bytes
* AT24C02  - 256 bytes
* AT24C04  - 512 bytes
* AT24C08  - 1024 bytes
* AT24C16  - 2048 bytes
* AT24C32  - 4096 bytes
* AT24C64  - 8192 bytes
* AT24C128 - 16384 bytes
* AT24C256 - 32768 bytes

The library interface is drop in compatible with the Arduino built in EEPROM API, so code written for the internal EEPROM will work with this library without modification.

The library has the following features:
* Uses the page write feature of the AT24C chips which is up to 64 times faster than doing single byte writes and reduces wear on the memory cells.
* Keeps track of page sizes for the different chips and adjusts writes to page borders
* Handles the Write Cycle Time of the chips, avoiding the risk of trying to access the chip while it is busy processing an earlier write
* Can read and write arbitrarily long buffers, managing limitations of buffer sizes in I2C libraries
* Transparent error handling making it easy to detect errors in the communication with the chip (very useful in early stages of a project)
* Can read and write basic types and structs directly

# Examples

## Setting up
You need to include the .h file for the chip type you are using and create a chip-object with the address it is configured at. There are constants defined for all eight possible addresses of the chip, AT24C_ADDRESS_0 - AT24C_ADDRESS_7. Since the I2C-bus is used, you also have to start the Wire-interface:
```C++
#include <at24c256.h>

AT24C256 eprom(AT24C_ADDRESS_0);

void setup() {
  Wire.begin();
 
  uint8_t byte = eprom.read(0);
}
```

## Reading and writing basic types
All basic types, such as int, long, double can be read or written with the `put` and `get` methods. You just specify the memory address and the variable to read or write:
```C++
int foo = 42;
eprom.put(0, foo);      // Write the integer value 42 to address 0
int foo_in;
eprom.get(0, foo_in);   // Read the integer value at address 0 into variable foo_in
```
## Reading and writing complex types
Also complex types, such as structs can be read or written with the `put` and `get` methods:
```C++
struct Coordinate {
  int x;
  int y;
};

Coordinate point = {17, 42};
eprom.put(0, point);     // Write the struct point to address 0
Coordinate point_in;
eprom.get(0, point_in);  // Read the values of the struct point_in from address 0
```
## Reading and writing byte buffers
Byte buffers of any length (that fits the memory) can be read and written. The library handles limitations in the underlying I2C libraris so the reads and writes are partitioned in small enough chunks:
```C++
uint8_t out[15] = "Test of buffer";
eprom.writeBuffer(0, out, 15);
uint8_t in[15];
eprom.readBuffer(0, in, 15);
```
## Reading raw bytes
The library can also read write and update single bytes:
```C++
eprom.write(0, 77);             // Writes the value 77 to byte at address 0
eprom.update(0, 77);            // In this case `update` does nothing, since it only writes if the value differs from the current
uint8_t value = eprom.read(0);  // Reads the value of the byte at address 0
```
## Connecting multiple chips
The libray allows you to create multiple chips (also of different types). You just have to create an object for each chip of the right type:
```C++
#include <at24c256.h>
#include <at24c02.h>

AT24C256 eprom0(AT24C_ADDRESS_0);
AT24C256 eprom1(AT24C_ADDRESS_1);
AT24C02 eprom2(AT24C_ADDRESS_2);

void setup() {
  Wire.begin();
 
  uint8_t byte = eprom0.read(0);
}
```
## Error handling
Since this is an external memory chip that is connected through the I2C-bus, there is always a risk that the communication failes, due to physical errors in the setup. You can always check if the latest operation succeeded via the `getLastError()` method:
```C++
uint8_t data = eprom.read(0);
if (eprom.getLastError() != 0) {
  Serial.print("Error reading from eeprom");
} 
```
Value 0 means no error, 1 means internal buffer overflow, 2 means NACK when addressing the chip and this is the error you will get if a chip is not connected to the specified address.

It is good practice to check for error at least once in setup so you get early feedback if there is a bad connection to the chip.
## Specifying TwoWire interface
Some Arduino boards have multiple I2C busses. The library allows you to specify which TwoWire bus to use for each chip object:
```C++
#include <at24c256.h>

AT24C256 eprom0(AT24C_ADDRESS_0, Wire);
```
## Specifying Write Cycle Time
The library waits for the chip to process a write (write cycle time). The default time is 6 mS, which with some margin covers the standard 5 mS specified for the chips. However some old versions of AT24C256A may have up to 20 mS write cycle time, and in that case you can specify a higher time for a chip:
```C++
#include <at24c256.h>

AT24C256 eprom0(AT24C_ADDRESS_0, Wire, 20);
```
If your application is really time critical and you only write single bytes, you can also set the write cycle time to 0 to avoid the internal wait. I that case you have to keep track on that your application does not try to access the chip too fast after a write operation.
## Getting the size of a chip
To be compatible with the built in EEPROM library, this library can also return the size of the memory chip. This makes it possible for your code automatically to adapt to different chip types:
```C++
int size = eprom.length());
```
## Limitations on chips
Some of the at24c chips have limitations in how many i2c addresses are available for the chips:
* AT24C04 has 4 addresses
* AT24C08 has 2 addresses
* AT24C16 has only 1 hardcoded address

Because of this, AT24C04 and AT24C08 have their own address enums and the AT24C16 does not have an address parameter in the constructor.
```C++
#include <at24c04.h>
#include <at24c08.h>
#include <at24c16.h>

AT24C04 eprom0(AT24C04_ADDRESS_1);
AT24C08 eprom1(AT24C08_ADDRESS_1);
AT24C16 eprom2();
```
