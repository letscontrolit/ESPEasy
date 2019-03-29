Tools
*****

Log
===

Info
====

Advanced
========

Special and Experimental Settings
---------------------------------

Fixed IP Octet
^^^^^^^^^^^^^^

Sets the last byte(octet) of the IP address to this value, regardless of what IP is given using DHCP (all other settings received via DHCP will be used)

So if you receive 192.168.1.234 from your DHCP server and this value is set to "10",
then the used IP in your node is 192.168.1.10.
But since you're receiving more information from the DHCP server,
like subnet mask / gateway / DNS, it may still be useful.
This allows a somewhat static IP in your network (N.B. use it with an 'octet' outside the range of the DHCP IPs) while still having set to DHCP.
So if you take the node to another network which does use 192.168.52.x then you will know it will be on 192.168.52.10 (when setting this value to "10")

I2C ClockStretchLimit
^^^^^^^^^^^^^^^^^^^^^

- `I2C-bus.org - Clock Stretching <https://www.i2c-bus.org/clock-stretching/>`_
- `ESPeasy wiki - Basics: The I2C Bus <https://www.letscontrolit.com/wiki/index.php/Basics:_The_I%C2%B2C_Bus>`_

WD I2C Address
^^^^^^^^^^^^^^

The Watchdog timer can be accessed via I2C.
What can be read/set/changed must still be documented.

Use SSDP
^^^^^^^^

Is disabled for now since it is causing crashes.
SSDP can be used to help auto discovery of a node.
For example Windows uses it to find hosts on a network.

Connection Failure Threshold
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Number of failed network connect attempts before issuing a reboot (0 = disabled)
A side effect is that trying to reach some server which is offline, may also result
in reboots of the ESP node.

Force WiFi B/G
^^^^^^^^^^^^^^

Force the WiFi to use only 802.11-B or -G protocol (not -N)
Since the 802.11 G mode of the ESP is more tolerant to noise, it may improve link
stability on some nodes.

Restart WiFi on lost conn.
^^^^^^^^^^^^^^^^^^^^^^^^^^

Force a complete WiFi radio shutdown & restart when connection with access point is lost.

Force WiFi no sleep
^^^^^^^^^^^^^^^^^^^

This option will set the WiFi sleep mode to no sleep.
This may cause the node to consume maximum power and should only be used for testing purposes.
It may even lead to more instability on nodes where the power supply is not
sufficient or the extra heat cannot be dissipated.

Since changing the mode back to the default setting may lead to crashes in some core versions, this option is only enabled when starting the node.
To activate a change of this setting, a reboot is required.

Periodical send Gratuitous ARP
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The ESP node may sometimes miss ARP broadcast packets and thus not answer them if needed.
This may lead to the situation where a packet sent to the node cannot be delivered,
since the switch does not know how to route the packet.
To overcome this, the ESP node may send a *Gratuitous ARP* packet, which is
essentially an answer to a request which hasn't been made.
These gratuitous ARP packets however may help the switch to remember which
MAC address is connected via what port.

By default the ESP will send out such a gratuitous ARP packet every time it
receives an IP address and also when it was unable to make a connection to a host.
It could be the other host was replying, but the packet was not routable to the ESP node.

This *Periodical send Gratuitous ARP* option will send these kind of ARP packets
continuously with some interval.
This interval is defined in the source code in ``TIMER_GRATUITOUS_ARP_MAX`` (e.g. 5000 msec)


CPU Eco mode
^^^^^^^^^^^^

Will call delay() from scheduler during idle loops.
This will result in a significant energy reduction of up-to 0.2 Watt.

However, it is no guarantee the power consumption will be reduced.
For example when the host is receiving continuous ping requests, it will never activate the power save mode.

If the power save mode is active, the node may miss some broadcast packets.
For example the ESPeasy p2p packets will be missed every now and then, so do not
activate this mode when response time  on received packets is important.

If the node is only sending packets (e.g. only a sensor connected and sending to some server),
then this is a great way to save energy and also reduce heat.



Show JSON
=========

Timing Stats
============

System Variables
================


Factory Reset
=============

Sometimes it can be useful to start over with a fresh setup.
The Factory Reset allows just that, and more.

- Format flash filesystem (so called SPIFFS)
- Re-create new settings files
- Already store some existing values to keep
- Allow for some pre-defined module config

Pre-defined module configurations help to setup the following:

- GPIO connected to button => plugin switch configured
- GPIO connected to relay => plugin switch configured
- If there is a conflict with default I2C pins, then those are set to no pin assigned for I2C
- Status LED GPIO
- Added rule to combine button and relay.

.. image:: images/FactoryReset_screenshot.png

Only pre-defined options for modules will be enabled for selection when they match the detected flash chip size.
For example, the Sonoff POW modules will not be selectable on a module with 1 MB flash
and the Sonoff Basic cannot be selected on a board with 4 MB flash.

.. warning:: Pressing the red "Factory Reset" button will immediately perform the reset with the set selection.
