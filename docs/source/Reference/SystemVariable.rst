System Variables
****************

To use other values than those created by sensors you need to go to 
system variables enclosed with "%". 
Not all of these can be used in rules tests (i.e. "=, >, <" etc.) 
since some of them output a string value and not a float. 
Which ones that are float are marked below.

Available System Variables
^^^^^^^^^^^^^^^^^^^^^^^^^^

These can be used in templates for HTTP, MQTT, OLED and LCD displays and within rules. 
More uses of these system variables can be seen in the rules section and formula section.

.. note:: Refer to the system variables page in the ESPEasy web interface to get a list of all included variables and their actual values.


.. list-table:: System Variables
   :widths: 25 25 50 10
   :header-rows: 1

   * - Syntax
     - Example output
     - Extra information
     - Float output
   * - ``%sysname%``
     - ESP_Easy
     - Name as configured through the webgui.
     - 
   * - ``%bootcause%``
     - 0
     - (re)boot cause as integer value, to be used in rules. 
       
       * ``0`` = manual reboot (reset btn)
       * ``1`` = cold boot
       * ``2`` = deep sleep
       * ``3`` = soft restart
       * ``10`` = ext Watchdog
       * ``11`` = SW Watchdog
       * ``12`` = Exception
       * ``20`` = Power unstable
     - Yes
   * - ``%systime%``
     - 01:23:54
     - Current time if NTP is enabled (hh:mm:ss, hh:mm prior to v2.0).
     - 
   * - ``%systm_hm%``
     - 1:23
     - Current time if NTP is enabled (hh:mm "old behavior").
     - 
   * - ``%systm_hm_0%``
     - 01:23
     - Current time if NTP is enabled (0-prefixed if hour < 10).
     - 
   * - ``%systm_hm_sp%``
     - ` 1:23`
     - Current time if NTP is enabled (space-prefixed if hour < 10).
     - 
   * - ``%systime_am%``
     - 1:23:54 AM
     - Current AM/PM time if NTP is enabled (hh:mm:ss xM).
     - 
   * - ``%systime_am_0%``
     - 01:23:54 AM
     - Current AM/PM time if NTP is enabled (0-prefixed if hour < 10).
     - 
   * - ``%systime_am_sp%``
     - ` 1:23:54 AM`
     - Current AM/PM time if NTP is enabled (space-prefixed if hour < 10).
     - 
   * - ``%systm_hm_am%``
     - 1:23 AM
     - Current AM/PM time if NTP is enabled (hh:mm:ss xM).
     - 
   * - ``%systm_hm_am_0%``
     - 01:23 AM
     - Current AM/PM time if NTP is enabled (0-prefixed if hour < 10).
     - 
   * - ``%systm_hm_am_sp%``
     - ` 1:23 AM`
     - Current AM/PM time if NTP is enabled (space-prefixed if hour < 10).
     - 
   * - ``%lcltime%``
     - 2020-03-16 01:23:54
     - Current date/time if NTP is enabled (YYYY-MM-DD hh:mm:ss).
     - 
   * - ``%sunrise%``
     - 5:04
     - Time of sunrise on current day, when NTP is active and coordinates set. 
       If you want to postpone or trigger something earlier but still using the sunset/sunrise time as reference you can use this syntax: 
       
       * ``%sunrise+10m%``
       * ``%sunset-1h%``
       
       Where the offset must be a integer with the postfix "m" for minutes or "h" for hours. Minus or plus is used to tell if the offset is prior or later than the sunset/sunrise. Any other letter positioned between the number and '%' is regarded as "seconds" notation.
     - 
   * - ``%s_sunrise%``
     - 19296
     - Seconds since midnight of sunrise on current day, when NTP is active and coordinates set. 

       Does not have the ``+xm`` and ``-xh`` calculations that ``%sunrise%`` and ``%sunset%`` support.
     -
   * - ``%m_sunrise%``
     - 321
     - Minutes since midnight of sunrise on current day, when NTP is active and coordinates set. 

       Does not have the ``+xm`` and ``-xh`` calculations that ``%sunrise%`` and ``%sunset%`` support.
     - 
   * - ``%sunset%``
     - 22:03
     - Time of sunset on current day, when NTP is active and coordinates set. For example on how to offset this time see the information for ``%sunrise%``.
     - 
   * - ``%s_sunset%``
     - 78216
     - Seconds since midnight of sunset on current day, when NTP is active and coordinates set. 

       Does not have the ``+xm`` and ``-xh`` calculations that ``%sunrise%`` and ``%sunset%`` support.
     - 
   * - ``%m_sunset%``
     - 1303
     - Minutes since midnight of sunset on current day, when NTP is active and coordinates set. 

       Does not have the ``+xm`` and ``-xh`` calculations that ``%sunrise%`` and ``%sunset%`` support.
     - 
   * - ``%lcltime_am%``
     - 2020-03-16 1:23:54 AM
     - Current date/time (AM/PM) if NTP is enabled (YYYY-MM-DD hh:mm:ss xM).
     - 
   * - ``%syshour%`` (``%syshour_0%``)
     - 6 (06)
     - Current hour (hh). ``%syshour%`` omits leading zeros.
     - Yes
   * - ``%sysmin%`` (``%sysmin_0%``)
     - 9 (09)
     - Current minute (mm). ``%sysmin%`` omits leading zeros.
     - Yes
   * - ``%syssec%`` (``%syssec_0%``)
     - 5 (05)
     - Current second (ss). ``%syssec%`` omits leading zeros.
     - Yes
   * - ``%sysday%`` (``%sysday_0%``)
     - 7 (07)
     - Current day of month (DD). ``%sysday%`` omits leading zeros.
     - Yes
   * - ``%sysmonth%`` (``%sysmonth_0%``)
     - 3 (03)
     - Current month (MM).
     - Yes
   * - ``%sysmonth_s%``
     - Mar
     - Current month as 3 characters, abbreviated, US-English notation. (MMM)
     -
   * - ``%sysyear%`` (``%sysyear_0%``)
     - 2020 (2020)
     - 4 digits (YYYY).
     - Yes
   * - ``%sysyears%``
     - 18
     - 2 digits (YY).
     - Yes
   * - ``%sysweekday%``
     - 5
     - Weekday (integer) - 1, 2, 3... (1=Sunday, 2=Monday etc.).
     - Yes
   * - ``%sysweekday_s%``
     - Fri
     - Weekday (verbose) - Sun, Mon, Tue...
     - 
   * - ``%systzoffset%``
     - +0100
     - System time-zone offset from UTC, using ``[+|-]HHMM`` format, + or -, hours and minutes both in 2 digits, zero-prefixed. Does take DST into account.
     - 
   * - ``%unixtime%``
     - 1521731277
     - Unix time (seconds since epoch, 1970-01-01 00:00:00)
       
       Example: 1521731277 = 2018-03-22 15:07:57
     - Yes
   * - ``%uptime%``
     - 3244
     - Uptime in minutes.
     - Yes
   * - ``%uptime_ms%``
     - 2095803
     - Uptime in milliseconds.
     -
   * - ``%rssi%``
     - -45
     - WiFi signal strength (dBm).
     - Yes
   * - ``%ip%``
     - 192.168.0.123
     - Current IP address.
     - 
   * - ``%unit%``
     - 32
     - Unit number.
     - Yes
   * - ``%unit_0%``
     - 001, 032, 110
     - Unit number, prefixed with zeros to a total size of 3 digits.
     - Yes
   * - ``%ssid%``
     - H4XX0R njietwork!
     - Name of current WiFi.
     - 
   * - ``%bssid%``
     - 00:15:E9:2B:99:3C
     - MAC of current AP.
     - 
   * - ``%wi_ch%``
     - 11
     - WiFi channel of current AP.
     - Yes
   * - ``%iswifi%``
     - 7
     - Bitset of WiFi connection state

       * ``0`` = disconnected
       * ``1`` = Connected
       * ``3`` = Got IP && Connected
       * ``7`` = Got IP && Connected && Completed to set all flags WiFi is initialized
     - Yes
   * - ``%vcc%``
     - 5.2
     - VCC value, this is only available in the VCC builds of FW (with "VCC" in the file name).
       
       If the variable output is "-1.0" it means that the VCC is not activated or that a reading has not been completed (could be due to incorrect cabling, interval set to "0", etc. etc.).
     - Yes
   * - ``%mac%``
     - 00:14:22:01:23:45
     - MAC address.
     - 
   * - ``%mac_int%``
     - 2212667
     - MAC address in integer to be used in rules (only the last 24 bit).
     - Yes
   * - ``%isntp%``
     - 1
     - Indicates whether time was set
     - Yes
   * - ``%ismqtt%``
     - 1
     - Indicates whether a configured MQTT broker is active
     - Yes
   * - ``%dns%``
     - 10.0.0.1 / (IP unset)
     - The configured Domain Name Server IP-addresses
     -
   * - ``%dns1%``
     - 10.0.0.1
     - The configured primary Domain Name Server IP-address
     -
   * - ``%dns2%``
     - (IP unset)
     - The configured secondary Domain Name Server IP-address
     -
   * - ``%flash_freq%``
     - 40
     - Actual frequency in MHz the flash is running at.
     -
   * - ``%flash_size%``
     - 4194304
     - Detected size of the flash chip in Bytes.
     -
   * - ``%flash_chip_vendor%``
     - 0x20
     - Vendor ID of the flash chip in Hex notation.
     -
   * - ``%flash_chip_model%``
     - 0x4016
     - Model nr of the flash chip, in Hex notation.
     -
   * - ``%fs_free%``
     - 135722
     - Free space of the file system in bytes.
     -
   * - ``%fs_size%``
     - 290156
     - Total size of the file system in bytes.
     -
   * - ``%cpu_id%``
     - 0x45B368
     - CPU ID of the processor in Hex notation. Often the last 3 bytes of the MAC address.
     -
   * - ``%cpu_freq%``
     - 240
     - Actual CPU frequency in MHz.
     -
   * - ``%cpu_model%``
     - ESP32-D0WDQ5	
     - Model nr of the CPU chip.
     -
   * - ``%cpu_rev%``
     - 1
     - Chip revision of the CPU chip (only on ESP32 variants)
     -
   * - ``%cpu_cores%``
     - 2
     - Number of CPU cores present.
     -
   * - ``%board_name%``
     - Espressif Generic ESP32 4M Flash, ESPEasy 1810k Code/OTA, 316k FS
     - Description of the used board definition to build the ESPEasy binary.
     -

