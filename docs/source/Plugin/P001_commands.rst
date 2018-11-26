.. These P001 commands should be moved to core (P000)...

|P000_typename| RTTTL and tone

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

|P000_typename| GPIO handling

.. csv-table::
  :header: "Command", "GPIO", "Value", "Extra information", "HTTP example", "MQTT example (topic: <MQTT subscribe template>/cmd)"
  :widths: 15, 8, 30, 45, 45, 45

  "
  ``GPIO,<gpio>,<value>``
  ","
  0...16
  ","
  **2** (HIGH-Z), **1** (HIGH), or **0** (LOW)
  ","
  **Basic on/off**. 
  We can control a pin with simple http URL commands. To change the pin to high or low steady output. Setting GPIO to "2" means
  that it will be able to detect low level relays (with high impedance, "Z").
  ","

  .. code-block:: html

    http://<espeasyip>/control?cmd=GPIO,12,1
    http://<espeasyip>/control?cmd=GPIO,12,0

  ","

  .. code-block:: html

    GPIO,12,1
    GPIO,12,0

  "
  "
  ``LongPulse,<GPIO>,<value>``
  ","
  0...16
  ","
  <state>,<duration>
  ","
  **To send a long pulse to a certain pin**.
  A long pulse is basically the same as the plain pulse. The only difference is the time base in seconds rather than in milliseconds.
  So it's more suitable for longer duration. Example to send an active high (1) pulse on GPIO 15 for 10 minutes.
  ","

  .. code-block:: html

    http://<espeasyip>/control?cmd=LongPulse,15,1,600

  ","

  .. code-block:: html

     LongPulse,15,1,600

  "
