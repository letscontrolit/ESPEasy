.. include:: _controller_substitutions.repl

Controller
**********

A controller is a component to enable a plugin to send data elsewhere.

- Up-to 3 controllers can be active in ESPEasy.
- Per plugin up-to 3 active controllers can be selected.
- For some controllers an additional parameter can be given.

For example, Domoticz needs an 'IDX' value to identify the configured entry in
Domoticz for which new data is sent.

.. _Controller Plugins:

Controller Plugins
==================

.. csv-table::
   :header: "Plugin name", "Plugin status", "Plugin number"
   :widths: 10, 8, 5

   ":ref:`C001_page`","|C001_status|","C001"
   ":ref:`C002_page`","|C002_status|","C002"
   ":ref:`C003_page`","|C003_status|","C003"
   ":ref:`C004_page`","|C004_status|","C004"
   ":ref:`C005_page`","|C005_status|","C005"
   ":ref:`C006_page`","|C006_status|","C006"
   ":ref:`C007_page`","|C007_status|","C007"
   ":ref:`C008_page`","|C008_status|","C008"
   ":ref:`C009_page`","|C009_status|","C009"
   ":ref:`C010_page`","|C010_status|","C010"
   ":ref:`C011_page`","|C011_status|","C011"
   ":ref:`C012_page`","|C012_status|","C012"
   ":ref:`C013_page`","|C013_status|","C013"
   ":ref:`C014_page`","|C014_status|","C014"
   ":ref:`C016_page`","|C016_status|","C016"
   ":ref:`C017_page`","|C017_status|","C017"
   ":ref:`C018_page`","|C018_status|","C018"


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
- **Allow Expire** - Remove a queued message from the queue after <timeout> x <queue depth> x <retries>.
- **De-duplicate** - Do not add a message to the queue if the same message from the same task is already present.
- **Check Reply** - When set to false, a sent message is considered always successful.
- **Client Timeout** - Timeout in msec for an network connection used by the controller.
- **Sample Set Initiator** - Some controllers (e.g. C018 LoRa/TTN) can mark samples to belong to a set of samples. A new sample from set task index will increment this counter.
  Especially useful for controllers which cannot send samples in a burst. This makes the receiving time stamp useless to detect what samples were taken around the same time.
  The sample set counter value can help matching received samples to a single set.

.. note::
  Be careful when setting the timeout too high.
  For almost all controllers, sending data is a blocking call, so it may halt execution of other code on the node.
  With timouts longer than 2 seconds, the ESP may reboot as the software watchdog may step in.

TLS configuration
-----------------

Added: 2024-10-02

Some protocols like MQTT may use TLS to provide a secure connection to the host.
Where the default port for not encrypted connections to a MQTT broker is port 1883, its TLS counterpart is by default using port 8883.

.. note:: The current (2024-10-02) implementation does only allow to set to use TLS for MQTT controllers. There is not yet a proper validation of the used certificate.

Future implementations will add various ways to validate the used certificates using:

- Root CA, allowing to validate whether a certificate was signed by a known certificate authority (CA).
- Fingerprint, check whether a certificate is still the same as before.
- Check whether a certificate has expired.

To summarize, the current implementation does allow to encrypt the connection to the MQTT broker.
However a man-in-the-middle attack is still perfectly possible as the used certificates are not validated.

This does make using it extremely simple as even self-signed certificates can be used.
However do not consider this to be a 'secure' method since some attacker can redirect to another host and serve some false certificate.



Sample ThingSpeak configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Some controllers, like ThingSpeak, need a specific configuration.
ThingSpeak only allows a message every 15 seconds for the free accounts.

- **Minimum Send Interval** - 15000 msec
- **Max Queue Depth** - 1 (only report the last value)
- **Max Retries** - 2
- **Full Queue Action** - Delete Oldest
- **Check Reply** - Check Acknowledgment (VERY IMPORTANT)
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


