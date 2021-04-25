Config page
***********

Main Settings
=============

Unit Name
---------

The name of this node.
This is mainly used when connecting to a MQTT broker or in p2p communications.
It is also used to generate a hostname in the local network, for example when using mDNS.


Unit Number
-----------

The number of this node.
It can be appended to the hostname.

The main use of the unit number is in ESPEasy p2p communications.
When used in p2p communications, make sure not to use unit number ``0`` or ``255`` as those are reserved.

Range: 0 ... 255

Append Unit Number to hostname
------------------------------

A flag to indicate how the hostname must be generated from *Unit Name* and *Unit Number*.

N.B. currently it is used in all occurences where a hostname is needed, but this appeared to be not working well for some MQTT brokers.
So expect it to be changed later to split this flag for every occasion where a hostname is needed.


Admin Password
--------------

A password to prevent altering settings in the web interface.

Default password is empty.

N.B. when asked for a login prompt, the user name is ``admin``

WiFi Settings
=============

A WiFi capable device can operate in two modes:

* Station mode (STA), device needs to connect to an access point.
* Access Point mode (AP), other STA devices can connect to this device.

An AP will broadcast an identifier string, called the SSID.

For an STA device to connect to an AP, two parameters are needed:

* SSID - The identifier string of an AP.
* WPA key - The password or pass phrase to make a connection.

N.B. when using the mentioned commands to set SSID/key, don't forget to call the ``save`` command.


SSID
----

The AP identifier string to connect to.
This is needed when running in STA mode.

Can also be set via the command ``wifissid``.


WPA Key
-------

The password or pass phrase to connect to the AP with given SSID.

Can also be set via the command ``wifikey``.


Fallback SSID / WPA Key
-----------------------

This is an alternate to the first configured SSID/key.

There are various use cases to have multiple AP configurations stored on a node.
For example to be able to move to anothe location where only another AP can be reached.
Or as a backup, for example to let the node connect to a local hotspot on your phone.

Can also be set via the commands: 

* ``wifissid2``
* ``wifikey2``


WPA AP Mode Key
---------------

It is possible a node cannot connect to an AP:

* When there is no SSID configured
* The configured AP's are not in reach
* The configured AP's do not allow the node to connect

In these situations the ESP node will start its own local AP.

As SSID the Unit Name will be used, which is by default ``ESP_Easy_0``

The default WPA key to connect to this AP is ``configesp``, but can be changed here.

This is often used to perform the initial configuration like connecting to the local network.

Can also be set via the command ``WiFiAPKey``.


Custom Build WiFi credentials
-----------------------------

For custom builds, one can set default WiFi credentials by defining some default credentials at build.
See the examples in `Custom-sample.h`:

* ``#define DEFAULT_SSID  "MyHomeSSID"            // Enter your network SSID``
* ``#define DEFAULT_KEY   "MySuperSecretPassword" // Enter your network WPA key``
* ``#define DEFAULT_SSID2 ""                      // Enter your fallback network SSID``
* ``#define DEFAULT_KEY2  ""                      // Enter your fallback network WPA key``

Custom builds can also be customized a bit more to allow for a deployment SSID configuration and an emergency fallback.

Deployment & Support SSID will be used only when the configured SSIDs are not reachable and/or no credentials are set.
This to make deployment or support of large number of nodes easier.
This configured set of credentials will be considered a "low priority" set, thus it will be tried as last resort.
Therefore it may take a while for a unit to connect to it if there are lots of 'hidden SSID' APs and connecting to hidden SSIDs is allowed.

* ``#define CUSTOM_DEPLOYMENT_SSID ""  // Enter SSID not shown in UI, to be used on custom builds to ease deployment``
* ``#define CUSTOM_DEPLOYMENT_KEY  ""  // Enter key not shown in UI, to be used on custom builds to ease deployment``
* ``#define CUSTOM_SUPPORT_SSID    ""  // Enter SSID not shown in UI, to be used on custom builds to ease support``
* ``#define CUSTOM_SUPPORT_KEY     ""  // Enter key not shown in UI, to be used on custom builds to ease support``



Emergency fallback SSID will only be attempted in the first 10 minutes after reboot.
When found, the unit will connect to it and depending on the built in flag, it will either just connect to it, or clear set credentials.

