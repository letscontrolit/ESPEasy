
There is a number of ESP32 boards available with Ethernet connected via SPI

However it is not always clear which configuration should be used.

.. note:: SPI Ethernet is only supported on builds based on ESP-IDF 5.1 or newer. Thus only on ESP32-builds with LittleFS support made after 2024/02.


N.B. As there is not yet support in ESPEasy for multiple SPI busses, it cannot be configured.
  When later versions of ESPEasy will support multiple SPI busses, the SPI bus information may be needed for the configuration.
  Therefore it is included in the table below.

.. list-table:: SPI Ethernet Boards
   :widths: 30 15 15 10 10 10 10 10 10 10 15
   :header-rows: 1

   * - Board
     - ESP chip
     - Ethernet Chip
     - Addr
     - CS
     - IRQ/INT
     - RST
     - SPI Clock
     - SPI MISO
     - SPI MOSI
     - SPI bus
   * - ETH01-EVO
     - ESP32-C3
     - DM9051
     - 1
     - 9
     - 8
     - 6
     - 7
     - 3
     - 10
     - SPI2_HOST
   * - M5Stack PoECAM
     - ESP32-classic
     - W5500
     - 
     - 4
     - 
     - 
     - 23
     - 38
     - 13
     - SPI2_HOST
   * - M5Stack Atom PoE Kit (ATOM LITE)
     - ESP32-classic
     - W5500
     - 
     - 19
     - 
     - 
     - 22
     - 23
     - 33
     - SPI2_HOST
   * - M5Stack Atom PoE Kit (AtomS3)
     - ESP32-S3
     - W5500
     - 
     - 6
     - 
     - 
     - 5
     - 7
     - 8
     - SPI2_HOST
   * - M5Stack Base LAN (End-of-life)
     - M5Core
     - W5500
     - 
     - 26
     - 34
     - 13
     - 18
     - 19
     - 23
     - SPI2_HOST
   * - M5Stack LAN (PoE) BASE V12
     - M5Core
     - W5500
     - 
     - 26
     - 34
     - 13
     - 18
     - 19
     - 23
     - SPI2_HOST
   * - M5Stack LAN Module 13.2
     - M5Core
     - W5500
     - 
     - 5/15
     - 35/34
     - 0/13
     - 18
     - 19
     - 23
     - SPI2_HOST
   * - M5Stack LAN Module 13.2
     - M5Core2
     - W5500
     - 
     - 33/2
     - 35/34
     - 0/19
     - 18
     - 38
     - 23
     - SPI2_HOST
   * - M5Stack LAN Module 13.2
     - M5CoreS3
     - W5500
     - 
     - 1/13
     - 10/14
     - 0/7
     - 36
     - 35
     - 37
     - SPI2_HOST
   * - T-ETH-Lite-ESP32S3
     - ESP32-S3
     - W5500
     - 1
     - 9
     - 13
     - 14
     - 10
     - 11
     - 12
     - SPI2_HOST


See:

* `M5 Stack Base LAN <https://docs.m5stack.com/en/base/lan_base>`_
* `M5 Stack LAN Base V12 <https://docs.m5stack.com/en/base/lan_v12>`_
* `M5 Stack PoECAM <https://docs.m5stack.com/en/unit/poe_cam>`_
* `M5 Stack LAN PoE Base V12 <https://docs.m5stack.com/en/base/lan_poe_v12>`_
* `M5 Stack LAN Module V13.2 <https://docs.m5stack.com/en/module/LAN%20Module%2013.2>`_
* `M5 Stack ATOM PoE <https://docs.m5stack.com/en/atom/atom_poe>`_ `ATOM Lite <https://docs.m5stack.com/en/core/ATOM%20Lite>`_ `AtomS3 <https://docs.m5stack.com/en/core/AtomS3>`_
* `M5 Stack Base PoE <https://docs.m5stack.com/en/base/w5500PoE>`_
* `TTGO/LilyGO Ethernet boards <https://github.com/Xinyuan-LilyGO/LilyGO-T-ETH-Series/blob/dda7a2ad4ab33d550c8dbaff5db1e61a0eda5aad/examples/ETHOTA/utilities.h#L12>`_
