.. include:: ../Plugin/_plugin_substitutions_p00x.repl
.. _P001_Door_switch_page:

Door switch
===========

|P001_typename|
|P001_status|

.. image:: P001_Door_switch_1.jpg

Introduction
------------

Any door switch module can be used as an digital input. Most none contact door switches use a technology called
reed switch. By using a magnet (the part with no wires) the reed inside the other part (with wires) will connect
the two parts of the reed and making a signal go low or high depending on how you have connected the switch.
You may use it by simply connect it to either a GPIO that is normally high (pull-up) or normally low (pull-down).

Specifications:
 * Give a high or low signal to a GPIO

Wiring
------

.. code-block:: html

  ESP               Door switch
  GPIO (X)   <-->   Signal

  Power
  3.3V       <-->   Signal
              or
  GND        <-->   Signal


Setup
-----

Selecting the plugin "Switch input - Switch" gives you the options to setup a switch.


Rules examples
--------------

.. code-block:: html

    on Door#State=1 do
      timerSet,1,1
    endon

    on rules#timer=1 do
     if [Door#State]=0
      //Action if door is closed
     else
      //Action if door is opened
     endif
    endon


Indicators (recommended settings)
---------------------------------

.. csv-table::
  :header: "Indicator", "Value Name", "Interval", "Decimals", "Extra information"
  :widths: 8, 5, 5, 5, 40

  "XXXXXX", "N/A", "", "", ""

Where to buy
------------

.. csv-table::
  :header: "Type", "Link"
  :widths: 5, 40

  "Door switch","`AliExpress 1 ($) <http://s.click.aliexpress.com/e/ccCdXCVi>`_"
  "Reed switch","`AliExpress 2 ($) <http://s.click.aliexpress.com/e/0gMmOoY>`_"

|affiliate|


More pictures
-------------