Use case: User connects to a public AP which does need to agree on an agreement page for the rules of conduct (e.g. open APs)
This is seen as a valid connection, so the unit will not reconnect to another node and thus becomes inaccessible.
The AP configured with these fallback credentials will then act as a master key to regain access to a node.

These will never be set in nightly builds and only allowed for custom builds for obvious reasons.

* ``#define CUSTOM_EMERGENCY_FALLBACK_SSID ""  // Enter SSID not shown in UI, to be used to regain access to the node``
* ``#define CUSTOM_EMERGENCY_FALLBACK_KEY  ""  // Enter key not shown in UI, to be used to regain access to the node``
* ``#define CUSTOM_EMERGENCY_FALLBACK_RESET_CREDENTIALS  false``
* ``#define CUSTOM_EMERGENCY_FALLBACK_START_AP           false``
* ``#define CUSTOM_EMERGENCY_FALLBACK_ALLOW_MINUTES_UPTIME 10``


Client IP filtering
===================

Sometimes it can be useful to only allow access to the web interface of a node from a specific range of IP-addresses.

For example if a node should only be configured from the local subnet.



Client IP block level
---------------------

* **Allow All** - No filtering applied, the web interface can be accessed from any IP able to reach the node.  (default)
* **Allow Local Subnet** - Only allow access to the web interface from the local subnet.
* **Allow IP range** - Only allow access to the web interface from a specific IP range.

Access IP lower range
---------------------

When *Allow IP range* is set, this field defines the lower bound of the range.


Access IP upper range
---------------------

When *Allow IP range* is set, this field defines the upper bound of the range.



WiFi/Ethernet IP Settings
=========================

For both WiFi and Ethernet a node can be configured to receive an IP automatically via DHCP, or use a static IP.

If all fields for IP, GW, subnet mask and DNS are left empty, the configuration offered via DHCP will be used.

* **IP** The static IPv4 address to use for this node
* **Gateway** IPv4 address of the gateway to reach hosts outside this subnet. (typically the IP of the router)
* **Subnetmask** Mask applied to define the local subnet. (typically: ``255.255.255.0`` )
* **DNS** IPv4 address of a DNS server to use. (typically the IP of the router or some assigned by the ISP)

Sleep Mode
==========

Description
-----------

The ESP can be put into deep sleep for a set amount of time.

On an ESP8266, GPIO-16 must be connected to the RST pin to be able to wake up again.

As long as the node is in deep sleep mode, you can't connect to the web interface. 
In fact, the ESP is turned off and only a dedicated timer circuit is still powered to wake up the main ESP core.

There are three ways to get out of sleep mode:

Cold boot
^^^^^^^^^

Power off the ESP and reconnect power. 
You will have 30 seconds to connect to the Web interface and disable the Sleep Mode function.
(You will get feedback in the serial interface)


Temporary disable Deep Sleep via jumper setting
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If 30 seconds is too short for you, you can use this method.

* Disconnect GPIO-16 from RST and connect it to GND. Restart your ESP if neccesary.
* Now deep sleep will be disabled and your ESP will function normally, until you connect GPIO-16 to RST again.

This requires ESPEasy v2.0.0-dev6 or newer.

Factory reset
^^^^^^^^^^^^^

If all else fails, just do a factory reset. **You will lose all your settings!**

* Connect the RX and TX pin together while you restart your ESP. (will clear all stored settings)
* Power off the device. 
* Remove the connector across the RX and TX pins. 
* Restart and then configure the ESPEasy firmware again.  (at restart factory default settings will be loaded)

If this doesn't work:

* Try loading the blank image to match the size of the memory installed on the device.
* Reboot and wait 5 minutes. 
* Then load on the firmware you are wanting to use.
* Reboot and wait 5 minutes.


Sleep awake time
----------------

This setting defines the minimum time in seconds a node should be awake from deep sleep.

If set to 0, the node will not go into deep sleep automatically.

At boot from deep sleep, a timer is started based on this setting.
If this timer expires, the node will enter deep sleep again.

As soon as the node has a successful WiFi connection, the timer will be restarted to allow to send out sensor data.


N.B. It is possible to put a node into deepsleep via the command ``deepsleep``, regardless this setting.



Sleep time
----------

The duration in seconds for a node to go into deep sleep.

Typical max. duration is 4294 seconds (Roughly 71 minutes)

N.B. the maximum possible duration depends on the used core library version and is mentioned at the configuration page.


Sleep on connection failure
---------------------------

