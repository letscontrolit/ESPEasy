Controller
**********

A controller is a component to enable a plugin to send data elsewhere.

- Up-to 3 controllers can be active in ESPEasy.
- Per plugin up-to 3 active controllers can be selected.
- For some controllers an additional parameter can be given.

For example, Domoticz needs an 'IDX' value to identify the configured entry in
Domoticz for which new data is sent.

Controller Parameters
=====================

Generic fields
--------------

- **Protocol** - The type of controller (e.g. ThingSpeak/OpenHAB MQTT/etc.)
- **Locate Controller** - Selection between hostname/IP
- **Controller Hostname/IP**  - The address to reach the selected service
- **Controller Port** - TCP/UDP Port number (0...65536)
- **Enabled** - Whether or not the controller is active.

Send queue parameters
---------------------

Controllers have a queue to keep unsent messages.
This queue is used to handle message bursts and also store messages which are recorded
before WiFi connection is made or during lost connection.

- **Minimum Send Interval** - Minimum time between two messages in msec.
- **Max Queue Depth** - Maximum length of the buffer queue to keep unsent messages.
- **Max Retries** - Maximum number of retries to send a message.
- **Full Queue Action** - How to handle when queue is full, ignore new or delete oldest message.
- **Client Timeout** - Timeout in msec for an network connection used by the controller.
- **Check Reply** - When set to false, a sent message is considered always successful.


Sample ThingSpeak configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Some controllers, like ThingSpeak, need a specific configuration.
ThingSpeak only allows a message every 15 seconds for the free accounts.

- **Minimum Send Interval** - 15000 msec
- **Max Queue Depth** - 1 (only report the last value)
- **Max Retries** - 2
- **Full Queue Action** - Delete Oldest
- **Client Timeout** - 500 msec (server is online, so timeout must be a bit longer)


Controller user credentials
---------------------------

- **Controller User** - User name (optional)
- **Controller Password** - Password (optional)

MQTT related settings
---------------------

- **Controller Subscribe** - Subscribe to the given topic.
- **Controller Publish** - Publish to the given topic.
- **Controller lwl topic** - Topic to which LWT (Last Will Testament) messages should be sent.
- **LWT Connect Message** - Connection established message.
- **LWT Disconnect Message** - Connection lost message (sent to broker during connect and published by broker when connection is lost)
