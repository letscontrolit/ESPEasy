GPIO
****



Best pins to use on ESP8266
---------------------------

Most board do not have made all pins available.
The NodeMCU boards have almost all usable pins made available and some which need special attention.

.. list-table:: Best pins to use (best to worst)
   :widths: 10 25 25 40
   :header-rows: 1

   * - GPIO
     - Input
     - Output
     - Remarks
   * - 5 (D1)
     - :green:`OK`
     - :green:`OK`
     - Often used for I2C
   * - 4 (D2)
     - :green:`OK`
     - :green:`OK`
     - Often used for I2C
   * - 14 (D5)
     - :green:`OK`
     - :green:`OK`
     -
   * - 12 (D6)
     - :green:`OK`
     - :green:`OK`
     -
   * - 13 (D7)
     - :green:`OK`
     - :green:`OK`
     -
   * - 0 (D3)
     - :yellow:`pulled up`
     - :yellow:`OK`
     - :yellow:`Boot fail if pulled low`
   * - 2 (D4)
     - :yellow:`pulled up`
     - :yellow:`OK`
     - :yellow:`Boot fail if pulled low`
   * - 15 (D8)
     - :red:`pulled to GND`
     - :yellow:`OK`
     - :yellow:`Boot fail if pulled high`
   * - 16 (D0)
     - :red:`High at boot, pull down to GND`:yellow:`no interrupt`
     - :yellow:`no I2C support`
     - Used to wake up from deep sleep
   * - 9 (SD2)
     - :red:`High at boot`
     -
     - :yellow:`See notes`
   * - 10 (SD3)
     - :red:`High at boot`
     -
     - :yellow:`See notes`
   * - 3 (RX)
     - :yellow:`High at boot`
     - :red:`is RX`
     - RX channel of serial0
   * - 1 (TX)
     - :red:`is TX`
     - :yellow:`debug output at boot`
     - :yellow:`Boot fail if pulled low`


The labels used in the tabel above are the labels on NodeMCU boards.

Sources used:

* `The Hook Up - How To: Pick the right pins on the NodeMCU ESP8266 and ESP32 <https://www.youtube.com/watch?v=7h2bE2vNoaY>`_
* `ESP8266 GPIO pin "features" <https://docs.google.com/spreadsheets/d/1pQuBocanzCZJ3KPUpu3VvpOOBzboqXDuwQDVP5UYcXc/edit#gid=0>`_
* `Rabbithole - ESP8266 GPIO Behaviour at Boot <http://rabbithole.wwwdotorg.org/2017/03/28/esp8266-gpio.html>`_

Special notes on GPIO 6 - 11
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Almost any ESP8266 board has a flash chip connected to some or all of GPIO6-GPIO11.

GPIO 9 & 10 can be used on ESP8285, since that chip has 1 MB flash embedded and is not using those pins.
One or both of pins may sometimes also be available on ESP8266 boards, depending on the flash used and the flash operation mode.

* Quad IO (QIO) uses 4 lines for data (6 pins total)
* Dual IO (DIO) uses 2 lines for data (4 pins total)
* Standard uses a single line for data (3 pins total)

It is best not to use any of the GPIO pins 6 - 11.


Special notes on GPIO 16
~~~~~~~~~~~~~~~~~~~~~~~~

GPIO-16 is a special pin.
The RTC will send a short (low) pulse when the sleep timer does send a wake-up signal.

In order to wake from deep sleep, GPIO-16 has to be connected to the RST pin.
If connected, this pin should not be used as GPIO pin, or else the module will reboot.

Other limitations are:

* GPIO16 has a built-in pull-down resistor (all others have built-in pull-up)
* To enable the pull-down resistor for GPIO16, you have to use ``INPUT_PULLDOWN_16``


Best pins to use on ESP32
-------------------------

There is also a NodeMCU version using the ESP32.
Fortunately the designers used the GPIO numbers as labels on the board.

N.B. This table does NOT describe the pins of other ESP32 variants like the ESP32-S2.

