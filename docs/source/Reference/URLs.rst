URLs
****


JSON
----

A lot of information can be fetched in JSON format via the ``http://<espeasyip>/json`` url.

At the root of the JSON output is a value names ``TTL`` which reflects the lowest task interval of all tasks included in the output.


.. csv-table::
  :header: "URL", "Description"
  :widths: 15, 30

  "
  ``http://<espeasyip>/json``
  ","
  Most elaborate dump of information including:

  * System - System information
  * WiFi - Network/WiFi related information
  * Ethernet - Network/Ethernet related information.  (only when ethernet support is included in the build)
  * nodes - List of known other nodes in the network (all with the same UDP port for ESPEasy p2p)
  * Sensors - List of all tasks with their configured controllers, task interval and task values with their names, settings, etc.

  "
  "
  ``http://<espeasyip>/json?view=sensorupdate``
  ","
  All task values of all tasks and needed information to format the values.
  "
  "
  ``http://<espeasyip>/json?view=sensorupdate&tasknr=2``
  ","
  All task values of a specific task nr and needed information to format the values.

  N.B. task nr starts at 1.
  "



CSV
---

Task values and their names can be fetched in simple CSV format via a GET url.

N.B. task number and variable number do count starting at 0.

.. csv-table::
  :header: "URL", "Description"
  :widths: 15, 30

  "
  ``http://<espeasyip>/csv?tasknr=1``
  ","
  All values of a task with header.

  .. code-block:: html

     T;H;P
     26.08;43.10;1012.50
  "
  "
  ``http://<espeasyip>/csv?tasknr=1&valnr=0``
  ","
  A single value of a task with header.

  .. code-block:: html

     T;
     26.08;
  "
  "
  ``http://<espeasyip>/csv?tasknr=1&valnr=0&header=0``
  ","
  A single value of a task without header.

  .. code-block:: html

     26.08;
  "




Control
-------
