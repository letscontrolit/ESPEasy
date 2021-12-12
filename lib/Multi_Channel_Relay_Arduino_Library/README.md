# Multi_Channel_Relay_Arduino_Library  [![Build Status](https://travis-ci.com/Seeed-Studio/Multi_Channel_Relay_Arduino_Library.svg?branch=master)](https://travis-ci.com/Seeed-Studio/Multi_Channel_Relay_Arduino_Library)
This is the Arduino library for Seeed multi channel relay. 

<!-- <img src= width=400> -->

<!-- [Grove - OLED Display 0.96"](https://www.seeedstudio.com/s/Grove-OLED-Display-0.96%22-p-781.html) -->

<!-- Description for this product -->

### How to use this library
you can download from Arduino Library Manager or directlly download from this repository.
Connect the 8-channel Solid state relay v1.0 to Arduino board's I2C port, compile and upload four_channel_relay_control.ino. Open the serial monitor the relay should go as expected some massage should show as below:

```
Channel 1 on
Channel 2 on
Channel 3 on
Channel 4 on
Turn all channels on, State: 1111
Turn 1 3 channels on, State: 101
Turn 2 4 channels on, State: 1010
Turn off all channels, State: 0
```

- Upload eight_channel_relay_control.ino, open the serial monitor and show the below logs:

```
Channel 1 on
Channel 2 on
Channel 3 on
Channel 4 on
Channel 5 on
Channel 6 on
Channel 7 on
Channel 8 on
Turn all channels on, State: 11111111
Turn 1 3 5 7 channels on, State: 1010101
Turn 2 4 6 8 channels on, State: 10101010
Turn off all channels, State: 0

```

- This module can be set I2C address by software, open change_i2c_address.ino modify new_i2c_address as you want; compile and upload the sketch, open the serial monitor and show the below logs:

```
Scanning...
I2C device found at address 0x11  !
Found 1 devices
Address 0x21 has been saved to flash.
```

- Read firmware version of the module by example read_firmware_version.ino. Serial logs as below:

```
firmware version: 0x1
```

### Functionalities
- The relay board is an I2C device, device address is changeable, refer to **changeI2CAddress(uint8_t new_addr, uint8_t old_addr)** .
- Use **getChannelState()** to know the state of every channel.
- Use **getFirmwareVersion()** to recognize firmware burn in the on board MCU.
- Use **channelCtrl(uint8_t state)** to change all channel immediately, the **state** parameter represents channel 1 to 8.  
- Use **turn_on_channel(uint8_t channel)** to turn on single channel.
- Use **turn_off_channel(uint8_t channel)**	to turn off single channel.
- External functionality **scanI2CDevice()**, use for scan device address.


<!-- For more information, please refer to [Grove_OLED_Display_128X64 wiki][1] -->

----
This software is written by lambor for seeed studio and is licensed under The MIT License.<br>

Contributing to this software is warmly welcomed. You can do this basically by<br>
[forking](https://help.github.com/articles/fork-a-repo), committing modifications and then [pulling requests](https://help.github.com/articles/using-pull-requests) (follow the links above<br>
for operating guide). Adding change log and your contact into file header is encouraged.<br>
Thanks for your contribution.

Seeed is a hardware innovation platform for makers to grow inspirations into differentiating products. By working closely with technology providers of all scale, Seeed provides accessible technologies with quality, speed and supply chain knowledge. When prototypes are ready to iterate, Seeed helps productize 1 to 1,000 pcs using in-house engineering, supply chain management and agile manufacture forces. Seeed also team up with incubators, Chinese tech ecosystem, investors and distribution channels to portal Maker startups beyond.


[1]:http://wiki.seeedstudio.com/Grove-OLED_Display_0.96inch/


<!-- [![Analytics](https://ga-beacon.appspot.com/UA-46589105-3/OLED_Display_128X64)](https://github.com/igrigorik/ga-beacon) -->