Standard Conversions
^^^^^^^^^^^^^^^^^^^^

ESPEasy also supports a number of standard conversions.
The conversion always outputs a string, but not all of these can be converted back to a numerical (int or float).


.. list-table:: Standard Conversions
   :widths: 25 25 50
   :header-rows: 1

   * - Input string
     - Output string
     - Description
   * - Wind Dir.:    ``%c_w_dir%(123.4)``
     - Wind Dir.: ``ESE``
     - Degree to wind direction
   * - {D}C to {D}F: ``%c_c2f%(20.4)``
     - °C to °F: ``68.72``
     - Degree Celsius to Fahrenheit
   * - m/s to Bft:   ``%c_ms2Bft%(5.1)``
     - m/s to Bft: ``3``
     - Meter/sec to Beaufort
   * - Dew point(T,H): ``%c_dew_th%(18.6,67)``
     - Dew point(T,H): ``12.31``
     - Compute dew point given 2 values, temperature and relative humidity
   * - Altitude(air,sea): ``%c_alt_pres_sea%(850,1000)``
     - Altitude(air,sea): ``1350.03``
     - Compute Altitude (m) given 2 values, atmospheric pressure and pressure at sea level (hPa). (Added: 2021/04/27)
   * - PressureElevation(air,alt): ``%c_sea_pres_alt%(850,1350.03)``
     - PressureElevation(air,alt): ``1000.00``
     - Compensate air pressure for measured atmospheric pressure (hPa) and given altitude (m). (Added: 2021/04/27)
   * - cm to imperial: ``%c_cm2imp%(190)``
     - cm to imperial: ``6'2.8"``
     - Centimeter to imperial units
   * - mm to imperial: ``%c_mm2imp%(1900)``
     - mm to imperial: ``6'2.8"``
     - Millimeter to imperial units
   * - Mins to days: ``%c_m2day%(1900)``
     - Mins to days: ``1.32``
     - Minutes expressed in days
   * - Mins to dh:   ``%c_m2dh%(1900)``
     - Mins to dh: ``1d07h``
     - Minutes to days/hours notation
   * - Mins to dhm:  ``%c_m2dhm%(1900)``
     - Mins to dhm: ``1d07h40m``
     - Minutes to days/hours/minutes notation
   * - Mins to hcm:  ``%c_m2hcm%(482)``
     - Mins to hcm: ``08:02``
     - Minutes to hours/colon/minutes (hh:mm) notation (days are ignored when value > 1440 minutes!)
   * - Secs to dhms: ``%c_s2dhms%(100000)``
     - Secs to dhms: ``1d03:46:40``
     - Seconds to days/hours/minutes/seconds notation
   * - To HEX: ``%c_2hex%(100000)``
     - To HEX: ``186A0``
     - Convert integer value to HEX notation.  (Added: 2020/10/07)
   * - Unit to IP: ``%c_u2ip%(%unit%,0)``
     - Unit to IP: ``192.168.1.67``
     - Convert a (known) unit number to its IP Address. (Added: 2020/11/08)

       f_opt: for invalid IP: 0 = ``(IP unset)`` 1 = (empty string)  2 = ``0``


