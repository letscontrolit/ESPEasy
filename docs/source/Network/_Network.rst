.. include:: _network_substitutions.repl

Network
*******

A Network plugin is comparable with a 'driver' for a network adapter.

The first 2 entries will be the same for every ESPEasy setup and these cannot be removed.

- Wi-Fi Station: To connect the ESP board to an access point.
- Wi-Fi AP: To let the ESP board act as an access point.


.. note::
   The network code of ESPEasy has been rewritten in 2025/2026.
   Older builds of ESPEasy do have a different organisation of network related parameters.
   Most of these were accessible via the ``Tools->Advanced`` page.



.. _Network Plugins:

Network Plugins
==================

.. note::
   ESP32 builds do have a lot more networking capabilities compared to ESP8266.
   For ESP8266 we only have support for WiFi and no other network interfaces will be added for ESP8266.

.. csv-table::
   :header: "Plugin name", "Plugin status", "Plugin number"
   :widths: 10, 8, 5

   ":ref:`NW001_page`","|NW001_status|","NW001"
   ":ref:`NW002_page`","|NW002_status|","NW002"
   ":ref:`NW003_page`","|NW003_status|","NW003"
   ":ref:`NW004_page`","|NW004_status|","NW004"
   ":ref:`NW005_page`","|NW005_status|","NW005"

Network Parameters
==================

Route Priority
--------------

(ESP32 only)

When using multiple network interfaces, like WiFi, Ethernet or PPP LTE Modem, it must be made clear which interface should be used for new connections initiated from ESPEasy to some other host.

The connected network interface with the highest Route Priority is considered to be the default route.

Default Route Priority values are:

* WiFi STA = 100
* Ethernet = 50
* PPP (LTE modem) = 20
* WiFi AP = 10


Fallback Interface
------------------

(ESP32 only)

.. note::
   The concept of a Fallback Interface is available on ESP8266, but only for WiFi AP.
   This is a special case, which is discussed here: :ref:`NW002_page` .

A network interface can be marked as "Fallback Interface".

A fallback interface will only be scheduled to start when:

* A non-fallback interface failed to connect.
* Route Priority changed to a value which is less than the set priority for the fallback interface.

The scheduled delay to start the fallback interface is set as "Delay Startup" (see below).

A fallback interface will be stopped when there is a default route with a route priority higher than the set Route Priority of the fallback interface.

It is possible to set multiple interfaces as Fallback Interface. 
The fallback order can be tweaked using the Delay Startup and Route Priority.

N.B. Network interfaces which should be started at boot, should not be marked as Fallback Interface.

Delay Startup
-------------

For various reasons, it can be useful to not immediately start a network interface at boot.
For example to reduce the power consumption as most network interfaces may draw significant more power for a short time when starting.

Another use case can be to check some sensor value before deciding to either start the network interface or enter deep sleep again.

The set value (in msec) is the delay from boot before starting the network interface.

Delay Startup for Fallback Interface
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A Fallback Interface is not started at boot.
The set Delay Startup is then used as delay to schedule starting the network interface.
See "Fallback Interface" for more information.


Block Web Access
----------------

When checked, the ESPEasy web interface cannot be accessed via the IP-range of the network interface.

.. note::
   The PPP Network Interface (ESP32-only) will have this checked by default, since you typically can't access devices from the network of the mobile provider.
   If this is possible for some odd reason, you very likely would never want to allow this.

Enable IPv6
-----------

(ESP32 only)

Checking this checkbox, will allow the network device to use IPv6.
