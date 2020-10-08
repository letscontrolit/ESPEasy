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
   * - ``%systime%``
     - 01:23:54
     - Current time if NTP is enabled (hh:mm:ss, hh:mm prior to v2.0).
     - 
   * - ``%systm_hm%``
     - 01:23
     - Current time if NTP is enabled (hh:mm "old behavior").
     - 
   * - ``%systime_am%``
     - 1:23:54 AM
     - Current AM/PM time if NTP is enabled (hh:mm:ss xM).
     - 
   * - ``%systm_hm_am%``
     - 1:23 AM
     - Current AM/PM time if NTP is enabled (hh:mm:ss xM).
     - 
   * - ``%lcltime%``
     - 2020-03-16 01:23:54
     - Current date/time if NTP is enabled (YYYY-MM-DD hh:mm:ss).
     - 
   * - ``%sunrise%``
     - 5:04
     - Time of sunrise on current day, when NTP is active and coordinates set. If you want to postpone or trigger something earlier but still using the sunset/sunrise time as reference you can use this syntax: '''%sunrise+10m%''' or '''%sunset-1h%'''. Where the offset must be a integer with the prefix "m" for minutes or "h" for hours. Minus or plus is used to tell if the offset is prior or later than the sunset/sunrise. Any other letter positioned between the number and '%' is regarded as "seconds" notation.
     - 
   * - ``%sunset%``
     - 22:03
     - Time of sunset on current day, when NTP is active and coordinates set. For example on how to offset this time see the information for ``%sunrise%``.
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
   * - ``%unixtime%``
     - 1521731277
     - Unix time (seconds since epoch, 1970-01-01 00:00:00)<br>Example: 1521731277 = 2018-03-22 15:07:57
     - Yes
   * - ``%uptime%``
     - 3244
     - Uptime in minutes.
     - Yes
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
   * - ``%vcc%``
     - 5.2
     - VCC value, this is only available in the VCC versions of FW. If the variable output is "-1.0" it means that the VCC is not activated or that a reading has not been completed (could be due to incorrect cabling, interval set to "0", etc. etc.).
     - Yes
   * - ``%mac%``
     - 00:14:22:01:23:45
     - MAC address.
     - 
   * - ``%mac_int%``
     - 2212667
     - MAC address in integer to be used in rules (only the last 24 bit).
     - Yes


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
   * - Secs to dhms: ``%c_s2dhms%(100000)``
     - Secs to dhms: ``1d03:46:40``
     - Seconds to days/hours/minutes/seconds notation
   * - To HEX: ``%c_2hex%(100000)``
     - To HEX: ``186A0``
     - Convert integer value to HEX notation.  (Added: 2020/10/07)