Task Formulas
^^^^^^^^^^^^^

Most tasks support using formulas.
These will be called when a task's ``PLUGIN_READ`` is called.

The formula can perform basic calculations.
In these calculations the new read value can be referred to via ``%value%``.
It is also possible to refer to the previous value, from before ``PLUGIN_READ`` is called.
This previous value can be referred to via ``%pvalue%``


Examples
--------

.. note::
 Use of "Standard Conversions" and referring other task values in formula was added on 2021-08-06


Convert from Celsius to Fahrenheit
""""""""""""""""""""""""""""""""""

* Using a formula: ``(%value%*9/5)+32``
* Using above mentioned "Standard Conversions": ``%c_c2f%(%value%)``


Compute dew point
"""""""""""""""""

In formulas one may also refer to other task values.
For example when using a BME280, which can measure temperature and humidity, it could be useful to output the dew point temperature instead of the actual temperature.

For this conversion, ``%c_dew_th%`` can be used, but it does need 2 input values:

* Temperature
* Humidity

Let's assume we have a task called "bme" which has a task value named "H" (humidity).
To replace the measured temperature with the dew point, one may want to use the following conversion:

.. code-block:: none

   %c_dew_th%(%value%,[bme#H])

Compute altitude based on air pressure
""""""""""""""""""""""""""""""""""""""

An ESPEasy node may receive sensor data from another remote node.
For example a node may have 2 tasks:

* "local" receiving the air pressure from a sensor
* "remote" which has a task value  "P" which contains the remote air pressure.

.. code-block:: none

   %c_alt_pres_sea%(%value%,[remote#P])

With this formula set at the "local" task which measures the air pressure, the unit of measure is converted from air pressure to altitude in meters, compared to the remote sensor.

This "remote" task may be received via ESPEasy p2p or can be set by the ``TaskValueSet`` command in rules to a dummy task.



Finite Impulse Response Filter
""""""""""""""""""""""""""""""

A Finate Impulse Response Filter (FIR) does only add a fraction of the change to the new value.
This does dampen the effect of a sudden spike in the readings and just follows the trend of the measured value.

It can also be used as a simple interpolate function for some values that may flip a number of times between 2 discrete values.
For example most A/D converters may flip between 2 discrete levels, where this flipping may be regarded as a duty cycle corresponding to where the actual value may be between both discrete levels of the ADC.

The factor used in an FIR is a trade-off between strength of filtering and adding a delay to the response time.

Since formulas only can refer to one previous value, we can only make a FIR filter with order N = 2.

An example with a weight of 0.25:

.. code-block:: none

   %pvalue% + (%value%-%pvalue%)/4

