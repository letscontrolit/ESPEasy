

There is a number of ESP32 boards available with Ethernet connected via RMII.

However it is not always clear which configuration should be used.


.. warning:: 
    **Important notice**: It is possible to actually damage the Ethernet chip when a wrong configuration is used.

.. list-table:: RMII PHY Boards
   :widths: 30 15 10 10 10 20 30
   :header-rows: 1

   * - Board
     - Ethernet Chip
     - Addr
     - MDC
     - MDIO
     - Power/RST
     - Clock
   * - Olimex ESP32 PoE
     - LAN8720
     - 0
     - 23
     - 18
     - 12
     - 50MHz Inv Output GPIO17
   * - Olimex ESP32 EVB
     - LAN8720
     - 0
     - 23
     - 18
     - (no power pin)
     - External clock
   * - Olimex ESP32 Gateway
     - LAN8720
     - 0
     - 23
     - 18
     - 5
     - 50MHz Inv Output GPIO17
   * - wESP32
     - LAN8720
     - 0
     - 16
     - 17
     - (no power pin)
     - External clock
   * - WT32 ETH01
     - LAN8720
     - 1
     - 23
     - 18
     - ?
     - External clock
   * - TTGO T-Internet-POE ESP32
     - LAN8720
     - 0
     - 23
     - 18
     - 5 (RST)
     - 50MHz Inv Output GPIO17
   * - TTGO T-ETH-POE-PRO
     - LAN8720
     - 0
     - 23
     - 18
     - 5 (RST)
     - 50MHz Output GPIO0
   * - TTGO T-INTER_COM
     - LAN8720
     - 0
     - 23
     - 18
     - 4 (RST)
     - 50MHz Output GPIO0
   * - TTGO T-EH-Lite-ESP32
     - RTL8201
     - 0
     - 23
     - 18
     - 12
     - External clock

See:

* `ESP32 datasheet <https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf>`_
* `TTGO/LilyGO Ethernet boards <https://github.com/Xinyuan-LilyGO/LilyGO-T-ETH-Series/blob/dda7a2ad4ab33d550c8dbaff5db1e61a0eda5aad/examples/ETHOTA/utilities.h#L12>`_
* `Olimex ESP32-PoE <https://www.olimex.com/Products/IoT/ESP32/ESP32-POE/open-source-hardware>`_
* `Olimex ESP32-POE-ISO <https://www.olimex.com/Products/IoT/ESP32/ESP32-POE-ISO/open-source-hardware>`_
* `Olimex ESP32-EVB <https://www.olimex.com/Products/IoT/ESP32/ESP32-EVB/open-source-hardware>`_
* `Olimex ESP32-GATEWAY <https://www.olimex.com/Products/IoT/ESP32/ESP32-GATEWAY/open-source-hardware>`_
