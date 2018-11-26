Command Reference
*****************

ESP Easy offers a set of commands to control hardware devices and provide some basic local control using rules. There are several ways to launch commands on ESP Easy:

.. csv-table::
  :header: "Protocol", "Syntax", "Extra information"
  :widths: 8, 15, 15

  "
  HTTP
  ","
  **http://<espeasyip>/control?cmd=** ``<command>``
  ","
  Send commands over the HTTP protocol.
  "
  "
  MQTT
  ","
  **<MQTT subscribe template>/cmd** with payload: ``<command>``
  ","
  Send commands over the MQTT protocol.
  "
  "
  Serial (TTL)
  ","
  ``<command>``
  ","
  Send commands using serial (RX/TX). Just type the ``<command>``
  "
  "
  UDP
  ","
  **SendTo,<unit nr>,** ``<command>``
  ","
  Send commands from one ESP Easy unit to another. Setup UDP ESP Easy peer-2-peer controller first.
  "
  "
  Rules
  ","
  ``<command>``
  ","
  Internally within ESP Easy. Just enter the ``<command>`` within an event block or conditional block.
  "

Commands are divided into several classes:

:red:`Internal` can be run from serial and rules engine

:green:`Rules` can be run from serial and rules engine

:cyan:`Plugin` can be run from serial, rules engine, HTTP, MQTT

:blue:`Special` can be used from any source


Commands listed (core commands and plugin commands)
---------------------------------------------------

.. include:: ../Plugin/P000_commands.rst


Internal GPIO handling and ringtones
------------------------------------

.. include:: ../Plugin/P001_commands_GPIO.rst

.. include:: ../Plugin/P001_commands_RTTTL.rst

Plugin based commands
---------------------

.. .. include:: ../Plugin/P002_commands.rst
