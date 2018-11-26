.. _P000_Relay_page:

Relay
=====

|P000_typename|
|P000_status|

.. image:: P000_Relay_1.jpg

Introduction
------------

The ESP8266 can't switch high currents or voltages and no AC at all. But it can switch a relay that can switch high load, isolating high
voltage from the ESP circuit. There are several relay types in the wild.

The ESP can control a relay using one of it's GPIO pins. All relay boards that work with Arduino should also work with the ESP module.
Note this is only a possible application of the ESPs ability to send a logic 1/0 (TTL at 3.3V) on a GPIO. These relay are usually available
as 1,2,4,8,16 Relay modules and work electromagnetically. They can switch both AC/DC(or imitate a button press of a remote for example)
as they are "power operated mechanical switches". Each relay has the screw terminal. The middle one is usually "always connected" terminal.
The NC contact will be connected to the middle terminal when the relay is not powered. The NO is connected when the relay is powered
and activated. Note that there are "active high" and "active low" versions of these relays.

There are mainly two relay types in the wild: The good old mechanical relay with the nice "click-clack" sound and the electronic relays, so called
"Solid State Relays" (SSR) which are completely silent.

Specifications:
 * Opto coupler (mechanical)
 * Solid state (SSR)

Relays are the most used actuators in home automation. Most people use breakout boards with a relay on it. Sadly there is no "standard" relay board.
It seems every small manufacturing garage in china has it's own layout and schematic. Some use 5V, some use 12V, some do with opto couplers, some without...
the perfect chaos. Several problems result from this chaos. Relays keeping switched on no mater what your ESP says. Relays not switching at all.
And last but not least - sometimes they work. Let's get some order into that chaos.

Mechanical relay
----------------

The mechanical relay is a simple construction.
It consist of a coil, one or more contacts and a spring. The spring keeps the contact in the "open" position. If the coil gets current, the magnetic
field pulls the contact into the second position, the contact closes. As soon as the current is switched off the spring pulls the contact back.
Mechanical relays may just have one contact switching to closed if the coil works (Normally open or NO). The contact might be a changeover contact which
has a "common" connector and a NO (open without coil current, "normally open") and a NC (closed without coil current, "normally closed") connection.
There might even be several contacts in one relay.

There are several special types of mechanical relay
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Latching relay:

.. image:: P000_Relay_2.jpg

One current pulse switches on and keeps on when current goes off, a second pulse switches off.


Polarized relay:

.. image:: P000_Relay_3.jpg

These relays have a defined + and - pin for the coil that must be followed.

.. warning:: These micro relays can not switch mains voltage! Use them for low voltage door openers or similar.

Pros and cons of mechanical relays

:green:`+` Several contacts in one relay available.

:green:`+` NO/NC available.

:green:`+` Limited resistance to overload and overvoltage.

:red:`-` Noisy, might disturb in a living room or bedroom.

:red:`-` Mechanical, moving parts that might wear out over time.

:red:`-` Contact bumping when switching.

:red:`-` High current for the coil, needs a transistor or FET for driving and generates peaks on power supply.

:red:`-` The coil induces high voltage when switching off.

Solid state relay
-----------------

The solid state relay, SSR for short, is a complex electronic circuit. It uses an opto coupler for input, this isolating the driving circuit from the load voltage.
Isolation voltage is usually 2000V, 4000V or more. The switching element usually is a triac for AC switching types. For DC there are types with a MOSFET as switching element.

There are several special types of SSR relay
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. image:: P000_Relay_4.jpg

As with the mechanical relays there are specialized types of SSR's.

**AC Type**

Can only (!!) switch AC.

AC with zero crossing circuit. This type switches on and off if the AC voltage is at the zero point. **Expensive** and necessary only for special applications.

**DC Type**

Can only (!!) switch DC!

And a lot more that are reserved for special cases.

Pros and cons of SSR relays

:green:`+` Completely noiseless
:green:`+` Low driving current and voltage available. Some types can be driven directly from the ESP.

:green:`+` No mechanical parts, no wearing out.

:green:`+` No contact bumping.

:green:`+` No voltage peaks on the low voltage side.

:green:`+` Available for very high current for an affordable price.

:red:`-` Very sensitive to overvoltage and overcurrent.

:red:`-` No NC available usually.

:red:`-` Usually with only one or sometimes three "normally open" circuits.

:red:`-` Needs cooling! Smaller relays up to 5A must be mounted with at least 1cm distance to other parts.

:red:`-` Bigger relays usually need a heat sink. Watch out: Heat sink may carry mains voltage with some types!

:red:`-` Leak current! These relays leak some current even if switched off.

Leak currents
~~~~~~~~~~~~~

Leak Current from SSR's might lead into problems under certain circumstances.
The both types shown above ("Hoymk D3805KH" and "Mager GJ-6-L", 5A AC) were tested here.
The Mager GJ-5-L showed a low leaking current and is useable for most purposes.
The D3805HK showed a really high leaking current, enough for a 10W LED to glow and a small solenoid valve to keep open!
This is not recommended for normal use.

Meanwhile a third sample arrived: The SSR-D32A380/5 sold by Pollin.
Similar to the D3805HK it shows a leaking current, as the test LED is slightly glowing in the dark when off.
It seems to be a lower leaking then with the D3805HK, this is subject to some further investigation.
The advantage of the Pollin SSR: It contains a small LED on the top indicating if the relay is on.
This SSR is a bit bigger then the other two types tested, it just fits the case. So holes for the LEDs
can be drilled on the top side of the case.

.. danger::
   Always remember that a switched-off SSR is not completely switched off!
   Even a small leaking current is enough to give a shock. Nice surprise if you're on the ladder changing bulbs....

Wiring
------

.. code-block:: html

  ESP               S8
  GPIO (X)   <-->   TX
  GPIO (X)   <-->   RX


  Power
  5.0V       <-->   VCC
  GND        <-->   GND


Setup
-----

No device is needed, generally you use rules to activate relays, or you send commands to activate them. Below you find more information on how to do this.

Rules examples
--------------

.. code-block:: html

  //Code below...


Indicators (recommended settings)
---------------------------------

No device is needed.

Where to buy
------------

.. csv-table::
  :header: "Store", "Link"
  :widths: 5, 40

  "AliExpress","`Link 1 ($) <http://s.click.aliexpress.com/e/cM59HXVq>`_ `Link 2 ($) <http://s.click.aliexpress.com/e/cerop6s0>`_"

|affiliate|


.. More pictures
.. -------------
