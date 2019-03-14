.. include:: ../Plugin/_plugin_substitutions_p07x.repl
.. _P079_WemosMotorshield_page:

Wemos Motorshield
=================

|P079_typename|
|P079_status|

Introduction
------------

Firmware Upgrade
----------------
Piotr Bugalski has re-written the firmware for this motorshield, and published it at https://github.com/pbugalski/wemos_motor_shield.

The new firmware is compatible with the old one, minus the hanging and crashing, so it should be perfect for a drop-in replacement.

The original firmware has several issues like possible lockup of the I2C bus when performing an I2C scan for devices.

In order to flash the firmware, the module must be connected to a USB2TTL adapter.

Preparations to flash:

* Short the RTS and the 3V pins on the shield together, `like this <https://cdn.hackaday.io/images/3123331492797505129.jpg>`_
* Connect the main pins of the shield to your USB2TTL as follows:
  * GND ↔ GND
  * 3V3 ↔ 3V3 (or VCC or whatever it is called on your USB2TTL)
  * D2 ↔ TX
  * D1 ↔ RX

Downloads:

* Download the `motor_shield.bin <https://github.com/letscontrolit/ESPEasy/tree/mega/misc/Wemos_motor_shield/motor_shield.bin>`_
  This binary is built from `github.com/pbugalski/wemos_motor_shield <https://github.com/pbugalski/wemos_motor_shield>`_
* Download `STM32Flash <https://sourceforge.net/projects/stm32flash/files/>`_
* Extract and put the above files into a directory
* Plug in your USB serial device and make a note of the COM port it uses


Those of you who want to do this using Window's 8/10:

Window Command Prompt cd to the directory

1. ``stm32flash.exe COM9`` replace COM9 with your COM port number
2. ``stm32flash.exe -k COM9`` This will unlock your shield
3. ``stm32flash.exe -f -v -w motor_shield.bin COM9`` This will flash the bin and re-lock the shield.


Commands available
~~~~~~~~~~~~~~~~~~

.. include:: P079_commands.repl
