.. These P001 commands should be moved to core (P000)...

:ref:`P000_page` |P000_status| |P000_usedby_GPIO|

.. csv-table::
  :header: "Command", "GPIO", "Value", "Extra information"
  :widths: 15, 10, 30, 45

  "
  ``GPIO,<gpio>,<value>``
  ","
  0 ... 16
  ","
  **2** (HIGH-Z)

  **1** (HIGH)

  **0** (LOW)
  ","
  **Basic on/off.**.
  We can control a pin with simple http URL commands. To change the pin to high or low steady output. Setting GPIO to **2** means
  that it will be able to detect low level relays (with high impedance, Z).
  "
  "
  ``LongPulse,<GPIO>,<value>``
  ","
  0 ... 16
  ","
  <state>,<duration>
  ","
  **To send a *long* pulse to a certain pin.**.
  A long pulse is basically the same as the plain pulse. The only difference is the time base in seconds rather than in milliseconds.
  So it's more suitable for longer duration. Example to send an active high (1) pulse on GPIO 15 for 10 minutes.
  "
  "
  ``LongPulse_mS,<GPIO>,<value>``
  ","
  0 ... 16
  ","
  <state>,<duration>
  ","
  **To send a *long* pulse to a certain pin.**
  A long pulse (mS) is the same as the regular long pulse. The only difference is the time base in milliseconds rather than in seconds.
  Example to send an active high (1) pulse on GPIO 15 for 1 second.
  "
  "
  ``Pulse,<GPIO>,<value>``
  ","
  0 ... 16
  ","
  <state>,<duration>
  ","
  **To send a *short* pulse to a certain pin.**
  Example to send an active high (1) pulse on GPIO 14 for 500 mSeconds. Pulse duration is in milliseconds. State is 1 or 0.
  "
  "
  ``PWM,<GPIO>,<state>``

  ``PWM,<GPIO>,<state>,<duration>``
  ","
  0... **15**
  ","
  0 ... 1023
  ","
  **To set a certain PWM level.**
  If you have set a certain GPIO to a PWM level and want to use it as a regular HIGH/LOW pin you need to reset by setting the PWM level to 0.
  You can use the duration (ms) parameter to create a fading.
  "
  "
  ``Servo,<value1>,<GPIO>,<value2>``
  ","
  0 ... 16
  ","
  value1: servo number

  value2: position
  ","
  **To control a servo motor.**
  We currently support a maximum of two servo motors so you can build a pan & tilt device if you like. Example to set servo 1 on GPIO 2 to a 90 degree position and servo 2 on GPIO 16 to a 45 degree position.
  "
