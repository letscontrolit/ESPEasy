WiFi
****

WiFi State Machine
==================

WiFi STA states
---------------

#. STA off                 -> ESPEASY_WIFI_DISCONNECTED
#. STA connecting          
#. STA connected           -> ESPEASY_WIFI_CONNECTED
#. STA got IP              -> ESPEASY_WIFI_GOT_IP
#. STA connected && got IP -> ESPEASY_WIFI_SERVICES_INITIALIZED

The flag wifiConnectAttemptNeeded indicates whether a new connect attempt is needed.
This is set to true when:

- Security settings have been saved with AP mode enabled. FIXME TD-er, this may not be the best check.
- WiFi connect timeout reached
- Wifi is reset
- WiFi setup page has been loaded with SSID/pass values.


WiFi AP mode states
-------------------

#. AP on                        -> reset AP disable timer
#. AP client connect/disconnect -> reset AP disable timer
#. AP off                       -> AP disable timer = 0;

AP mode will be disabled when both apply:

- AP disable timer (timerAPoff) expired
- No client is connected to the AP.

AP mode will be enabled when at least one applies:

- No valid WiFi settings
- Start AP timer (timerAPstart) expired

Start AP timer is set or cleared at:

- Set timerAPstart when "valid WiFi connection" state is observed.
- Disable timerAPstart when ESPEASY_WIFI_SERVICES_INITIALIZED wifi state is reached.



WiFi disconnect reasons
=======================

Beacon timeout (200)
--------------------

For more information on the Beacon frame, see `Wikipedia <https://en.wikipedia.org/wiki/Beacon_frame>`_.

In short, the access point sends periodically a packet containing information about the network.
This interval is typically 100 TU (102.4 msec).
The ESP module is trying to listen to this beacon every time, but for a number of reasons it may
miss a beacon frame.
The timeout is longer than 100 TU, so it must miss a number of these beacon frames
to report a beacon timeout.
Reasons to miss such a beacon frame are:

- too busy processing other blocking tasks (very likely)
- access point not sending a beacon due to high traffic demands of others (depending on brand/model/settings)
- RF disturbances (not likely given how often this occurs)
- clock drift (not really likely)

So the "beacon timeout" is happening every now and then on the ESP nodes and ESPeasy will try to reconnect.