.. list-table:: Sorted in reverse GPIO pinnr order
   :widths: 10 15 15 10 50
   :header-rows: 1

   * - GPIO
     - Input
     - Output
     - ADC
     - Remarks
   * - 39
     - :green:`OK`
     -
     - ADC1_CH3
     - :yellow:`input only` :yellow:`See Notes`
   * - 38
     - :green:`OK`
     -
     - ADC1_CH2
     - :yellow:`input only`
   * - 37
     - :green:`OK`
     -
     - ADC1_CH1
     - :yellow:`input only`
   * - 36
     - :green:`OK`
     -
     - ADC1_CH0
     - :yellow:`input only` :yellow:`See Notes`
   * - 35
     - :green:`OK`
     -
     - ADC1_CH7
     - :yellow:`input only`
   * - 34
     - :green:`OK`
     -
     - ADC1_CH6
     - :yellow:`input only`
   * - 33
     - :green:`OK`
     - :green:`OK`
     - ADC1_CH5
     -
   * - 32
     - :green:`OK`
     - :green:`OK`
     - ADC1_CH4
     -
   * - 27
     - :green:`OK`
     - :green:`OK`
     - ADC2_CH7
     -
   * - 26
     - :green:`OK`
     - :green:`OK`
     - ADC2_CH9
     -
   * - 25
     - :green:`OK`
     - :green:`OK`
     - ADC2_CH8
     -
   * - 23
     - :green:`OK`
     - :green:`OK`
     -
     -
   * - 22
     - :green:`OK`
     - :green:`OK`
     -
     -
   * - 21
     - :green:`OK`
     - :green:`OK`
     -
     -
   * - 19
     - :green:`OK`
     - :green:`OK`
     -
     -
   * - 18
     - :green:`OK`
     - :green:`OK`
     -
     -
   * - 17
     - :green:`OK`
     - :green:`OK`
     -
     - :yellow:`See notes`
   * - 16
     - :green:`OK`
     - :green:`OK`
     -
     - :yellow:`See notes`
   * - 15
     - :green:`OK`
     - :green:`OK`
     - ADC2_CH3
     - :yellow:`output PWM signal at boot, internal pull-up` Silences boot messages when pulled low at boot.
   * - 14
     - :green:`OK`
     - :green:`OK`
     - ADC2_CH6
     - :yellow:`output PWM signal at boot`
   * - 13
     - :green:`OK`
     - :green:`OK`
     - ADC2_CH4
     -
   * - 12
     -
     - :green:`OK`
     - ADC2_CH5
     - :yellow:`Boot fail if pulled high` :red:`May damage flash if low at boot on 1.8V flash chips`
   * - 9, 10, 11
     - :red:`High at boot`
     -
     -
     - :red:`See notes`
   * - 6, 7, 8
     -
     -
     -
     - :red:`See notes`
   * - 5
     - :green:`OK`
     - :green:`OK`
     -
     - :yellow:`output PWM signal at boot, internal pull-up`
   * - 4
     - :green:`OK`
     - :green:`OK`
     - ADC2_CH0
     - :yellow:`Internal pull-down`
   * - 3 (RX)
     - :yellow:`High at boot`
     - :red:`is RX`
     -
     - RX channel of serial0
   * - 2
     - :green:`OK`
     - :green:`OK`
     - ADC2_CH2
     - Often connected to LED, :yellow:`Internal pull-down`
   * - 1 (TX)
     - :red:`is TX`
     - :yellow:`debug output at boot`
     -
     - TX channel of serial0
   * - 0
     - :yellow:`pulled up`
     - :yellow:`OK`
     - ADC2_CH1
     - :yellow:`Boot fail if pulled low & output PWM signal at boot`

Source used: 

* `The Hook Up - How To: Pick the right pins on the NodeMCU ESP8266 and ESP32 <https://www.youtube.com/watch?v=7h2bE2vNoaY>`_
* `Random Nerd Tutorials - ESP32 Pinout Reference: Which GPIO pins should you use? <https://randomnerdtutorials.com/esp32-pinout-reference-gpios/>`_
* `Espressif Docs - GPIO & RTC GPIO <https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html>`_

