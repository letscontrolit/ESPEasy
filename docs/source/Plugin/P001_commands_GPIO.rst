.. include:: _plugin_substitutions_p00x.repl
.. These P001 commands should be moved to core (P000)...

:ref:`P000_page` |P000_status|

Supported hardware: |P000_usedby_GPIO|

.. csv-table::
  :header: "Command (GPIO/Value)", "Extra information"
  :widths: 20, 30

  "
  ``GPIO,<gpio>,<state>``

  GPIO: 0 ... 16

  State:

  **2** (HIGH-Z)

  **1** (HIGH)

  **0** (LOW)
  ","
  **Basic on/off.**.
  We can control a pin with simple http URL commands. To change the pin to high or low steady output. Setting GPIO to **2** means
  that it will be able to detect low level relays (with high impedance, Z).
  "
  "
  ``LongPulse,<GPIO>,<state>,<duration>``

  GPIO: 0 ... 16

  State: 1/0

  Duration: 1 ... 999 S
  ","
  **To send a *long* pulse to a certain pin.**.
  A long pulse is basically the same as the plain pulse. The only difference is the time base in seconds rather than in milliseconds.
  So it's more suitable for longer duration.
  "
  "
  ``LongPulse_mS,<GPIO>,<state>,<duration>``

  GPIO: 0 ... 16

  State: 1/0

  Duration: 1000 ... 15000 mS
  ","
  **To send a *long* pulse to a certain pin.**
  A long pulse (mS) is the same as the regular long pulse. The only difference is the time base in milliseconds rather than in seconds.
  "
  "
  ``Pulse,<GPIO>,<state>,<duration>``

  GPIO: 0 ... 16

  State: 1/0

  Duration: 0 ... 1000 mS
  ","
  **To send a *short* pulse to a certain pin.**
  Example to send an active high (1) pulse on GPIO 14 for 500 mSeconds. Pulse duration is in milliseconds. State is 1 or 0.
  "
  "
  ``PWM,<GPIO>,<state>``

  ``PWM,<GPIO>,<state>,<duration>``

  GPIO: 0 ... **15**

  State: 0 ... 1023

  Duration: 100 ... 15000 mS (optional)
  ","
  **To set a certain PWM level.**
  If you have set a certain GPIO to a PWM level and want to use it as a regular HIGH/LOW pin you need to reset by setting the PWM level to 0.
  You can use the duration (ms) parameter to create a fading.
  "
  "
  ``Servo,<servo>,<GPIO>,<position>``

  GPIO: 0 ... **15**

  Servo: 1/2

  Position: -180 ... 180 (see warning below)
  ","
  **To control a servo motor.**
  We currently support a maximum of two servo motors so you can build a pan & tilt device if you like.

  .. warning::
     Most servos are not able to turn full 360°! Normally the servos are able to go from **-90°** to **90°**, some rare servos do allow for -135° to 135°.

  "
