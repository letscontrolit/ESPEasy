.. include:: ../Plugin/_plugin_substitutions_p05x.repl
.. _P052_tSense_K70_page:

tSense (K70)
============

|P052_typename|
|P052_status|

.. image:: P052_tSense_K70_1.jpg

Introduction
------------

Using the plugin P052 Senseair you can use the tSense (K70) as a sensor for temperature,
humidity and carbon dioxide. You can even use it to monitor the status of the internal relay,
or control the relay as well. If you have the EFA version of the tSense the temperature
adjustment is also possible to monitor.

The standard model of tSense display temperature, humidity and carbon dioxide level on the main page.
The relay can only be set to trigger by the settings menu where you can have a target level where the relay will activate.

.. note::
   For both models the user password for the settings is by default "1111" and for the advanced settings level the password is "2001".

Specifications:
 * 0 to 2000 ppm
 * 12VDC, 24VAC/DC
 * UART communication (Modbus (MB) or BACnet (BAC) protocol over RS485)
 * 15 years life span
 * No burn-in needed, will stabilize after 8 days at worst depending on placement environment

.. warning::
  CONNECTING THIS PLUGIN WILL VOID THE WARRANTY OF YOUR tSENSE!

Wiring
------

.. code-block:: html

    ESP               tSense
    GPIO (7)   <-->   TX (see pictures further down)
    GPIO (8)   <-->   RX (see pictures further down)

    Power
    5.0V       <-->   VCC (see pictures further down)
    GND        <-->   GND (see pictures further down)

Set up the K70 according to this simple schematics. If you want to take extra precautions you should add 47R
resistor or similar to the TX and RX signals (this is not used in the pictures below). In this example we
use the Wemos D1 mini and piggy back the power from the K70 5V via.

Setup
-----

.. image:: P052_Setup_tSense_K70_1.png

Task settings
~~~~~~~~~~~~~

* **Device**: Name of plugin
* **Name**: Name of the task (example name **CO2**)
* **Enable**: Should the task be enabled or not

Sensor
^^^^^^

* **GPIO <-- TX**: TX is generally set to **GPIO 13 (D7)**.
* **GPIO --> RX**: RX is generally set to **GPIO 15 (D8)**.
* **Sensor**: In this example we use the carbon dioxide sensor.

.. note:: TX GPIO 1 (D10) and RX GPIO 3 (D9) is hardware serial and have been reported to work better for some users over time compared to the
   bit-banging (soft) serial used over GPIO 13 and 15.

Data acquisition
^^^^^^^^^^^^^^^^

* **Send to controller** 1..3: Check which controller (if any) you want to publish to. All or no controller can be used.
* **Interval**: How often should the task publish its value (5..15 seconds is normal).

Indicators (recommended settings)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. csv-table::
   :header: "Indicator", "Value Name", "Interval", "Decimals", "Extra information"
   :widths: 8, 5, 5, 5, 40

   "Error status", "Error", "15", "0",  "
   Used to present errors (if any):

   ``-1`` No error

   ``0`` No ability to communicate with CO2 sensor module.

   ``1`` CO2 measurement error.

   ``2`` Temp measurement error.

   ``3`` No ability to communicate with RH/T sensor module.

   ``4`` RH measurement error

   ``5`` Temp measurement error, sensor will use CO2 sensor temperature if RH/T temperature is unavailable. S_Temp will be set to NTC_Temp.

   ``6`` (not used)

   ``7`` (not used)

   ``8`` Error in output configuration. Output is still updated, i.e. can be 0-10V.

   ``9`` One or several bytes of sensors parameter memory (settings) are corrupt.
   "
   "Carbon dioxide", "ppm", "15", "0", ""
   "Temperature", "C", "15", "1", ""
   "Humidity", "RH", "15", "0", ""
   "Relay status", "Relay", "1", "0", "
   This indicator monitor the software relay status, whereas the yellow/gray wiring monitor the hardware relay status
   (it's actual output). Both methods are good, just different. The software way of monitoring the status might be up
   to 1 second longer (if delay is 1 sec) due to the fact that a switch status is send instant compared to the relay
   status indicator which is sent once every X seconds.
   "
   "Temperature adjustment", "Level", "1", "0", "
   Only available in some versions of the tSense.

   This raw value goes from 0 - 1000 in 9 steps.
   To have them displayed as -4 to 4 use this formula:

   ``%value%*0.008-4``

   To have them displayed as 1 to 9 use this formula:

   ``%value%*0.008+1``
   "
   "ABC period", "N/A", "", "", ""

.. note:: If you want to use the relative carbon dioxide percentage (2000ppm = 100% and 350ppm = 0%) you should use this
          :code:`100-(2000-%value%)/(2000-350)*100` as a formula. And instead of :code:`ppm` as value name you should use
          :code:`RCO2` (relative CO2) and 1 decimal.

Rules examples
--------------

.. code-block:: html

 on CO2#Level do
  if [CO2#Level]>2000
    Publish,%sysname%/Alarm,CO2 level is too high!
  endif
 endon

Commands available
~~~~~~~~~~~~~~~~~~

.. include:: P052_commands.repl

Where to buy
------------

.. csv-table::
   :header: "Store", "Link"
   :widths: 5, 40

   "Senseair","`Link 1 <https://senseair.com/products/tsense/tsense-display/>`_"

|affiliate|


More pictures
-------------

There's two main ones, the EFA model and the standard model (see different GUI below).
The hardware is the same (K70) but different firmware separates them. No one is better then the other, they are just different.

The EFA model of tSense generally displays the temperature and carbon dioxide level. Other than that you have a
software button for controlling the internal relay (on and off). It even have a 9 step slider used to control the
indoor temperature (monitoring the status of this slider is possible using the plugin).

.. image:: P052_tSense_K70_2.jpg

The standard model of tSense display temperature, humidity and carbon dioxide level on the main page.
The relay can only be set to trigger by the settings menu where you can have a target level where the relay will activate.

.. image:: P052_tSense_K70_1.jpg

Mounting the D1 mini inside enclosure
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When you first open up the housing you find this:

.. image:: P052_tSense_K70_3.jpg

Remove the plastic cover to expose the pcb-board:

.. image:: P052_tSense_K70_4.jpg

To get power and ground to the ESP we hi-jack these vias:

.. image:: P052_tSense_K70_5.jpg

Connect these to the ESPs 5V and G:

.. image:: P052_tSense_K70_6.jpg

The ESP monitor the tSense through the serial ports, green to D7 (GPIO-13) and white to D8 (GPIO-15):

.. image:: P052_tSense_K70_7.jpg

Use the 3.5mm pads or the vias on the tSense:

.. image:: P052_tSense_K70_8.jpg

.. image:: P052_tSense_K70_9.jpg

.. image:: P052_tSense_K70_10.jpg

Monitoring the relay (hard wired)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. image:: P052_tSense_K70_11.jpg

.. image:: P052_tSense_K70_12.jpg

.. image:: P052_tSense_K70_13.jpg

.. note::
     Please observe that you cannot CONTROL the relay using this method, only monitor its status.
     To control the relay you need the latest plugin and use ``senseair_setrelay`` command.

To fixate the ESP board you could use a tape pad with double sided adhesive:

.. image:: P052_tSense_K70_14.jpg

Before re-positioning the unit onto the back panel you need to remove a plastic wall (just bend it and it will break):

.. image:: P052_tSense_K70_15.jpg

.. image:: P052_tSense_K70_16.jpg

DONE! Power up using the tSense's 24VDC/24VAC (the unit is possible to function with input power
9VDC - 24VDC with no problem) power adaptor.