Special notes on GPIO 6 - 11 & 16, 17
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Almost any ESP32 board has a flash chip connected to some or all of GPIO6-GPIO11.

It is best not to use any of the GPIO pins 6 - 11.

GPIO 6, 7 & 8 may output some PWM signals at boot.
GPIO 9, 10 & 11 output high at boot and may fail to boot of pulled either high or low.

GPIO 16 & 17 are usually connected to the SPI flash and PSRAM. These do show some activity during boot to detect the presence of PSRAM.
For example GPIO 17 may still show some high frequency signal until it is specifically set to a high or low output state.
When PSRAM is present, these pins should not be used.

Special notes on GPIO 34-39
~~~~~~~~~~~~~~~~~~~~~~~~~~~

GPIO 34-39 can only be set as input mode and do not have software-enabled pullup or pulldown functions.

GPIO 36 & 39 should not be used for interrupts when also using ADC or WiFi and Bluetooth.
These pins are on some boards labelled as:

* ``VP``: GPIO 36 (ADC1_CH0)
* ``VN``: GPIO 39 (ADC1_CH3)

The internal Hall sensor in the ESP32 is connected to these two pins.

The Hall Sensor uses channels 0 and 3 of ADC1. 
Do not configure these channels for use as ADC channels when also reading from the Hall sensor.


Typical uses in ESPEasy where an interrupt of a GPIO pin is used are:

* Switch input
* Monitoring of a pin in rules
* Pulse Count plugin
* RX pin of a serial port

Special notes on ADC pins
~~~~~~~~~~~~~~~~~~~~~~~~~

ADC2: ADC2 pins cannot be used when Wi-Fi is used. 
So, if you are having trouble getting the value from an ADC2 GPIO while using Wi-Fi, you may consider using an ADC1 GPIO instead, which should solve your problem.

Some ADC pins are strapped to a pull-up or pull-down resistor to set the ESP in a specific state during boot.
This requires special attention when applying some voltage to these pins which may be present at boot.



Boot Strapping Pins
~~~~~~~~~~~~~~~~~~~

The logic state of some pins during boot determines how the ESP32 will boot and into what mode.

* GPIO-0: Low: flash mode. High: Normal execution mode
* GPIO-2: Must also be either left unconnected/floating, or driven Low, in order to enter the serial bootloader.  In normal boot mode (GPIO0 high), GPIO2 is ignored.
* GPIO-12: If driven High, flash voltage (VDD_SDIO) is 1.8V not default 3.3V. Has internal pull-down, so unconnected = Low = 3.3V. May prevent flashing and/or booting if 3.3V flash is used and this pin is pulled high, causing the flash to brownout. See the ESP32 datasheet for more details.
* GPIO-15: If driven Low, silences boot messages printed by the ROM bootloader. Has an internal pull-up, so unconnected = High = normal output.

There is a number of pins that will be read at boot and their value is displayed in the "boot" value:

.. code-block:: none

  ets Jun  8 2016 00:22:57
  rst:0x1 (POWERON_RESET),boot:0x3 (DOWNLOAD_BOOT(UART0/UART1/SDIO_REI_REO_V2))

``boot:0xNN (DESCRIPTION)`` is the hex value of the strapping pins, as represented in the GPIO_STRAP register. The individual bit values are as follows:

* ``0x01`` - GPIO5
* ``0x02`` - MTDO (GPIO15)
* ``0x04`` - GPIO4
* ``0x08`` - GPIO2
* ``0x10`` - GPIO0
* ``0x20`` - MTDI (GPIO12)

See `boot_mode.h <https://github.com/espressif/esp-idf/blob/1cb31e50943bb757966ca91ed7f4852692a5b0ed/components/soc/esp32/include/soc/boot_mode.h>`_ for more information.

See ``System#BootMode`` event to access the boot strap pin values.



Pins used for RMII Ethernet PHY
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. include:: ../Reference/Ethernet_PHY_ESP32.rst
