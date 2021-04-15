Safety
******

Some devices or modules which can run ESPeasy are connected to mains power.
As always, care must be taken when connecting something to mains power.

Most of these only switch mains via some relay and have a transformer inside
which may isolate the low voltage part from mains power.

One may never (!!) expect it to be isolated, so always make sure to disconnect
the device before operating on the low voltage part.
For example programming via the serial port may only be done when the device is
not connected to mains power.
When still connected, it may burn the mainboard of your computer, or cause some serious injuries or fire.

Apart from operating on the internals, one must also notice the dangers of sensors which can be touched.
For example the Sonoff TH10/16 has the option to have some sensors outside the enclosure.
The official sensors may be relatively safe to use even when the device is connected to mains.
But the same enclosure is used for the Sonoff POW and POW r2.
So these have the same hole on the side used on the TH10/16 to connect sensors.
This hole may seem like an invitation to connect some external sensor to these devices, but don't be tempted to do so.


HWL8012 & CSE7766
=================

Some ESP8266 powered devices have an energy monitoring sensor on board.
Well known examples are the Sonoff POW and POW r2, but there are many others.
Almost all use either the HWL8012 or CSE7766 chip.

These chips are not isolated from mains power, which means all electronics connected
to them does also have a direct connection to mains power lines.

So if you connect a sensor to a device with one of these chips, it is very
likely these sensors will also be connected to either N or L from mains power.

So make sure these sensors are properly isolated and cannot be touched by a human or animal.
