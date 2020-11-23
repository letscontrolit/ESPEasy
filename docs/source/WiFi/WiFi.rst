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

N.B. the states are flags, meaning both "connected" and "got IP" must be set
to be considered ESPEASY_WIFI_SERVICES_INITIALIZED

The flag wifiConnectAttemptNeeded indicates whether a new connect attempt is needed.
This is set to true when:

- Security settings have been saved with AP mode enabled. FIXME TD-er, this may not be the best check.
- WiFi connect timeout reached  &  No client is connected to the AP mode of the node.
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

Quick reconnect (using BSSID/channel of last connection) when both apply:

- If wifi_connect_attempt < 3
- RTC.lastBSSID is known

Change of wifi settings when both apply:

- "other" settings valid
- (wifi_connect_attempt % 2) == 0

Reset of wifi_connect_attempt to 0 when both apply:

- connection successful
- Connection stable (connected for > 5 minutes)


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

- ECO mode is enabled (see below)
- too busy processing other blocking tasks (very likely)
- access point not sending a beacon due to high traffic demands of others (depending on brand/model/settings)
- RF disturbances (not likely given how often this occurs)
- clock drift (not really likely)

So the "beacon timeout" is happening every now and then on the ESP nodes and ESPeasy will try to reconnect.

.. _cpu-eco-mode-explanation:

ECO mode
========

If the "ECO" mode is enabled, the ESP does turn off the radio for longer amounts of time.

Normally an access point does send out a beacon at every so called "beacon interval".
A typical beacon interval is ~100 msec (102.4 msec).
When you try to send data to a connected WiFi station, your access point first tries to send directly to the node and if it doesn't immediately reply, the access point includes a notification for this node in such a beacon package.
Meaning, if you send a ping (or just any package) to a WiFi connected node, the first reply typically takes half this interval. (later ping replies may receive a response more quickly as the radio may be on continuously)

If the ESP (or any other WiFi device in "power save mode") is not listening to each beacon interval, it may take longer to reach the node.
Such a "listen interval" is called a DTIM interval and is often set between 1 and 3 for almost all WiFi devices.
I'm not entirely sure what the ESP uses when consuming ~80 mA, but I think it has the radio on all the time, effectively receiving packets before the next beacon interval.

Here a quick test performed on 2 ESPEasy nodes.
One with "Eco mode" enabled and one disabled:

.. code-block:: none

   gijs@DESKTOP-QLB7N8I:~/GitHub/letscontrolit/ESPEasy$ ping 192.168.10.92
   PING 192.168.10.92 (192.168.10.92) 56(84) bytes of data.
   64 bytes from 192.168.10.92: icmp_seq=1 ttl=254 time=6.51 ms
   64 bytes from 192.168.10.92: icmp_seq=2 ttl=254 time=4.24 ms
   64 bytes from 192.168.10.92: icmp_seq=3 ttl=254 time=104 ms
   64 bytes from 192.168.10.92: icmp_seq=4 ttl=254 time=3.01 ms
   ^C
   --- 192.168.10.92 ping statistics ---
   4 packets transmitted, 4 received, 0% packet loss, time 3005ms
   rtt min/avg/max/mdev = 3.007/29.329/103.562/42.876 ms
   gijs@DESKTOP-QLB7N8I:~/GitHub/letscontrolit/ESPEasy$ ping 192.168.10.211
   PING 192.168.10.211 (192.168.10.211) 56(84) bytes of data.
   64 bytes from 192.168.10.211: icmp_seq=2 ttl=254 time=885 ms
   64 bytes from 192.168.10.211: icmp_seq=4 ttl=254 time=40.9 ms
   64 bytes from 192.168.10.211: icmp_seq=5 ttl=254 time=34.8 ms
   64 bytes from 192.168.10.211: icmp_seq=7 ttl=254 time=85.6 ms
   ^C
   --- 192.168.10.211 ping statistics ---
   13 packets transmitted, 4 received, 69.2308% packet loss, time 12323ms
   rtt min/avg/max/mdev = 34.788/261.588/885.111/360.524 ms


As you can see, one of them did not reply on all ping packets and the first ping took quite some time to get a reply.
N.B. both are connected to the same access point.

What happens if you try to reach any host on your network?

Case 1, your PC does not know the MAC address of the ESP:

* Your PC tries to find the MAC address belonging to an IP address using an ARP packet (question is like: "Who has 1.2.3.4?")
* ARP packets gets broadcasted to the entire network
* If the ESP misses such an ARP request, you cannot route the IP packet to your ESP. -> fail

Case 2, your PR does know the MAC address of the ESP, but a switch or access point does not:

* Your PC sends out a packet for MAC belonging to your ESP
* Switch does not know how to rout and discards packet
* timeout on your PC.

Work around: Send Gratuitous ARP packets (answer to a question nobody asked)

* ESP sends "AA:BB:CC:DD:EE:FF has IP 1.2.3.4"
* All switches, access points and hosts on your network receiving this packet update their ARP table
* Roundtrip time of routing packet depends only on DTIM interval of the ESP.

TL;DR
-----

If you only send data from your ESP to something like a broker, then using "ECO" mode does probably have little to no effect on WiFi performance.
If you need a swift response (e.g. turning on a light), then you should not use "ECO" mode.