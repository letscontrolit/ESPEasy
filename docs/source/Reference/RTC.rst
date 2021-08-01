RTC Reference
*************

The ESP microprocessors we use, have some special memory which will retain its information as long as the node remains powered.
Espressif calls this _RTC_ RAM and thus we're using the same term for it.

The name may be a bit strange, as RTC usually refers to *Real Time Clock*.

Currently we only support RTC functions on the ESP8266 and ESP8285.
ESP32 does also have RTC memory, but that's organised a bit different.


RTC layout ESPEasy
------------------

ESP8266
^^^^^^^

On the ESP82xx the RTC memory is addressable per 32 bit.

In total, there is 768 bytes (192 addressable blocks).


* 0 .. 63  (256 bytes) Reserved for esp8266/Arduino library.
* 64 .. 73 (40 bytes) ``RTCStruct``.
* 74 .. 121 (200 bytes) UserVar - Task variables of 12 tasks
* 122 .. 123 UserVar checksum:  RTC_BASE_USERVAR + (sizeof(UserVar) / 4)
* 128 .. 131 (16 bytes) Cache Controller (C016) meta data
* 132 .. 191 (240 bytes) Cache Controller (C016) data 6 blocks per sample => max 10 samples


ESP32
^^^^^

On ESP32, the compiler determines where an object is stored in RTC.
Thus data stored in RTC may appear corrupt to a newly flashed build if the addresses where an object is stored may have changed.

Structures stored in RTC:

* RTC Struct
* UserVar (task values)


RTC Struct
----------

We store several variables in this struct that may be useful to survive a reboot.

WiFi related settings for a quick reconnect:

* Last connected WiFi channel
* Last WiFi Settings Index
* Last BSSID (48 bit)

Counters and states for diagnostics, recovering or protection:

* Deep Sleep State (To signal whether we just woke up from deep sleep)
* Boot Failed Count (to start disable plugins and/or controllers to find the source of crashes)
* Factory Reset Counter  (to protect the flash when format + clean install crashes)
* Flash Day Counter (to limit the number of writes per day)
* Flash Counter (32 bit)
* Boot Counter (32 bit)
* Last Scheduled job (32 bit)

System Time:
* Last Sys Time (32 bit) - Last known Unix time.

This last value, the last known system (Unix) time, is stored in RTC to have some idea of time after a warm reboot. (e.g. deliberate reboot, or crash)

Sometimes, when a node crashes, it may not be able to connect to WiFi right away or reach a NTP service.
If this node does need to act on (local) time based rules, it would be very practical if it has some concept of time.

There is an unknown period between storing the last known Unix time and restoring it after reboot.
This does lead to some (unknown) offset in local time, for each reboot.

Typical this offset is roughly between 900 and 1500 msec per (deliberate) reboot.
If the reboot was caused by a crash (e.g. Watchdog timer crash), the offset can be quite a bit longer.

The last known system time is stored on every scheduler action, so a few times a second at least.

