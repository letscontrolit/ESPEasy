.. include:: ../Plugin/_plugin_substitutions_p03x.repl
.. _P031_SHT1X_page:

SHT1X
=====

|P031_typename|
|P031_status|


Introduction
------------


Specifications:
 * Temperature
 * Humidity


Wiring
------


.. code-block:: none

  ESP               S8
  GPIO (X)   <-->   TX
  GPIO (X)   <-->   RX


  Power
  5.0V       <-->   VCC
  GND        <-->   GND


Setup
-----

This sensor does not communicate via some standard protocol.
The clock line can be shared with other instances of this plugin in order to
accommodate for more sensors on the same node.

Using long wires can cause a decay of the signal, due to the internal capacitance of the wire.
To overcome this signal attenuation, the clock speed of the signal can be lowered.

For example, a delay of 10 usec can be enough to use 30m CAT6 UTP cable with these sensors.
With a delay of 0, on a 80 MHz ESP8266, the clock signal is about 200 kHz.
With a delay of 10, on the same node, the clock signal is about 40 kHz.



Rules examples
--------------

.. code-block:: none

  //Code below...


Indicators (recommended settings)
---------------------------------

.. csv-table::
  :header: "Indicator", "Value Name", "Interval", "Decimals", "Extra information"
  :widths: 8, 5, 5, 5, 40

  "XXXXXX", "N/A", "", "", ""

Where to buy
------------

.. csv-table::
  :header: "Store", "Link"
  :widths: 5, 40

  "AliExpress","`Link 1 ($) <http://s.click.aliexpress.com/e/cg1fhDDI>`_"

|affiliate|


More pictures
-------------
