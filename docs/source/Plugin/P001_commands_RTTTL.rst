.. These P001 commands should be moved to core (P000)...

:ref:`P000_page` |P000_status| |P000_usedby_RTTTL| (Ringtones etc.)

.. csv-table::
  :header: "Command", "GPIO", "Value", "Extra information"
  :widths: 15, 10, 30, 45

  "
  ``tone,<gpio>,<value>``
  ","
  12...16
  ","
  <tone>,<duration>
  ","
  You should try to use GPIO 12...16 since these generally aren't used. The recommended tone range is 20...13000 but you could try tones outside this range. Duration is set in ms.
  "
  "
  ``rtttl,<gpio>,<value>``
  ","
  12...16
  ","
  d=<duration>,o=<octave>,b=<tempo>,<notes...>
  ","
  You should try to use GPIO 12...16 since these generally aren't used by ESP internal functions.
  "
