WiFi
****

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
